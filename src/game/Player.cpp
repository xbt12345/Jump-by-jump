#include "Player.h"
#include "Platform.h"
#include "../geometry/Primitives.h"
#include "../graphics/stb_image.h"
#include <cmath>
#include <algorithm>
#include <filesystem>

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Player::Player() 
    : m_position(0.0f, 0.95f, 0.0f)
    , m_lastPosition(0.0f, 0.95f, 0.0f)
    , m_velocity(0.0f)
    , m_acceleration(0.0f)
    , m_moveInput(0.0f)
    , m_gravity(-9.8f)
    , m_jumpForce(5.0f)
    , m_slideSpeed(7.2f)
    , m_maxFallSpeed(-15.0f)
    , m_moveSpeed(3.2f)
    , m_airControl(0.35f)
    , m_strafeSpeed(4.0f)
    , m_strafeAcceleration(12.0f)
    , m_strafeDamping(10.0f)
    , m_jumpBufferTime(0.16f)
    , m_jumpBufferTimer(0.0f)
    , m_coyoteTime(0.12f)
    , m_coyoteTimer(0.0f)
    , m_speedBoostTimer(0.0f)
    , m_speedBoostMultiplier(1.25f)
    , m_dashTimer(0.0f)
    , m_dashCooldown(0.0f)
    , m_dashDuration(0.18f)
    , m_dashBoost(3.6f)
    , m_dashImpulse(0.0f)
    , m_bounceWindowTimer(0.0f)
    , m_bounceBoostMultiplier(1.2f)
    , m_speedScale(1.0f)
    , m_boostCharge(0.0f)
    , m_chargeMultiplier(1.0f)
    , m_allowJumpInput(true)
    , m_rhythmMode(false)
    , m_state(PlayerState::WAITING_START)
    , m_isGrounded(true)
    , m_isSliding(false)
    , m_isCharging(false)
    , m_chargeLevel(0.0f)
    , m_chargeTime(0.0f)
    , m_nextPlatformPos(0.0f)
    , m_hasNextPlatform(false)
    , m_jumpAngle(0.0f)
    , m_jumpPower(0.0f)
    , m_score(0)
    , m_combo(0)
    , m_maxCombo(0)
    , m_perfectHits(0)
    , m_greatHits(0)
    , m_goodHits(0)
    , m_misses(0)
    , m_centerStreak(0)
    , m_currentPlatformType(0)
    , m_platformVelocity(0.0f)
    , m_platformCenter(0.0f)
    , m_animationTime(0.0f)
    , m_particleSystem(nullptr)
    , m_glowIntensity(0.6f)
    , m_trailColor(0.2f, 0.8f, 1.0f)
    , m_scaleAnimation(1.0f)
    , m_rotationSpeed(0.0f)
    , m_lastBeatAccuracy(0.0f)
    , m_lastRating(BeatRating::MISS)
    , m_squashScale(1.0f)
    , m_bounceScale(1.0f)
    , m_VAO(0), m_VBO(0), m_EBO(0)
    , m_modelMatrix(1.0f)
    , m_currentTextureIndex(0)
    , m_rollRotation(0.0f)
    , m_ballRadius(0.5f) {
}

Player::~Player() {
    Cleanup();
}

bool Player::Initialize() {
    SetupMesh();
    Reset();
    std::cout << "Player initialized" << std::endl;
    return true;
}

void Player::Update(float deltaTime, Input* input) {
    m_lastPosition = m_position;
    if (input) {
        // 在等待开始状态时，只响应空格键启动
        if (m_state == PlayerState::WAITING_START) {
            // 在等待状态时不进行其他更新
            m_moveInput = glm::vec2(0.0f);
            UpdateAnimation(deltaTime);
            UpdateModelMatrix();
            return;
        }
        
        if (m_allowJumpInput && input->IsKeyPressed(GLFW_KEY_SPACE)) {
            m_jumpBufferTimer = m_jumpBufferTime;
        }

        float strafeInput = 0.0f;
        if (input->IsKeyDown(GLFW_KEY_A)) {
            strafeInput -= 1.0f;
        }
        if (input->IsKeyDown(GLFW_KEY_D)) {
            strafeInput += 1.0f;
        }
        m_moveInput = glm::vec2(0.0f, std::clamp(strafeInput, -1.0f, 1.0f));

        // W键冲刺 - 在两种模式下都可用
        if (input->IsKeyPressed(GLFW_KEY_W) && m_dashCooldown <= 0.0f) {
            m_dashTimer = m_dashDuration;
            m_dashCooldown = 0.7f;
            m_dashImpulse = m_dashBoost;
        }
    }

    if (m_jumpBufferTimer > 0.0f) {
        m_jumpBufferTimer = std::max(0.0f, m_jumpBufferTimer - deltaTime);
    }
    if (m_isGrounded) {
        m_coyoteTimer = m_coyoteTime;
    } else {
        m_coyoteTimer = std::max(0.0f, m_coyoteTimer - deltaTime);
    }

    if (m_speedBoostTimer > 0.0f) {
        m_speedBoostTimer = std::max(0.0f, m_speedBoostTimer - deltaTime);
    }
    if (m_dashCooldown > 0.0f) {
        m_dashCooldown = std::max(0.0f, m_dashCooldown - deltaTime);
    }
    if (m_dashTimer > 0.0f) {
        m_dashTimer = std::max(0.0f, m_dashTimer - deltaTime);
    }
    if (m_bounceWindowTimer > 0.0f) {
        m_bounceWindowTimer = std::max(0.0f, m_bounceWindowTimer - deltaTime);
    }

    if (m_jumpBufferTimer > 0.0f && m_isGrounded) {
        Jump(JumpType::NORMAL);
        m_jumpBufferTimer = 0.0f;
    }
    
    // 更新物理
    UpdatePhysics(deltaTime);
    
    // 更新动画
    UpdateAnimation(deltaTime);
    
    // 更新视觉效果
    UpdateVisualEffects(deltaTime);
    
    // 更新模型矩阵
    UpdateModelMatrix();
}

void Player::UpdatePhysics(float deltaTime) {
    // 在等待开始状态时不进行物理更新
    if (m_state == PlayerState::WAITING_START) {
        return;
    }
    
    // 应用重力
    if (!m_isGrounded) {
        m_velocity.y += m_gravity * deltaTime;
        
        // 限制最大下落速度
        if (m_velocity.y < m_maxFallSpeed) {
            m_velocity.y = m_maxFallSpeed;
        }
    }
    
    // 滑行时的水平移动（仅在落地时强制速度）
    float forwardSpeed = m_isSliding ? m_slideSpeed : m_moveSpeed;
    if (m_rhythmMode) {
        forwardSpeed *= 0.8f;
    }
    if (m_rhythmMode && m_isSliding) {
        forwardSpeed = m_slideSpeed * 0.6f;
    }
    float strafeSpeed = m_strafeSpeed;
    float accel = m_strafeAcceleration;
    float damping = m_strafeDamping;

    if (m_isGrounded) {
        switch (m_currentPlatformType) {
            case 1: // SLIDE
                forwardSpeed *= 1.05f;
                forwardSpeed += 0.2f;
                strafeSpeed *= 0.9f;
                damping *= 0.55f;
                break;
            case 2: // MOVING
                strafeSpeed *= 0.85f;
                damping *= 1.4f; // 更稳
                break;
            case 3: // BOUNCE
                forwardSpeed *= 1.06f;
                break;
            case 4: // BOOST
            case 5: // BOOST_LEFT
            case 6: // BOOST_RIGHT
                // forward speed unchanged for lateral belt
                strafeSpeed *= 1.4f;
                damping *= 0.6f;
                break;
            default:
                break;
        }
    }

    float scaledStrafe = 1.0f + (m_speedScale - 1.0f) * 0.7f;
    forwardSpeed *= m_speedScale;
    strafeSpeed *= scaledStrafe;

    float beltPush = 0.0f;
    const int beltLegacy = static_cast<int>(PlatformType::BOOST);
    const int beltLeft = static_cast<int>(PlatformType::BOOST_LEFT);
    const int beltRight = static_cast<int>(PlatformType::BOOST_RIGHT);
    bool isBelt = (m_currentPlatformType == beltLegacy ||
                   m_currentPlatformType == beltLeft ||
                   m_currentPlatformType == beltRight);
    if (m_isGrounded && isBelt) {
        m_boostCharge = std::min(1.0f, m_boostCharge + deltaTime * 1.2f);
        float beltDir = 1.0f;
        if (m_currentPlatformType == beltLeft) {
            beltDir = -1.0f;
        } else if (m_currentPlatformType == beltRight) {
            beltDir = 1.0f;
        }
        beltPush = beltDir * (3.0f + m_boostCharge * 2.2f);
    } else {
        m_boostCharge = std::max(0.0f, m_boostCharge - deltaTime * 0.9f);
    }

    if (m_speedBoostTimer > 0.0f) {
        float boostT = std::clamp(m_speedBoostTimer / 0.5f, 0.0f, 1.0f);
        float boostFactor = 1.0f + (m_speedBoostMultiplier - 1.0f) * boostT;
        forwardSpeed *= boostFactor;
        strafeSpeed *= 1.0f + (boostFactor - 1.0f) * 0.4f;
    }

    // 冲刺加速 - 在两种模式下都生效
    if (m_dashTimer > 0.0f) {
        forwardSpeed += m_dashBoost;
    }

    // 应用冲刺脉冲（瞬时加速）
    if (m_dashImpulse > 0.0f) {
        m_velocity.x += m_dashImpulse;
        m_dashImpulse = 0.0f;
    }

    // 判断是否使用智能跳跃轨迹速度
    bool useJumpVelocity = !m_isGrounded && m_hasNextPlatform;
    
    if (useJumpVelocity) {
        // 跳跃中：保持计算好的轨迹速度，只确保不低于基础速度
        if (m_dashTimer > 0.0f) {
            m_velocity.x = std::max(m_velocity.x, forwardSpeed);
        }
    } else if (m_isGrounded) {
        // 在地面上：平滑过渡到目标速度，避免突然加速
        float blendRate = 8.0f * deltaTime; // 平滑过渡速率
        m_velocity.x = glm::mix(m_velocity.x, forwardSpeed, std::min(1.0f, blendRate));
    } else {
        // 不在地面且没有目标平台：保持当前速度或使用基础速度
        m_velocity.x = std::max(m_velocity.x, forwardSpeed * 0.8f);
    }

    float targetZ = m_moveInput.y * strafeSpeed + beltPush;
    float response = (std::abs(m_moveInput.y) > 0.01f) ? accel : damping;
    if (isBelt) {
        response = std::max(response, 16.0f);
    }
    if (!m_isGrounded) {
        response *= m_airControl;
    }
    float blend = std::min(1.0f, response * deltaTime);
    m_velocity.z = glm::mix(m_velocity.z, targetZ, blend);
    
    // 更新位置
    m_position += m_velocity * deltaTime;
    if (m_isGrounded) {
        m_position.x += m_platformVelocity.x * deltaTime;
        m_position.z += m_platformVelocity.z * deltaTime;
    }
    
    // 计算滚动旋转 - 根据移动距离计算旋转角度
    // 旋转角度 = 移动距离 / 球半径（弧度）
    glm::vec3 movement = m_position - m_lastPosition;
    if (glm::length(movement) > 0.0001f) {
        // 前进方向(X轴)的移动导致绕Z轴旋转（反方向）
        float forwardDistance = movement.x;
        m_rollRotation.z -= forwardDistance / m_ballRadius;
        
        // 侧向(Z轴)的移动导致绕X轴旋转
        float sidewaysDistance = movement.z;
        m_rollRotation.x += sidewaysDistance / m_ballRadius;
    }

    // Moving platforms shouldn't drag the player laterally.
    
    if (!m_isGrounded && m_velocity.y < 0.0f) {
        m_state = PlayerState::FALLING;
    }
}

void Player::UpdateAnimation(float deltaTime) {
    m_animationTime += deltaTime;
    
    // 等待开始状态的特殊动画
    if (m_state == PlayerState::WAITING_START) {
        // 轻微的呼吸效果，提示玩家可以开始
        float breathingEffect = sin(m_animationTime * 2.0f) * 0.05f + 1.0f;
        m_bounceScale = breathingEffect;
        
        // 轻微的发光脉冲效果
        m_glowIntensity = 0.5f + sin(m_animationTime * 3.0f) * 0.3f;
        return;
    }
    
    // 按压动画
    if (m_isSliding) {
        m_squashScale = glm::mix(m_squashScale, 0.7f, 5.0f * deltaTime);
    } else {
        m_squashScale = glm::mix(m_squashScale, 1.0f, 8.0f * deltaTime);
    }
    
    // 弹跳动画
    if (m_state == PlayerState::JUMPING) {
        float bouncePhase = sin(m_animationTime * 10.0f) * 0.1f + 1.0f;
        m_bounceScale = bouncePhase;
    } else {
        m_bounceScale = glm::mix(m_bounceScale, 1.0f, 5.0f * deltaTime);
    }
}

void Player::Jump(JumpType type) {
    if (m_isGrounded || m_coyoteTimer > 0.0f) {
        float jumpMultiplier = 1.0f;
        float horizontalScale = 1.0f;
        
        switch (type) {
            case JumpType::PERFECT:
                jumpMultiplier = 1.3f; // 完美跳跃增强30%
                m_glowIntensity = 1.5f; // 增强发光效果
                EmitPerfectHitParticles();
                break;
            case JumpType::POWER:
                jumpMultiplier = 1.0f + m_chargeLevel * 0.8f; // 蓄力跳跃
                EmitChargeParticles();
                break;
            case JumpType::COMBO:
                jumpMultiplier = 1.0f + (m_combo * 0.05f); // 连击加成
                break;
            default:
                jumpMultiplier = 1.0f;
                break;
        }

        if (m_currentPlatformType == 3) { // BOUNCE
            jumpMultiplier *= 1.18f;
            if (m_bounceWindowTimer > 0.0f) {
                jumpMultiplier *= m_bounceBoostMultiplier;
                m_bounceWindowTimer = 0.0f;
            }
            horizontalScale = 1.0f + (jumpMultiplier - 1.0f) * 0.35f;
        }
        
        if (m_hasNextPlatform) {
            JumpToNextPlatform();
            m_velocity.y *= jumpMultiplier;
            m_velocity.x *= horizontalScale;
            m_velocity.z *= horizontalScale;
        } else {
            // 普通跳跃
            m_velocity.y = m_jumpForce * jumpMultiplier;
            float forwardSpeed = m_isSliding ? m_slideSpeed : m_moveSpeed;
            m_velocity.x = forwardSpeed; // 保持前进
        }
        
        m_isGrounded = false;
        m_coyoteTimer = 0.0f;
        m_state = PlayerState::JUMPING;
        m_animationTime = 0.0f;
        
        // 发射跳跃粒子效果
        EmitJumpParticles();
    }
}

void Player::StartCharging() {
    if (m_isGrounded) {
        m_isCharging = true;
        m_chargeLevel = 0.0f;
        m_chargeTime = 0.0f;
        m_state = PlayerState::CHARGING;
    }
}

void Player::ReleaseCharge() {
    if (m_isCharging) {
        m_isCharging = false;
        Jump(JumpType::POWER);
        m_chargeLevel = 0.0f;
    }
}

void Player::OnBeatHit(BeatRating rating) {
    m_lastRating = rating;

    switch (rating) {
        case BeatRating::FLAWLESS:
        case BeatRating::PERFECT:
            m_perfectHits++;
            break;
        case BeatRating::GREAT:
            m_greatHits++;
            break;
        case BeatRating::GOOD:
            m_goodHits++;
            break;
        default:
            break;
    }

    m_glowIntensity = std::min(1.5f, m_glowIntensity + 0.2f);
}

void Player::OnBeatMiss() {
    m_combo = 0;
    m_misses++;
    m_glowIntensity *= 0.5f; // 降低发光效果
}

void Player::UpdateCharging(float deltaTime) {
    if (m_isCharging) {
        m_chargeTime += deltaTime;
        m_chargeLevel = std::min(1.0f, m_chargeTime / 2.0f); // 2秒充满
        
        // 蓄力视觉效果
        m_glowIntensity = 0.5f + m_chargeLevel * 1.0f;
        m_rotationSpeed = m_chargeLevel * 180.0f; // 旋转速度随蓄力增加
        
        // 蓄力粒子效果
        if (m_particleSystem && m_chargeLevel > 0.3f) {
            glm::vec3 particlePos = m_position + glm::vec3(0.0f, 0.5f, 0.0f);
            glm::vec3 chargeColor = glm::mix(glm::vec3(0.2f, 0.8f, 1.0f), 
                                           glm::vec3(1.0f, 0.8f, 0.2f), m_chargeLevel);
            m_particleSystem->Emit(particlePos, glm::vec3(0.0f, 1.0f, 0.0f), 
                                 chargeColor, 1.0f, 0.5f);
        }
    }
}

void Player::UpdateVisualEffects(float deltaTime) {
    // 更新蓄力效果
    UpdateCharging(deltaTime);
    
    // 更新发光效果
    UpdateGlowEffect(deltaTime);
    
    // 更新拖尾颜色
    if (m_combo > 0) {
        float comboIntensity = std::min(1.0f, m_combo / 10.0f);
        m_trailColor = glm::mix(glm::vec3(0.2f, 0.8f, 1.0f), 
                               glm::vec3(1.0f, 0.2f, 0.8f), comboIntensity);
    } else {
        m_trailColor = glm::vec3(0.2f, 0.8f, 1.0f);
    }
}

void Player::UpdateGlowEffect(float deltaTime) {
    // 发光强度自然衰减
    m_glowIntensity = std::max(0.2f, m_glowIntensity - deltaTime * 0.5f);
    
    // 连击时的脉冲效果
    if (m_combo > 0) {
        float pulseEffect = sin(glfwGetTime() * 8.0f) * 0.2f + 0.8f;
        m_glowIntensity *= pulseEffect;
    }
}

void Player::EmitJumpParticles() {
    if (m_particleSystem) {
        glm::vec3 particlePos = m_position;
        particlePos.y -= 0.5f; // 从玩家脚下发射
        m_particleSystem->EmitBurst(particlePos, 8, glm::vec3(0.2f, 0.8f, 1.0f), 3.0f, 1.0f);
    }
}

void Player::EmitLandingParticles() {
    if (m_particleSystem) {
        glm::vec3 particlePos = m_position;
        particlePos.y -= 0.5f;
        m_particleSystem->EmitBurst(particlePos, 12, glm::vec3(1.0f, 0.8f, 0.2f), 2.0f, 1.5f);
    }
}

void Player::EmitPerfectHitParticles() {
    if (m_particleSystem) {
        glm::vec3 particlePos = m_position;
        glm::vec3 perfectColor = glm::vec3(1.0f, 0.2f, 0.8f); // 粉色完美特效
        m_particleSystem->EmitBurst(particlePos, 15, perfectColor, 4.0f, 2.0f);
    }
}

void Player::EmitChargeParticles() {
    if (m_particleSystem) {
        glm::vec3 particlePos = m_position;
        glm::vec3 chargeColor = glm::vec3(1.0f, 0.8f, 0.2f); // 金色蓄力特效
        m_particleSystem->EmitBurst(particlePos, 20, chargeColor, 5.0f, 2.5f);
    }
}

float Player::GetAccuracyPercentage() const {
    int totalHits = m_perfectHits + m_greatHits + m_goodHits + m_misses;
    if (totalHits == 0) return 100.0f;
    
    int successfulHits = m_perfectHits + m_greatHits + m_goodHits;
    return (float)successfulHits / totalHits * 100.0f;
}

void Player::UpdateScoreMultiplier() {
    // 根据连击数更新分数倍数（在UI中显示）
    // 这里可以添加更复杂的分数计算逻辑
}

void Player::TriggerLandingEffects(int platformType) {
    EmitLandingParticles();
    
    // 根据平台类型触发不同效果
    switch (platformType) {
        case 3: // PlatformType::BOUNCE
            m_glowIntensity += 0.5f;
            break;
        case 1: // PlatformType::SLIDE
            m_trailColor = glm::vec3(1.0f, 0.4f, 0.2f);
            break;
        case 4: // PlatformType::BOOST
        case 5: // PlatformType::BOOST_LEFT
        case 6: // PlatformType::BOOST_RIGHT
            m_trailColor = glm::vec3(1.0f, 0.7f, 0.2f);
            break;
        case 2: // PlatformType::MOVING
            m_trailColor = glm::vec3(0.2f, 0.9f, 0.6f);
            break;
        default:
            break;
    }
}

void Player::StartSlide() {
    if (m_isGrounded) {
        m_isSliding = true;
        m_state = PlayerState::SLIDING;
    }
}

void Player::EndSlide() {
    if (m_isSliding) {
        m_isSliding = false;
        Jump(); // 滑行结束时跳跃
    }
}

void Player::Render(Renderer* renderer) {
    if (renderer && m_VAO != 0) {
        // 创建玩家材质
        Material playerMaterial;
        playerMaterial.ambient = glm::vec3(1.0f, 1.0f, 1.0f);  // 使用白色让贴图颜色正确显示
        playerMaterial.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
        playerMaterial.specular = glm::vec3(0.5f, 0.5f, 0.5f);
        playerMaterial.shininess = 32.0f;
        
        // 如果有贴图则使用贴图
        if (!m_ballTextures.empty() && m_currentTextureIndex >= 0 && 
            m_currentTextureIndex < static_cast<int>(m_ballTextures.size())) {
            playerMaterial.useTexture = true;
            playerMaterial.diffuseTexture = m_ballTextures[m_currentTextureIndex];
            playerMaterial.specularTexture = 0;  // 不使用镜面贴图
        } else {
            // 没有贴图时使用默认蓝色
            playerMaterial.ambient = glm::vec3(0.2f, 0.6f, 1.0f);
            playerMaterial.diffuse = glm::vec3(0.3f, 0.8f, 1.0f);
            playerMaterial.specular = glm::vec3(0.8f, 0.8f, 0.8f);
            playerMaterial.shininess = 64.0f;
            playerMaterial.useTexture = false;
        }
        
        // 使用渲染器绘制
        int segments = 16;
        int indexCount = segments * segments * 6; // 每个四边形2个三角形，每个三角形3个顶点
        renderer->DrawMesh(m_VAO, m_modelMatrix, playerMaterial, indexCount, true, false, 0); // 玩家不需要平台类型
    }
}

void Player::Reset(bool resetScore) {
    m_position = glm::vec3(0.0f, 0.95f, 0.0f);
    m_lastPosition = m_position;
    m_velocity = glm::vec3(0.0f);
    m_acceleration = glm::vec3(0.0f);
    m_moveInput = glm::vec2(0.0f);
    m_state = PlayerState::WAITING_START;
    m_isGrounded = true;
    m_isSliding = false;
    m_animationTime = 0.0f;
    m_squashScale = 1.0f;
    m_bounceScale = 1.0f;
    m_jumpBufferTimer = 0.0f;
    m_coyoteTimer = 0.0f;
    m_speedBoostTimer = 0.0f;
    m_bounceWindowTimer = 0.0f;
    m_speedScale = 1.0f;
    m_boostCharge = 0.0f;
    m_dashTimer = 0.0f;
    m_dashCooldown = 0.0f;
    m_dashImpulse = 0.0f;
    m_nextPlatformPos = glm::vec3(0.0f);
    m_hasNextPlatform = false;
    m_rollRotation = glm::vec3(0.0f);  // 重置滚动旋转
    if (resetScore) {
        m_score = 0;
    }
    // 保留分数时，只重置连击
    m_combo = 0;
    m_centerStreak = 0;
    m_currentPlatformType = 0;
    m_platformVelocity = glm::vec3(0.0f);
    m_platformCenter = m_position;
}

void Player::StartGame() {
    if (m_state == PlayerState::WAITING_START) {
        m_state = PlayerState::IDLE;
        // 可以在这里添加开始游戏的特效
        EmitJumpParticles();
    }
}

void Player::SetupMesh() {
    // 生成球体网格数据
    auto vertices = Primitives::GenerateSphere(0.5f, 16);
    auto indices = Primitives::GetSphereIndices(16);
    
    // 创建VAO, VBO, EBO
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
    
    glBindVertexArray(m_VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 法向量属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(1);
    
    // 纹理坐标属性
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
}

void Player::UpdateModelMatrix() {
    m_modelMatrix = glm::mat4(1.0f);
    m_modelMatrix = glm::translate(m_modelMatrix, m_position);
    
    // 应用滚动旋转 - 先绕X轴，再绕Z轴
    m_modelMatrix = glm::rotate(m_modelMatrix, m_rollRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    m_modelMatrix = glm::rotate(m_modelMatrix, m_rollRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    
    // 应用动画变换（压缩/弹跳效果）
    glm::mat4 animMatrix = GetAnimationMatrix();
    m_modelMatrix = m_modelMatrix * animMatrix;
}

glm::mat4 Player::GetAnimationMatrix() const {
    glm::mat4 matrix(1.0f);
    
    // 压缩效果（Y轴压缩，XZ轴补偿放大）
    float scaleY = m_squashScale;
    float scaleXZ = sqrt(1.0f / scaleY); // 保持体积
    
    matrix = glm::scale(matrix, glm::vec3(scaleXZ * m_bounceScale, scaleY, scaleXZ * m_bounceScale));
    
    return matrix;
}

bool Player::CheckPlatformCollision(const glm::vec3& platformPos, const glm::vec3& platformSize) {
    const float radius = 0.5f;
    const glm::vec3 halfSize = platformSize * 0.5f;

    const float dx = std::max(std::abs(m_position.x - platformPos.x) - halfSize.x, 0.0f);
    const float dz = std::max(std::abs(m_position.z - platformPos.z) - halfSize.z, 0.0f);
    const float horizontalLimit = m_isGrounded ? radius * 0.85f : radius * 0.65f;
    if ((dx * dx + dz * dz) > (horizontalLimit * horizontalLimit)) {
        return false;
    }

    const float platformTop = platformPos.y + halfSize.y;
    const float bottom = m_position.y - radius;
    const float lastBottom = m_lastPosition.y - radius;

    if (m_velocity.y > 0.1f) {
        return false;
    }
    const float verticalTolerance = m_isGrounded ? 0.22f : 0.06f;
    if (bottom > platformTop + verticalTolerance) {
        return false;
    }
    const float belowTolerance = m_isGrounded ? radius * 0.6f : radius * 0.5f;
    if (bottom < platformTop - belowTolerance) {
        if (lastBottom < platformTop - 0.02f) {
            return false;
        }
    }

    return true;
}

bool Player::CheckSideCollision(const glm::vec3& platformPos, const glm::vec3& platformSize, glm::vec3& pushOut) {
    // 侧面碰撞检测：检查小球是否与平台侧面发生碰撞
    const float radius = 0.5f;
    const glm::vec3 halfSize = platformSize * 0.5f;
    
    // 计算小球中心到平台最近点的距离
    float closestX = std::clamp(m_position.x, platformPos.x - halfSize.x, platformPos.x + halfSize.x);
    float closestY = std::clamp(m_position.y, platformPos.y - halfSize.y, platformPos.y + halfSize.y);
    float closestZ = std::clamp(m_position.z, platformPos.z - halfSize.z, platformPos.z + halfSize.z);
    
    float dx = m_position.x - closestX;
    float dy = m_position.y - closestY;
    float dz = m_position.z - closestZ;
    float distSq = dx * dx + dy * dy + dz * dz;
    
    // 如果距离小于半径，发生碰撞
    if (distSq < radius * radius && distSq > 0.0001f) {
        float dist = std::sqrt(distSq);
        float penetration = radius - dist;
        
        // 计算推开方向（从最近点指向小球中心）
        glm::vec3 normal = glm::vec3(dx, dy, dz) / dist;
        
        // 只处理侧面碰撞（不处理顶部碰撞，顶部由CheckPlatformCollision处理）
        float platformTop = platformPos.y + halfSize.y;
        if (m_position.y - radius < platformTop - 0.05f) {
            // 小球在平台侧面或下方，需要推开
            pushOut = normal * (penetration + 0.02f);
            return true;
        }
    }
    
    pushOut = glm::vec3(0.0f);
    return false;
}

void Player::OnLandOnPlatform(const glm::vec3& platformPos, const glm::vec3& platformSize, int platformType, bool awardScore) {
    // 着陆在平台上
    float radius = 0.5f;
    float platformTop = platformPos.y + platformSize.y * 0.5f;
    m_position.y = platformTop + radius + 0.01f;
    m_velocity.y = 0.0f;
    m_isGrounded = true;
    m_currentPlatformType = platformType;
    m_isSliding = (platformType == 1);
    m_platformCenter = platformPos;
    
    if (m_state == PlayerState::JUMPING || m_state == PlayerState::FALLING) {
        m_state = PlayerState::IDLE;

        if (platformType == 3) {
            m_bounceWindowTimer = 0.25f;
        }

        if (awardScore) {
            float dx = m_position.x - platformPos.x;
            float dz = m_position.z - platformPos.z;
            float centerX = platformSize.x * 0.2f;
            float centerZ = platformSize.z * 0.2f;
            bool isCenter = std::abs(dx) <= centerX && std::abs(dz) <= centerZ;

            if (isCenter) {
                m_centerStreak++;
                m_score += m_centerStreak * 2;
                float boostTime = 0.3f + 0.05f * static_cast<float>(std::min(m_centerStreak, 6));
                m_speedBoostTimer = std::max(m_speedBoostTimer, boostTime);
            } else {
                m_centerStreak = 0;
                m_score += 1;
            }
        } else {
            m_centerStreak = 0;
        }

        // 触发着陆效果
        TriggerLandingEffects(platformType);
    }
}

void Player::OnMissPlatform() {
    // 错过平台，重置连击但保留分数
    m_combo = 0;
    Reset(false);
}

void Player::SetGrounded(bool grounded) {
    m_isGrounded = grounded;
    if (!grounded) {
        m_currentPlatformType = 0;
        m_platformVelocity = glm::vec3(0.0f);
        m_platformCenter = m_position;
        m_isSliding = false;
        m_bounceWindowTimer = 0.0f;
        m_boostCharge = 0.0f;
    }
    if (grounded && (m_state == PlayerState::JUMPING || m_state == PlayerState::FALLING)) {
        m_state = PlayerState::IDLE;
        m_velocity.y = 0.0f;
    }
}

void Player::SetNextPlatformPosition(const glm::vec3& nextPlatformPos) {
    m_nextPlatformPos = nextPlatformPos;
    m_hasNextPlatform = true;
}

void Player::JumpToNextPlatform() {
    if (!m_hasNextPlatform) return;

    // 计算跳跃到下一个平台的方向
    glm::vec3 direction = m_nextPlatformPos - m_position;
    float horizontalDistance = sqrt(direction.x * direction.x + direction.z * direction.z);
    float verticalDistance = direction.y;

    if (horizontalDistance < 0.1f) {
        m_velocity.x = 0.0f;
        m_velocity.z = 0.0f;
        m_velocity.y = std::max(m_jumpForce, verticalDistance + 1.0f);
        return;
    }

    float gravity = -m_gravity;
    float maxHorizontalSpeed = m_rhythmMode ? 10.0f : 8.0f;

    float desiredPeak = std::max(1.0f, verticalDistance + 1.5f);
    float initialVy = std::sqrt(2.0f * gravity * desiredPeak);
    float timeUp = initialVy / gravity;
    float timeDown = std::sqrt(std::max(0.0f, 2.0f * (desiredPeak - verticalDistance) / gravity));
    float timeToTarget = timeUp + timeDown;

    float minTime = horizontalDistance / maxHorizontalSpeed;
    if (timeToTarget < minTime) {
        timeToTarget = minTime;
    }

    float horizontalSpeed = horizontalDistance / timeToTarget;
    float verticalSpeed = (verticalDistance + 0.5f * gravity * timeToTarget * timeToTarget) / timeToTarget;

    m_velocity.x = horizontalSpeed * (direction.x / horizontalDistance);
    m_velocity.z = horizontalSpeed * (direction.z / horizontalDistance);
    m_velocity.y = verticalSpeed;
}

void Player::Cleanup() {
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_EBO);
        m_VAO = m_VBO = m_EBO = 0;
    }
    // 清理球贴图
    for (unsigned int texId : m_ballTextures) {
        if (texId != 0) {
            glDeleteTextures(1, &texId);
        }
    }
    m_ballTextures.clear();
    m_textureNames.clear();
}

// 加载球贴图
void Player::LoadBallTextures(const std::string& ballsFolder) {
    namespace fs = std::filesystem;
    
    m_ballTextures.clear();
    m_textureNames.clear();
    
    // 检查目录是否存在
    if (!fs::exists(ballsFolder)) {
        std::cout << "Ball textures folder not found: " << ballsFolder << std::endl;
        return;
    }
    
    try {
        for (const auto& entry : fs::directory_iterator(ballsFolder)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                // 转换为小写
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                
                if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
                    std::string filePath = entry.path().string();
                    std::string fileName = entry.path().stem().string();
                    
                    // 加载贴图
                    int width, height, channels;
                    stbi_set_flip_vertically_on_load(true);
                    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 4);
                    
                    if (data) {
                        unsigned int texId = 0;
                        glGenTextures(1, &texId);
                        glBindTexture(GL_TEXTURE_2D, texId);
                        
                        // 使用简单的线性过滤，不使用 mipmap
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                        
                        stbi_image_free(data);
                        
                        m_ballTextures.push_back(texId);
                        m_textureNames.push_back(fileName);
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cout << "Error loading ball textures: " << e.what() << std::endl;
    }
    
    // 随机选择一个初始贴图
    if (!m_ballTextures.empty()) {
        m_currentTextureIndex = rand() % static_cast<int>(m_ballTextures.size());
        std::cout << "Loaded " << m_ballTextures.size() << " ball textures" << std::endl;
    }
}

void Player::SetBallTexture(int index) {
    if (index >= 0 && index < static_cast<int>(m_ballTextures.size())) {
        m_currentTextureIndex = index;
    }
}