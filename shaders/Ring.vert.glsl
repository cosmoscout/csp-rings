#version 440 compatibility

uniform vec3 uSunDirection;
uniform vec2 uRadii;
uniform mat4 uMatModelView;
uniform mat4 uMatProjection;

// inputs
layout(location = 0) in vec2 iGridPos;

// outputs
out vec2 vTexCoords;
out vec3 vPosition;
const float PI = 3.141592654;

void main() {
    vTexCoords = iGridPos.yx;

    vec2 vDir = vec2(sin(iGridPos.x * 2.0 * PI), cos(iGridPos.x * 2.0 * PI));

    vec2 vPos = mix(vDir * uRadii.x, vDir * uRadii.y, iGridPos.y);

    vPosition   = (uMatModelView * vec4(vPos.x, 0, vPos.y, 1.0)).xyz;
    gl_Position =  uMatProjection * vec4(vPosition, 1);
}