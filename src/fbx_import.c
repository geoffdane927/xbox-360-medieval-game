#include "fbx_import.h"
#include "ufbx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* X360M v1 disk structs: exact packed field order from docs/X360M_FORMAT.md.
   This first importer intentionally bakes the evaluated bind pose as a rigid
   root bone. It imports every mesh node so the complete multi-part character
   is visible; skeletal animation baking is the next format/runtime step. */
typedef struct { char magic[8]; unsigned int version, mesh_count, bone_count, clip_count; } Hdr;
typedef struct { char name[32]; int parent; float inv_bind[16]; } Bone;
typedef struct { unsigned int vertex_count, index_count; } MeshHdr;
typedef struct { float p[3], n[3], uv[2]; unsigned short bone[4]; float weight[4]; } Vertex;
typedef struct { char name[32]; float duration, sample_rate; unsigned int frame_count; } Clip;

static void matrix16(float out[16], const ufbx_matrix *m)
{
    out[0]=(float)m->m00; out[1]=(float)m->m10; out[2]=(float)m->m20; out[3]=0;
    out[4]=(float)m->m01; out[5]=(float)m->m11; out[6]=(float)m->m21; out[7]=0;
    out[8]=(float)m->m02; out[9]=(float)m->m12; out[10]=(float)m->m22; out[11]=0;
    out[12]=(float)m->m03; out[13]=(float)m->m13; out[14]=(float)m->m23; out[15]=1;
}
static int write_all(FILE *f, const void *p, size_t n) { return fwrite(p,1,n,f)==n; }
static void write_name(char dst[32], ufbx_string s)
{
    size_t n=s.length<31?s.length:31; memset(dst,0,32); if(n) memcpy(dst,s.data,n);
}
static int valid_cache(const char *path)
{
    FILE *f=fopen(path,"rb"); Hdr h; int ok=0;
    if(!f) return 0;
    if(fread(&h,1,sizeof(h),f)==sizeof(h) && !memcmp(h.magic,"X360M01",7) &&
       h.version==1 && h.mesh_count==1 && h.bone_count==1 && h.clip_count==1) ok=1;
    fclose(f); return ok;
}
int fbx_convert_to_x360m(const char *fbx_path, const char *cache_path)
{
    ufbx_load_opts opts; ufbx_error error; ufbx_scene *scene=NULL;
    FILE *f=NULL; Hdr h; Bone bone; MeshHdr mh; Clip clip;
    Vertex *verts=NULL; unsigned short *idx=NULL;
    size_t i, total=0, cursor=0; int ok=0;
    memset(&opts,0,sizeof(opts));
    opts.generate_missing_normals=true;
    scene=ufbx_load_file(fbx_path,&opts,&error);
    if(!scene || scene->meshes.count==0) goto done;

    for(i=0;i<scene->meshes.count;i++) {
        ufbx_mesh *m=scene->meshes.data[i];
        total+=m->num_triangles*3;
    }
    /* X360M v1 indices are uint16. Refuse overflow rather than corrupting. */
    if(!total || total>65535 || total>200000) goto done;
    verts=(Vertex*)calloc(total,sizeof(Vertex));
    idx=(unsigned short*)malloc(total*sizeof(unsigned short));
    if(!verts || !idx) goto done;

    for(i=0;i<scene->nodes.count;i++) {
        ufbx_node *node=scene->nodes.data[i];
        ufbx_mesh *m=node->mesh; size_t fi;
        if(!m) continue;
        for(fi=0;fi<m->faces.count;fi++) {
            ufbx_face face=m->faces.data[fi]; uint32_t tri[256]; size_t nt,t;
            ufbx_matrix normal_m;
            if(face.num_indices<3) continue;
            if(face.num_indices>256) goto done;
            nt=ufbx_triangulate_face(tri,256,m,face);
            if(!nt || nt%3) goto done;
            normal_m=ufbx_matrix_for_normals(&node->geometry_to_world);
            for(t=0;t<nt;t++) {
                uint32_t ix=tri[t]; Vertex *v=&verts[cursor];
                ufbx_vec3 p=ufbx_get_vertex_vec3(&m->vertex_position,ix);
                ufbx_vec3 n=ufbx_get_vertex_vec3(&m->vertex_normal,ix);
                p=ufbx_transform_position(&node->geometry_to_world,p);
                n=ufbx_transform_direction(&normal_m,n);
                v->p[0]=(float)p.x; v->p[1]=(float)p.y; v->p[2]=(float)p.z;
                v->n[0]=(float)n.x; v->n[1]=(float)n.y; v->n[2]=(float)n.z;
                if(m->vertex_uv.exists) {
                    ufbx_vec2 uv=ufbx_get_vertex_vec2(&m->vertex_uv,ix);
                    v->uv[0]=(float)uv.x; v->uv[1]=1.0f-(float)uv.y;
                }
                v->bone[0]=0; v->weight[0]=1.0f;
                idx[cursor]=(unsigned short)cursor; cursor++;
            }
        }
    }
    if(cursor==0 || cursor>65535) goto done;
    f=fopen(cache_path,"wb"); if(!f) goto done;
    memset(&h,0,sizeof(h)); memcpy(h.magic,"X360M01",7);
    h.version=1; h.mesh_count=1; h.bone_count=1; h.clip_count=1;
    memset(&bone,0,sizeof(bone)); strcpy(bone.name,"RigidRoot"); bone.parent=-1;
    bone.inv_bind[0]=bone.inv_bind[5]=bone.inv_bind[10]=bone.inv_bind[15]=1.0f;
    mh.vertex_count=(unsigned int)cursor; mh.index_count=(unsigned int)cursor;
    memset(&clip,0,sizeof(clip)); strcpy(clip.name,"BindPose");
    clip.duration=1.0f; clip.sample_rate=1.0f; clip.frame_count=1;
    {
        float identity[16]; memset(identity,0,sizeof(identity));
        identity[0]=identity[5]=identity[10]=identity[15]=1.0f;
        if(!write_all(f,&h,sizeof(h)) || !write_all(f,&bone,sizeof(bone)) ||
           !write_all(f,&mh,sizeof(mh)) || !write_all(f,verts,sizeof(Vertex)*cursor) ||
           !write_all(f,idx,sizeof(unsigned short)*cursor) ||
           !write_all(f,&clip,sizeof(clip)) || !write_all(f,identity,sizeof(identity))) goto done;
    }
    if(fflush(f)!=0) goto done;
    ok=1;
done:
    if(f) fclose(f);
    if(!ok && cache_path) remove(cache_path);
    free(verts); free(idx); if(scene) ufbx_free_scene(scene);
    return ok;
}
int fbx_cache_ensure(const char *fbx_path, const char *cache_path)
{
    if(valid_cache(cache_path)) return 1;
    return fbx_convert_to_x360m(fbx_path,cache_path);
}
