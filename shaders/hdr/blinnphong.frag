// GLSL 统一光源类型定义
struct Light {
    int type;           // 0=directional, 1=point, 2=spot

    vec3 position;      // 点光、聚光用
    vec3 direction;     // 方向光、聚光用

    vec3 color;
    float intensity;

    // 衰减参数，仅点光和聚光有效
    float constant;
    float linear;
    float quadratic;

    // 聚光灯专用角度
    float innerCutOff;
    float outerCutOff;
};

#define MAX_LIGHTS 4
uniform int u_LightCount;
uniform Light u_Lights[MAX_LIGHTS];

uniform vec3 u_CameraPos;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D u_DiffuseTexture;

const float shininess = 32.0;

vec3 CalcDirectionalLight(Light light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);

    vec3 ambient = 0.1 * light.color * light.intensity;
    vec3 diffuse = diff * light.color * light.intensity;
    vec3 specular = spec * light.color * light.intensity;
    return ambient + diffuse + specular;
}

vec3 CalcPointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    vec3 ambient = 0.1 * light.color * light.intensity * attenuation;
    vec3 diffuse = diff * light.color * light.intensity * attenuation;
    vec3 specular = spec * light.color * light.intensity * attenuation;
    return ambient + diffuse + specular;
}

vec3 CalcSpotLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir) {
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
    vec3 specular = spec * light.color * light.intensity * attenuation * intensity;
    return ambient + diffuse + specular;
}

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(u_CameraPos - FragPos);

    vec3 lighting = vec3(0.0);

    for (int i = 0; i < u_LightCount; ++i) {
        Light light = u_Lights[i];
        if (light.type == 0) {           // Directional
            lighting += CalcDirectionalLight(light, norm, viewDir);
        } else if (light.type == 1) {    // Point
            lighting += CalcPointLight(light, norm, FragPos, viewDir);
        } else if (light.type == 2) {    // Spot
            lighting += CalcSpotLight(light, norm, FragPos, viewDir);
        }
    }

    vec3 texColor = texture(u_DiffuseTexture, TexCoords).rgb;
    vec3 finalColor = lighting * texColor;

    FragColor = vec4(finalColor, 1.0);
}
