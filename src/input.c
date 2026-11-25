#include "engine.h"

void input_poll(GameInput *out) {
    static WORD previous_buttons = 0;
    XINPUT_STATE state;
    WORD buttons;
    ZeroMemory(out,sizeof(*out));
    ZeroMemory(&state,sizeof(state));
    if(XInputGetState(0,&state)!=ERROR_SUCCESS) {
        previous_buttons=0;
        return;
    }
    buttons=state.Gamepad.wButtons;
    {
        SHORT lx=state.Gamepad.sThumbLX, ly=state.Gamepad.sThumbLY;
        SHORT rx=state.Gamepad.sThumbRX, ry=state.Gamepad.sThumbRY;
        const float dead=7849.0f;
        if(fabsf((float)lx)>dead) out->move_x=(float)lx/32767.0f;
        if(fabsf((float)ly)>dead) out->move_y=(float)ly/32767.0f;
        if(fabsf((float)rx)>dead) out->look_x=(float)rx/32767.0f;
        if(fabsf((float)ry)>dead) out->look_y=(float)ry/32767.0f;
    }
    /* Rising edge only: holding A must not skip through every clip. */
    out->next_animation=((buttons & XINPUT_GAMEPAD_A)!=0) &&
                        ((previous_buttons & XINPUT_GAMEPAD_A)==0);
    out->reset_camera=(buttons & XINPUT_GAMEPAD_BACK)!=0;
    previous_buttons=buttons;
}
