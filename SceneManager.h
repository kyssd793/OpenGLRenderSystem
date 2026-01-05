#pragma once
#include "SceneNode.h"

class SceneManager {
public:
    SceneManager();
    // 几何体类型枚举
    enum class PrimitiveType { PLANE };
    // 场景操作
    SceneNode::Ptr GetRoot() const { return m_RootNode; }
    // 平面生成方法
    SceneNode& CreatePrimitiveNode(const std::string& name, PrimitiveType type);
    unsigned int GenerateWhiteTexture();
    void RenderScene(Shader& shader);

    // 快捷创建方法
    SceneNode::Ptr CreateNode(const std::string& name);
    SceneNode::Ptr CreateModelNode(const std::string& name, const std::string& modelPath);

    std::vector<std::shared_ptr<SceneNode>> nodes;
private:
    SceneNode::Ptr m_RootNode;
};

