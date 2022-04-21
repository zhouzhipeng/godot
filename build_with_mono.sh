set -eux

export HTTPS_PROXY=127.0.0.1:7890

## Build temporary binary
#scons p=windows tools=yes module_mono_enabled=yes mono_glue=no -j10
## Generate glue sources
#bin/godot.windows.tools.64.mono --generate-mono-glue modules/mono/glue -j10

### Build binaries normally
# Editor
scons p=windows target=debug tools=yes module_mono_enabled=no -j10