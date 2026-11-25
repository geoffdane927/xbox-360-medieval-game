# Runtime FBX import and one-time cache

## Source asset inspected

The supplied Adventurer asset is a binary FBX 7.4 file (FBX version 7400), 8,259,852 bytes.

- SHA-256: `b3a28d7a6578e2039908b11d1516c98130f7b4f1a5553141be51349c8fc63bf3`
- Intended runtime role: AI opponent only. The player remains first-person and does not render a player body.

The binary itself has **not** been committed by this change. The GitHub file-writing connector accepts UTF-8 files and Git blobs, but this session has no binary upload/LFS transfer path for the 8.3 MB source asset. Do not treat this metadata as the model file.

## Required console boot flow

The intended runtime contract is:

1. Look for `game:\media\cache\adventurer.x360m`.
2. Validate a cache header containing a format version, source FBX SHA-256, and payload length.
3. If the cache is valid, call `model_load_x360m` and do not parse FBX again.
4. If the cache is absent or stale, read `game:\media\source\adventurer.fbx`, parse and convert it, write a temporary cache file, flush/close it, then rename it to `adventurer.x360m`.
5. Load the finished cache. If conversion fails, report a useful error and do not overwrite a previously valid cache.

The source FBX should remain on the console so a cache can be rebuilt after a format-version change. A cache is only rebuilt when missing, invalid, or generated from a different source hash/version.

## FBX 7.4 importer requirements

A real on-console converter must parse the binary FBX node tree, decode little-endian scalar properties, inflate compressed array properties (Deflate/zlib), and resolve object connections. It must then convert:

- Geometry: control points, polygon vertex indices, normals, UVs, triangulation, and material-independent mesh grouping.
- Skinning: Skin/Cluster links, bone hierarchy, bind pose and inverse-bind matrices, and normalized top-four vertex influences.
- Animation: animation stacks/layers, curve nodes and curves, key times/values, interpolation, and sampled local/global bone transforms.
- Coordinate conversion: FBX axis system, units, handedness, and matrix conventions into the renderer's X360M convention.

The parser must reject unsupported or malformed input with bounded allocations; it must not assume the file is trusted.

## Important status

This document defines the intended architecture, **not an implemented FBX importer**. The current renderer only loads the already-converted X360M format. The Blender exporter remains the working conversion route until the native FBX parser, zlib dependency, cache writer, and actual XDK build have been implemented and tested. Do not claim that direct FBX loading works on the Xbox 360 yet.
