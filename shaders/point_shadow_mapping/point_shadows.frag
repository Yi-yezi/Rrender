#version 330 core
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


in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D u_DiffuseTexture;
uniform samplerCube u_PointShadowMap;

uniform float u_PointLightFarPlane;
uniform vec3 u_CameraPos;

const float shininess = 32.0;


vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);


// 计算阴影
float ShadowCalculation(vec3 fragPos, vec3 pointLightPos) {
    vec3 fragToLight = fragPos - pointLightPos;
    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(u_CameraPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / u_PointLightFarPlane)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(u_PointShadowMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= u_PointLightFarPlane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
        
    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    
        
    return shadow;
}




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

    // 计算阴影
    float shadow = ShadowCalculation(fragPos, light.position);
    return ambient + (1.0 - shadow) * (diffuse + specular);
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
