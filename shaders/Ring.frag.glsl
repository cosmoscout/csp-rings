#version 440 compatibility

uniform sampler2D uSurfaceTexture;
uniform float uAmbientBrightness;
uniform float uFarClip;

// inputs
in vec2 vTexCoords;
in vec3 vSunDirection;
in vec3 vPosition;

// outputs
layout(location = 0) out vec4 oColor;

void main() {
    oColor = texture(uSurfaceTexture, vTexCoords);
    gl_FragDepth = length(vPosition) / uFarClip;
}