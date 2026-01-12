#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include <sstream>

class Shader {
public:
    unsigned int ID; // 着色器程序ID

    // 构造函数：接受顶点/片段着色器文件路径
    Shader(const char* vertexPath, const char* fragmentPath);

    // 激活着色器程序
    void use() const;

    // uniform工具函数
    void setFloat(const std::string& name, float value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

    void setVec3(const std::string& name, float x, float y, float z) const;
    //void setVec3(const std::string& name, const glm::vec3& value) const; // glm向量支持
    void setVec3(const std::string& name, const glm::vec3& value) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    // 补充的常用方法（后续开发可用）
    void setInt(const std::string& name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setBool(const std::string& name, bool value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }

    void setMat3(const std::string& name, const glm::mat3& mat) const {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    bool isCompiledSuccessfully() const { return m_CompileSuccess; }
    // 检查着色器编译/链接错误
    bool checkCompileErrors(unsigned int shader, const std::string& type);
    
private:


    bool m_CompileSuccess = false; // 状态标志
};
