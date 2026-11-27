# ufbx dependency

The runtime FBX importer uses ufbx (https://github.com/ufbx/ufbx), a single-source C FBX loader. It supports binary and ASCII FBX, compressed arrays, skin clusters, mesh attributes, and animation stacks.

Initialize it from the repository root:

    git submodule add https://github.com/ufbx/ufbx.git vendor/ufbx
    git submodule update --init --recursive

Compile vendor/ufbx/ufbx.c as a separate C translation unit and add vendor/ufbx to the include path. ufbx is MIT licensed; retain its upstream LICENSE file.

IMPORTANT: ufbx's documented/tested toolchains do not include the legacy Xbox 360 XDK compiler. This integration is a porting target until ufbx compiles with the user's exact XDK. Do not assume a successful XEX build.
