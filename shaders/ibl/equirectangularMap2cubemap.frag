#version 330 core

in vec3 WorldPos;

out vec4 FragColor;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183); // 1 / (2 * PI), 1 / (PI)
// ----------------------------------------------------------------------------

vec2 equirectangularToCubemapUV(vec3 dir){
    vec2 uv = vec2(atan(dir.z, dir.x), asin(dir.y)) * invAtan;
    uv = uv + 0.5; // shift from [-1, 1] to [0, 1]
    return uv;
}

void main(){
    vec3 dir = normalize(WorldPos);
    vec2 uv = equirectangularToCubemapUV(dir);

    // Sample the equirectangular map
    vec4 color = texture(equirectangularMap, uv);

    // Output the color
    FragColor = color;

}