#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2DArray u_shadowMap;
uniform int u_layer; // 要显示的array层

void main()
{
    float depth = texture(u_shadowMap, vec3(TexCoords, u_layer)).r;
    FragColor = vec4(vec3(depth), 1.0); // 可视化深度
}