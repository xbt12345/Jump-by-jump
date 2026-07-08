#include "Camera.h"
#include <algorithm>

Camera::Camera() 
    : m_position(0.0f, 0.0f, 3.0f)
    , m_target(0.0f, 0.0f, 0.0f)
    , m_worldUp(0.0f, 1.0f, 0.0f)
    , m_yaw(-90.0f)
    , m_pitch(0.0f)
    , m_movementSpeed(2.5f)
    , m_mouseSensitivity(0.1f)
    , m_fov(45.0f)
    , m_aspect(16.0f / 9.0f)
    , m_nearPlane(0.1f)
    , m_farPlane(100.0f)
    , m_followOffset(-8.0f, 6.0f, 0.0f)
    , m_followSpeed(3.0f)
    , m_orbitCenter(0.0f, 1.0f, 0.0f)
    , m_orbitYaw(-90.0f)
    , m_orbitPitch(30.0f)
    , m_mode(CameraMode::FOLLOW)
    , m_collisionRadius(0.5f) {
    
    UpdateCameraVectors();
}

void Camera::SetPosition(const glm::vec3& position) {
    m_position = position;
    UpdateCameraVectors();
}

void Camera::SetTarget(const glm::vec3& target) {
    m_target = target;
    m_front = glm::normalize(target - m_position);
    
    // 从 front 向量计算 yaw 和 pitch
    m_pitch = glm::degrees(asin(m_front.y));
    m_yaw = glm::degrees(atan2(m_front.z, m_front.x));
    
    // 重新计算相机向量（基于新的 yaw/pitch）
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

void Camera::SetUp(const glm::vec3& up) {
    m_worldUp = up;
    UpdateCameraVectors();
}

void Camera::SetPerspective(float fov, float aspect, float near, float far) {
    m_fov = fov;
    m_aspect = aspect;
    m_nearPlane = near;
    m_farPlane = far;
}

void Camera::FollowTarget(const glm::vec3& targetPos, float deltaTime) {
    if (m_mode != CameraMode::FOLLOW) return;
    
    // 相机位置：在小球后面和上方
    glm::vec3 desiredPosition = targetPos + m_followOffset;
    
    // 使用更平滑的插值更新相机位置
    float smoothFactor = glm::clamp(m_followSpeed * deltaTime, 0.0f, 1.0f);
    m_position = glm::mix(m_position, desiredPosition, smoothFactor);
    
    // 始终让相机看向小球的中心
    glm::vec3 directionToTarget = glm::normalize(targetPos - m_position);
    m_front = glm::mix(m_front, directionToTarget, smoothFactor);
    
    // 重新计算相机向量
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

void Camera::Orbit(const glm::vec3& center, float deltaTime) {
    // 环绕逻辑实现
    float radius = glm::length(m_position - center);
    m_yaw += 30.0f * deltaTime; // 每秒30度
    
    m_position.x = center.x + radius * cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_position.y = center.y + radius * sin(glm::radians(m_pitch));
    m_position.z = center.z + radius * sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    
    m_front = glm::normalize(center - m_position);
    UpdateCameraVectors();
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset) {
    xoffset *= m_mouseSensitivity;
    yoffset *= m_mouseSensitivity;

    m_yaw += xoffset;
    m_pitch += yoffset;

    // 限制俯仰角
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

    UpdateCameraVectors();
}


void Camera::ZoomIn(float factor) {
    m_followOffset *= factor;
}

void Camera::ZoomOut(float factor) {
    m_followOffset *= factor;
}

void Camera::ZoomToFit(const glm::vec3& center, float radius) {
    float distance = radius / tan(glm::radians(m_fov * 0.5f));
    m_position = center - m_front * distance;
    m_orbitCenter = center;
}

void Camera::ZoomByScroll(float scrollDelta) {
    // 向前或向后移动相机
    float zoomSpeed = 2.0f;
    glm::vec3 direction = glm::normalize(m_orbitCenter - m_position);
    float currentDist = glm::length(m_position - m_orbitCenter);
    float newDist = currentDist - scrollDelta * zoomSpeed;
    newDist = std::clamp(newDist, 2.0f, 50.0f); // 限制距离范围
    m_position = m_orbitCenter - direction * newDist;
    m_front = direction;
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

void Camera::Pan(float deltaX, float deltaY) {
    float panSpeed = 0.01f * glm::length(m_position - m_orbitCenter);
    glm::vec3 offset = m_right * (-deltaX * panSpeed) + m_up * (deltaY * panSpeed);
    m_position += offset;
    m_orbitCenter += offset;
}

void Camera::OrbitByMouse(float deltaX, float deltaY, const glm::vec3& center) {
    m_orbitCenter = center;
    float sensitivity = 0.3f;
    
    m_orbitYaw += deltaX * sensitivity;
    m_orbitPitch -= deltaY * sensitivity;
    
    // 限制俯仰角
    m_orbitPitch = std::clamp(m_orbitPitch, -85.0f, 85.0f);
    
    // 计算相机位置
    float distance = glm::length(m_position - m_orbitCenter);
    distance = std::max(distance, 2.0f);
    
    float x = distance * cos(glm::radians(m_orbitPitch)) * cos(glm::radians(m_orbitYaw));
    float y = distance * sin(glm::radians(m_orbitPitch));
    float z = distance * cos(glm::radians(m_orbitPitch)) * sin(glm::radians(m_orbitYaw));
    
    m_position = m_orbitCenter + glm::vec3(x, y, z);
    m_front = glm::normalize(m_orbitCenter - m_position);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

// 自由漫游控制方法
void Camera::MoveForward(float deltaTime) {
    m_position += m_front * m_movementSpeed * deltaTime;
}

void Camera::MoveBackward(float deltaTime) {
    m_position -= m_front * m_movementSpeed * deltaTime;
}

void Camera::MoveLeft(float deltaTime) {
    m_position -= m_right * m_movementSpeed * deltaTime;
}

void Camera::MoveRight(float deltaTime) {
    m_position += m_right * m_movementSpeed * deltaTime;
}

void Camera::MoveUp(float deltaTime) {
    m_position += m_worldUp * m_movementSpeed * deltaTime;
}

void Camera::MoveDown(float deltaTime) {
    m_position -= m_worldUp * m_movementSpeed * deltaTime;
}

void Camera::RotateByMouse(float deltaX, float deltaY) {
    float sensitivity = 0.15f;
    m_yaw += deltaX * sensitivity;
    m_pitch -= deltaY * sensitivity;
    
    // 限制俯仰角
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
    
    UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::GetProjectionMatrix() const {
    return glm::perspective(glm::radians(m_fov), m_aspect, m_nearPlane, m_farPlane);
}

void Camera::UpdateCameraVectors() {
    // 计算新的前向量
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);
    
    // 重新计算右向量和上向量
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

// 带碰撞检测的移动方法
bool Camera::TryMoveForward(float deltaTime, const std::function<bool(const glm::vec3&, float)>& collisionCheck) {
    glm::vec3 newPos = m_position + m_front * m_movementSpeed * deltaTime;
    if (!collisionCheck || !collisionCheck(newPos, m_collisionRadius)) {
        m_position = newPos;
        return true;
    }
    return false;
}

bool Camera::TryMoveBackward(float deltaTime, const std::function<bool(const glm::vec3&, float)>& collisionCheck) {
    glm::vec3 newPos = m_position - m_front * m_movementSpeed * deltaTime;
    if (!collisionCheck || !collisionCheck(newPos, m_collisionRadius)) {
        m_position = newPos;
        return true;
    }
    return false;
}

bool Camera::TryMoveLeft(float deltaTime, const std::function<bool(const glm::vec3&, float)>& collisionCheck) {
    glm::vec3 newPos = m_position - m_right * m_movementSpeed * deltaTime;
    if (!collisionCheck || !collisionCheck(newPos, m_collisionRadius)) {
        m_position = newPos;
        return true;
    }
    return false;
}

bool Camera::TryMoveRight(float deltaTime, const std::function<bool(const glm::vec3&, float)>& collisionCheck) {
    glm::vec3 newPos = m_position + m_right * m_movementSpeed * deltaTime;
    if (!collisionCheck || !collisionCheck(newPos, m_collisionRadius)) {
        m_position = newPos;
        return true;
    }
    return false;
}

bool Camera::TryMoveUp(float deltaTime, const std::function<bool(const glm::vec3&, float)>& collisionCheck) {
    glm::vec3 newPos = m_position + m_worldUp * m_movementSpeed * deltaTime;
    if (!collisionCheck || !collisionCheck(newPos, m_collisionRadius)) {
        m_position = newPos;
        return true;
    }
    return false;
}

bool Camera::TryMoveDown(float deltaTime, const std::function<bool(const glm::vec3&, float)>& collisionCheck) {
    glm::vec3 newPos = m_position - m_worldUp * m_movementSpeed * deltaTime;
    if (!collisionCheck || !collisionCheck(newPos, m_collisionRadius)) {
        m_position = newPos;
        return true;
    }
    return false;
}