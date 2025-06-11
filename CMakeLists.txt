cmake_minimum_required(VERSION 3.14)

# ✅ This line enables the LSP-friendly compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(raylib_web_project)

set(CMAKE_CXX_STANDARD 17)

include_directories(./ lib/raylib/src)
add_definitions(-DPLATFORM_WEB)

set(SOURCES src/main.cpp)

add_executable(app ${SOURCES})
target_link_libraries(app ${CMAKE_SOURCE_DIR}/lib/raylib/src/libraylib.web.a)
