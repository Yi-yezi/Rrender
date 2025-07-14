#version 330 core
in vec4 FragPos;

uniform vec3 u_PointLightPos;
uniform float u_PointLightFarPlane;

void main() {
    float lightDistance = length(FragPos.xyz - u_PointLightPos);
    lightDistance = lightDistance / u_PointLightFarPlane;
    gl_FragDepth = lightDistance;
}