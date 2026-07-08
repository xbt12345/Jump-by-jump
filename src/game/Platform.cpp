#include "Platform.h"
#include "../geometry/Primitives.h"

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace {
struct TextureSet {
    unsigned int diffuse = 0;
    unsigned int specular = 0;
};

unsigned int CreateTexture2D(const std::vector<unsigned char>& data, int width, int height) {
    unsigned int texId = 0;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    return texId;
}

TextureSet CreatePlatformTexture(PlatformType type) {
    const int width = 64;
    const int height = 64;
    std::vector<unsigned char> diffuse(width * height * 4, 255);
    std::vector<unsigned char> specular(width * height * 4, 255);

    auto clampByte = [](float v) -> unsigned char {
        int value = static_cast<int>(v * 255.0f);
        return static_cast<unsigned char>(std::clamp(value, 0, 255));
    };

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float u = static_cast<float>(x) / static_cast<float>(width - 1);
            float v = static_cast<float>(y) / static_cast<float>(height - 1);

            float r = 1.0f;
            float g = 1.0f;
            float b = 1.0f;
            float spec = 0.4f;

            switch (type) {
                case PlatformType::NORMAL: {
                    float wave = sin((u + v) * 10.0f) * 0.08f;
                    float sparkle = (sin(u * 22.0f) * sin(v * 20.0f) > 0.92f) ? 0.2f : 0.0f;
                    r = 0.2f + wave + sparkle * 0.3f;
                    g = 0.5f + wave + sparkle * 0.2f;
                    b = 0.95f + wave;
                    spec = 0.45f;
                    break;
                }
                case PlatformType::SLIDE: {
                    float crack = (sin(u * 18.0f) + sin(v * 16.0f)) * 0.03f;
                    float grit = sin((u - v) * 10.0f) * 0.02f;
                    r = 0.45f + crack + grit;
                    g = 0.48f + crack + grit;
                    b = 0.52f + crack + grit;
                    spec = 0.35f;

                    // 箭头沿u方向绘制（对应X轴正方向，即玩家前进方向）
                    float tile = fmod(u * 4.0f, 1.0f);
                    float center = 0.5f;
                    float arrow = 0.0f;
                    // 箭头尾部（细长条）
                    if (tile > 0.12f && tile < 0.32f && std::abs(v - center) < 0.05f) {
                        arrow = 1.0f;
                    // 箭头头部（三角形）
                    } else if (tile >= 0.32f && tile <= 0.55f) {
                        float t = (tile - 0.32f) / 0.23f;
                        float arrowWidth = (1.0f - t) * 0.18f;
                        if (std::abs(v - center) < arrowWidth) {
                            arrow = 1.0f;
                        }
                    }
                    if (arrow > 0.0f) {
                        r = 1.0f;
                        g = 0.85f;
                        b = 0.2f;
                        spec = 0.6f;
                    }
                    break;
                }
                case PlatformType::MOVING: {
                    float grid = (fmod(u * 8.0f, 1.0f) < 0.12f || fmod(v * 8.0f, 1.0f) < 0.12f) ? 0.3f : 0.0f;
                    r = 0.18f + grid;
                    g = 0.7f + grid * 0.6f;
                    b = 0.5f + grid * 0.3f;
                    spec = 0.5f;
                    break;
                }
                case PlatformType::BOUNCE: {
                    float dx = u - 0.5f;
                    float dy = v - 0.5f;
                    float dist = sqrt(dx * dx + dy * dy);
                    float glow = std::max(0.0f, 1.0f - dist * 1.6f);
                    float star = (sin(u * 28.0f) * sin(v * 26.0f) > 0.95f) ? 0.2f : 0.0f;
                    r = 0.55f + glow * 0.25f + star;
                    g = 0.2f + glow * 0.08f;
                    b = 0.85f + glow * 0.35f;
                    spec = 0.55f;
                    break;
                }
                case PlatformType::BOOST:
                case PlatformType::BOOST_LEFT:
                case PlatformType::BOOST_RIGHT: {
                    float base = 0.06f;
                    r = base;
                    g = base;
                    b = base;
                    spec = 0.2f;

                    float arrow = 0.0f;
                    float center = 0.5f;
                    
                    if (type == PlatformType::BOOST) {
                        // 向前加速：沿 u 方向（X 轴正方向）绘制箭头
                        float tile = fmod(u * 3.0f, 1.0f);
                        // 箭头尾部
                        if (tile > 0.14f && tile < 0.34f && std::abs(v - center) < 0.07f) {
                            arrow = 1.0f;
                        // 箭头头部
                        } else if (tile >= 0.34f && tile <= 0.62f) {
                            float t = (tile - 0.34f) / 0.28f;
                            float arrowWidth = (1.0f - t) * 0.22f;
                            if (std::abs(v - center) < arrowWidth) {
                                arrow = 1.0f;
                            }
                        }
                    } else {
                        // 向左/向右加速：沿 v 方向（Z 轴）绘制箭头
                        float tile = fmod(v * 3.0f, 1.0f);
                        // BOOST_LEFT 向左（v 增加方向 = Z 减少 = 左）
                        // BOOST_RIGHT 向右（v 减少方向 = Z 增加 = 右）
                        bool toLeft = (type == PlatformType::BOOST_LEFT);
                        float axis = toLeft ? tile : (1.0f - tile);
                        // 箭头尾部
                        if (axis > 0.14f && axis < 0.34f && std::abs(u - center) < 0.07f) {
                            arrow = 1.0f;
                        // 箭头头部
                        } else if (axis >= 0.34f && axis <= 0.62f) {
                            float t = (axis - 0.34f) / 0.28f;
                            float arrowWidth = (1.0f - t) * 0.22f;
                            if (std::abs(u - center) < arrowWidth) {
                                arrow = 1.0f;
                            }
                        }
                    }
                    
                    if (arrow > 0.0f) {
                        r = 1.0f;
                        g = 0.85f;
                        b = 0.2f;
                        spec = 0.7f;
                    }
                    break;
                }
                default:
                    break;
            }

            int idx = (y * width + x) * 4;
            diffuse[idx + 0] = clampByte(r);
            diffuse[idx + 1] = clampByte(g);
            diffuse[idx + 2] = clampByte(b);
            diffuse[idx + 3] = 255;

            unsigned char specByte = clampByte(spec);
            specular[idx + 0] = specByte;
            specular[idx + 1] = specByte;
            specular[idx + 2] = specByte;
            specular[idx + 3] = 255;
        }
    }

    TextureSet textures;
    textures.diffuse = CreateTexture2D(diffuse, width, height);
    textures.specular = CreateTexture2D(specular, width, height);
    return textures;
}

TextureSet GetPlatformTexture(PlatformType type) {
    static std::array<TextureSet, 7> cached;
    static std::array<bool, 7> created = {false, false, false, false, false, false, false};
    int index = std::clamp(static_cast<int>(type), 0, 6);
    if (!created[index]) {
        cached[index] = CreatePlatformTexture(type);
        created[index] = true;
    }
    return cached[index];
}
} // namespace

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Platform::Platform(const glm::vec3& position, const glm::vec3& size, PlatformType type)
    : m_position(position)
    , m_originalPosition(position)
    , m_size(size)
    , m_type(type)
    , m_isActive(true)
    , m_isPressed(false)
    , m_playerOnPlatform(false)
    , m_isFixed(false)
    , m_animationTime(0.0f)
    , m_currentDepth(0.0f)
    , m_moveDirection(0.0f, 0.0f, 1.0f)
    , m_moveSpeed(1.0f)
    , m_moveRange(2.0f)
    , m_speedMultiplier(1.0f)
    , m_collisionHeightScale(1.8f)
    , m_VAO(0), m_VBO(0), m_EBO(0)
    , m_lastPosition(position)
    , m_velocity(0.0f) {
    switch (type) {
        case PlatformType::NORMAL:
            m_pressDepth = 0.06f;
            m_material.ambient = glm::vec3(0.1f, 0.2f, 0.4f);
            m_material.diffuse = glm::vec3(0.3f, 0.5f, 0.9f);
            m_material.specular = glm::vec3(0.9f, 0.95f, 1.0f);
            m_material.shininess = 128.0f;
            break;

        case PlatformType::SLIDE:
            m_pressDepth = 0.1f;
            m_material.ambient = glm::vec3(0.3f, 0.1f, 0.05f);
            m_material.diffuse = glm::vec3(0.8f, 0.3f, 0.1f);
            m_material.specular = glm::vec3(1.0f, 0.8f, 0.6f);
            m_material.shininess = 256.0f;
            break;

        case PlatformType::BOUNCE:
            m_pressDepth = 0.1f;
            m_material.ambient = glm::vec3(0.2f, 0.05f, 0.3f);
            m_material.diffuse = glm::vec3(0.6f, 0.2f, 0.9f);
            m_material.specular = glm::vec3(1.0f, 0.8f, 1.0f);
            m_material.shininess = 512.0f;
            break;

        case PlatformType::MOVING:
            m_pressDepth = 0.07f;
            m_material.ambient = glm::vec3(0.05f, 0.25f, 0.2f);
            m_material.diffuse = glm::vec3(0.2f, 0.7f, 0.5f);
            m_material.specular = glm::vec3(0.8f, 1.0f, 0.9f);
            m_material.shininess = 200.0f;
            break;

        case PlatformType::BOOST:
        case PlatformType::BOOST_LEFT:
        case PlatformType::BOOST_RIGHT:
            m_pressDepth = 0.06f;
            m_material.ambient = glm::vec3(0.35f, 0.2f, 0.05f);
            m_material.diffuse = glm::vec3(0.95f, 0.6f, 0.15f);
            m_material.specular = glm::vec3(1.0f, 0.85f, 0.5f);
            m_material.shininess = 220.0f;
            break;

        default:
            m_pressDepth = 0.06f;
            m_material.ambient = glm::vec3(0.2f, 0.2f, 0.25f);
            m_material.diffuse = glm::vec3(0.6f, 0.6f, 0.7f);
            m_material.specular = glm::vec3(0.9f, 0.9f, 1.0f);
            m_material.shininess = 150.0f;
            break;
    }

    m_material.diffuseTexture = 0;
    m_material.specularTexture = 0;
    m_material.useTexture = true;
    m_material.ambient = glm::vec3(1.0f);
    m_material.diffuse = glm::vec3(1.0f);
    m_material.specular = glm::vec3(1.0f);

    if (type == PlatformType::SLIDE || type == PlatformType::BOOST ||
        type == PlatformType::BOOST_LEFT || type == PlatformType::BOOST_RIGHT) {
        m_collisionHeightScale = 1.0f;
    } else if (type == PlatformType::BOUNCE) {
        m_collisionHeightScale = 1.2f;
    } else {
        m_collisionHeightScale = 1.1f;
    }
}

Platform::~Platform() {
    Cleanup();
}

bool Platform::Initialize() {
    SetupMesh();
    if (m_material.useTexture) {
        TextureSet textures = GetPlatformTexture(m_type);
        m_material.diffuseTexture = textures.diffuse;
        m_material.specularTexture = textures.specular;
    }
    return true;
}

void Platform::Update(float deltaTime) {
    m_lastPosition = m_position;
    m_animationTime += deltaTime;

    UpdateAnimation(deltaTime);

    if (deltaTime > 0.0f) {
        glm::vec3 rawVelocity = (m_position - m_lastPosition) / deltaTime;
        if (m_type == PlatformType::MOVING) {
            m_velocity = glm::mix(m_velocity, rawVelocity, 0.25f);
        } else {
            m_velocity = rawVelocity;
        }
    } else {
        m_velocity = glm::vec3(0.0f);
    }
}

void Platform::UpdateAnimation(float deltaTime) {
    float targetDepth = m_isPressed ? m_pressDepth * 1.1f : 0.0f;
    float animationSpeed = m_isPressed ? 10.0f : 6.0f;
    m_currentDepth = glm::mix(m_currentDepth, targetDepth, animationSpeed * deltaTime);

    m_position = m_originalPosition;
    m_position.y -= m_currentDepth;

    if (m_isFixed) {
        m_currentDepth = 0.0f;
        m_position = m_originalPosition;
        return;
    }

    switch (m_type) {
        case PlatformType::BOUNCE:
            if (m_playerOnPlatform) {
                float bounceIntensity = 0.06f;
                float bounceFreq = 7.0f;
                float decay = exp(-m_animationTime * 4.0f);
                float bounceEffect = sin(m_animationTime * bounceFreq) * bounceIntensity * decay;
                m_currentDepth += bounceEffect;
                m_position.x = m_originalPosition.x;
                m_position.z = m_originalPosition.z;
            } else {
                float recovery = exp(-m_animationTime * 3.0f) * sin(m_animationTime * 8.0f) * 0.01f;
                m_position.y += recovery;
                m_position.x = m_originalPosition.x;
                m_position.z = m_originalPosition.z;
            }
            break;

        case PlatformType::SLIDE:
            if (m_isPressed) {
                float trackDepth = m_pressDepth * 1.5f;
                float waveOffset = sin(m_animationTime * 6.0f) * 0.05f;
                m_currentDepth = trackDepth + waveOffset;

                float sideWave = sin(m_animationTime * 5.0f) * 0.04f;
                float lengthWave = cos(m_animationTime * 3.5f) * 0.03f;
                m_position.x = m_originalPosition.x + sideWave;
                m_position.z = m_originalPosition.z + lengthWave;

                float liquidEffect = sin(m_animationTime * 10.0f) * 0.02f;
                m_position.y += liquidEffect;
            } else {
                float bounceBack = sin(m_animationTime * 10.0f) * 0.08f * exp(-m_animationTime * 2.5f);
                float elasticRebound = cos(m_animationTime * 12.0f) * 0.04f * exp(-m_animationTime * 3.0f);
                m_position.y += bounceBack + elasticRebound;

                float swayBack = sin(m_animationTime * 6.0f) * 0.03f * exp(-m_animationTime * 2.0f);
                m_position.x = m_originalPosition.x + swayBack;
            }
            break;

        case PlatformType::BOOST:
        case PlatformType::BOOST_LEFT:
        case PlatformType::BOOST_RIGHT:
            if (m_playerOnPlatform) {
                float beltPulse = sin(m_animationTime * 6.0f) * 0.015f;
                m_currentDepth += beltPulse;
            } else {
                float beltFlow = sin(m_animationTime * 4.0f) * 0.008f;
                m_position.y += beltFlow;
            }
            break;

        case PlatformType::MOVING: {
            float moveOffset = sin(m_animationTime * m_moveSpeed * m_speedMultiplier) * m_moveRange;
            glm::vec3 basePos = m_originalPosition + m_moveDirection * moveOffset;
            basePos.y -= m_currentDepth;
            m_position = basePos;

            float hover = sin(m_animationTime * 2.0f) * 0.01f;
            m_position.y += hover;
            break;
        }

        case PlatformType::NORMAL:
        default:
            if (m_playerOnPlatform) {
                float gentlePulse = sin(m_animationTime * 3.0f) * 0.015f;
                float breathingEffect = cos(m_animationTime * 2.0f) * 0.01f;
                m_currentDepth += gentlePulse + breathingEffect;

                float liveliness = sin(m_animationTime * 4.0f) * 0.005f;
                m_position.x = m_originalPosition.x + liveliness;
                m_position.z = m_originalPosition.z + cos(m_animationTime * 3.5f) * 0.004f;
            } else {
                float gentleRebound = sin(m_animationTime * 5.0f) * 0.01f * exp(-m_animationTime * 3.0f);
                m_position.y += gentleRebound;
            }
            break;
    }
}

void Platform::Render(Renderer* renderer) {
    if (renderer && m_VAO != 0) {
        glm::mat4 modelMatrix = GetModelMatrix();
        int platformTypeInt = static_cast<int>(m_type);
        renderer->DrawMesh(m_VAO, modelMatrix, m_material, 60, false, true, platformTypeInt);
    }
}

bool Platform::CheckCollision(const glm::vec3& point, float radius) const {
    glm::vec3 halfSize = glm::vec3(m_size.x, m_size.y * m_collisionHeightScale, m_size.z) * 0.5f;
    glm::vec3 minBounds = m_position - halfSize;
    glm::vec3 maxBounds = m_position + halfSize;

    minBounds -= glm::vec3(radius);
    maxBounds += glm::vec3(radius);

    return (point.x >= minBounds.x && point.x <= maxBounds.x &&
            point.y >= minBounds.y && point.y <= maxBounds.y &&
            point.z >= minBounds.z && point.z <= maxBounds.z);
}

glm::vec3 Platform::GetSurfacePoint(const glm::vec3& point) const {
    glm::vec3 surfacePoint = point;
    surfacePoint.y = m_position.y + m_size.y * 0.5f;
    return surfacePoint;
}

void Platform::OnPlayerLand() {
    m_playerOnPlatform = true;
    SetPressed(true);

    if (m_type != PlatformType::MOVING) {
        m_animationTime = 0.0f;
    }
}

void Platform::OnPlayerLeave() {
    m_playerOnPlatform = false;
    SetPressed(false);

    if (m_type != PlatformType::MOVING) {
        m_animationTime = 0.0f;
    }
}

void Platform::SetPressed(bool pressed) {
    m_isPressed = pressed;
}

void Platform::ConfigureMovement(const glm::vec3& direction, float speed, float range) {
    if (glm::length(direction) > 0.001f) {
        m_moveDirection = glm::normalize(direction);
    }
    m_moveSpeed = std::max(0.1f, speed);
    m_moveRange = std::max(0.1f, range);
}

void Platform::SetSpeedMultiplier(float multiplier) {
    m_speedMultiplier = std::max(0.5f, multiplier);
}

void Platform::SetFixed(bool fixed) {
    m_isFixed = fixed;
    if (m_isFixed) {
        m_animationTime = 0.0f;
    }
}

void Platform::SetupMesh() {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    switch (m_type) {
        case PlatformType::SLIDE:
            vertices = Primitives::GenerateSlideTrack(m_size.z, m_size.x, m_size.y);
            indices = Primitives::GetCubeIndices();
            break;
        case PlatformType::BOOST:
        case PlatformType::BOOST_LEFT:
        case PlatformType::BOOST_RIGHT:
            vertices = Primitives::GenerateSlideTrack(m_size.z, m_size.x, m_size.y);
            indices = Primitives::GetCubeIndices();
            break;
        case PlatformType::BOUNCE:
            vertices = Primitives::GenerateBouncePlatform(m_size.x, m_size.z, m_size.y);
            indices = Primitives::GetCubeIndices();
            break;
        case PlatformType::MOVING:
            vertices = Primitives::GenerateTechPlatform(m_size.x, m_size.z, m_size.y);
            indices = Primitives::GetCubeIndices();
            break;
        default:
            vertices = Primitives::GeneratePlatform(m_size.x, m_size.z, m_size.y);
            indices = Primitives::GetCubeIndices();
            break;
    }

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

glm::mat4 Platform::GetModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, m_position);

    switch (m_type) {
        case PlatformType::BOUNCE:
            if (m_playerOnPlatform) {
                float bounce = sin(m_animationTime * 9.0f) * 0.03f + 1.0f;
                float squash = 1.0f - (m_currentDepth / m_pressDepth) * 0.06f;
                float stretch = 1.0f + (1.0f - squash) * 0.04f;
                float irregularX = 1.0f + sin(m_animationTime * 5.0f) * 0.01f;
                float irregularZ = 1.0f + cos(m_animationTime * 4.0f) * 0.008f;

                model = glm::scale(model, glm::vec3(stretch * irregularX,
                                                  squash * bounce,
                                                  stretch * irregularZ));

                float rotationAngle = sin(m_animationTime * 4.5f) * 0.01f;
                model = glm::rotate(model, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
            } else {
                float recovery = exp(-m_animationTime * 3.2f) * sin(m_animationTime * 9.0f) * 0.015f;
                float elasticX = 1.0f + recovery;
                float elasticY = 1.0f - recovery * 0.15f;
                float elasticZ = 1.0f + recovery * 0.08f;
                model = glm::scale(model, glm::vec3(elasticX, elasticY, elasticZ));
            }
            break;

        case PlatformType::SLIDE:
            if (m_isPressed) {
                float stretchLength = 1.0f + m_currentDepth * 0.6f;
                float compressHeight = 0.85f;
                float waveStretch = 1.0f + sin(m_animationTime * 3.0f) * 0.05f;

                model = glm::scale(model, glm::vec3(stretchLength * waveStretch,
                                                  compressHeight,
                                                  1.0f + sin(m_animationTime * 2.5f) * 0.03f));

                float bendAngle = sin(m_animationTime * 3.5f) * 0.015f;
                model = glm::rotate(model, bendAngle, glm::vec3(0.0f, 0.0f, 1.0f));
            } else {
                float elasticRebound = exp(-m_animationTime * 3.5f) * sin(m_animationTime * 8.0f) * 0.03f;
                model = glm::scale(model, glm::vec3(1.0f + elasticRebound,
                                                  1.0f - elasticRebound * 0.08f,
                                                  1.0f + elasticRebound * 0.03f));
            }
            break;

        case PlatformType::BOOST: {
        case PlatformType::BOOST_LEFT:
        case PlatformType::BOOST_RIGHT:
            float beltPulse = 1.0f + sin(m_animationTime * 4.0f) * 0.015f;
            model = glm::scale(model, glm::vec3(1.0f + m_currentDepth * 0.2f, beltPulse, beltPulse));
            break;
        }

        case PlatformType::MOVING: {
            if (m_playerOnPlatform) {
                float mechanicalCompress = 1.0f - sin(m_animationTime * 12.0f) * 0.02f;
                float vibrationScale = 1.0f + cos(m_animationTime * 10.0f) * 0.01f;

                model = glm::scale(model, glm::vec3(vibrationScale,
                                                  mechanicalCompress,
                                                  vibrationScale));
            }

            float moveStretch = 1.0f + abs(sin(m_animationTime * m_moveSpeed * m_speedMultiplier)) * 0.03f;
            model = glm::scale(model, glm::vec3(moveStretch, 1.0f, 1.0f));
            break;
        }

        case PlatformType::NORMAL:
        default:
            if (m_isPressed) {
                float compression = 1.0f - (m_currentDepth / m_pressDepth) * 0.08f;
                float expansion = 1.0f + (1.0f - compression) * 0.04f;
                model = glm::scale(model, glm::vec3(expansion, compression, expansion));
            } else if (m_playerOnPlatform) {
                float pulse = 1.0f + sin(m_animationTime * 2.5f) * 0.01f;
                model = glm::scale(model, glm::vec3(pulse, 1.0f, pulse));
            }
            break;
    }

    return model;
}

void Platform::Cleanup() {
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_EBO);
        m_VAO = m_VBO = m_EBO = 0;
    }
}
