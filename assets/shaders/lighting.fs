#version 100
precision mediump float;
#define MAX_LIGHTS 4

varying vec3 vPos;
varying vec3 vNrm;
varying vec2 vUV;

uniform sampler2D texture0;
uniform vec4  colDiffuse;
uniform vec4  ambient;
uniform vec3  viewPos;

uniform float lightStrength;

struct Light {
    int  enabled;
    vec3 position;
    vec3 target;
    vec4 color;
};
uniform Light lights[MAX_LIGHTS];

void main()
{
    vec3 N = normalize(vNrm);
    vec3 V = normalize(viewPos - vPos);
    vec3 result = ambient.rgb * colDiffuse.rgb;

    for (int i=0;i<MAX_LIGHTS;i++)
    {
        if (lights[i].enabled == 0) continue;
        vec3 L = normalize(lights[i].position - vPos);

        float diff = max(dot(N,L),0.0);
        vec3  diffCol = diff * lights[i].color.rgb * colDiffuse.rgb;

        vec3 H = normalize(L+V);
        float spec = pow(max(dot(N,H),0.0),16.0);
        vec3  specCol = spec * lights[i].color.rgb;

        result += (diffCol + specCol) * lightStrength;
    }

    vec4 tex = texture2D(texture0, vUV);
    gl_FragColor = vec4(result,1.0) * tex;
}
