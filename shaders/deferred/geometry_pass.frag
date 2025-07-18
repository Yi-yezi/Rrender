#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec; // RGB: Albedo, A: Specular factor

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D u_DiffuseTexture;

void main()
{
    gPosition = FragPos;
    gNormal = normalize(Normal);
    vec3 albedo = texture(u_DiffuseTexture, TexCoords).rgb;
    gAlbedoSpec.rgb = albedo;
    gAlbedoSpec.a = 0.5; // Specular factor, can be adjusted as needed
}
