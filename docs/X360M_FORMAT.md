# X360M v1 binary layout

All integers and floats are little-endian. Matrices are 16 float32 values in column-major order. The runtime uses the same convention.

1. Header: 8-byte magic `X360M01\0`, then uint32 version (=1), mesh count, bone count, clip count.
2. Each bone: 32-byte UTF-8 name, int32 parent index (-1 for root), 16 float32 inverse-bind matrix.
3. Each mesh: uint32 vertex count, uint32 index count, then vertices, then uint16 triangle indices.
4. Vertex: float32 position[3], normal[3], uv[2], uint16 bone[4], float32 weight[4] (56 bytes).
5. Each clip: 32-byte name, float32 duration, float32 sample rate, uint32 frame count, then frame_count * bone_count matrices (16 float32 each).

The exporter emits one vertex per triangle corner so per-face UVs remain representable. The runtime CPU-skins positions and normals, then streams the result to a dynamic GPU vertex buffer.

This is an early format, not a stable interchange standard. Validate the exporter against the actual FBX before relying on it for final assets.
