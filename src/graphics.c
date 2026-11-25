#include "engine.h"

void graphics_set_camera(Graphics *g,Camera *c) {
    Vec3 forward={sinf(c->yaw)*cosf(c->pitch),sinf(c->pitch),cosf(c->yaw)*cosf(c->pitch)};
    Vec3 target=vec3_add(c->position,forward), up={0,1,0};
    Mat4 view=mat4_look_at(c->position,target,up);
    Mat4 projection=mat4_perspective(1.04719755f,(float)GAME_WIDTH/(float)GAME_HEIGHT,0.1f,500.0f);
    g->camera=*c;
    /* Column-vector convention: clip = Projection * View * Position. */
    g->view_projection=mat4_mul(projection,view);
}
