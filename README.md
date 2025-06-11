# Flame Jump

A lightweight WebAssembly/WebGL game for the MicroGim game jam using Raylib and Emscripten.

---

## Prerequisites

- **Git** (≥2.20)  
- **Emscripten SDK** (`emsdk`)  
- **CMake** (≥3.14)  
- **Make**  
- **Python** (for `emrun`, optional)

## Clone & Submodules

```bash
git clone https://github.com/heliumsneakers/FlameJump-Game-Jam.git
cd pico-raylib-web

# Initialize & update submodules (Raylib, emscripten SDK etc.)
git submodule update --init --recursive
```

## IMPORTANT

Ensure your shell sources Emscripten before building or launching your editor:
```bash
source lib/emsdk/emsdk_env.sh
```

## Building Raylib for Web

```bash
# From project root:
cd lib/raylib
source ../emsdk/emsdk_env.sh          # make emcc/em++ available
make PLATFORM=PLATFORM_WEB            # produces src/libraylib.web.a
```

Alternatively, you can use Emscripten CMake:
```bash
mkdir -p build_web && cd build_web
emcmake cmake .. -DPLATFORM=Web
emmake make
```

## CMake Project Build (Optional)

A helper script cmake.sh will generate build files and compile commands for LSP.
```bash
# Ensure emscripten is loaded
source lib/emsdk/emsdk_env.sh

# Generate build files
sh cmake.sh        # creates build/ directory and compile_commands.json
or
./cmake.sh
```

## Build project for Web
```bash
sh web.sh
or
./web.sh
```
## Info

Build files will be located in the *web_build* folder.

All assets must be contained within the *assets* folder.
