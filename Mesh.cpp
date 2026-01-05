#include "Mesh.h"
#include "Material.h"

Mesh::Mesh(const std::vector<Vertex>& vertices,
    const std::vector<unsigned int>& indices,
    const std::vector<Texture>& textures)
    : vertices(vertices), indices(indices), textures(textures)
{
    setupMesh();  // 调用私有初始化函数
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // 顶点数据
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // 索引数据
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // 顶点属性指针
    // 位置属性
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
    // 法线属性
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // 纹理坐标属性
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    // 新增：切线属性（location=3）
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    glBindVertexArray(0);
}

// 4. 修改Draw方法适配新材质系统
void Mesh::Draw(Shader& shader, const Material& material) const {
    // 1. 绑定纹理 - 简化逻辑
    bool hasDiffuse = false;
    bool hasSpecular = false;

    for (unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);

        if (textures[i].type == "texture_diffuse") {
            shader.setInt("material.texture_diffuse", i);
            hasDiffuse = true;
        }
        else if (textures[i].type == "texture_specular") {
            shader.setInt("material.texture_specular", i);
            hasSpecular = true;
        }

        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    // 2. 设置材质参数
    shader.setFloat("material.shininess", material.shininess);

    // 3. 标记纹理存在状态（关键修复）
    shader.setBool("hasDiffuseTexture", hasDiffuse);
    shader.setBool("hasSpecularTexture", hasSpecular);

    // 4. 绘制网格
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::SetupMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
    // 1. 转换顶点数据
    std::vector<Vertex> vertexStructs;
    for (size_t i = 0; i < vertices.size(); i += 11) {
        Vertex v;
        v.Position = glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
        v.Normal = glm::vec3(vertices[i + 3], vertices[i + 4], vertices[i + 5]);
        v.TexCoords = glm::vec2(vertices[i + 6], vertices[i + 7]);
        v.Tangent = glm::vec3(vertices[i + 8], vertices[i + 9], vertices[i + 10]); // 添加切线
        vertexStructs.push_back(v);
    }

    // 2. 设置网格数据
    this->vertices = vertexStructs;
    this->indices = indices;

    // 3. 调用已有的setupMesh()初始化OpenGL对象
    setupMesh();
}