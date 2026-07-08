#pragma once

#include <glm/glm.hpp>
#include "../core/Renderer.h"

enum class PlatformType {
    NORMAL,     // 普通平台
    SLIDE,      // 滑轨平台
    MOVING,     // 移动平台
    BOUNCE,     // 弹跳平台
    BOOST,      // Belt (legacy)
    BOOST_LEFT, // Belt left
    BOOST_RIGHT // Belt right
};

class Platform {
public:
    Platform(const glm::vec3& position, const glm::vec3& size, PlatformType type = PlatformType::NORMAL);
    ~Platform();
    
    bool Initialize();
    void Update(float deltaTime);
    void Render(Renderer* renderer);
    void Cleanup();
    
    // 碰撞检测
    bool CheckCollision(const glm::vec3& point, float radius = 0.5f) const;
    glm::vec3 GetSurfacePoint(const glm::vec3& point) const;
    
    // 平台效果
    void OnPlayerLand();
    void OnPlayerLeave();
    
    // 动画效果
    void SetPressed(bool pressed);
    void UpdateAnimation(float deltaTime);
    void ConfigureMovement(const glm::vec3& direction, float speed, float range);
    
    // 获取属性
    glm::vec3 GetPosition() const { return m_position; }
    glm::vec3 GetSize() const { return m_size; }
    glm::vec3 GetCollisionSize() const { return glm::vec3(m_size.x, m_size.y * m_collisionHeightScale, m_size.z); }
    PlatformType GetType() const { return m_type; }
    bool IsActive() const { return m_isActive; }
    glm::vec3 GetVelocity() const { return m_velocity; }
    void SetSpeedMultiplier(float multiplier);
    void SetFixed(bool fixed);

private:
    glm::vec3 m_position;
    glm::vec3 m_originalPosition;
    glm::vec3 m_size;
    PlatformType m_type;
    
    // 状态
    bool m_isActive;
    bool m_isPressed;
    bool m_playerOnPlatform;
    bool m_isFixed;
    
    // 动画参数
    float m_animationTime;
    float m_pressDepth;
    float m_currentDepth;
    
    // 移动平台参数
    glm::vec3 m_moveDirection;
    float m_moveSpeed;
    float m_moveRange;
    float m_speedMultiplier;
    float m_collisionHeightScale;
    
    // 渲染数据
    unsigned int m_VAO, m_VBO, m_EBO;
    Material m_material;

    glm::vec3 m_lastPosition;
    glm::vec3 m_velocity;
    
    void SetupMesh();
    void UpdateModelMatrix();
    glm::mat4 GetModelMatrix() const;
};
