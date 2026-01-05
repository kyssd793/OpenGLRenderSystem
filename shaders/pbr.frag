#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in mat3 TBN;

// ========== IBL纹理 ==========
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

// ========== 材质纹理 ==========
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D materialMask;

// ========== 材质参数 ==========
uniform float metallic;
uniform float roughness;
uniform float ao;
uniform float normalStrength = 1.0; // 法线贴图强度控制
uniform float aoStrength = 1.0;     // AO强度控制
uniform bool useMaterialMask;       // 是否使用材质遮罩
uniform float velvetRoughness;      // 天鹅绒材质粗糙度
uniform float velvetMetallic;       // 天鹅绒材质金属度

// ========== 光照参数 ==========
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];
uniform vec3 viewPos;

// ========== 调试控制 ==========
uniform int debugMode = 0;

const float PI = 3.14159265359;

// ===================== PBR核心函数 =====================
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// 1. 天鹅绒BRDF分布函数（Charlie分布）
float DistributionVelvet(float roughness, float NoH) {
    float alpha = roughness * roughness;
    float invAlpha = 1.0 / alpha;
    return (2.0 + invAlpha) * pow(1.0 - NoH, invAlpha) / (2.0 * PI);
}

// 2. 逆向菲涅尔函数
vec3 velvetFresnel(vec3 F0, float NoV) {
    float factor = pow(1.0 - abs(NoV), 5.0);
    return mix(F0, vec3(1.0), factor);
}
        // === 新增：天鹅绒参数 ===
uniform bool useVelvet;          // 新增：启用天鹅绒模式
uniform vec3 velvetColor;        // 新增：绒毛基础色
uniform float velvetStrength;    // 新增：绒毛强度
// ===================== 主渲染函数 =====================
void main() {



    // 采样材质贴图
    vec3 albedo = texture(albedoMap, TexCoords).rgb;
    
    // 法线贴图处理
    vec3 tangentNormal = texture(normalMap, TexCoords).rgb * 2.0 - 1.0;
    tangentNormal.xy *= normalStrength;
    tangentNormal = normalize(tangentNormal);
    vec3 normal = normalize(TBN * tangentNormal);
    
    // 材质参数处理
    float metallicVal = texture(metallicMap, TexCoords).r * metallic;
    float roughnessVal = texture(roughnessMap, TexCoords).r * roughness;
    float aoVal = mix(1.0, texture(aoMap, TexCoords).r, aoStrength) * ao;
    
    // 材质遮罩处理
    float maskVal = useMaterialMask ? texture(materialMask, TexCoords).r : 0.0;
    float finalRoughness = mix(roughnessVal, velvetRoughness, maskVal);
    float finalMetallic = mix(metallicVal, velvetMetallic, maskVal);
    
    // IBL计算
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, finalMetallic);
    vec3 V = normalize(viewPos - WorldPos);
    
    // 漫反射部分
    vec3 F = fresnelSchlickRoughness(max(dot(normal, V), 0.0), F0, finalRoughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - finalMetallic;
    vec3 irradiance = texture(irradianceMap, normal).rgb;
    vec3 diffuse = irradiance * albedo;
    
    // 镜面反射部分
    vec3 R = reflect(-V, normal);
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R, finalRoughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(normal, V), 0.0), finalRoughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    
    // 环境光组合
    vec3 ambient = (kD * diffuse + specular) * aoVal;
    
    // 直接光照计算
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; i++) {
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;
        
        // BRDF计算
        float NDF = DistributionGGX(normal, H, finalRoughness);
        float G = GeometrySmith(normal, V, L, finalRoughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - finalMetallic;
        
        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0);
        vec3 brdfSpecular = numerator / max(denominator, 0.001);
        
        // 光源贡献
        float NdotL = max(dot(normal, L), 0.0);
        Lo += (kD * albedo / PI + brdfSpecular) * radiance * NdotL;
    }
        // === 在直接光照循环后添加 ===
    vec3 velvetTerm = vec3(0.0);
    if (useVelvet && maskVal > 0.01) {
        // 计算绒毛方向向量
        vec3 H = normalize(V + normal);
        float NoH = max(dot(normal, H), 0.0);
        
        // 绒毛分布计算
        float D_velvet = DistributionVelvet(finalRoughness, NoH);
        
        // 逆向菲涅尔效应
        float NoV = max(dot(normal, V), 0.001);
        vec3 F_velvet = velvetFresnel(F0, NoV);
        
        // 能量补偿项
        float energyCompensation = 1.0 / (1.0 + finalRoughness);
        
        // 合成绒毛项
        velvetTerm = velvetColor * D_velvet * F_velvet * 
                     velvetStrength * energyCompensation;
    }
    // 最终颜色组合
    vec3 color = ambient + Lo +velvetTerm;
    color = color / (color + vec3(1.0));  // ACES色调映射
    color = pow(color, vec3(1.0/2.2));    // Gamma校正
    
    // 调试视图
    if(debugMode == 1) FragColor = vec4(normal * 0.5 + 0.5, 1.0);
    else if(debugMode == 2) FragColor = vec4(vec3(aoVal), 1.0);
    else if(debugMode == 3) FragColor = vec4(albedo, 1.0);
    else FragColor = vec4(color, 1.0);
}
