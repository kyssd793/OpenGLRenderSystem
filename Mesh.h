#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "Shader.h"
#include "Material.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent; // 添加切线属性
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
public:
    // 构造函数参数包含顶点、索引和纹理
    Mesh(const std::vector<Vertex>& vertices,
        const std::vector<unsigned int>& indices,
        const std::vector<Texture>& textures);

    // 修复SetupMesh声明
    void SetupMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);
    void Draw(Shader& shader, const Material& material) const;
    const std::vector<Texture>& GetTextures() const { return textures; }

private:
    unsigned int VAO, VBO, EBO;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    void setupMesh();
};
