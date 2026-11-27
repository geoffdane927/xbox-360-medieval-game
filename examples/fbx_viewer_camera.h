#ifndef FBX_VIEWER_CAMERA_H
#define FBX_VIEWER_CAMERA_H
typedef struct {
    float x, y, z;
    float yaw, pitch;
} FbxViewerCamera;
void fbx_viewer_camera_reset(FbxViewerCamera *camera);
void fbx_viewer_camera_update(FbxViewerCamera *camera, float dt);
#endif
