#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream> // 添加错误输出

class ShadowMapper {
public:
    GLuint depthMapFBO = 0;
    GLuint depthMap = 0;
    const GLuint SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

    ShadowMapper() {
        // 检查OpenGL上下文
        if (!gladLoadGL()) {
            std::cerr << "ERROR::SHADOWMAPPER: GLAD not initialized!" << std::endl;
            return;
        }

        // 创建帧缓冲
        glGenFramebuffers(1, &depthMapFBO);

        // 创建深度纹理
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
            SHADOW_WIDTH, SHADOW_HEIGHT, 0,
            GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

        // 深度比较模式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        // 绑定帧缓冲
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        // 检查帧缓冲完整性
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "ERROR::SHADOWMAPPER: Framebuffer not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Cleanup() {
        glDeleteFramebuffers(1, &depthMapFBO);
        glDeleteTextures(1, &depthMap);
    }
};
