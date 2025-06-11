# Flame Jump

A lightweight WebAssembly/WebGL game for the MicroGim game jam using Raylib and Emscripten.

---

## Prerequisites

- **Git** (≥2.20)  
- **Emscripten SDK** (`emsdk`)  
- **CMake** (≥3.14)  
- **Make**  
- **Python** (for `emrun`, optional)

Ensure your shell sources Emscripten before building or launching your editor:

```bash
source lib/emsdk/emsdk_env.sh
```
## Clone & Submodules
```bash
# Clone this repo (replace URL accordingly)
git clone https://github.com/yourusername/pico-raylib-web.git
cd pico-raylib-web

# Initialize & update submodules (Raylib, etc.)
git submodule update --init --recursive
```

## Building Raylib for Web
```bash
# From project root:
cd lib/raylib
source ../emsdk/emsdk_env.sh          # make emcc/em++ available
make PLATFORM=PLATFORM_WEB            # produces src/libraylib.web.a
```
Alternatively, you can use CMake:
```bash
mkdir -p build_web && cd build_web
emcmake cmake .. -DPLATFORM=Web
emmake make
```

## CMake Build (Optional)

A helper script cmake.sh will generate build files and compile commands for LSP.
```bash
# Ensure emscripten is loaded
source lib/emsdk/emsdk_env.sh

# Generate build files
sh cmake.sh        # creates build/ directory and compile_commands.json
or
./cmake.sh
```

## Build project
```bash
sh web.sh
or
./web.sh
```
## Info

Build files will be located in the *web_build* folder.

All assets must be contained within the *assets* folder.
