// Material.h
#pragma once
#include <glm/glm.hpp>
#include <glm/detail/type_vec.hpp>

struct Material {
    // 传统phong参数
    float shininess = 32.0f;
    // 公共参数
    float opacity = 1.0f;

    // PBR材质参数
    float metallic = 0.5f;
    float roughness = 0.5f;
    float ao = 1.0f;
    float velvetRoughness = 0.85f;  // 天鹅绒粗糙度
    float velvetMetallic = 0.05f;   // 天鹅绒金属度

    // 材质类型标识
    enum Type { PHONG, PBR } type = PHONG;

    // 材质遮罩支持
    bool useMaterialMask = false;
    // 天鹅绒参数
    glm::vec3 velvetColor = glm::vec3(0.8f, 0.1f, 0.1f); // 默认深红色绒毛
    float velvetStrength = 0.7f;               // 绒毛强度系数
    bool useVelvet = false;                     // 启用天鹅绒特效
    //这里不知道是否要使用先不删除
    // 纹理类型标记
    bool isPBR = false;
};