# Xbox 360 Medieval Game

Single-player first-person medieval archery game for Xbox 360. The player is first-person (no player body mesh); the Adventurer model is intended for AI opponents.

## Renderer milestone

This branch adds a modular XDK renderer and a first-pass animated-model pipeline.

- `src/main.c` — startup and frame loop
- `src/game.c` — game-level update/draw orchestration
- `src/graphics.c` — D3D9-style XDK device, camera, buffers and draw calls
- `src/input.c` — Xbox 360 controller input
- `src/model.c` — X360M model loading and CPU skinning
- `src/animation.c` — clip selection and animation time
- `src/math3d.c` — vector and matrix math
- `include/` — shared declarations
- `shaders/` — model shaders
- `tools/export_x360m.py` — Blender FBX-to-X360M exporter
- `docs/X360M_FORMAT.md` — binary format specification

## Asset conversion

1. Import the Adventurer FBX into Blender 2.79.
2. Run `tools/export_x360m.py` from Blender's Text Editor.
3. Set `OUTPUT_PATH` near the top of the script to your desired output path.
4. Copy the resulting file to `game:\\media\\adventurer.x360m`.

The exporter writes meshes, UVs, normals, up to four bone weights per vertex, inverse-bind matrices, and sampled Blender actions. It exports one vertex per triangle corner so per-face UVs are retained.

## Build status

The code targets the Xbox 360 XDK's `xtl.h`, XInput and D3D9-style interfaces. The .c files are intended to be compiled as C++ translation units (`/TP`) because the D3D interfaces use COM-style methods. Shader binaries must be compiled with the shader compiler supplied by the installed XDK and placed at:

- `game:\\media\\shaders\\model_vs.bin`
- `game:\\media\\shaders\\model_ps.bin`

The GitHub connector cannot run the proprietary XDK toolchain. This is source scaffolding and has not yet been verified as a built XEX on hardware. XDK releases can differ in compiler and library names, so the final project/link settings still need to be confirmed against the installed SDK.

The first renderer uses CPU skeletal skinning and uploads deformed vertices to a dynamic vertex buffer each frame. Texture/material support and optimization are later milestones.

## Controls

- Left stick: move debug camera
- Right stick: look
- A: next animation clip
- Back: reset camera

## Current limitation

The FBX itself is not in this repository, and the exporter output has not yet been validated against the actual Adventurer file. Put the converted `.x360m` file in the media path above. If you re-upload the FBX here, I can verify the action names and adjust the exporter against the actual asset.


## Runtime FBX preview experiment

The branch now includes a ufbx submodule and a first-pass runtime converter:

- Run `git submodule update --init --recursive` to fetch ufbx.
- Compile `vendor/ufbx/ufbx.c` and `src/fbx_import.c`.
- Put the source asset at `game:\\media\\Adventurer.fbx`.
- Call `fbx_cache_ensure()` to create/reuse `game:\\media\\adventurer.x360m`.
- Load the cache through `model_load_x360m()`.

The current converter imports all mesh nodes and bakes them into a rigid bind-pose preview. It does **not** yet preserve the skeleton or animations. The Xbox controller camera helper is in `examples/fbx_viewer_camera.c`.

This is not yet a runnable viewer or a verified XEX. The repository still needs a complete engine header, app entry point, renderer initialization/draw integration, and an XDK compatibility/build test. See `docs/FBX_RUNTIME_IMPORT.md` and `examples/README.md` for exact scope and limitations.
