#include "SceneManager.h"
#include "Mesh.h"
#include <memory>

SceneManager::SceneManager() {
    m_RootNode = std::make_shared<SceneNode>("Root");
}

void SceneManager::RenderScene(Shader& shader) {
    m_RootNode->Draw(shader);
}

SceneNode::Ptr SceneManager::CreateNode(const std::string& name) {
    auto node = std::make_shared<SceneNode>(name);
    m_RootNode->AddChild(node);
    return node;
}

SceneNode::Ptr SceneManager::CreateModelNode(const std::string& name, const std::string& modelPath) {
    auto node = CreateNode(name);
    auto model = std::make_shared<Model>(modelPath.c_str());
    node->AttachModel(model);
    return node;
}
SceneNode& SceneManager::CreatePrimitiveNode(const std::string& name, PrimitiveType type) {
    auto node = std::make_shared<SceneNode>(name);
    nodes.push_back(node); // 添加到节点列表
    m_RootNode->AddChild(node); // 添加到场景树

    if (type == PrimitiveType::PLANE) {
        // 顶点数据（位置、法线、纹理坐标）
        std::vector<Vertex> planeVertices = {
            {glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
            {glm::vec3(1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
            {glm::vec3(1.0f, 0.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
            {glm::vec3(-1.0f, 0.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)}
        };
        std::vector<Texture> planeTextures;
        Texture whiteTex;
        whiteTex.id = GenerateWhiteTexture();
        whiteTex.type = "texture_diffuse";
        planeTextures.push_back(whiteTex);
        

        std::vector<unsigned int> planeIndices = { 0, 1, 2, 0, 2, 3 };
        //std::vector<Texture> planeTextures; // 空纹理列表（基础几何体通常不需要纹理）

        // 创建网格对象（使用三参数构造函数）
        Mesh planeMesh(planeVertices, planeIndices, planeTextures);
        node->AddMesh(planeMesh);

    }

    return *node; // 返回引用
}

// 生成纯白纹理
unsigned int SceneManager::GenerateWhiteTexture() {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // 1x1 纯白色像素
    unsigned char data[] = { 255, 255, 255, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}