#include "MusicVisualizer.h"
#include <iostream>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

MusicVisualizer::MusicVisualizer()
    : m_screenFlashIntensity(0.0f)
    , m_screenFlashColor(1.0f, 1.0f, 1.0f)
    , m_colorPulsePhase(0.0f)
    , m_cameraShakeOffset(0.0f)
    , m_shakeIntensity(0.0f)
    , m_shakeDuration(0.0f)
    , m_baseLightColor(1.0f, 0.95f, 0.8f)
    , m_currentLightColor(1.0f, 0.95f, 0.8f)
    , m_lightPulsePhase(0.0f)
    , m_backgroundWavePhase(0.0f)
    , m_particleSystem(nullptr) {
}

MusicVisualizer::~MusicVisualizer() {
    Cleanup();
}

bool MusicVisualizer::Initialize() {
    std::cout << "MusicVisualizer initialized" << std::endl;
    return true;
}

void MusicVisualizer::Update(float deltaTime, const BeatInfo& beatInfo) {
    // 更新所有视觉效果
    UpdateEffects(deltaTime);
    
    // 节拍触发效果
    if (beatInfo.isBeat) {
        TriggerBeatEffect(beatInfo.intensity);
    }
    
    // 更新相机震动
    UpdateCameraShake(deltaTime);
    
    // 更新屏幕闪烁
    UpdateScreenFlash(deltaTime);
    
    // 更新背景动画
    UpdateBackgroundAnimation(beatInfo);
}

void MusicVisualizer::Render(Renderer* renderer) {
    // 这里可以渲染背景效果、波形等
    (void)renderer; // 避免未使用参数警告
}

void MusicVisualizer::Cleanup() {
    m_activeEffects.clear();
    m_spectrumHistory.clear();
}

void MusicVisualizer::TriggerBeatEffect(float intensity) {
    // 屏幕闪烁效果
    if (intensity > 0.7f) {
        glm::vec3 flashColor = FrequencyToColor(0.8f, 0.6f, 0.4f); // 暖色调
        SetScreenFlash(flashColor, intensity * 0.3f, 0.1f);
    }
    
    // 相机震动
    if (intensity > 0.8f) {
        TriggerCameraShake(intensity * 0.2f, 0.15f);
    }
    
    // 添加脉冲效果
    VisualEffect pulseEffect;
    pulseEffect.type = VisualEffectType::COLOR_PULSE;
    pulseEffect.intensity = intensity;
    pulseEffect.duration = 0.5f;
    pulseEffect.color = FrequencyToColor(0.6f, 0.8f, 1.0f);
    pulseEffect.frequency = 8.0f;
    pulseEffect.isActive = true;
    pulseEffect.timeRemaining = pulseEffect.duration;
    
    m_activeEffects.push_back(pulseEffect);
}

void MusicVisualizer::TriggerSectionChange(int newSection) {
    // 根据音乐段落改变整体色调
    switch (newSection) {
        case 0: // INTRO
            m_baseLightColor = glm::vec3(0.8f, 0.9f, 1.0f); // 冷色调
            break;
        case 1: // VERSE
            m_baseLightColor = glm::vec3(1.0f, 0.95f, 0.8f); // 暖色调
            break;
        case 2: // CHORUS
            m_baseLightColor = glm::vec3(1.0f, 0.8f, 0.6f); // 橙色调
            break;
        case 3: // CLIMAX
            m_baseLightColor = glm::vec3(1.0f, 0.6f, 0.8f); // 粉色调
            TriggerCameraShake(0.3f, 1.0f); // 长时间震动
            break;
        case 4: // OUTRO
            m_baseLightColor = glm::vec3(0.6f, 0.8f, 1.0f); // 蓝色调
            break;
        default:
            break;
    }
}

void MusicVisualizer::TriggerPerfectHit() {
    // 完美命中的特殊效果
    SetScreenFlash(glm::vec3(1.0f, 0.2f, 0.8f), 0.5f, 0.2f); // 粉色闪烁
    TriggerCameraShake(0.1f, 0.1f); // 轻微震动
    
    // 添加粒子爆发效果
    VisualEffect burstEffect;
    burstEffect.type = VisualEffectType::PARTICLE_BURST;
    burstEffect.intensity = 1.0f;
    burstEffect.duration = 1.0f;
    burstEffect.color = glm::vec3(1.0f, 0.2f, 0.8f);
    burstEffect.isActive = true;
    burstEffect.timeRemaining = burstEffect.duration;
    
    m_activeEffects.push_back(burstEffect);
}

void MusicVisualizer::UpdateDynamicLighting(Renderer* renderer, const BeatInfo& beatInfo) {
    // 根据音乐特征更新光照颜色
    m_currentLightColor = GetMusicDrivenLightColor(beatInfo);
    
    // 这里需要调用渲染器的光照更新方法
    (void)renderer; // 避免未使用参数警告
}

glm::vec3 MusicVisualizer::GetMusicDrivenLightColor(const BeatInfo& beatInfo) {
    // 基于节拍特征计算光照颜色
    glm::vec3 spectrumColor = FrequencyToColor(beatInfo.energy, 
                                              beatInfo.intensity, 
                                              beatInfo.energy * beatInfo.intensity);
    
    // 与基础颜色混合
    float mixFactor = 0.3f; // 30%的音乐影响
    return glm::mix(m_baseLightColor, spectrumColor, mixFactor);
}

void MusicVisualizer::SetScreenFlash(const glm::vec3& color, float intensity, float duration) {
    m_screenFlashColor = color;
    m_screenFlashIntensity = intensity;
    
    // 添加闪烁效果到活动效果列表
    VisualEffect flashEffect;
    flashEffect.type = VisualEffectType::SCREEN_FLASH;
    flashEffect.intensity = intensity;
    flashEffect.duration = duration;
    flashEffect.color = color;
    flashEffect.isActive = true;
    flashEffect.timeRemaining = duration;
    
    m_activeEffects.push_back(flashEffect);
}

void MusicVisualizer::TriggerCameraShake(float intensity, float duration) {
    m_shakeIntensity = intensity;
    m_shakeDuration = duration;
}

void MusicVisualizer::UpdateEffects(float deltaTime) {
    // 更新所有活动效果
    for (auto it = m_activeEffects.begin(); it != m_activeEffects.end();) {
        it->timeRemaining -= deltaTime;
        
        if (it->timeRemaining <= 0.0f) {
            it->isActive = false;
            it = m_activeEffects.erase(it);
        } else {
            ++it;
        }
    }
}

void MusicVisualizer::UpdateCameraShake(float deltaTime) {
    if (m_shakeDuration > 0.0f) {
        m_shakeDuration -= deltaTime;
        
        // 生成随机震动偏移
        float shakeX = (rand() % 200 - 100) / 100.0f * m_shakeIntensity;
        float shakeY = (rand() % 200 - 100) / 100.0f * m_shakeIntensity;
        float shakeZ = (rand() % 200 - 100) / 100.0f * m_shakeIntensity * 0.5f;
        
        m_cameraShakeOffset = glm::vec3(shakeX, shakeY, shakeZ);
        
        // 震动强度随时间衰减
        m_shakeIntensity *= 0.95f;
    } else {
        m_cameraShakeOffset = glm::vec3(0.0f);
        m_shakeIntensity = 0.0f;
    }
}

void MusicVisualizer::UpdateScreenFlash(float deltaTime) {
    // 屏幕闪烁强度自然衰减
    if (m_screenFlashIntensity > 0.0f) {
        m_screenFlashIntensity -= deltaTime * 3.0f; // 快速衰减
        m_screenFlashIntensity = std::max(0.0f, m_screenFlashIntensity);
    }
}

void MusicVisualizer::UpdateBackgroundAnimation(const BeatInfo& beatInfo) {
    // 更新背景波形相位
    m_backgroundWavePhase += beatInfo.bpm / 60.0f * 0.1f; // 根据BPM调整速度
    
    // 记录频谱历史用于波形显示
    float averageEnergy = beatInfo.energy;
    m_spectrumHistory.push_back(averageEnergy);
    
    // 限制历史记录长度
    if (m_spectrumHistory.size() > 100) {
        m_spectrumHistory.erase(m_spectrumHistory.begin());
    }
}

void MusicVisualizer::ProcessMusicFeatures(const BeatInfo& beatInfo) {
    // 更新光照脉冲相位
    m_lightPulsePhase += beatInfo.bpm / 60.0f * 2.0f * M_PI;
    
    // 更新颜色脉冲相位
    m_colorPulsePhase += beatInfo.bpm / 60.0f * 4.0f * M_PI;
}

glm::vec3 MusicVisualizer::FrequencyToColor(float lowFreq, float midFreq, float highFreq) {
    // 将频率能量映射到RGB颜色
    float r = lowFreq;      // 低频 → 红色
    float g = midFreq;      // 中频 → 绿色  
    float b = highFreq;     // 高频 → 蓝色
    
    // 归一化并增强对比度
    r = std::pow(r, 0.8f);
    g = std::pow(g, 0.8f);
    b = std::pow(b, 0.8f);
    
    return glm::vec3(r, g, b);
}

float MusicVisualizer::CalculateEffectIntensity(const BeatInfo& beatInfo, VisualEffectType type) {
    switch (type) {
        case VisualEffectType::SCREEN_FLASH:
            return beatInfo.isBeat ? 1.0f : 0.0f;
        case VisualEffectType::COLOR_PULSE:
            return beatInfo.energy;
        case VisualEffectType::PARTICLE_BURST:
            return beatInfo.intensity;
        case VisualEffectType::CAMERA_SHAKE:
            return beatInfo.energy * beatInfo.intensity;
        case VisualEffectType::LIGHT_STROBE:
            return (beatInfo.energy + beatInfo.intensity) * 0.5f;
        case VisualEffectType::BACKGROUND_WAVE:
            return beatInfo.energy;
        default:
            return 0.5f;
    }
}