#pragma once
#include <glad/glad.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class IBL {
public:
    IBL(const std::string& hdrPath);
    ~IBL();

    void BindIrradianceMap(GLenum textureUnit) const;
    void BindPrefilterMap(GLenum textureUnit) const;
    void BindBRDFLUT(GLenum textureUnit) const;

private:
    GLuint m_envCubemap;      // 环境立方体贴图
    GLuint m_irradianceMap;    // 漫反射辐照度贴图
    GLuint m_prefilterMap;     // 镜面反射预滤波贴图
    GLuint m_brdfLUT;          // BRDF查找纹理
    GLuint m_captureFBO;       // 帧缓冲对象
    GLuint m_captureRBO;       // 渲染缓冲对象

    // 使用vector存储视图矩阵（原数组会导致初始化问题）
    std::vector<glm::mat4> captureViews;
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

    // 辅助渲染方法
    void RenderCube();
    void RenderQuad();

    // IBL预计算核心流程
    void PrecomputeIrradianceMap();
    void PrecomputePrefilterMap();
    void PrecomputeBRDFLUT();
};
