#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <string>
#include <glm/glm.hpp>
#include "../core/Renderer.h"
#include "Platform.h"

struct BeatInfo {
    float currentTime;
    float bpm;
    bool isBeat;
    float energy;
    float intensity;
    bool isStrongBeat;
    bool isJumpBeat;
    bool isLongNote;
    float longNoteDuration;
    float jumpInterval;
    float lastJumpTime;
    float nextJumpTime;
    int sectionIndex;
};

struct RhythmNote {
    float time;
    float energy;
    float intensity;
    float holdDuration;
    float jumpInterval;
    bool strongBeat;
};

struct RhythmSection {
    float startTime;
    float endTime;
    float intensity;
};

struct RhythmSequence {
    std::string name;
    float bpm;
    float duration;
    float beatInterval;
    std::vector<RhythmNote> notes;
    std::vector<RhythmSection> sections;
};

// 平台生成规则
struct PlatformRule {
    float energyMin, energyMax;     // 能量范围
    float intensityMin, intensityMax; // 强度范围
    PlatformType type;              // 平台类型
    glm::vec2 sizeRange;           // 大小范围
    glm::vec2 heightRange;         // 高度范围
    float probability;             // 生成概率
};

// 关卡模板
struct LevelTemplate {
    std::string name;
    std::vector<PlatformRule> rules;
    float baseSpacing;             // 基础间距
    float difficultyMultiplier;    // 难度系数
};

class LevelGenerator {
public:
    LevelGenerator();
    ~LevelGenerator();
    
    bool Initialize();
    void Update(float deltaTime, const BeatInfo& beatInfo);
    void Render(Renderer* renderer);
    void Cleanup();
    void Reset();
    void SetSpeedScale(float scale) { m_speedScale = std::max(1.0f, scale); }
    void SetRhythmMode(bool enabled) { m_rhythmMode = enabled; if (!enabled) { m_preGenerated = false; } }
    bool IsRhythmMode() const { return m_rhythmMode; }
    void SetStartPlatformLength(float length) { m_startPlatformLength = std::max(6.0f, length); }
    
    void GeneratePlatformsFromBeat(const BeatInfo& beatInfo);
    void GenerateStaticLevel();
    void GenerateLevelFromSequence(const RhythmSequence& sequence);
    
    // 生成固定演示场景（用于场景漫游）
    void GenerateDemoScene();
    
    // 平台管理
    void AddPlatform(const glm::vec3& position, const glm::vec3& size, PlatformType type = PlatformType::NORMAL, bool fixed = false);
    void RemoveOldPlatforms(float playerX);
    
    // 碰撞检测
    std::vector<Platform*> GetNearbyPlatforms(const glm::vec3& position, float radius = 5.0f);

private:
    std::vector<std::unique_ptr<Platform>> m_platforms;
    
    // 生成参数
    float m_lastGeneratedX;
    float m_platformSpacing;
    float m_generationDistance;
    float m_speedScale;
    PlatformType m_lastGeneratedType;
    bool m_rhythmMode;
    float m_startPlatformLength;
    bool m_preGenerated;
    
    // 音乐映射参数
    float m_lastBeatTime;
    float m_beatThreshold;
    
    void UpdateGeneration(float deltaTime, const BeatInfo& beatInfo);
    glm::vec3 CalculatePlatformPosition(float x, const BeatInfo& beatInfo);
    glm::vec3 CalculatePlatformSize(const BeatInfo& beatInfo);
    int CalculatePlatformType(const BeatInfo& beatInfo);
};
