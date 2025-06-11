#!/bin/bash
set -euo pipefail

# Path to Emscripten
EMSDK_PATH="lib/emsdk"

# Source the Emscripten environment
cd $EMSDK_PATH
source emsdk_env.sh

# Navigate back to your project directory
cd ..
cd ..

# Build to WebAssembly
emcc -o ./web_build/index.html \
   src/main.cpp \
    -Os -Wall \
    -I./ \
    -I lib/raylib/src \
    -L lib/raylib/src -lraylib.web \
    -s USE_GLFW=3 \
    -s ASYNCIFY \
    --shell-file ./shell.html \
    --embed-file assets@/assets \
    -s TOTAL_STACK=64MB \
    -s INITIAL_MEMORY=256MB \
    -s MAXIMUM_MEMORY=512MB \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s ASSERTIONS=2 \
    -s SAFE_HEAP=1 \
    -DPLATFORM_WEB

emrun --no_browser --port 8080 web_build/index.html

