#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <functional>

enum class CameraMode {
    FOLLOW,     // 跟随玩家
    ORBIT,      // 环绕观察
    FREE        // 自由漫游
};

class Camera {
public:
    Camera();
    
    // 基本设置
    void SetPosition(const glm::vec3& position);
    void SetTarget(const glm::vec3& target);
    void SetUp(const glm::vec3& up);
    
    // 投影设置
    void SetPerspective(float fov, float aspect, float near, float far);
    
    // 相机控制
    void FollowTarget(const glm::vec3& targetPos, float deltaTime);
    void Orbit(const glm::vec3& center, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset);
    
    // 获取矩阵
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;
    
    // 相机模式
    void SetMode(CameraMode mode) { m_mode = mode; }
    CameraMode GetMode() const { return m_mode; }
    
    // Zoom功能
    void ZoomIn(float factor = 0.9f);
    void ZoomOut(float factor = 1.1f);
    void ZoomToFit(const glm::vec3& center, float radius);
    void ZoomByScroll(float scrollDelta);
    
    // Pan功能
    void Pan(float deltaX, float deltaY);
    
    // Orbit功能 - 环绕观察
    void OrbitByMouse(float deltaX, float deltaY, const glm::vec3& center);
    void SetOrbitCenter(const glm::vec3& center) { m_orbitCenter = center; }
    glm::vec3 GetOrbitCenter() const { return m_orbitCenter; }
    float GetOrbitDistance() const { return glm::length(m_position - m_orbitCenter); }
    
    // 自由漫游 - 键盘控制位置，鼠标控制视角
    void MoveForward(float deltaTime);
    void MoveBackward(float deltaTime);
    void MoveLeft(float deltaTime);
    void MoveRight(float deltaTime);
    void MoveUp(float deltaTime);
    void MoveDown(float deltaTime);
    void RotateByMouse(float deltaX, float deltaY);
    void SetMoveSpeed(float speed) { m_movementSpeed = speed; }
    
    // FOV控制
    float GetFOV() const { return m_fov; }
    void SetFOV(float fov) { m_fov = fov; }
    
    // 碰撞检测 - 用于漫游模式
    void SetCollisionRadius(float radius) { m_collisionRadius = radius; }
    float GetCollisionRadius() const { return m_collisionRadius; }
    bool TryMoveForward(float deltaTime, const std::function<bool(const glm::vec3&, float)>& collisionCheck);
    bool TryMoveBackward(float deltaTime, const std::function<bool(const glm::vec3&, float)>& collisionCheck);
    bool TryMoveLeft(float deltaTime, const std::function<bool(const glm::vec3&, float)>& collisionCheck);
    bool TryMoveRight(float deltaTime, const std::function<bool(const glm::vec3&, float)>& collisionCheck);
    bool TryMoveUp(float deltaTime, const std::function<bool(const glm::vec3&, float)>& collisionCheck);
    bool TryMoveDown(float deltaTime, const std::function<bool(const glm::vec3&, float)>& collisionCheck);
    
    // 获取相机属性
    glm::vec3 GetPosition() const { return m_position; }
    glm::vec3 GetFront() const { return m_front; }
    glm::vec3 GetRight() const { return m_right; }
    glm::vec3 GetUp() const { return m_up; }

private:
    // 相机属性
    glm::vec3 m_position;
    glm::vec3 m_target;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;
    
    // 欧拉角
    float m_yaw;
    float m_pitch;
    
    // 相机选项
    float m_movementSpeed;
    float m_mouseSensitivity;
    
    // 投影参数
    float m_fov;
    float m_aspect;
    float m_nearPlane;
    float m_farPlane;
    
    // 跟随参数
    glm::vec3 m_followOffset;
    float m_followSpeed;
    
    // 环绕参数
    glm::vec3 m_orbitCenter;
    float m_orbitYaw;
    float m_orbitPitch;
    
    // 相机模式
    CameraMode m_mode;
    
    // 碰撞检测
    float m_collisionRadius;
    
    // 内部方法
    void UpdateCameraVectors();
};