#version 100
precision mediump float;
attribute vec3 vertexPosition;
attribute vec3 vertexNormal;
attribute vec2 vertexTexCoord;

varying vec3 vPos;
varying vec3 vNrm;
varying vec2 vUV;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

void main()
{
    vPos = vec3(matModel * vec4(vertexPosition,1.0));
    vNrm = normalize(vec3(matNormal * vec4(vertexNormal,0.0)));
    vUV  = vertexTexCoord;
    gl_Position = mvp * vec4(vertexPosition,1.0);
}
