#version 330 core

out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main() {
    vec3 N = normalize(WorldPos);
    vec3 irradiance = vec3(0.0);

    // Define the up vector and right vector based on the normal
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));

    // Sample the environment map in a hemisphere around the normal
    float sampleDelta = 0.025;
    float sampleCount = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
            // Spherical coordinates to Cartesian coordinates
            vec3 sampleDir = vec3(
                sin(theta) * cos(phi),
                sin(theta) * sin(phi),
                cos(theta)
            );

            // Transform sample direction to world space
            sampleDir = normalize(sampleDir.x * right + sampleDir.y * up + sampleDir.z * N);

            irradiance += texture(environmentMap, sampleDir).rgb * cos(theta) * sin(theta); 
            sampleCount++;
        }
    }
    irradiance = PI * irradiance * (1.0 / sampleCount);
    FragColor = vec4(irradiance, 1.0);
}