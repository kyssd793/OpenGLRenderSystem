#include "SceneNode.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

SceneNode::SceneNode(const std::string& name)
    : m_Name(name),
    m_Position(0.0f),
    m_Rotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
    m_Scale(1.0f) {
}

void SceneNode::AddChild(Ptr child) {
    m_Children.push_back(child);
}

void SceneNode::RemoveChild(Ptr child) {
    auto it = std::remove(m_Children.begin(), m_Children.end(), child);
    m_Children.erase(it, m_Children.end());
}

void SceneNode::SetPosition(const glm::vec3& position) {
    m_Position = position;
}

void SceneNode::SetRotation(float angle, const glm::vec3& axis) {
    m_Rotation = glm::angleAxis(glm::radians(angle), glm::normalize(axis));
}

void SceneNode::SetScale(const glm::vec3& scale) {
    m_Scale = scale;
}

void SceneNode::AttachModel(std::shared_ptr<Model> model) {
    m_Model = model;
}

void SceneNode::AddMesh(const Mesh& mesh) {
    m_Meshes.push_back(mesh);
}

void SceneNode::UpdateTransform(const glm::mat4& parentTransform) {
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_Position);
    glm::mat4 rotation = glm::mat4_cast(m_Rotation);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), m_Scale);

    m_LocalTransform = translation * rotation * scale;
    m_WorldTransform = parentTransform * m_LocalTransform;
}

void SceneNode::Draw(Shader& shader, const glm::mat4& parentTransform) {
    // 1. 更新当前节点变换
    UpdateTransform(parentTransform);

    // 2. 设置材质参数到着色器
    shader.setFloat("material.shininess", m_Material.shininess);

    // 3. 绘制当前节点
    if (m_Model) {
        shader.setMat4("model", m_WorldTransform);
        m_Model->Draw(shader, m_Material);
    }
    else if (!m_Meshes.empty()) {
        shader.setMat4("model", m_WorldTransform);
        for (auto& mesh : m_Meshes) {
            mesh.Draw(shader, m_Material);
        }
    }

    // 4. 递归绘制子节点
    for (auto& child : m_Children) {
        child->Draw(shader, m_WorldTransform);
    }
}

// 成员访问方法
Material& SceneNode::GetMaterial() {
    return m_Material;
}

glm::mat4 SceneNode::GetWorldTransform() const {
    return m_WorldTransform;
}

glm::mat4 SceneNode::GetLocalTransform() const {
    return m_LocalTransform;
}
