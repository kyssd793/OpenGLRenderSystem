#include "Shader.h"
#include<iostream>

Shader::Shader(const char* vertexPath, const char* fragmentPath) {

    // 初始化状态标志
    m_CompileSuccess = true;

    // 1. 读取文件内容
    std::string vertexCode, fragmentCode;
    std::ifstream vShaderFile, fShaderFile;


    // 确保ifstream对象能抛出异常
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        // 打开文件 → 读取 → 关闭
        vShaderFile.open(vertexPath);
        std::stringstream vShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        vertexCode = vShaderStream.str();
        vShaderFile.close();

        fShaderFile.open(fragmentPath);
        std::stringstream fShaderStream;
        fShaderStream << fShaderFile.rdbuf();
        fragmentCode = fShaderStream.str();
        fShaderFile.close();
    }
    catch (...) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
        m_CompileSuccess = false;  // 标记失败
        return;  // 直接返回，避免后续操作
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // 2. 编译着色器


    // 顶点着色器
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    if (!checkCompileErrors(vertex, "VERTEX")) {  // 修改检查函数返回值
        m_CompileSuccess = false;
        glDeleteShader(vertex);  // 立即清理资源
    }
    // 片段着色器
    unsigned int  fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    if (!checkCompileErrors(fragment, "FRAGMENT")) {
        m_CompileSuccess = false;
        glDeleteShader(fragment);
    }
    // 如果任一着色器失败则提前返回
    if (!m_CompileSuccess) return;

    // 3. 链接着色器程序
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    if (!checkCompileErrors(ID, "PROGRAM")) {
        m_CompileSuccess = false;
        glDeleteProgram(ID);  // 清理失败的程序
    }

    // 4. 清理资源
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::use() const {
    glUseProgram(ID);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}


//// glm向量版本
//void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
//    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
//}

bool Shader::checkCompileErrors(unsigned int shader, const std::string& type) {
    int success;
    char infoLog[1024];

    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "SHADER_COMPILATION_ERROR: " << type << "\n" // 添加std::
                << infoLog << std::endl;
            return false;
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "PROGRAM_LINKING_ERROR\n" << infoLog << std::endl; // 添加std::
            return false;
        }
    }
    return true;
}
