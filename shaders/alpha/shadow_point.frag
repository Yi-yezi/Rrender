#version 450 core

in vec4 FragPos;

uniform vec3 u_lightPos;
uniform float u_farPlane;

void main()
{
    // 计算当前片段到光源的距离
    float lightDistance = length(FragPos.xyz - u_lightPos);
    
    // 将距离映射到[0,1]范围
    lightDistance = lightDistance / u_farPlane;
    
    // 写入深度值
    gl_FragDepth = lightDistance;
}