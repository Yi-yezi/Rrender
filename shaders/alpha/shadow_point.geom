#version 440 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 u_lightVP[6]; // 立方体的6个面VP矩阵

out vec4 FragPos; // 传递给片元着色器的位置

void main() {
    for(int face = 0; face < 6; face++) {
        gl_Layer = face; // 设置当前图层为立方体的一个面
        for(int i = 0; i < 3; ++i) {
            FragPos = gl_in[i].gl_Position; // 获取输入顶点的位置
            gl_Position = u_lightVP[face] * FragPos; // 将顶点位置转换到光源空间
            EmitVertex(); // 发出当前顶点
        }
        EndPrimitive(); // 结束当前三角形
    }
}


