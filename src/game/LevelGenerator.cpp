#include "LevelGenerator.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>

namespace {

bool IsBeltType(PlatformType type) {
    return type == PlatformType::BOOST ||
           type == PlatformType::BOOST_LEFT ||
           type == PlatformType::BOOST_RIGHT;
}

PlatformType ChooseBeltType(std::mt19937& rng) {
    std::uniform_int_distribution<int> dist(0, 1);
    return dist(rng) == 0 ? PlatformType::BOOST_LEFT : PlatformType::BOOST_RIGHT;
}

} // namespace

LevelGenerator::LevelGenerator() 
    : m_lastGeneratedX(0.0f)
    , m_platformSpacing(4.0f)
    , m_generationDistance(50.0f)
    , m_speedScale(1.0f)
    , m_lastGeneratedType(PlatformType::NORMAL)
    , m_rhythmMode(false)
    , m_startPlatformLength(8.0f)
    , m_preGenerated(false)
    , m_lastBeatTime(0.0f)
    , m_beatThreshold(0.1f) {
}

LevelGenerator::~LevelGenerator() {
    Cleanup();
}

bool LevelGenerator::Initialize() {
    // ���ɳ�ʼ��̬�ؿ����ڲ���
    GenerateStaticLevel();
    
    std::cout << "LevelGenerator initialized with " << m_platforms.size() << " platforms" << std::endl;
    return true;
}

void LevelGenerator::Reset() {
    m_platforms.clear();
    m_lastGeneratedX = 0.0f;
    m_lastBeatTime = 0.0f;
    m_speedScale = 1.0f;
    m_lastGeneratedType = PlatformType::NORMAL;
    m_preGenerated = false;
    if (m_rhythmMode) {
        glm::vec3 startSize(m_startPlatformLength, 0.55f, 3.8f);
        AddPlatform(glm::vec3(0.0f, 0.0f, 0.0f), startSize, PlatformType::NORMAL, true);
        m_lastGeneratedX = 0.0f;
    } else {
        GenerateStaticLevel();
    }
}

void LevelGenerator::Update(float deltaTime, const BeatInfo& beatInfo) {
    // ��������ƽ̨
    for (auto& platform : m_platforms) {
        platform->SetSpeedMultiplier(m_speedScale);
        platform->Update(deltaTime);
    }
    
    // ��������������ƽ̨
    UpdateGeneration(deltaTime, beatInfo);
    
    // �Ƴ�Զ���ƽ̨���򻯰棩
    RemoveOldPlatforms(-20.0f);
}

void LevelGenerator::Render(Renderer* renderer) {
    for (auto& platform : m_platforms) {
        platform->Render(renderer);
    }
}

void LevelGenerator::GenerateStaticLevel() {
    glm::vec3 startSize(m_startPlatformLength, 0.55f, 3.8f);
    AddPlatform(glm::vec3(0.0f, 0.0f, 0.0f), startSize, PlatformType::NORMAL, true);

    std::random_device rd;
    auto nowSeed = static_cast<unsigned int>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::seed_seq seq{rd(), rd(), nowSeed};
    std::mt19937 rng(seq);

    std::uniform_real_distribution<float> spacingDist(3.1f, 4.4f);
    std::uniform_real_distribution<float> zOffsetDist(-1.4f, 1.4f);
    std::uniform_real_distribution<float> heightDeltaDist(-0.2f, 0.45f);
    std::uniform_real_distribution<float> normalSizeDist(1.9f, 2.4f);
    std::uniform_real_distribution<float> normalHeightDist(0.55f, 0.7f);
    std::uniform_real_distribution<float> slideWidthDist(3.2f, 4.3f);
    std::uniform_real_distribution<float> slideDepthDist(1.2f, 1.6f);
    std::uniform_real_distribution<float> slideHeightDist(0.55f, 0.7f);
    std::uniform_real_distribution<float> movingSizeDist(2.0f, 2.4f);
    std::uniform_real_distribution<float> movingHeightDist(0.55f, 0.7f);
    std::uniform_real_distribution<float> bounceSizeDist(2.1f, 2.6f);
    std::uniform_real_distribution<float> bounceHeightDist(0.65f, 0.85f);
    std::uniform_real_distribution<float> boostLengthDist(4.8f, 6.6f);
    std::uniform_real_distribution<float> boostWidthDist(1.6f, 2.0f);
    std::uniform_real_distribution<float> boostHeightDist(0.4f, 0.55f);
    std::discrete_distribution<int> typeDist({55, 14, 14, 9, 8});

    float lastX = 0.0f;
    float lastY = 0.0f;
    PlatformType prevType = PlatformType::NORMAL;
    float prevHalfX = startSize.x * 0.5f;

    for (int i = 1; i <= 28; ++i) {
        int typePick = typeDist(rng);
        if (i < 4) {
            typePick = 0;
        }

        PlatformType type = PlatformType::NORMAL;
        glm::vec3 size(2.0f, 0.6f, 2.0f);
        if (typePick == 1) {
            type = PlatformType::SLIDE;
            size = glm::vec3(slideWidthDist(rng), slideHeightDist(rng), slideDepthDist(rng));
        } else if (typePick == 2) {
            type = PlatformType::MOVING;
            float side = movingSizeDist(rng);
            size = glm::vec3(side, movingHeightDist(rng), side);
        } else if (typePick == 3) {
            type = PlatformType::BOUNCE;
            float side = bounceSizeDist(rng);
            size = glm::vec3(side, bounceHeightDist(rng), side);
        } else if (typePick == 4) {
            type = ChooseBeltType(rng);
            size = glm::vec3(boostLengthDist(rng), boostHeightDist(rng), boostWidthDist(rng));
        } else {
            float side = normalSizeDist(rng);
            size = glm::vec3(side, normalHeightDist(rng), side);
        }

        float spacing = spacingDist(rng);
        if (prevType == PlatformType::BOUNCE || prevType == PlatformType::SLIDE) {
            spacing = std::min(spacing, 4.0f);
        } else if (IsBeltType(prevType)) {
            spacing = std::min(spacing, 4.2f);
        }
        if (i == 1) {
            spacing = std::max(spacing, 5.8f);
        } else if (i < 4) {
            spacing = std::max(spacing, 3.6f);
        }

        float minSpacing = prevHalfX + size.x * 0.5f + 0.6f;
        spacing = std::max(spacing, minSpacing);

        lastX += spacing;

        float heightDelta = heightDeltaDist(rng);
        if (prevType == PlatformType::BOUNCE || prevType == PlatformType::SLIDE) {
            heightDelta = std::clamp(heightDelta, -0.1f, 0.25f);
        } else if (IsBeltType(prevType)) {
            heightDelta = std::clamp(heightDelta, -0.15f, 0.3f);
        }
        lastY = std::clamp(lastY + heightDelta, 0.0f, 1.1f);

        float z = zOffsetDist(rng);
        if (prevType == PlatformType::BOUNCE || prevType == PlatformType::SLIDE) {
            z *= 0.6f;
        } else if (IsBeltType(prevType)) {
            z *= 0.7f;
        }
        if (i < 4) {
            z *= 0.5f;
        }

        AddPlatform(glm::vec3(lastX, lastY, z), size, type);

        prevType = type;
        prevHalfX = size.x * 0.5f;
    }

    m_lastGeneratedX = lastX;
    std::cout << "Static level generated with randomized layout" << std::endl;
}

void LevelGenerator::GenerateDemoScene() {
    // 清除现有平台
    m_platforms.clear();
    m_lastGeneratedX = 0.0f;
    m_preGenerated = true;
    
    // 生成固定的演示场景，展示所有平台类型
    // 平台之间不重叠，间距固定
    
    float currentX = 0.0f;
    float baseY = 0.0f;
    float spacing = 4.5f; // 固定间距
    
    // 1. 起始平台 - 长平台
    glm::vec3 startSize(10.0f, 0.6f, 4.0f);
    AddPlatform(glm::vec3(currentX, baseY, 0.0f), startSize, PlatformType::NORMAL, true);
    currentX += startSize.x * 0.5f + spacing;
    
    // 2. 普通平台组 - 阶梯上升
    for (int i = 0; i < 3; ++i) {
        glm::vec3 size(2.5f, 0.6f, 2.5f);
        float y = baseY + i * 0.3f;
        float z = (i - 1) * 1.5f;
        AddPlatform(glm::vec3(currentX + size.x * 0.5f, y, z), size, PlatformType::NORMAL);
        currentX += size.x + spacing;
    }
    
    // 3. 滑行平台 - 较长
    glm::vec3 slideSize(5.0f, 0.55f, 1.8f);
    AddPlatform(glm::vec3(currentX + slideSize.x * 0.5f, baseY + 0.5f, 0.0f), slideSize, PlatformType::SLIDE);
    currentX += slideSize.x + spacing;
    
    // 4. 弹跳平台组
    for (int i = 0; i < 2; ++i) {
        glm::vec3 size(2.8f, 0.7f, 2.8f);
        float z = (i == 0) ? -1.2f : 1.2f;
        AddPlatform(glm::vec3(currentX + size.x * 0.5f, baseY + 0.3f, z), size, PlatformType::BOUNCE);
        currentX += size.x + spacing * 0.7f;
    }
    currentX += spacing * 0.3f;
    
    // 5. 加速平台 - 直行
    glm::vec3 boostSize(6.0f, 0.5f, 2.0f);
    AddPlatform(glm::vec3(currentX + boostSize.x * 0.5f, baseY + 0.2f, 0.0f), boostSize, PlatformType::BOOST);
    currentX += boostSize.x + spacing;
    
    // 6. 左右加速平台
    glm::vec3 boostLRSize(5.0f, 0.5f, 1.8f);
    AddPlatform(glm::vec3(currentX + boostLRSize.x * 0.5f, baseY + 0.3f, -1.5f), boostLRSize, PlatformType::BOOST_LEFT);
    currentX += boostLRSize.x + spacing * 0.5f;
    AddPlatform(glm::vec3(currentX + boostLRSize.x * 0.5f, baseY + 0.3f, 1.5f), boostLRSize, PlatformType::BOOST_RIGHT);
    currentX += boostLRSize.x + spacing;
    
    // 7. 移动平台组
    for (int i = 0; i < 2; ++i) {
        glm::vec3 size(2.5f, 0.6f, 2.5f);
        float y = baseY + 0.4f + i * 0.2f;
        AddPlatform(glm::vec3(currentX + size.x * 0.5f, y, 0.0f), size, PlatformType::MOVING);
        currentX += size.x + spacing;
    }
    
    // 8. 综合展示区 - 多种平台交替
    float mixY = baseY + 0.5f;
    PlatformType mixTypes[] = {PlatformType::NORMAL, PlatformType::SLIDE, PlatformType::BOUNCE, 
                               PlatformType::BOOST, PlatformType::NORMAL};
    for (int i = 0; i < 5; ++i) {
        glm::vec3 size(3.0f, 0.6f, 2.5f);
        if (mixTypes[i] == PlatformType::SLIDE) size = glm::vec3(4.5f, 0.55f, 1.6f);
        if (mixTypes[i] == PlatformType::BOOST) size = glm::vec3(5.5f, 0.5f, 1.8f);
        float z = (i % 2 == 0) ? -0.8f : 0.8f;
        AddPlatform(glm::vec3(currentX + size.x * 0.5f, mixY, z), size, mixTypes[i]);
        currentX += size.x + spacing * 0.8f;
    }
    
    // 9. 终点大平台
    glm::vec3 endSize(12.0f, 0.6f, 5.0f);
    AddPlatform(glm::vec3(currentX + endSize.x * 0.5f + spacing, baseY, 0.0f), endSize, PlatformType::NORMAL, true);
    
    m_lastGeneratedX = currentX + endSize.x + spacing;
    std::cout << "Demo scene generated with " << m_platforms.size() << " platforms" << std::endl;
}

void LevelGenerator::GenerateLevelFromSequence(const RhythmSequence& sequence) {
    m_platforms.clear();
    m_lastGeneratedX = 0.0f;
    m_lastGeneratedType = PlatformType::NORMAL;
    m_preGenerated = true;

    glm::vec3 startSize(m_startPlatformLength, 0.55f, 3.8f);
    AddPlatform(glm::vec3(0.0f, 0.0f, 0.0f), startSize, PlatformType::NORMAL, true);

    float lastEndX = startSize.x * 0.5f;
    float lastY = 0.0f;
    float lastZ = 0.0f;

    std::mt19937 rng(static_cast<unsigned int>(sequence.notes.size() * 131u + 17u));
    std::uniform_real_distribution<float> zJitter(-0.4f, 0.4f);

    const float forwardSpeed = 2.4f;
    const float baseJumpDistance = 2.2f;

    for (const auto& note : sequence.notes) {
        float jumpInterval = std::max(0.35f, note.jumpInterval);
        float jumpDistance = std::clamp(baseJumpDistance + note.intensity * 0.5f, 2.0f, 3.1f);

        PlatformType type = PlatformType::NORMAL;
        if (note.holdDuration > 0.25f) {
            type = PlatformType::SLIDE;
        } else if (note.strongBeat && note.energy > 0.6f) {
            type = PlatformType::BOUNCE;
        } else if (note.intensity > 0.6f) {
            type = PlatformType::MOVING;
        } else if (note.intensity > 0.4f && note.energy < 0.55f) {
            type = PlatformType::BOOST;
        }

        if (type == PlatformType::BOOST) {
            type = ChooseBeltType(rng);
        }

        glm::vec3 size(2.0f, 0.6f, 2.0f);
        if (type == PlatformType::SLIDE) {
            float length = std::clamp(note.holdDuration * forwardSpeed * 2.4f, 6.0f, 16.0f);
            size = glm::vec3(length, 0.55f, 1.6f);
        } else if (IsBeltType(type)) {
            float length = 4.6f + note.intensity * 1.2f;
            size = glm::vec3(length, 0.45f, 1.8f);
        } else if (type == PlatformType::MOVING) {
            float side = 2.0f + note.intensity * 0.5f;
            size = glm::vec3(side, 0.6f, side);
        } else if (type == PlatformType::BOUNCE) {
            float side = 2.2f + note.energy * 0.6f;
            size = glm::vec3(side, 0.7f, side);
        } else {
            float side = 2.0f + (1.0f - note.intensity) * 0.4f;
            size = glm::vec3(side, 0.6f, side);
        }

        float dwellLength = std::clamp(jumpInterval * forwardSpeed * 2.0f, 2.4f, 8.5f);
        size.x = std::max(size.x, dwellLength);

        if (type == PlatformType::BOUNCE) {
            jumpDistance = std::min(jumpDistance, 2.8f);
        } else if (type == PlatformType::SLIDE) {
            jumpDistance = std::min(jumpDistance, 3.0f);
        }

        float nextX = lastEndX + jumpDistance + size.x * 0.5f;

        float heightDelta = (note.energy - 0.5f) * 0.6f;
        if (type == PlatformType::SLIDE) {
            heightDelta = std::clamp(heightDelta, -0.1f, 0.2f);
        } else if (IsBeltType(type)) {
            heightDelta = std::clamp(heightDelta, -0.15f, 0.3f);
        } else if (type == PlatformType::BOUNCE) {
            heightDelta = std::clamp(heightDelta, 0.0f, 0.4f);
        } else {
            heightDelta = std::clamp(heightDelta, -0.15f, 0.35f);
        }
        lastY = std::clamp(lastY + heightDelta, 0.0f, 1.2f);

        float z = sin(note.time * 0.8f) * (0.6f + note.intensity * 0.3f) + zJitter(rng);
        z = std::clamp(z, -1.2f, 1.2f);
        if (type == PlatformType::SLIDE || type == PlatformType::BOUNCE) {
            z *= 0.5f;
        }
        if (note.holdDuration > 0.25f) {
            z = glm::mix(lastZ, z, 0.3f);
        }
        lastZ = z;

        AddPlatform(glm::vec3(nextX, lastY, z), size, type);
        lastEndX = nextX + size.x * 0.5f;
        m_lastGeneratedType = type;
    }

    m_lastGeneratedX = lastEndX;
}
void LevelGenerator::UpdateGeneration(float /*deltaTime*/, const BeatInfo& beatInfo) {
    if (m_rhythmMode && m_preGenerated) {
        return;
    }
    if (m_rhythmMode) {
        if (beatInfo.isJumpBeat && (beatInfo.currentTime - m_lastBeatTime) > m_beatThreshold) {
            m_lastBeatTime = beatInfo.currentTime;
            GeneratePlatformsFromBeat(beatInfo);
        }
        return;
    }

    // �򻯵Ľ�����Ӧ����
    if (beatInfo.isBeat && (beatInfo.currentTime - m_lastBeatTime) > m_beatThreshold) {
        m_lastBeatTime = beatInfo.currentTime;
        
        // ��������ǿ�Ⱥ���������ƽ̨
        GeneratePlatformsFromBeat(beatInfo);
    }
    
    // ���������Ա���ǰ�����㹻��ƽ̨
    float playerX = 0.0f; // ����Ӧ�ô������ȡ���λ��
    if (m_lastGeneratedX - playerX < m_generationDistance) {
        // ������һ��ƽ̨
        float spacing = m_platformSpacing * (0.8f + beatInfo.intensity * 0.4f);
        if (m_lastGeneratedType == PlatformType::BOUNCE || m_lastGeneratedType == PlatformType::SLIDE) {
            spacing = std::min(spacing, 4.0f);
        } else if (IsBeltType(m_lastGeneratedType)) {
            spacing = std::min(spacing, 4.2f);
        }
        float nextX = m_lastGeneratedX + spacing;
        
        glm::vec3 position(nextX, 0.0f, 0.0f);
        glm::vec3 size(2.0f, 0.2f, 2.0f);
        
        AddPlatform(position, size, PlatformType::NORMAL);
        m_lastGeneratedX = nextX;
    }
}

void LevelGenerator::GeneratePlatformsFromBeat(const BeatInfo& beatInfo) {
    static std::mt19937 beltRng(std::random_device{}());
    // ������һ��ƽ̨λ��
    float spacing = m_platformSpacing * (0.8f + beatInfo.intensity * 0.4f);
    if (m_rhythmMode && beatInfo.jumpInterval > 0.0f) {
        float holdDuration = std::max(0.0f, beatInfo.longNoteDuration);
        float airTime = std::max(0.2f, beatInfo.jumpInterval - holdDuration);
        float baseSpeed = 2.7f;
        float slideSpeed = 4.4f;
        spacing = (baseSpeed * airTime + slideSpeed * holdDuration) * 0.9f;
        spacing = std::clamp(spacing, 2.6f, 5.0f);
    }
    if (!m_rhythmMode || beatInfo.longNoteDuration <= 0.0f) {
        if (m_lastGeneratedType == PlatformType::BOUNCE || m_lastGeneratedType == PlatformType::SLIDE) {
            spacing = std::clamp(spacing, 3.0f, 4.0f);
        } else if (IsBeltType(m_lastGeneratedType)) {
            spacing = std::clamp(spacing, 3.2f, 4.2f);
        }
    }
    float nextX = m_lastGeneratedX + spacing;

    // ������������ѡ��ƽ̨����
    PlatformType type = PlatformType::NORMAL;
    if (beatInfo.intensity > 0.8f && beatInfo.energy > 0.7f) {
        type = PlatformType::BOUNCE;
    } else if (beatInfo.intensity > 0.5f && beatInfo.energy > 0.6f) {
        type = PlatformType::SLIDE;
    } else if (beatInfo.intensity > 0.35f && beatInfo.energy < 0.55f) {
        type = PlatformType::BOOST;
    } else if (beatInfo.intensity > 0.3f) {
        type = PlatformType::MOVING;
    }
    if (type == PlatformType::BOOST) {
        type = ChooseBeltType(beltRng);
    }
    if (m_rhythmMode && beatInfo.longNoteDuration > 0.0f) {
        type = PlatformType::SLIDE;
    }

    // ����ƽ̨��С - ǿ��Խ�ߣ�ƽ̨ԽС���Ѷ�Խ��
    float sizeVariation = (1.0f - beatInfo.intensity) * 0.8f;
    float width = 1.4f + sizeVariation;
    float depth = 1.4f + sizeVariation;
    float height = 0.2f + beatInfo.energy * 0.1f;

    if (IsBeltType(type)) {
        float length = 4.8f + beatInfo.intensity * 1.4f;
        float narrow = 1.6f + sizeVariation * 0.3f;
        width = length;
        depth = narrow;
        height = 0.48f + beatInfo.energy * 0.05f;
    }
    if (m_rhythmMode && beatInfo.longNoteDuration > 0.0f) {
        float slideSpeed = 4.4f;
        float length = std::clamp(beatInfo.longNoteDuration * slideSpeed * 2.0f, 6.0f, 14.0f);
        width = length;
        depth = 1.6f;
        height = 0.55f;
    }

    // ����߶ȱ仯
    float heightVariation = (beatInfo.energy - 0.5f) * 2.5f;
    if (m_lastGeneratedType == PlatformType::BOUNCE || m_lastGeneratedType == PlatformType::SLIDE) {
        heightVariation = std::clamp(heightVariation, -0.2f, 0.4f);
    } else if (IsBeltType(m_lastGeneratedType)) {
        heightVariation = std::clamp(heightVariation, -0.25f, 0.45f);
    }
    float y = heightVariation;

    // ����ƫ�� - �����ڶ�
    float z = sin(beatInfo.currentTime * 1.2f) * (0.8f + beatInfo.intensity);
    if (m_lastGeneratedType == PlatformType::BOUNCE || m_lastGeneratedType == PlatformType::SLIDE) {
        z *= 0.6f;
    } else if (IsBeltType(m_lastGeneratedType)) {
        z *= 0.7f;
    }
    if (beatInfo.isBeat && beatInfo.intensity > 0.7f) {
        z += (fmod(beatInfo.currentTime, 2.0f) > 1.0f) ? 1.2f : -1.2f;
    }

    glm::vec3 position(nextX, y, z);
    glm::vec3 size(width, height, depth);

    if (beatInfo.isBeat && beatInfo.intensity > 0.85f) {
        // ��ǿ��ʱ����˫ƽ̨
        glm::vec3 offset(0.0f, 0.0f, 1.0f + beatInfo.energy);
        glm::vec3 twinSize(width * 0.9f, height, depth * 0.9f);
        AddPlatform(position - offset, twinSize, type);
        AddPlatform(position + offset, twinSize, type);
    } else {
        AddPlatform(position, size, type);
    }

    m_lastGeneratedX = nextX;
}

void LevelGenerator::AddPlatform(const glm::vec3& position, const glm::vec3& size, PlatformType type, bool fixed) {
    auto platform = std::make_unique<Platform>(position, size, type);
    if (type == PlatformType::MOVING) {
        auto hash = [](float v) {
            float s = sin(v * 12.9898f) * 43758.5453f;
            return s - floor(s);
        };
        float seed = hash(position.x + position.z * 0.31f);
        float dirChoice = (seed < 0.5f) ? -1.0f : 1.0f;
        glm::vec3 direction(0.0f, 0.0f, dirChoice);
        float speed = 0.3f + seed * 0.6f;
        float range = 0.4f + seed * 0.6f;
        platform->ConfigureMovement(direction, speed, range);
    }
    platform->SetFixed(fixed);
    if (platform->Initialize()) {
        m_platforms.push_back(std::move(platform));
        m_lastGeneratedType = type;
    }
}

void LevelGenerator::RemoveOldPlatforms(float playerX) {
    m_platforms.erase(
        std::remove_if(m_platforms.begin(), m_platforms.end(),
            [playerX](const std::unique_ptr<Platform>& platform) {
                return platform->GetPosition().x < playerX;
            }),
        m_platforms.end()
    );
}

std::vector<Platform*> LevelGenerator::GetNearbyPlatforms(const glm::vec3& position, float radius) {
    std::vector<Platform*> nearby;
    
    for (auto& platform : m_platforms) {
        float distance = glm::length(platform->GetPosition() - position);
        if (distance <= radius) {
            nearby.push_back(platform.get());
        }
    }
    
    return nearby;
}

glm::vec3 LevelGenerator::CalculatePlatformPosition(float x, const BeatInfo& beatInfo) {
    float y = beatInfo.energy * 2.0f; // ����Ӱ��߶�
    float z = sin(beatInfo.currentTime * 2.0f) * beatInfo.intensity * 3.0f; // ǿ��Ӱ�����ƫ��
    
    return glm::vec3(x, y, z);
}

void LevelGenerator::Cleanup() {
    m_platforms.clear();
}
