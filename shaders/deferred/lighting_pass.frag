#version 330 core

struct Light {
    int type;           // 0=directional, 1=point, 2=spot

    vec3 position;      
    vec3 direction;     

    vec3 color;
    float intensity;

    float constant;
    float linear;
    float quadratic;

    float innerCutOff;
    float outerCutOff;
};

#define MAX_LIGHTS 4
uniform int u_LightCount;
uniform Light u_Lights[MAX_LIGHTS];

uniform vec3 u_CameraPos;

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

const float shininess = 32.0;

// ----------------------- Light Calculations ------------------------
vec3 CalcDirectionalLight(Light light, vec3 normal, vec3 viewDir, float specularStrength) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);

    vec3 ambient = 0.1 * light.color * light.intensity;
    vec3 diffuse = diff * light.color * light.intensity;
    vec3 specular = spec * light.color * light.intensity * specularStrength;
    return ambient + diffuse + specular;
}

vec3 CalcPointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, float specularStrength) {
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    vec3 ambient = 0.1 * light.color * light.intensity * attenuation;
    vec3 diffuse = diff * light.color * light.intensity * attenuation;
    vec3 specular = spec * light.color * light.intensity * attenuation * specularStrength;
    return ambient + diffuse + specular;
}

vec3 CalcSpotLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, float specularStrength) {
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.innerCutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 ambient = 0.1 * light.color * light.intensity * attenuation * intensity;
    vec3 diffuse = diff * light.color * light.intensity * attenuation * intensity;
    vec3 specular = spec * light.color * light.intensity * attenuation * intensity * specularStrength;
    return ambient + diffuse + specular;
}

// ------------------------ Main ------------------------
void main() {
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;

    vec3 viewDir = normalize(u_CameraPos - FragPos);

    vec3 lighting = vec3(0.0);
    for (int i = 0; i < u_LightCount; ++i) {
        Light light = u_Lights[i];
        if (light.type == 0) {
            lighting += CalcDirectionalLight(light, Normal, viewDir, Specular);
        } else if (light.type == 1) {
            lighting += CalcPointLight(light, Normal, FragPos, viewDir, Specular);
        } else if (light.type == 2) {
            lighting += CalcSpotLight(light, Normal, FragPos, viewDir, Specular);
        }
    }

    vec3 finalColor = lighting * Albedo;
    FragColor = vec4(finalColor, 1.0);
}
