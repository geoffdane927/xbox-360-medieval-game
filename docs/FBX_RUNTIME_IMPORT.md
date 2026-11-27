# Runtime FBX import and first-run cache

## Asset inspected

The supplied Adventurer file is binary FBX 7.4 (version 7400), 8,259,852 bytes. Its take list includes CharacterArmature|Idle, Walk, Run, Death, HitRecieve, Sword_Slash, Wave, and other takes. The source FBX is not committed to this repository; place it at game:\\media\\Adventurer.fbx on the console.

## Parser dependency

The project pins ufbx as a Git submodule at vendor/ufbx. Initialize it with:

    git submodule update --init --recursive

ufbx is a portable C FBX loader that supports binary/ASCII FBX and compressed arrays. Compile vendor/ufbx/ufbx.c separately and add vendor/ufbx to the include path. Keep the upstream MIT license with the dependency.

**XDK warning:** ufbx is documented for C99/C++11 and mainstream modern toolchains. The legacy Xbox 360 XDK compiler is not in its tested compiler list. This integration has not been compiled with the proprietary XDK; compatibility work may be required.

## Current importer behavior

src/fbx_import.c parses the FBX through ufbx, triangulates every mesh node, transforms geometry into world space, and writes an X360M v1 cache. It currently bakes the model as one rigid root bone and writes a single BindPose clip. It imports positions, normals and the first UV set. This is enough for a static model preview, not an animated character.

The current X360M v1 index field is 16-bit. The importer refuses files above 65,535 emitted vertices rather than truncating indices. It does not yet write the source hash into the cache, so remove the cache manually if the FBX changes.

## Cache boot flow

Call before model_load_x360m():

    if (!fbx_cache_ensure("game:\\media\\Adventurer.fbx",
                          "game:\\media\\adventurer.x360m")) {
        /* show import failure; do not load a partial model */
    }
    if (!model_load_x360m(&opponent_model,
                          "game:\\media\\adventurer.x360m")) {
        /* show cache load failure */
    }

A valid X360M cache header is reused; otherwise the FBX is parsed and the cache is generated. Delete adventurer.x360m to force a rebuild after replacing the FBX.

## Not implemented yet

- Skeletal hierarchy and inverse-bind conversion.
- Sampling FBX animation stacks into X360M animation clips.
- Camera-viewer application loop and actual draw integration.
- Cache source hashing, atomic temporary-file replacement, and stale-cache detection.
- Textures/materials, blend shapes, tangents, and multiple UV sets.

The renderer in this branch is still source scaffolding. There is no verified XEX build or on-console display test. Do not treat the static importer as a finished animated-model pipeline.
