#include "camera.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(GLFWwindow* window, glm::vec3 position)
    : m_Window(window), Position(position) {
} // 初始化窗口指针

glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    float verticalVelocity = VerticalSpeed * deltaTime; // 垂直速度

    if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
        Position += Front * velocity;
    if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
        Position -= Front * velocity;
    if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS)
        Position -= glm::normalize(glm::cross(Front, Up)) * velocity;
    if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS)
        Position += glm::normalize(glm::cross(Front, Up)) * velocity;
   
    // QE垂直控制
    if (glfwGetKey(m_Window, GLFW_KEY_E) == GLFW_PRESS)
        Position += glm::vec3(0.0f, 1.0f, 0.0f) * verticalVelocity;
    if (glfwGetKey(m_Window, GLFW_KEY_Q) == GLFW_PRESS)
        Position -= glm::vec3(0.0f, 1.0f, 0.0f) * verticalVelocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset) {
    float sensitivity = 0.1f;
    Yaw += xoffset * sensitivity;
    Pitch += yoffset * sensitivity;

    // 限制俯仰角
    if (Pitch > 89.0f)  Pitch = 89.0f;
    if (Pitch < -89.0f) Pitch = -89.0f;

    updateVectors();
}
// 滚轮缩放处理
void Camera::ProcessMouseScroll(float yoffset) {
    Zoom -= yoffset;
    if (Zoom < 1.0f) Zoom = 1.0f;
    if (Zoom > 90.0f) Zoom = 90.0f;
}

void Camera::updateVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
}