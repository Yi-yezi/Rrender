#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2DArray u_shadowMap;        // 2D 数组阴影贴图
uniform samplerCubeArray u_pointShadowMap; // 立方体贴图数组阴影贴图
uniform int u_layer;
uniform int u_face;        // 立方体贴图面索引
uniform int u_mapType;     // 0: 2D数组, 1: 立方体数组

// 立方体贴图面方向向量
vec3 GetCubeDirection(vec2 uv, int face) {
    vec2 coord = uv * 2.0 - 1.0; // 转换到 [-1, 1] 范围
    
    switch(face) {
        case 0: return vec3( 1.0, -coord.y, -coord.x); // +X
        case 1: return vec3(-1.0, -coord.y,  coord.x); // -X
        case 2: return vec3( coord.x,  1.0,  coord.y); // +Y
        case 3: return vec3( coord.x, -1.0, -coord.y); // -Y
        case 4: return vec3( coord.x, -coord.y,  1.0); // +Z
        case 5: return vec3(-coord.x, -coord.y, -1.0); // -Z
    }
    return vec3(0.0);
}

void main() {
    float depthValue;
    
    if (u_mapType == 0) {
        // 2D 数组阴影贴图 - 这个分支会被执行
        depthValue = texture(u_shadowMap, vec3(TexCoords, u_layer)).r;
    } else {
        // 立方体贴图数组阴影贴图 - 这个分支不会执行
        vec3 direction = GetCubeDirection(TexCoords, u_face);
        depthValue = texture(u_pointShadowMap, vec4(direction, u_layer)).r;
    }
    
    // 将深度值可视化
    FragColor = vec4(vec3(depthValue), 1.0);
}