#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;


out vec4 FragPos;

uniform mat4 u_ShadowMatrices[6];

void main() {
    for(int face = 0; face < 6; ++face) {
        gl_Layer = face;
        for(int i = 0; i < 3; ++i) {
            FragPos = gl_in[i].gl_Position;
            gl_Position = u_ShadowMatrices[face] * FragPos;
            EmitVertex(); // Emit the transformed vertex for the current face
        }
        EndPrimitive(); // End the triangle for the current face
    }
}