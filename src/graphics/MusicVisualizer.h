#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "../core/Renderer.h"
#include "../audio/AudioAnalyzer.h"

// 视觉效果类型
enum class VisualEffectType {
    SCREEN_FLASH,       // 屏幕闪烁
    COLOR_PULSE,        // 颜色脉冲
    PARTICLE_BURST,     // 粒子爆发
    CAMERA_SHAKE,       // 相机震动
    LIGHT_STROBE,       // 光源频闪
    BACKGROUND_WAVE     // 背景波动
};

// 视觉效果配置
struct VisualEffect {
    VisualEffectType type;
    float intensity;        // 强度 [0, 1]
    float duration;         // 持续时间
    glm::vec3 color;       // 颜色
    float frequency;        // 频率（用于周期性效果）
    bool isActive;
    float timeRemaining;
};

class MusicVisualizer {
public:
    MusicVisualizer();
    ~MusicVisualizer();
    
    bool Initialize();
    void Update(float deltaTime, const BeatInfo& beatInfo);
    void Render(Renderer* renderer);
    void Cleanup();
    
    // 效果触发
    void TriggerBeatEffect(float intensity);
    void TriggerSectionChange(int newSection);
    void TriggerPerfectHit();
    
    // 动态光照
    void UpdateDynamicLighting(Renderer* renderer, const BeatInfo& beatInfo);
    glm::vec3 GetMusicDrivenLightColor(const BeatInfo& beatInfo);
    
    // 后处理效果
    void SetScreenFlash(const glm::vec3& color, float intensity, float duration);
    
    // 背景效果
    void UpdateBackgroundAnimation(const BeatInfo& beatInfo);
    
    // 相机效果
    glm::vec3 GetCameraShakeOffset() const { return m_cameraShakeOffset; }
    void TriggerCameraShake(float intensity, float duration);

private:
    std::vector<VisualEffect> m_activeEffects;
    
    // 屏幕效果
    float m_screenFlashIntensity;
    glm::vec3 m_screenFlashColor;
    float m_colorPulsePhase;
    
    // 相机震动
    glm::vec3 m_cameraShakeOffset;
    float m_shakeIntensity;
    float m_shakeDuration;
    
    // 动态光照
    glm::vec3 m_baseLightColor;
    glm::vec3 m_currentLightColor;
    float m_lightPulsePhase;
    
    // 背景动画
    float m_backgroundWavePhase;
    std::vector<float> m_spectrumHistory;
    
    // 粒子系统引用
    class ParticleSystem* m_particleSystem;
    
    // 内部方法
    void UpdateEffects(float deltaTime);
    void UpdateCameraShake(float deltaTime);
    void UpdateScreenFlash(float deltaTime);
    void ProcessMusicFeatures(const BeatInfo& beatInfo);
    
    // 频率到颜色映射
    glm::vec3 FrequencyToColor(float lowFreq, float midFreq, float highFreq);
    
    // 效果强度计算
    float CalculateEffectIntensity(const BeatInfo& beatInfo, VisualEffectType type);
};