#include "engine.h"

Vec3 vec3_add(Vec3 a, Vec3 b) { Vec3 r={a.x+b.x,a.y+b.y,a.z+b.z}; return r; }
Vec3 vec3_sub(Vec3 a, Vec3 b) { Vec3 r={a.x-b.x,a.y-b.y,a.z-b.z}; return r; }
Vec3 vec3_scale(Vec3 v,float s) { Vec3 r={v.x*s,v.y*s,v.z*s}; return r; }
float vec3_dot(Vec3 a,Vec3 b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
Vec3 vec3_cross(Vec3 a,Vec3 b) { Vec3 r={a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; return r; }
Vec3 vec3_normalize(Vec3 v) {
    float n=sqrtf(vec3_dot(v,v));
    if(n<0.00001f) { Vec3 z={0,0,0}; return z; }
    return vec3_scale(v,1.0f/n);
}
Mat4 mat4_identity(void) {
    Mat4 r={{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1}}; return r;
}
/* Matrices are column-major throughout the runtime. */
Mat4 mat4_mul(Mat4 a,Mat4 b) {
    Mat4 r; int row,col,k;
    for(col=0;col<4;col++) for(row=0;row<4;row++) {
        r.m[col*4+row]=0;
        for(k=0;k<4;k++) r.m[col*4+row]+=a.m[k*4+row]*b.m[col*4+k];
    }
    return r;
}
Mat4 mat4_translation(Vec3 p) {
    Mat4 r=mat4_identity(); r.m[12]=p.x; r.m[13]=p.y; r.m[14]=p.z; return r;
}
Mat4 mat4_rotation_y(float a) {
    Mat4 r=mat4_identity(); float c=cosf(a),s=sinf(a);
    r.m[0]=c; r.m[2]=-s; r.m[8]=s; r.m[10]=c; return r;
}
Mat4 mat4_perspective(float f,float aspect,float n,float z) {
    Mat4 r={{0}}; float y=1.0f/tanf(f*0.5f);
    r.m[0]=y/aspect; r.m[5]=y; r.m[10]=z/(z-n); r.m[11]=1.0f;
    r.m[14]=(-n*z)/(z-n); return r;
}
Mat4 mat4_look_at(Vec3 eye,Vec3 target,Vec3 up) {
    Vec3 z=vec3_normalize(vec3_sub(target,eye));
    Vec3 x=vec3_normalize(vec3_cross(up,z));
    Vec3 y=vec3_cross(z,x);
    Mat4 r=mat4_identity();
    r.m[0]=x.x; r.m[1]=y.x; r.m[2]=z.x;
    r.m[4]=x.y; r.m[5]=y.y; r.m[6]=z.y;
    r.m[8]=x.z; r.m[9]=y.z; r.m[10]=z.z;
    r.m[12]=-vec3_dot(x,eye); r.m[13]=-vec3_dot(y,eye); r.m[14]=-vec3_dot(z,eye);
    return r;
}
Vec3 mat4_transform_point(Mat4 m,Vec3 p) {
    Vec3 r;
    r.x=p.x*m.m[0]+p.y*m.m[4]+p.z*m.m[8]+m.m[12];
    r.y=p.x*m.m[1]+p.y*m.m[5]+p.z*m.m[9]+m.m[13];
    r.z=p.x*m.m[2]+p.y*m.m[6]+p.z*m.m[10]+m.m[14];
    return r;
}
Vec3 mat4_transform_vector(Mat4 m,Vec3 p) {
    Vec3 r;
    r.x=p.x*m.m[0]+p.y*m.m[4]+p.z*m.m[8];
    r.y=p.x*m.m[1]+p.y*m.m[5]+p.z*m.m[9];
    r.z=p.x*m.m[2]+p.y*m.m[6]+p.z*m.m[10];
    return r;
}
