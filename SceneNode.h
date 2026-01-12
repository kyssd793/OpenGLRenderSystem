#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "Model.h"
// 添加四元数支持头文件 
#include <glm/gtc/quaternion.hpp>  
#include "Material.h"
class SceneNode {
public:
    using Ptr = std::shared_ptr<SceneNode>;

    SceneNode(const std::string& name);

    // 变换方法
    void SetPosition(const glm::vec3& position);
    void SetRotation(float angle, const glm::vec3& axis);
    void SetScale(const glm::vec3& scale);

    // 节点关系
    void AddChild(Ptr child);
    void RemoveChild(Ptr child);

    // 渲染组件
    void AttachModel(std::shared_ptr<Model> model);
    void AddMesh(const Mesh& mesh);

    // 渲染方法
    void UpdateTransform(const glm::mat4& parentTransform);
    void Draw(Shader& shader, const glm::mat4& parentTransform = glm::mat4(1.0f));

    // 访问方法
    Material& GetMaterial();
    glm::mat4 GetWorldTransform() const;
    glm::mat4 GetLocalTransform() const;

private:
    std::string m_Name;
    glm::vec3 m_Position;
    glm::quat m_Rotation;
    glm::vec3 m_Scale;
    glm::mat4 m_LocalTransform;
    glm::mat4 m_WorldTransform;

    std::vector<Ptr> m_Children;
    std::shared_ptr<Model> m_Model;
    std::vector<Mesh> m_Meshes;
    Material m_Material;
};
