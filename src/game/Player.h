#pragma once

#include <glm/glm.hpp>
#include <algorithm>
#include <vector>
#include <string>
#include "../core/Input.h"
#include "../graphics/ParticleSystem.h"

enum class PlayerState {
    WAITING_START, // 等待开始状态（在第一个平台上�?
    IDLE,       // 静止状�?
    JUMPING,    // 跳跃�?
    SLIDING,    // 滑行�?
    FALLING,    // 下落�?
    CHARGING,   // 蓄力状态（长按时）
    PERFECT_LAND // 完美着陆状�?
};

// 跳跃类型
enum class JumpType {
    NORMAL,     // 普通跳�?
    PERFECT,    // 完美节拍跳跃
    POWER,      // 蓄力跳跃
    COMBO       // 连击跳跃
};

class Player {
public:
    Player();
    ~Player();
    
    bool Initialize();
    void Update(float deltaTime, Input* input);
    void Render(Renderer* renderer);
    void Cleanup();
    
    // 设置粒子系统引用
    void SetParticleSystem(ParticleSystem* particleSystem) { m_particleSystem = particleSystem; }
    
    // 玩家控制 - 增强�?
    void Jump(JumpType type = JumpType::NORMAL);
    void StartSlide();
    void EndSlide();
    void StartCharging();              // 开始蓄�?
    void ReleaseCharge();             // 释放蓄力
    
    // 节拍同步功能
    void OnBeatHit(BeatRating rating); // 节拍命中回调
    void OnBeatMiss();                 // 节拍错过回调
    
    // 物理更新
    void UpdatePhysics(float deltaTime);
    void UpdateAnimation(float deltaTime);
    void UpdateVisualEffects(float deltaTime);
    
    // 碰撞检�?
    bool CheckPlatformCollision(const glm::vec3& platformPos, const glm::vec3& platformSize);    bool CheckSideCollision(const glm::vec3& platformPos, const glm::vec3& platformSize, glm::vec3& pushOut);    void OnLandOnPlatform(const glm::vec3& platformPos, const glm::vec3& platformSize, int platformType, bool awardScore);
    void OnMissPlatform();
    void SetGrounded(bool grounded);
    void SetPlatformVelocity(const glm::vec3& velocity) {
        if (m_currentPlatformType == 2) {
            m_platformVelocity = glm::vec3(0.0f);
            return;
        }
        m_platformVelocity = velocity;
    }
    void SetPlatformCenter(const glm::vec3& center) { m_platformCenter = center; }
    void SetSpeedScale(float scale) { m_speedScale = std::clamp(scale, 0.6f, 2.0f); }
    void SetAllowJumpInput(bool allow) { m_allowJumpInput = allow; }
    void SetRhythmMode(bool enabled) { m_rhythmMode = enabled; }
    
    // 智能跳跃
    void SetNextPlatformPosition(const glm::vec3& nextPlatformPos);
    void ClearNextPlatform() { m_hasNextPlatform = false; }
    void JumpToNextPlatform();
    
    // 获取状�?
    glm::vec3 GetPosition() const { return m_position; }
    glm::vec3 GetVelocity() const { return m_velocity; }
    PlayerState GetState() const { return m_state; }
    bool IsGrounded() const { return m_isGrounded; }
    bool CanJump() const { return m_isGrounded || m_coyoteTimer > 0.0f; }
    int GetScore() const { return m_score; }
    int GetCombo() const { return m_combo; }
    float GetChargeLevel() const { return m_chargeLevel; }
    
    // 计分系统增强
    int GetPerfectHits() const { return m_perfectHits; }
    int GetGreatHits() const { return m_greatHits; }
    int GetGoodHits() const { return m_goodHits; }
    int GetMisses() const { return m_misses; }
    float GetAccuracyPercentage() const;
    
    // 设置状态
    void SetPosition(const glm::vec3& position) { m_position = position; m_lastPosition = position; }
    void Reset(bool resetScore = true);
    void StartGame(); // 开始游戏（从等待状态转换到活动状态）
    
    // 球贴图相关
    void LoadBallTextures(const std::string& ballsFolder);
    void SetBallTexture(int index);
    int GetCurrentTextureIndex() const { return m_currentTextureIndex; }
    const std::vector<std::string>& GetTextureNames() const { return m_textureNames; }
    int GetTextureCount() const { return static_cast<int>(m_textureNames.size()); }

private:
    // Position and movement
    glm::vec3 m_position;
    glm::vec3 m_lastPosition;
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;
    glm::vec2 m_moveInput;
    
    // 物理参数
    float m_gravity;
    float m_jumpForce;
    float m_slideSpeed;
    float m_maxFallSpeed;
    float m_moveSpeed;
    float m_airControl;
    float m_strafeSpeed;
    float m_strafeAcceleration;
    float m_strafeDamping;
    float m_jumpBufferTime;
    float m_jumpBufferTimer;
    float m_coyoteTime;
    float m_coyoteTimer;
    float m_speedBoostTimer;
    float m_speedBoostMultiplier;
    float m_dashTimer;
    float m_dashCooldown;
    float m_dashDuration;
    float m_dashBoost;
    float m_dashImpulse;
    float m_bounceWindowTimer;
    float m_bounceBoostMultiplier;
    float m_speedScale;
    float m_boostCharge;
    float m_chargeMultiplier;         // 蓄力倍数
    bool m_allowJumpInput;
    bool m_rhythmMode;
    
    // 状�?
    PlayerState m_state;
    bool m_isGrounded;
    bool m_isSliding;
    bool m_isCharging;
    float m_chargeLevel;              // 蓄力等级 [0, 1]
    float m_chargeTime;
    
    // 智能跳跃
    glm::vec3 m_nextPlatformPos;
    bool m_hasNextPlatform;
    float m_jumpAngle;
    float m_jumpPower;
    
    // 计分系统
    int m_score;
    int m_combo;
    int m_maxCombo;
    int m_perfectHits;
    int m_greatHits;
    int m_goodHits;
    int m_misses;
    int m_centerStreak;
    int m_currentPlatformType;
    glm::vec3 m_platformVelocity;
    glm::vec3 m_platformCenter;
    
    // 动画参数
    float m_animationTime;
    
    // 视觉效果
    ParticleSystem* m_particleSystem;
    float m_glowIntensity;            // 发光强度
    glm::vec3 m_trailColor;          // 拖尾颜色
    float m_scaleAnimation;           // 缩放动画
    float m_rotationSpeed;            // 旋转速度
    
    // 音效和反馈
    float m_lastBeatAccuracy;
    BeatRating m_lastRating;
    
    float m_squashScale;        // 压缩缩放（按下时）
    float m_bounceScale;        // 弹跳缩放（松开时）
    
    // 渲染数据
    unsigned int m_VAO, m_VBO, m_EBO;
    glm::mat4 m_modelMatrix;
    
    // 球贴图
    std::vector<unsigned int> m_ballTextures;
    std::vector<std::string> m_textureNames;
    int m_currentTextureIndex;
    
    // 滚动旋转
    glm::vec3 m_rollRotation;         // 累积旋转角度（弧度）
    float m_ballRadius;               // 球的半径
    
    // 内部方法
    void CalculateJumpTrajectory();
    void UpdateCharging(float deltaTime);
    void UpdateGlowEffect(float deltaTime);
    void TriggerLandingEffects(int platformType);
    void UpdateScoreMultiplier();
    
    // 粒子效果
    void EmitJumpParticles();
    void EmitLandingParticles();
    void EmitPerfectHitParticles();
    void EmitChargeParticles();
    
    void SetupMesh();
    void UpdateModelMatrix();
    glm::mat4 GetAnimationMatrix() const;
};
