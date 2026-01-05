#pragma once
#include <glm/glm.hpp>
#include <GLFW/glfw3.h> // 新增头文件

class Camera {
public:
    Camera(GLFWwindow* window, glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f)); // 修改构造函
    void ProcessMouseScroll(float yoffset); // 新增
    float Zoom = 45.0f;                    // 新增缩放参数
    glm::mat4 GetViewMatrix();
    void ProcessKeyboard(float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset);

    // 摄像机参数
    glm::vec3 Position;
    glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
    float MovementSpeed = 2.5f;
    float VerticalSpeed = 3.0f; // 新增垂直速度控制

private:
    GLFWwindow* m_Window; // 新增窗口指针成员
    void updateVectors();
    float Yaw = -90.0f;    // 偏航角
    float Pitch = 0.0f;    // 俯仰角
};