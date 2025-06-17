#version 100
precision mediump float;

uniform float iTime;
uniform vec2  iResolution;
varying vec2  fragTexCoord;

float rand(vec2 co){
    return fract(sin(dot(co,vec2(12.9898,78.233)))*43758.5453);
}

// very simple noise
float noise1(vec2 p){
    float i = floor(p.x) + floor(p.y)*57.0;
    return rand(vec2(i,i));
}

void main() {
    // uv in 0..1
    vec2 uv = fragTexCoord;
    // scroll fire upward at 0.3 units/sec
    float t = iTime * 0.3;
    // aspect‐corrected coords
    vec2 pos = uv * vec2(iResolution.x/iResolution.y,1.0);
    pos.y -= t;
    // 1 octave of noise
    float n = noise1(pos * 10.0);
    // interpolate between two fire colors
    float g = smoothstep(0.0, n, 1.0 - uv.y);
    vec3 col = mix(vec3(1.0,0.65,0.1), vec3(1.0,0.0,0.15), g);
    gl_FragColor = vec4(col, g);
}
