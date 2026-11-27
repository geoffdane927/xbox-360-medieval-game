#ifndef X360_FBX_IMPORT_H
#define X360_FBX_IMPORT_H
int fbx_cache_ensure(const char *fbx_path, const char *cache_path);
int fbx_convert_to_x360m(const char *fbx_path, const char *cache_path);
#endif
