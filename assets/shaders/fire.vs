#version 100
#ifdef GL_ES
  precision mediump float;
#endif

// Raylib default vertex attributes:
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;

// Raylib uniform for Model·View·Projection:
uniform mat4 mvp;

// pass UV to fragment shader
varying vec2 fragUV;

void main() {
    fragUV = vertexTexCoord;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
