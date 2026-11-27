#include "fbx_viewer_camera.h"
#include <xtl.h>
#include <math.h>

void fbx_viewer_camera_reset(FbxViewerCamera *c)
{
    if(!c) return;
    c->x=0.0f; c->y=1.0f; c->z=-4.0f;
    c->yaw=0.0f; c->pitch=0.0f;
}
void fbx_viewer_camera_update(FbxViewerCamera *c, float dt)
{
    XINPUT_STATE state;
    float lx,ly,rx,ry,forward,side;
    if(!c) return;
    if(dt<0.0f) dt=0.0f; if(dt>0.05f) dt=0.05f;
    ZeroMemory(&state,sizeof(state));
    if(XInputGetState(0,&state)!=ERROR_SUCCESS) return;
    lx=(float)state.Gamepad.sThumbLX/32767.0f;
    ly=(float)state.Gamepad.sThumbLY/32767.0f;
    rx=(float)state.Gamepad.sThumbRX/32767.0f;
    ry=(float)state.Gamepad.sThumbRY/32767.0f;
    if(fabsf(lx)<0.18f) lx=0.0f;
    if(fabsf(ly)<0.18f) ly=0.0f;
    if(fabsf(rx)<0.18f) rx=0.0f;
    if(fabsf(ry)<0.18f) ry=0.0f;
    c->yaw+=rx*2.2f*dt;
    c->pitch-=ry*1.8f*dt;
    if(c->pitch>1.45f) c->pitch=1.45f;
    if(c->pitch< -1.45f) c->pitch= -1.45f;
    forward=ly*4.0f*dt; side=lx*4.0f*dt;
    c->x+=sinf(c->yaw)*forward+cosf(c->yaw)*side;
    c->z+=cosf(c->yaw)*forward-sinf(c->yaw)*side;
    c->y+=((float)state.Gamepad.bRightTrigger-(float)state.Gamepad.bLeftTrigger)/255.0f*3.0f*dt;
    if(state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) fbx_viewer_camera_reset(c);
}
