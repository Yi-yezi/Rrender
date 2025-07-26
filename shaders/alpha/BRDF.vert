#version 440 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

// 针对方向光和聚光灯
out vec4 FragPosDirLightSpace[4];
out vec4 FragPosSpotLightSpace[4];

// 点光源不需要 lightVP，只需光源位置

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

uniform int u_dirLightCount; // 方向光数量
uniform int u_spotLightCount; // 聚光灯数量

// 方向光和聚光灯的VP矩阵数组
uniform mat4 u_dirLightVP[4];
uniform mat4 u_spotLightVP[4];

void main()
{
    TexCoords = aTexCoords;
    WorldPos = vec3(u_model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(u_model))) * aNormal;

    // 针对每个方向光
    for(int i = 0; i < u_dirLightCount; ++i)
        FragPosDirLightSpace[i] = u_dirLightVP[i] * vec4(WorldPos, 1.0);

    // 针对每个聚光灯
    for(int i = 0; i < u_spotLightCount; ++i)
        FragPosSpotLightSpace[i] = u_spotLightVP[i] * vec4(WorldPos, 1.0);

    gl_Position = u_projection * u_view * vec4(WorldPos, 1.0);
}