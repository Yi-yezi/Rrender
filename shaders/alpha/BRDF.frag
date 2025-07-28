#version 440 core

out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in vec4 FragPosDirLightSpace[4];
in vec4 FragPosSpotLightSpace[4];

const float PI = 3.14159265359;

// Material workflow type
uniform int u_materialWorkflow; // 0: Metallic-Roughness, 1: Specular-Glossiness, 2: No-Texture

// Material texture samplers (使用 glTF 2.0 标准命名)
uniform sampler2D u_baseColorMap;         // glTF: baseColorTexture (替代 u_albedoMap)
uniform sampler2D u_normalMap;            // glTF: normalTexture
uniform sampler2D u_metallicRoughnessMap; // glTF: metallicRoughnessTexture
uniform sampler2D u_specularGlossinessMap; // glTF 扩展: specularGlossinessTexture (替代 u_specularGlossiness)
uniform sampler2D u_occlusionMap;         // glTF: occlusionTexture (替代 u_aoMap)
uniform sampler2D u_emissionMap;          // glTF: emissionTexture

// Material texture availability flags
uniform bool u_hasBaseColorTexture;
uniform bool u_hasMetallicRoughnessTexture;
uniform bool u_hasDiffuseTexture;
uniform bool u_hasSpecularGlossinessTexture;
uniform bool u_hasNormalTexture;
uniform bool u_hasOcclusionTexture;
uniform bool u_hasEmissionTexture;

// Material factor values (glTF 2.0 标准)
// Metallic-Roughness workflow
uniform vec3 u_baseColor;        // baseColorFactor
uniform float u_metallicFactor;  // metallicFactor
uniform float u_roughnessFactor; // roughnessFactor

// Specular-Glossiness workflow (KHR_materials_pbrSpecularGlossiness)
uniform vec3 u_diffuseColor;     // diffuseFactor
uniform vec3 u_specularColor;    // specularFactor
uniform float u_glossinessFactor; // glossinessFactor

// Common material parameters
uniform vec3 u_emissiveColor;    // emissiveFactor
uniform float u_normalScale;     // normalTexture.scale
uniform float u_occlusionStrength; // occlusionTexture.strength

// Alpha mode
uniform int u_alphaMode;         // 0: OPAQUE, 1: MASK, 2: BLEND
uniform float u_alphaCutoff;     // alphaCutoff

// IBL
uniform samplerCube u_irradianceMap;
uniform samplerCube u_prefilterMap;
uniform sampler2D u_brdfLUT;

// shadow开关
uniform bool u_enableShadow;

// shadow
uniform sampler2DArray u_dirShadowMap; // 方向光阴影贴图
uniform samplerCubeArray u_pointShadowMap; // 点光源阴影贴图
uniform sampler2DArray u_spotShadowMap; // 聚光灯阴影贴图

// IBL开关
uniform bool u_enableIBL;

// lights
uniform int u_dirLightCount;
uniform int u_pointLightCount;
uniform int u_spotLightCount;

struct DirLight {
    vec3 direction;
    vec3 color;
    int shadowLayer;
};
uniform DirLight u_dirLights[4];

struct PointLight {
    vec3 position;
    vec3 color;
    int shadowLayer;
};
uniform PointLight u_pointLights[4];

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float innerCutOff;
    float outerCutOff;
    int shadowLayer;
};
uniform SpotLight u_spotLights[4];

uniform vec3 u_camPos;

// ---- PBR helper functions ----
// 获取法线贴图的法线向量: 使用切线空间法线贴图转换为世界空间法线
vec3 GetNormalFromMap()
{
    if (!u_hasNormalTexture) {
        return normalize(Normal);
    }
    
    vec3 tangentNormal = texture(u_normalMap, TexCoords).xyz * 2.0 - 1.0;
    tangentNormal.xy *= u_normalScale; // 应用法线缩放
    
    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);
    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangentNormal);
}

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// Geometry function (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// Fresnel-Schlick approximation
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Fresnel-Schlick with roughness
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ---- 阴影采样函数 ----

// 方向光阴影采样 - 简化的 PCF 版本
float SampleDirectionalShadow(sampler2DArray shadowMap, int layer, vec4 shadowCoord, vec3 normal)
{
    if (shadowCoord.w <= 0.0) return 1.0;
    
    // 透视分割
    vec3 projCoords = shadowCoord.xyz / shadowCoord.w;
    
    // NDC [-1,1] 转换到 [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    
    // 边界检查
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || 
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0) {
        return 1.0;
    }
    
    // 动态偏移计算
    vec3 lightDir = normalize(-u_dirLights[layer].direction);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.01);
    float currentDepth = projCoords.z;
    
    // PCF 采样设置
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0).xy;
    
    // 核大小硬编码（可以根据需要调整）
    int kernelSize = 3; // 5x5 核（半径为2）
    float sampleRadius = 1.0; // 采样半径倍数
    
    // 计算总采样数
    int totalSamples = (2 * kernelSize + 1) * (2 * kernelSize + 1);
    
    for(int x = -kernelSize; x <= kernelSize; ++x)
    {
        for(int y = -kernelSize; y <= kernelSize; ++y)
        {
            vec2 offset = vec2(x, y) * texelSize * sampleRadius;
            vec3 uv = vec3(projCoords.xy + offset, float(layer));
            float closestDepth = texture(shadowMap, uv).r;
            
            shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
        }    
    }
    
    shadow /= float(totalSamples); // 平均化
    
    return 1.0 - shadow; // 返回光照值（1.0 = 无阴影，0.0 = 完全阴影）
}

// 点光源阴影采样 - PCF 版本
float SamplePointShadow(samplerCubeArray shadowMap, int layer, vec3 fragToLight)
{
    float currentDepth = length(fragToLight);
    float farPlane=10.5;
    if (currentDepth > farPlane) {
        return 1.0;
    }
    
    // PCF 采样偏移向量（立方体贴图专用）
    vec3 sampleOffsetDirections[20] = vec3[]
    (
       vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
       vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
       vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
       vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
       vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    );
    
    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(u_camPos - fragToLight);
    float diskRadius = (1.0 + (viewDistance / farPlane)) / 25.0; // 距离自适应半径
    
    for(int i = 0; i < samples; ++i)
    {
        vec3 sampleDir = fragToLight + sampleOffsetDirections[i] * diskRadius;
        float closestDepth = texture(shadowMap, vec4(sampleDir, layer)).r;
        closestDepth *= farPlane;
        
        shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
    }
    shadow /= float(samples);
    
    return 1.0 - shadow;
}

// 聚光灯阴影采样 - 完全使用全局变量版本
float SampleSpotShadow(sampler2DArray shadowMap, int layer, vec4 shadowCoord, vec3 normal, vec3 lightDirection)
{
    // 使用传入的阴影坐标和层索引
    if (shadowCoord.w <= 0.0) return 1.0;
    
    // 透视分割
    vec3 projCoords = shadowCoord.xyz / shadowCoord.w;
    
    // NDC [-1,1] 转换到 [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    
    // 边界检查
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || 
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0) {
        return 1.0;
    }
    
    // 计算偏移（基于法线和光线方向）
    float bias = max(0.05 * (1.0 - dot(normal, lightDirection)), 0.005);
    float currentDepth = projCoords.z;
    
    // 简化版PCF采样
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0).xy;
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    return 1.0 - shadow;
}

// ---- PBR Lighting Function ----
vec3 PBRLighting(vec3 N, vec3 V, vec3 L, vec3 H, vec3 radiance, vec3 albedo, float metallic, float roughness, vec3 F0)
{
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);

    // Cook-Torrance BRDF
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator    = D * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.001;
    vec3 specular     = numerator / denominator;

    // kS is specular, kD is diffuse
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    // Lambertian diffuse
    vec3 diffuse = kD * albedo / PI;

    return (diffuse + specular) * radiance * NdotL;
}

// ---- 材质参数获取函数 ----
struct MaterialParams {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
    vec3 emission;
    vec3 F0;
};

MaterialParams GetMaterialParams() {
    MaterialParams params;
    
    if (u_materialWorkflow == 0) { // Metallic-Roughness
        // 基础颜色
        if (u_hasBaseColorTexture) {
            // 修正：只有颜色纹理需要 gamma 校正，不是所有纹理
            vec4 baseColorTex = texture(u_baseColorMap, TexCoords);
            params.albedo = pow(baseColorTex.rgb, vec3(2.2)) * u_baseColor;
        } else {
            params.albedo = u_baseColor;
        }
        
        // 金属度和粗糙度（这些是线性值，不需要 gamma 校正）
        if (u_hasMetallicRoughnessTexture) {
            vec3 metallicRoughness = texture(u_metallicRoughnessMap, TexCoords).rgb;
            params.metallic = metallicRoughness.b * u_metallicFactor;   // B 通道
            params.roughness = metallicRoughness.g * u_roughnessFactor; // G 通道
        } else {
            params.metallic = u_metallicFactor;
            params.roughness = u_roughnessFactor;
        }
        
        params.F0 = mix(vec3(0.04), params.albedo, params.metallic);
        
    } else if (u_materialWorkflow == 1) { // Specular-Glossiness
        // 漫反射颜色
        if (u_hasDiffuseTexture) {
            params.albedo = pow(texture(u_baseColorMap, TexCoords).rgb, vec3(2.2)) * u_diffuseColor;
        } else {
            params.albedo = u_diffuseColor;
        }
        
        // 镜面反射和光滑度
        vec3 specular = u_specularColor;
        float glossiness = u_glossinessFactor;
        
        if (u_hasSpecularGlossinessTexture) {
            vec4 specularGlossiness = texture(u_specularGlossinessMap, TexCoords);
            specular *= specularGlossiness.rgb;
            glossiness *= specularGlossiness.a;
        }
        
        // 简化的转换：根据 specular 强度判断金属度
        // 非金属材质的 F0 通常在 0.02-0.05 范围
        // 金属材质的 F0 通常 > 0.5
        float specularIntensity = max(max(specular.r, specular.g), specular.b);
        params.metallic = specularIntensity;
        
        params.roughness = (1.0 - glossiness);
        params.F0 = mix(vec3(0.04), params.albedo, params.metallic);
        
    } else { // No-Texture
        params.albedo = u_baseColor;
        params.metallic = u_metallicFactor;
        params.roughness = u_roughnessFactor;
        params.F0 = mix(vec3(0.04), params.albedo, params.metallic);
    }
    
    // 修正：添加参数范围限制（在所有工作流处理之后）
    params.metallic = clamp(params.metallic, 0.0, 1.0);
    params.roughness = clamp(params.roughness, 0.04, 1.0); // 最小值 0.04 很重要
    params.albedo = max(params.albedo, vec3(0.0)); // 避免负值
    
    // 遮挡和自发光
    if (u_hasOcclusionTexture) {
        params.ao = texture(u_occlusionMap, TexCoords).r;
        params.ao = mix(1.0, params.ao, u_occlusionStrength); // 应用遮挡强度
    } else {
        params.ao = 1.0;
    }
    
    if (u_hasEmissionTexture) {
        params.emission = texture(u_emissionMap, TexCoords).rgb * u_emissiveColor;
    } else {
        params.emission = u_emissiveColor;
    }
    
    return params;
}

// ---- 主函数 ----
void main()
{
    // 获取材质参数
    MaterialParams material = GetMaterialParams();
    
    vec3 N = GetNormalFromMap(); // 从法线贴图获取法线向量
    vec3 V = normalize(u_camPos - WorldPos); // 视线向量
    
    // IBL
    vec3 ambient = vec3(0.0);
    if (u_enableIBL) {
        vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), material.F0, material.roughness);
        vec3 KS = F; // Fresnel反射系数
        vec3 KD = vec3(1.0) - KS; // 漫反射系数
        KD *= 1.0 - material.metallic; // 漫反射系数调整
        vec3 R = reflect(-V, N); // 反射向量
        vec3 irradiance = texture(u_irradianceMap, N).rgb;
        vec3 diffuseIBL = irradiance * material.albedo;
        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColor = textureLod(u_prefilterMap, R, material.roughness * MAX_REFLECTION_LOD).rgb;
        vec2 brdf = texture(u_brdfLUT, vec2(max(dot(N, V), 0.0), material.roughness)).rg;
        vec3 specularIBL = prefilteredColor * (F * brdf.x + brdf.y);
        ambient = (KD * diffuseIBL + specularIBL) * material.ao; // 环境光照
    }

    // 多光源
    vec3 Lo = vec3(0.0);

    // 方向光
    for(int i = 0; i < u_dirLightCount; ++i){
        vec3 L = normalize(-u_dirLights[i].direction);
        vec3 H = normalize(V + L);
        float shadow = 1.0;
        if (u_enableShadow) {
            // 直接传递原始阴影坐标
            shadow = SampleDirectionalShadow(u_dirShadowMap, u_dirLights[i].shadowLayer, FragPosDirLightSpace[i],N);
        }
        vec3 direct = PBRLighting(N, V, L, H, u_dirLights[i].color, material.albedo, material.metallic, material.roughness, material.F0);
        Lo += direct * shadow;
    }

    // 点光源
    for(int i = 0; i < u_pointLightCount; ++i) {
        vec3 L = normalize(u_pointLights[i].position - WorldPos);
        float distance = length(u_pointLights[i].position - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 H = normalize(V + L);
        float shadow = 1.0;
        if (u_enableShadow) {
            vec3 fragToLight = WorldPos - u_pointLights[i].position;
            shadow = SamplePointShadow(u_pointShadowMap, u_pointLights[i].shadowLayer, fragToLight);
        }
        vec3 direct = PBRLighting(N, V, L, H, u_pointLights[i].color * attenuation, material.albedo, material.metallic, material.roughness, material.F0);
        Lo += direct * shadow;
    }

    // 聚光灯
    for(int i = 0; i < u_spotLightCount; ++i) {
        vec3 L = normalize(u_spotLights[i].position - WorldPos);
        float distance = length(u_spotLights[i].position - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        float theta = dot(L, normalize(-u_spotLights[i].direction));
        float epsilon = u_spotLights[i].innerCutOff - u_spotLights[i].outerCutOff;
        float intensity = clamp((theta - u_spotLights[i].outerCutOff) / epsilon, 0.0, 1.0);
        vec3 H = normalize(V + L);
        float shadow = 1.0;
        
        if (u_enableShadow) {
            shadow = SampleSpotShadow(
            u_spotShadowMap, 
            u_spotLights[i].shadowLayer, // 使用当前聚光灯的阴影层
            FragPosSpotLightSpace[0],    // 使用对应索引的阴影坐标
            N, 
            normalize(-u_spotLights[i].direction) // 确保方向向量单位化
        );
        }
        
        vec3 direct = PBRLighting(N, V, L, H, u_spotLights[i].color * attenuation * intensity, 
                             material.albedo, material.metallic, material.roughness, material.F0);
        Lo += direct * shadow;
    }

    vec3 color = ambient + Lo + material.emission;

    // Alpha 处理
    float alpha = 1.0;
    if (u_materialWorkflow == 0 && u_hasBaseColorTexture) {
        alpha = texture(u_baseColorMap, TexCoords).a;
    } else if (u_materialWorkflow == 1 && u_hasDiffuseTexture) {
        alpha = texture(u_baseColorMap, TexCoords).a;
    }
    
    // Alpha 模式处理
    if (u_alphaMode == 1) { // MASK
        if (alpha < u_alphaCutoff) {
            discard;
        }
        alpha = 1.0;
    }

    // HDR tonemapping & gamma
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    // 修正：添加缺失的 FragColor 赋值
    FragColor = vec4(color, alpha);
    //FragColor = vec4(material.roughness);
}
