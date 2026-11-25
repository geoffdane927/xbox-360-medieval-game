#include "engine.h"

typedef struct {
    char magic[8];
    unsigned int version, mesh_count, bone_count, clip_count;
} X360MHeader;

static int read_exact(FILE *f,void *p,size_t n) { return fread(p,1,n,f)==n; }

void model_unload(AnimatedModel *m) {
    unsigned int i;
    if(!m) return;
    if(m->meshes) for(i=0;i<m->mesh_count;i++) {
        free(m->meshes[i].vertices); free(m->meshes[i].indices);
        if(m->deformed) free(m->deformed[i]);
    }
    if(m->clips) for(i=0;i<m->clip_count;i++) free(m->clips[i].bone_matrices);
    free(m->meshes); free(m->bones); free(m->clips); free(m->deformed);
    ZeroMemory(m,sizeof(*m));
}

int model_load_x360m(AnimatedModel *m,const char *path) {
    FILE *f; X360MHeader h; unsigned int i,j;
    if(!m || !path) return 0;
    ZeroMemory(m,sizeof(*m));
    f=fopen(path,"rb"); if(!f) return 0;
    if(!read_exact(f,&h,sizeof(h)) || memcmp(h.magic,MODEL_MAGIC,7)!=0 || h.version!=1 ||
       h.mesh_count==0 || h.mesh_count>64 || h.bone_count==0 ||
       h.bone_count>MODEL_MAX_BONES || h.clip_count==0 || h.clip_count>128) {
        fclose(f); return 0;
    }
    m->mesh_count=h.mesh_count; m->bone_count=h.bone_count; m->clip_count=h.clip_count;
    m->meshes=(ModelMesh*)calloc(h.mesh_count,sizeof(ModelMesh));
    m->bones=(ModelBone*)calloc(h.bone_count,sizeof(ModelBone));
    m->clips=(AnimationClip*)calloc(h.clip_count,sizeof(AnimationClip));
    m->deformed=(ModelVertex**)calloc(h.mesh_count,sizeof(ModelVertex*));
    if(!m->meshes || !m->bones || !m->clips || !m->deformed) { fclose(f); model_unload(m); return 0; }

    for(i=0;i<h.bone_count;i++) {
        if(!read_exact(f,&m->bones[i],sizeof(ModelBone))) { fclose(f); model_unload(m); return 0; }
    }
    for(i=0;i<h.mesh_count;i++) {
        ModelMesh *mesh=&m->meshes[i];
        if(!read_exact(f,&mesh->vertex_count,sizeof(unsigned int)) ||
           !read_exact(f,&mesh->index_count,sizeof(unsigned int)) ||
           mesh->vertex_count==0 || mesh->vertex_count>200000 ||
           mesh->index_count==0 || mesh->index_count>600000 || mesh->index_count%3) {
            fclose(f); model_unload(m); return 0;
        }
        mesh->vertices=(ModelVertex*)malloc(sizeof(ModelVertex)*mesh->vertex_count);
        mesh->indices=(unsigned short*)malloc(sizeof(unsigned short)*mesh->index_count);
        m->deformed[i]=(ModelVertex*)malloc(sizeof(ModelVertex)*mesh->vertex_count);
        if(!mesh->vertices || !mesh->indices || !m->deformed[i] ||
           !read_exact(f,mesh->vertices,sizeof(ModelVertex)*mesh->vertex_count) ||
           !read_exact(f,mesh->indices,sizeof(unsigned short)*mesh->index_count)) {
            fclose(f); model_unload(m); return 0;
        }
        for(j=0;j<mesh->vertex_count;j++) m->deformed[i][j]=mesh->vertices[j];
        for(j=0;j<mesh->index_count;j++) if(mesh->indices[j]>=mesh->vertex_count) {
            fclose(f); model_unload(m); return 0;
        }
    }
    for(i=0;i<h.clip_count;i++) {
        AnimationClip *c=&m->clips[i];
        unsigned int matrix_count;
        if(!read_exact(f,c->name,MODEL_NAME_BYTES) ||
           !read_exact(f,&c->duration,sizeof(float)) ||
           !read_exact(f,&c->sample_rate,sizeof(float)) ||
           !read_exact(f,&c->frame_count,sizeof(unsigned int)) ||
           c->frame_count==0 || c->frame_count>10000 || c->duration<=0.0f ||
           c->sample_rate<=0.0f) {
            fclose(f); model_unload(m); return 0;
        }
        matrix_count=c->frame_count*h.bone_count;
        c->bone_matrices=(Mat4*)malloc(sizeof(Mat4)*matrix_count);
        if(!c->bone_matrices || !read_exact(f,c->bone_matrices,sizeof(Mat4)*matrix_count)) {
            fclose(f); model_unload(m); return 0;
        }
    }
    fclose(f); m->active_clip=0; m->loaded=1; model_update_skin(m); return 1;
}

static Mat4 pose_matrix(AnimatedModel *m,unsigned int bone) {
    AnimationClip *c=&m->clips[m->active_clip];
    float frame=m->animation_time*c->sample_rate;
    unsigned int a=(unsigned int)frame, b;
    float t=frame-(float)a;
    Mat4 result;
    if(a>=c->frame_count) a=c->frame_count-1;
    b=(a+1<c->frame_count)?a+1:0;
    /* Element interpolation is a preview-quality approximation; use quaternion
       interpolation once the first target build is running. */
    {
        int k;
        Mat4 ma=c->bone_matrices[a*m->bone_count+bone];
        Mat4 mb=c->bone_matrices[b*m->bone_count+bone];
        for(k=0;k<16;k++) result.m[k]=ma.m[k]+(mb.m[k]-ma.m[k])*t;
    }
    return mat4_mul(result,m->bones[bone].inverse_bind);
}

void model_update_skin(AnimatedModel *m) {
    unsigned int mi,vi,k;
    if(!m || !m->loaded || !m->clip_count) return;
    for(mi=0;mi<m->mesh_count;mi++) {
        ModelMesh *mesh=&m->meshes[mi];
        for(vi=0;vi<mesh->vertex_count;vi++) {
            ModelVertex *src=&mesh->vertices[vi], *dst=&m->deformed[mi][vi];
            Vec3 p={0,0,0}, n={0,0,0};
            Vec3 original={src->position[0],src->position[1],src->position[2]};
            Vec3 normal={src->normal[0],src->normal[1],src->normal[2]};
            float total=0.0f;
            for(k=0;k<4;k++) if(src->weight[k]>0.0001f && src->bone[k]<m->bone_count) {
                Mat4 skin=pose_matrix(m,src->bone[k]);
                p=vec3_add(p,vec3_scale(mat4_transform_point(skin,original),src->weight[k]));
                n=vec3_add(n,vec3_scale(mat4_transform_vector(skin,normal),src->weight[k]));
                total+=src->weight[k];
            }
            if(total<0.0001f) { p=original; n=normal; }
            else { p=vec3_scale(p,1.0f/total); n=vec3_normalize(n); }
            dst->position[0]=p.x; dst->position[1]=p.y; dst->position[2]=p.z;
            dst->normal[0]=n.x; dst->normal[1]=n.y; dst->normal[2]=n.z;
        }
    }
}
