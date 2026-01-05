#version 330 core
in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;
uniform float normalStrength = 0.8; // 新增法线强度控制
// ========== 关键修复：先定义结构体，再声明uniform变量 ==========
struct Material {
    sampler2D texture_diffuse;
    sampler2D texture_specular;
    // sampler2D texture_normal1;
    float shininess;
};
// 新增uniform
uniform bool hasDiffuseTexture;
uniform bool hasSpecularTexture;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
};

// ========== 现在声明uniform变量 ==========
uniform sampler2D shadowMap;
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[2];
uniform SpotLight spotLight;
uniform bool enableSoftShadows = true; // 软阴影开关
uniform vec3 viewPos;
uniform bool useColorOnly = false;
uniform vec3 diffuseColor;
uniform float brightness;
out vec4 FragColor;

// ========== 阴影计算函数 ==========
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal) {
    // 透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    vec3 lightDir = normalize(glm::vec3(-0.5f, -1.0f, -0.5f));
    // 完整范围检查
    if(projCoords.z > 1.0 || 
       projCoords.x < 0.0 || projCoords.x > 1.0 || 
       projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 0.0;
    }
    
    // 动态bias减少acne现象
    float bias = max(0.01 * (1.0 - dot(normal, lightDir)), 0.001);//这里前一个0.05，后一个0.005原来是
    
    // 根据开关选择软/硬阴影
    if (enableSoftShadows) {
        // 5x5 PCF采样
        float shadow = 0.0;
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
        for(int x = -2; x <= 2; ++x) {
            for(int y = -2; y <= 2; ++y) {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += (projCoords.z - bias) > pcfDepth ? 1.0 : 0.0;
            }    
        }
        shadow /= 25.0;
        return shadow;
    } else {
        // 硬阴影计算
        float closestDepth = texture(shadowMap, projCoords.xy).r;
        return (projCoords.z - bias) > closestDepth ? 1.0 : 0.0;
    }
}

// ========== 光照计算函数 ==========
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow) {
    vec3 lightDir = normalize(-light.direction);
    
    // 漫反射颜色
    vec3 diffuseColor = hasDiffuseTexture ? 
        texture(material.texture_diffuse, TexCoord).rgb : 
        vec3(0.8, 0.8, 0.8);
    
    // 镜面反射颜色
    vec3 specularColor = hasSpecularTexture ? 
        texture(material.texture_specular, TexCoord).rgb : 
        vec3(0.3);
    
    // 环境光
    vec3 ambient = light.ambient * diffuseColor;
    
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * diffuseColor;
    
    // 镜面光
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularColor;
    
    // 应用阴影
    return ambient + (1.0 - shadow) * (diffuse + specular);
    // vec3 lightDir = normalize(-light.direction);
    
    // // 环境光
    // vec3 ambient = light.ambient * texture(material.texture_diffuse1, TexCoord).rgb; 
    
    // // 漫反射
    // float diff = max(dot(normal, lightDir), 0.0);
    // vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, TexCoord).rgb; 
    
    // // 镜面光
    // vec3 reflectDir = reflect(-lightDir, normal);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // vec3 specular = light.specular * spec * texture(material.texture_specular1, TexCoord).rgb;
    
    // // 应用阴影
    // return ambient + (1.0 - shadow) * (diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    // 距离衰减
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    vec3 lightDir = normalize(light.position - fragPos);
    
    // 环境光
    vec3 ambient = light.ambient * texture(material.texture_diffuse1, TexCoord).rgb;
    
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, TexCoord).rgb;
    
    // 镜面光
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.texture_specular1, TexCoord).rgb;
        // 修复2: 使用新版纹理名称
    vec3 diffuseColor = hasDiffuseTexture ? 
        texture(material.texture_diffuse, TexCoord).rgb : 
        vec3(0.8, 0.8, 0.8);
    
    vec3 specularColor = hasSpecularTexture ? 
        texture(material.texture_specular, TexCoord).rgb : 
        vec3(0.3);
    return (ambient + diffuse + specular) * attenuation;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    // 距离衰减
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // 环境光
    vec3 ambient = light.ambient * texture(material.texture_diffuse1, TexCoord).rgb;
    
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, TexCoord).rgb;
    
    // 镜面光
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.texture_specular1, TexCoord).rgb;
    
    return (ambient + (diffuse + specular) * intensity) * attenuation;
}

// ========== 主函数 ==========
void main() {
    if (useColorOnly) {
        FragColor = vec4(diffuseColor, 1.0);
        return;
    }
    // 直接使用顶点法线
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(-dirLight.direction); // 从片段指向光源
    
    // 计算阴影
    float shadow = ShadowCalculation(FragPosLightSpace, norm);
    
    // 计算各光源的贡献
    vec3 result = CalcDirLight(dirLight, norm, viewDir, shadow); // 平行光

    // 点光源贡献
    for(int i = 0; i < 2; i++) {
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }
    
    // 聚光灯贡献
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

    FragColor = vec4(result * brightness, 1.0);
}
