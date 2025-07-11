#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

void main() {
    FragPos = vec3(u_Model * vec4(a_Position, 1.0));
    Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
    TexCoords = a_TexCoords;

    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
}
    