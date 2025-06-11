FlameJump-Game-Jam

A lightweight WebAssembly/WebGL demo using Raylib and Emscripten.

⸻

Prerequisites
	•	Git (≥2.20)
	•	Emscripten SDK (emsdk)
	•	CMake (≥3.14)
	•	Make
	•	Python (for emrun, optional)

Ensure your shell sources Emscripten before building or launching your editor:

source lib/emsdk/emsdk_env.sh



⸻

Clone & Submodules

# Clone this repo (replace URL accordingly)
$ git clone https://github.com/yourusername/pico-raylib-web.git
$ cd pico-raylib-web

# Initialize & update submodules (Raylib, etc.)
$ git submodule update --init --recursive


⸻

Building Raylib for Web

# From project root:
$ cd lib/raylib
$ source ../emsdk/emsdk_env.sh          # make emcc/em++ available
$ make PLATFORM=PLATFORM_WEB            # produces src/libraylib.web.a

Alternatively, you can use CMake:

mkdir -p lib/raylib/build_web && cd lib/raylib/build_web
emcmake cmake .. -DPLATFORM=Web
emmake make



⸻

CMake Build (Optional)

A helper script cmake.sh will generate a desktop/Web build via CMake.

# Ensure emscripten is loaded
$ source lib/emsdk/emsdk_env.sh

# Generate build files
$ ./cmake.sh        # creates build/ directory and compile_commands.json

You can then build with:

$ cd build
$ make


⸻

Web Build & Run

Use the web.sh script to compile all sources under src/ to WebAssembly and launch a local server:

$ chmod +x web.sh
$ ./web.sh

This will:
	1.	Compile .cpp and .c files in src/
	2.	Link with libraylib.web.a and embed assets/
	3.	Produce web_build/index.html + .wasm
	4.	Launch emrun on port 8080

Open your browser at http://localhost:8080 to see the prism-player spinning and bobbing.

License

MIT
