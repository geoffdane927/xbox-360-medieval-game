# FBX preview demo integration

The current importer generates a static X360M bind-pose cache. Use this boot sequence in the game's startup code:

    #include "fbx_import.h"
    #include "fbx_viewer_camera.h"

    FbxViewerCamera preview_camera;
    AnimatedModel opponent_model;

    fbx_viewer_camera_reset(&preview_camera);

    if (!fbx_cache_ensure("game:\\media\\Adventurer.fbx",
                          "game:\\media\\adventurer.x360m")) {
        /* Show an FBX import error and stop the preview. */
    } else if (!model_load_x360m(&opponent_model,
                                "game:\\media\\adventurer.x360m")) {
        /* Show a cache load error. */
    }

Each frame, call:

    fbx_viewer_camera_update(&preview_camera, delta_seconds);

Then copy preview_camera.x/y/z/yaw/pitch into the renderer's Camera and submit opponent_model to the renderer's model draw function.

## Controller

- Left stick: move forward/back and strafe.
- Right stick: look around.
- LT / RT: move down / up.
- Back: reset camera to (0, 1, -4).

## Important current limitation

The repository does not yet contain a complete app entry point, renderer initialization/draw function, or shared engine header. This example documents the integration points and supplies the camera controller, but it is not yet a standalone runnable viewer. The importer currently creates a rigid bind-pose model; skeletal animation baking is not implemented yet.
