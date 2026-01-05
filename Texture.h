#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include "stb_image.h"
#include <iostream>

class Texture {
public:
    static unsigned int Load(const char* path) {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true); // 修复OpenGL纹理上下颠倒问题
        unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
            (nrChannels == 4) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);

        //纹理参数配置
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 错误检查
        if (glGetError() != GL_NO_ERROR) {
            std::cerr << "纹理参数设置失败" << std::endl;
        }


        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
        return textureID;
    }
};