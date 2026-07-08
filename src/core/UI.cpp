#include "UI.h"
#include "Input.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <vector>
#include <cmath>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

UI::UI() 
    : m_gameState(GameState::MENU)
    , m_currentScore(0)
    , m_stateTime(0.0f)
    , m_lastState(GameState::MENU)
    , m_lastBeatAccuracy(0.0f)
    , m_lastBeatRating(BeatRating::MISS)
    , m_ratingDisplayTime(0.0f)
    , m_ratingColor(0.8f, 0.85f, 0.9f)
    , m_rhythmCueIntensity(0.0f)
    , m_rhythmCueStrong(false)
    , m_rhythmCuePlatformType(-1)
    , m_lastScore(-1)
    , m_startPressed(false)
    , m_quitPressed(false)
    , m_settingsPressed(false)
    , m_rhythmPressed(false)
    , m_songConfirmPressed(false)
    , m_backPressed(false)
    , m_selectedSongIndex(0)
    , m_countdownTime(0.0f)
    , m_quadVAO(0)
    , m_quadVBO(0)
    , m_uiShader(nullptr)
    , m_textShader(nullptr)
    , m_viewportWidth(1280)
    , m_viewportHeight(720)
    , m_uiScale(1.0f)
    , m_uiOffset(0.0f, 0.0f) {
}

UI::~UI() {
    Cleanup();
}

bool UI::Initialize() {
    // 创建UI着色器
    CreateUIShader();
    CreateTextShader();
    
    // 创建用于渲染UI的四边形
    CreateQuadMesh();
    
    std::cout << "UI System initialized" << std::endl;
    return true;
}

void UI::Update(float deltaTime) {
    // UI更新逻辑
    if (m_gameState != m_lastState) {
        m_stateTime = 0.0f;
        m_lastState = m_gameState;
    }
    m_stateTime += deltaTime;

    if (m_gameState == GameState::COUNTDOWN) {
        m_countdownTime = std::max(0.0f, m_countdownTime - deltaTime);
    }
    
    // 可以在这里添加UI动画逻辑
}

void UI::Render(Renderer* renderer) {
    (void)renderer; // 避免未使用参数警�?
    
    if (!m_uiShader || m_quadVAO == 0) return;
    
    // 保存当前OpenGL状�?
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 使用UI着色器
    m_uiShader->Use();

    GLint viewport[4] = {0, 0, 1280, 720};
    if (glad_glGetIntegerv) {
        glad_glGetIntegerv(GL_VIEWPORT, viewport);
    }
    m_viewportWidth = std::max(1, viewport[2]);
    m_viewportHeight = std::max(1, viewport[3]);

    float scaleX = static_cast<float>(m_viewportWidth) / 1280.0f;
    float scaleY = static_cast<float>(m_viewportHeight) / 720.0f;
    m_uiScale = std::min(scaleX, scaleY);
    m_uiOffset = glm::vec2((m_viewportWidth - 1280.0f * m_uiScale) * 0.5f,
                           (m_viewportHeight - 720.0f * m_uiScale) * 0.5f);

    // Set orthographic projection
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(m_viewportWidth),
                                      static_cast<float>(m_viewportHeight), 0.0f, -1.0f, 1.0f);
    m_uiShader->SetMat4("projection", projection);
    
    // 根据游戏状态渲染不同的UI
    switch (m_gameState) {
        case GameState::MENU:
            RenderMenuUI();
            break;
        case GameState::SETTINGS:
            RenderSettingsUI();
            break;
        case GameState::SCENE_VIEW:
            RenderSceneViewUI();
            break;
        case GameState::SONG_SELECT:
            RenderSongSelectUI();
            break;
        case GameState::COUNTDOWN:
            RenderCountdownUI();
            break;
        case GameState::PLAYING:
            RenderGameUI();
            break;
        case GameState::GAME_OVER:
            RenderGameOverUI();
            break;
        default:
            break;
    }
    
    // 恢复OpenGL状�?
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void UI::Cleanup() {
    if (m_quadVAO != 0) {
        glDeleteVertexArrays(1, &m_quadVAO);
        glDeleteBuffers(1, &m_quadVBO);
        m_quadVAO = m_quadVBO = 0;
    }
    
    m_uiShader.reset();
    m_textShader.reset();

    for (auto& entry : m_textCache) {
        if (entry.second.textureId != 0) {
            glDeleteTextures(1, &entry.second.textureId);
        }
    }
    m_textCache.clear();

    if (m_scoreTexture.textureId != 0) {
        glDeleteTextures(1, &m_scoreTexture.textureId);
        m_scoreTexture = {};
    }
}

void UI::UpdateScore(int score, int combo) {
    m_currentScore = score;
    m_currentCombo = combo;

    if (m_currentScore != m_lastScore) {
        UpdateDynamicText(m_scoreTexture, std::to_string(m_currentScore), 28, true);
        m_lastScore = m_currentScore;
    }
    
    if (m_currentCombo != m_lastCombo) {
        UpdateDynamicText(m_comboTexture, "Combo: " + std::to_string(m_currentCombo), 24, false);
        m_lastCombo = m_currentCombo;
    }
}

// 新增：更新节拍准确度显示
void UI::UpdateBeatAccuracy(float accuracy, BeatRating rating) {
    m_lastBeatAccuracy = accuracy;
    m_lastBeatRating = rating;
    
    // 根据评级显示不同颜色的文�?
    std::string ratingText;
    glm::vec3 ratingColor;
    
    switch (rating) {
        case BeatRating::FLAWLESS:
            ratingText = "无瑕!";
            ratingColor = glm::vec3(1.0f, 0.2f, 0.8f); // 粉色
            break;
        case BeatRating::PERFECT:
            ratingText = "完美!";
            ratingColor = glm::vec3(1.0f, 0.8f, 0.2f); // 金色
            break;
        case BeatRating::GREAT:
            ratingText = "很好!";
            ratingColor = glm::vec3(0.2f, 1.0f, 0.2f); // 绿色
            break;
        case BeatRating::GOOD:
            ratingText = "良好";
            ratingColor = glm::vec3(0.2f, 0.8f, 1.0f); // 蓝色
            break;
        case BeatRating::MISS:
            ratingText = "错过";
            ratingColor = glm::vec3(1.0f, 0.2f, 0.2f); // 红色
            break;
    }
    
    // 更新评级显示纹理
    UpdateDynamicText(m_ratingTexture, ratingText, 24, false);
    m_ratingColor = ratingColor;
    m_ratingDisplayTime = 1.0f; // 显示1�?
}

void UI::SetRhythmCue(float intensity, bool strong, int platformType) {
    m_rhythmCueIntensity = std::clamp(intensity, 0.0f, 1.0f);
    m_rhythmCueStrong = strong;
    m_rhythmCuePlatformType = platformType;
}

void UI::StartCountdown(float seconds) {
    m_countdownTime = std::max(0.0f, seconds);
}

void UI::HandleInput(Input* input) {
    if (!input) return;

    bool anyKeyPressed = false;
    for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key) {
        if (input->IsKeyPressed(key)) {
            anyKeyPressed = true;
            break;
        }
    }

    bool mouseClick = input->IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
    glm::vec2 mouseUi = GetMouseUiPosition(input);
    auto inside = [](const glm::vec2& p, float x, float y, float w, float h) {
        return p.x >= x && p.x <= x + w && p.y >= y && p.y <= y + h;
    };

    switch (m_gameState) {
        case GameState::MENU: {
            if (input->IsKeyPressed(GLFW_KEY_1) || input->IsKeyPressed(GLFW_KEY_ENTER)) {
                m_startPressed = true;
            }
            if (input->IsKeyPressed(GLFW_KEY_2)) {
                m_rhythmPressed = true;
            }
            if (input->IsKeyPressed(GLFW_KEY_3)) {
                m_sceneViewButtonPressed = true;
            }
            if (input->IsKeyPressed(GLFW_KEY_S)) {
                m_settingsPressed = true;
            }
            if (input->IsKeyPressed(GLFW_KEY_ESCAPE)) {
                m_quitPressed = true;
            }
            if (mouseClick) {
                float buttonWidth = 520.0f;
                float buttonX = (1280.0f - buttonWidth) * 0.5f;
                float startY = 290.0f;
                float rhythmY = 350.0f;
                float sceneViewY = 410.0f;
                float settingsY = 470.0f;
                float quitY = 530.0f;
                if (inside(mouseUi, buttonX, startY, buttonWidth, 52.0f)) {
                    m_startPressed = true;
                } else if (inside(mouseUi, buttonX, rhythmY, buttonWidth, 52.0f)) {
                    m_rhythmPressed = true;
                } else if (inside(mouseUi, buttonX, sceneViewY, buttonWidth, 52.0f)) {
                    m_sceneViewButtonPressed = true;
                } else if (inside(mouseUi, buttonX, settingsY, buttonWidth, 52.0f)) {
                    m_settingsPressed = true;
                } else if (inside(mouseUi, buttonX, quitY, buttonWidth, 52.0f)) {
                    m_quitPressed = true;
                }
            }
            break;
        }
        case GameState::PLAYING:
            break;
        case GameState::SETTINGS: {
            if (input->IsKeyPressed(GLFW_KEY_ESCAPE)) {
                m_gameState = GameState::MENU;
                m_isDraggingSlider = false;
                break;
            }
            
            int itemCount = 9;
            float startY = 140.0f;
            float lineHeight = 45.0f;
            
            // 鼠标拖动滑块处理
            bool mouseDown = input->IsMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT);
            bool mousePressed = input->IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
            bool mouseReleased = input->IsMouseButtonReleased(GLFW_MOUSE_BUTTON_LEFT);
            
            if (mousePressed) {
                // 检查是否点击了滑块区域
                for (int i = 0; i < itemCount; ++i) {
                    float y = startY + i * lineHeight;
                    float sliderY = y - 5;  // 扩大Y检测范围
                    float sliderH = lineHeight;  // 使用整行高度
                    
                    // 球贴图选择 (i==0) 特殊处理：点击左右箭头或整行
                    if (i == 0) {
                        // 检查是否在整个第一行的Y范围内
                        if (mouseUi.y >= sliderY && mouseUi.y <= sliderY + sliderH) {
                            // 左半边区域 - 切换到上一个贴图
                            if (mouseUi.x >= m_sliderX && mouseUi.x <= m_sliderX + m_sliderWidth / 2) {
                                if (!m_ballTextureNames.empty()) {
                                    m_selectedBallTexture = (m_selectedBallTexture - 1 + static_cast<int>(m_ballTextureNames.size())) 
                                                            % static_cast<int>(m_ballTextureNames.size());
                                    m_ballTextureChanged = true;
                                }
                                m_settingsSelectedItem = 0;
                            }
                            // 右半边区域 - 切换到下一个贴图
                            else if (mouseUi.x >= m_sliderX + m_sliderWidth / 2 && mouseUi.x <= m_sliderX + m_sliderWidth + 30) {
                                if (!m_ballTextureNames.empty()) {
                                    m_selectedBallTexture = (m_selectedBallTexture + 1) % static_cast<int>(m_ballTextureNames.size());
                                    m_ballTextureChanged = true;
                                }
                                m_settingsSelectedItem = 0;
                            }
                        }
                        continue;
                    }
                    
                    // 判断是否是颜色滑块(3,4,5,7) - 注意索引偏移
                    bool isColorSlider = (i == 3 || i == 4 || i == 5 || i == 7);
                    
                    if (isColorSlider) {
                        // 颜色滑块有3个分量
                        float componentWidth = (m_sliderWidth - 20) / 3.0f;
                        for (int c = 0; c < 3; ++c) {
                            float compX = m_sliderX + c * (componentWidth + 10) + 15;
                            float compW = componentWidth - 15;
                            if (mouseUi.x >= compX && mouseUi.x <= compX + compW &&
                                mouseUi.y >= sliderY && mouseUi.y <= sliderY + sliderH) {
                                m_isDraggingSlider = true;
                                m_draggingSliderItem = i;
                                m_draggingSliderComponent = c;
                                m_settingsSelectedItem = i;
                                m_settingsSelectedComponent = c;
                                break;
                            }
                        }
                    } else {
                        // 普通滑块
                        if (mouseUi.x >= m_sliderX && mouseUi.x <= m_sliderX + m_sliderWidth &&
                            mouseUi.y >= sliderY && mouseUi.y <= sliderY + sliderH) {
                            m_isDraggingSlider = true;
                            m_draggingSliderItem = i;
                            m_draggingSliderComponent = -1;
                            m_settingsSelectedItem = i;
                            break;
                        }
                    }
                }
            }
            
            if (mouseReleased) {
                m_isDraggingSlider = false;
                m_draggingSliderItem = -1;
            }
            
            // 拖动时更新值
            if (m_isDraggingSlider && mouseDown && m_draggingSliderItem >= 1) {
                bool isColorSlider = (m_draggingSliderItem == 3 || m_draggingSliderItem == 4 || 
                                      m_draggingSliderItem == 5 || m_draggingSliderItem == 7);
                float percent;
                
                if (isColorSlider && m_draggingSliderComponent >= 0) {
                    float componentWidth = (m_sliderWidth - 20) / 3.0f;
                    float compX = m_sliderX + m_draggingSliderComponent * (componentWidth + 10) + 15;
                    float compW = componentWidth - 15;
                    percent = (mouseUi.x - compX) / compW;
                } else {
                    percent = (mouseUi.x - m_sliderX) / m_sliderWidth;
                }
                percent = std::clamp(percent, 0.0f, 1.0f);
                m_lightSettingsChanged = true;
                
                switch (m_draggingSliderItem) {
                    case 1: // 主光源高度 5-50
                        m_lightSettings.mainLightOffset.y = 5.0f + percent * 45.0f;
                        break;
                    case 2: // 主光源强度 0-2
                        m_lightSettings.mainLightIntensity = percent * 2.0f;
                        break;
                    case 3: // 环境光颜色
                        if (m_draggingSliderComponent >= 0)
                            m_lightSettings.mainLightAmbient[m_draggingSliderComponent] = percent;
                        break;
                    case 4: // 漫反射颜色
                        if (m_draggingSliderComponent >= 0)
                            m_lightSettings.mainLightDiffuse[m_draggingSliderComponent] = percent;
                        break;
                    case 5: // 镜面反射颜色
                        if (m_draggingSliderComponent >= 0)
                            m_lightSettings.mainLightSpecular[m_draggingSliderComponent] = percent;
                        break;
                    case 6: // 补光强度 0-1
                        m_lightSettings.playerLightIntensity = percent;
                        break;
                    case 7: // 补光颜色
                        if (m_draggingSliderComponent >= 0)
                            m_lightSettings.playerLightDiffuse[m_draggingSliderComponent] = percent;
                        break;
                    case 8: // 补光半径 1-10
                        m_lightSettings.playerLightRadius = 1.0f + percent * 9.0f;
                        break;
                }
            }
            
            // 上下选择设置项
            if (input->IsKeyPressed(GLFW_KEY_UP)) {
                m_settingsSelectedItem = (m_settingsSelectedItem - 1 + itemCount) % itemCount;
                m_settingsSelectedComponent = 0;
            }
            if (input->IsKeyPressed(GLFW_KEY_DOWN)) {
                m_settingsSelectedItem = (m_settingsSelectedItem + 1) % itemCount;
                m_settingsSelectedComponent = 0;
            }
            
            // Tab切换RGB分量
            if (input->IsKeyPressed(GLFW_KEY_TAB)) {
                m_settingsSelectedComponent = (m_settingsSelectedComponent + 1) % 3;
            }
            
            // R键重置
            if (input->IsKeyPressed(GLFW_KEY_R)) {
                m_lightSettings = LightSettings(); // 重置为默认值
                m_lightSettingsChanged = true;
            }
            
            // 左右调整值
            float adjustSpeed = input->IsKeyDown(GLFW_KEY_LEFT_SHIFT) ? 0.1f : 0.02f;
            bool leftPressed = input->IsKeyDown(GLFW_KEY_LEFT);
            bool rightPressed = input->IsKeyDown(GLFW_KEY_RIGHT);
            
            if (leftPressed || rightPressed) {
                // 球贴图选择特殊处理
                if (m_settingsSelectedItem == 0) {
                    if (input->IsKeyPressed(GLFW_KEY_LEFT) || input->IsKeyPressed(GLFW_KEY_RIGHT)) {
                        if (!m_ballTextureNames.empty()) {
                            int delta = rightPressed ? 1 : -1;
                            m_selectedBallTexture = (m_selectedBallTexture + delta + static_cast<int>(m_ballTextureNames.size())) 
                                                    % static_cast<int>(m_ballTextureNames.size());
                            m_ballTextureChanged = true;
                        }
                    }
                } else {
                    float delta = rightPressed ? adjustSpeed : -adjustSpeed;
                    m_lightSettingsChanged = true;
                    
                    switch (m_settingsSelectedItem) {
                        case 1: // 主光源高度
                            m_lightSettings.mainLightOffset.y = std::clamp(m_lightSettings.mainLightOffset.y + delta * 50.0f, 5.0f, 50.0f);
                            break;
                        case 2: // 主光源强度
                            m_lightSettings.mainLightIntensity = std::clamp(m_lightSettings.mainLightIntensity + delta * 2.0f, 0.0f, 2.0f);
                            break;
                        case 3: { // 环境光颜色
                            float* comp = &m_lightSettings.mainLightAmbient[m_settingsSelectedComponent];
                            *comp = std::clamp(*comp + delta, 0.0f, 1.0f);
                            break;
                        }
                        case 4: { // 漫反射颜色
                            float* comp = &m_lightSettings.mainLightDiffuse[m_settingsSelectedComponent];
                            *comp = std::clamp(*comp + delta, 0.0f, 1.0f);
                            break;
                        }
                        case 5: { // 镜面反射颜色
                            float* comp = &m_lightSettings.mainLightSpecular[m_settingsSelectedComponent];
                            *comp = std::clamp(*comp + delta, 0.0f, 1.0f);
                            break;
                        }
                        case 6: // 补光强度
                            m_lightSettings.playerLightIntensity = std::clamp(m_lightSettings.playerLightIntensity + delta, 0.0f, 1.0f);
                            break;
                        case 7: { // 补光颜色
                            float* comp = &m_lightSettings.playerLightDiffuse[m_settingsSelectedComponent];
                            *comp = std::clamp(*comp + delta, 0.0f, 1.0f);
                            break;
                        }
                        case 8: // 补光半径
                            m_lightSettings.playerLightRadius = std::clamp(m_lightSettings.playerLightRadius + delta * 10.0f, 1.0f, 10.0f);
                            break;
                    }
                }
            }
            
            // 滚轮切换小球贴图（当选中第0项时）
            float scrollDelta = input->GetScrollDelta();
            if (m_settingsSelectedItem == 0 && std::abs(scrollDelta) > 0.01f) {
                if (!m_ballTextureNames.empty()) {
                    int delta = scrollDelta > 0 ? -1 : 1;  // 滚轮向上 = 上一个贴图
                    m_selectedBallTexture = (m_selectedBallTexture + delta + static_cast<int>(m_ballTextureNames.size())) 
                                            % static_cast<int>(m_ballTextureNames.size());
                    m_ballTextureChanged = true;
                }
            }
            break;
        }
        case GameState::SONG_SELECT: {
            if (input->IsKeyPressed(GLFW_KEY_ESCAPE) || input->IsKeyPressed(GLFW_KEY_BACKSPACE)) {
                m_backPressed = true;
                break;
            }
            if (!m_songList.empty()) {
                int totalCount = static_cast<int>(m_songList.size());
                if (input->IsKeyPressed(GLFW_KEY_UP)) {
                    m_selectedSongIndex = (m_selectedSongIndex - 1 + totalCount) % totalCount;
                }
                if (input->IsKeyPressed(GLFW_KEY_DOWN)) {
                    m_selectedSongIndex = (m_selectedSongIndex + 1) % totalCount;
                }
                if (input->IsKeyPressed(GLFW_KEY_ENTER) || input->IsKeyPressed(GLFW_KEY_SPACE)) {
                    m_songConfirmPressed = true;
                }
                if (mouseClick) {
                    float panelWidth = 860.0f;
                    float panelX = (1280.0f - panelWidth) * 0.5f;
                    float panelY = 110.0f;
                    int visibleCount = std::min(totalCount, 8);
                    int startIndex = 0;
                    if (m_selectedSongIndex >= visibleCount) {
                        startIndex = m_selectedSongIndex - visibleCount + 1;
                    }
                    float listX = panelX + 80.0f;
                    float listY = panelY + 90.0f;
                    float listWidth = panelWidth - 160.0f;
                    float itemHeight = 42.0f;
                    float itemGap = 8.0f;
                    for (int i = 0; i < visibleCount; ++i) {
                        int index = startIndex + i;
                        float y = listY + i * (itemHeight + itemGap);
                        if (inside(mouseUi, listX, y, listWidth, itemHeight)) {
                            m_selectedSongIndex = index;
                            m_songConfirmPressed = true;
                            break;
                        }
                    }
                }
            }
            break;
        }
        case GameState::COUNTDOWN:
            break;
        case GameState::GAME_OVER:
            if (anyKeyPressed || mouseClick) {
                m_startPressed = true;
            }
            break;
        default:
            break;
    }
}

void UI::ResetButtons() {
    m_startPressed = false;
    m_quitPressed = false;
    m_settingsPressed = false;
    m_rhythmPressed = false;
    m_songConfirmPressed = false;
    m_backPressed = false;
}

void UI::SetSongList(const std::vector<std::string>& songs) {
    m_songList = songs;
    if (m_songList.empty()) {
        m_selectedSongIndex = 0;
        return;
    }
    m_selectedSongIndex = std::clamp(m_selectedSongIndex, 0, static_cast<int>(m_songList.size() - 1));
}



void UI::CreateUIShader() {
    // 创建简单的UI着色器
    std::string vertexShader = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoord;
        
        out vec2 TexCoord;
        
        uniform mat4 projection;
        uniform mat4 model;
        
        void main() {
            gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )";
    
    std::string fragmentShader = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec2 TexCoord;
        
        uniform vec3 color;
        uniform float alpha;
        
        void main() {
            // 简单的渐变效果
            float gradient = 1.0 - length(TexCoord - vec2(0.5));
            FragColor = vec4(color, alpha * gradient);
        }
    )";
    
    m_uiShader = std::make_unique<Shader>();
    m_uiShader->LoadFromString(vertexShader, fragmentShader);
}

void UI::CreateTextShader() {
    std::string vertexShader = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoord;

        out vec2 TexCoord;

        uniform mat4 projection;
        uniform mat4 model;

        void main() {
            gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )";

    std::string fragmentShader = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;

        uniform sampler2D fontTexture;
        uniform vec3 color;
        uniform float alpha;

        void main() {
            float a = texture(fontTexture, TexCoord).a;
            FragColor = vec4(color, alpha * a);
        }
    )";

    m_textShader = std::make_unique<Shader>();
    m_textShader->LoadFromString(vertexShader, fragmentShader);
}

UI::TextTexture UI::CreateTextTexture(const std::string& text, int fontSize, bool bold) {
    TextTexture result;

#if defined(_WIN32)
    if (text.empty()) {
        return result;
    }

    auto utf8ToWide = [](const std::string& input) -> std::wstring {
        if (input.empty()) return L"";
        int length = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, nullptr, 0);
        if (length <= 0) return L"";
        std::wstring wide(static_cast<size_t>(length), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, &wide[0], length);
        if (!wide.empty() && wide.back() == L'\0') {
            wide.pop_back();
        }
        return wide;
    };

    std::wstring wideText = utf8ToWide(text);
    if (wideText.empty()) {
        return result;
    }

    HDC hdc = CreateCompatibleDC(nullptr);
    if (!hdc) {
        return result;
    }

    int weight = bold ? FW_BOLD : FW_NORMAL;
    HFONT font = CreateFontW(-fontSize, 0, 0, 0, weight, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                             L"SimSun");
    if (!font) {
        DeleteDC(hdc);
        return result;
    }

    HGDIOBJ oldFont = SelectObject(hdc, font);

    RECT calcRect = {0, 0, 1, 1};
    DrawTextW(hdc, wideText.c_str(), static_cast<int>(wideText.size()), &calcRect,
              DT_CALCRECT | DT_LEFT | DT_NOPREFIX);

    int width = std::max(1L, calcRect.right - calcRect.left + 8L);
    int height = std::max(1L, calcRect.bottom - calcRect.top + 8L);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP dib = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!dib || !bits) {
        SelectObject(hdc, oldFont);
        DeleteObject(font);
        DeleteDC(hdc);
        return result;
    }

    HGDIOBJ oldBmp = SelectObject(hdc, dib);
    RECT rect = {0, 0, width, height};
    HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));
    DrawTextW(hdc, wideText.c_str(), static_cast<int>(wideText.size()), &rect,
              DT_LEFT | DT_NOPREFIX);

    std::vector<unsigned char> rgba(static_cast<size_t>(width) * height * 4, 0);
    unsigned char* src = static_cast<unsigned char*>(bits);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            size_t idx = static_cast<size_t>((y * width + x) * 4);
            unsigned char b = src[idx + 0];
            unsigned char g = src[idx + 1];
            unsigned char r = src[idx + 2];
            unsigned char a = std::max(r, std::max(g, b));
            rgba[idx + 0] = 255;
            rgba[idx + 1] = 255;
            rgba[idx + 2] = 255;
            rgba[idx + 3] = a;
        }
    }

    glGenTextures(1, &result.textureId);
    glBindTexture(GL_TEXTURE_2D, result.textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());

    result.width = width;
    result.height = height;

    SelectObject(hdc, oldBmp);
    SelectObject(hdc, oldFont);
    DeleteObject(dib);
    DeleteObject(font);
    DeleteDC(hdc);
#endif

    return result;
}

const UI::TextTexture* UI::GetTextTexture(const std::string& text, int fontSize, bool bold) {
    if (text.empty()) {
        return nullptr;
    }

    std::string key = text + "|" + std::to_string(fontSize) + (bold ? "|b" : "|r");
    auto it = m_textCache.find(key);
    if (it != m_textCache.end()) {
        return &it->second;
    }

    TextTexture texture = CreateTextTexture(text, fontSize, bold);
    if (texture.textureId == 0) {
        return nullptr;
    }

    auto inserted = m_textCache.emplace(std::move(key), texture);
    return &inserted.first->second;
}

void UI::UpdateDynamicText(TextTexture& target, const std::string& text, int fontSize, bool bold) {
    if (target.textureId != 0) {
        glDeleteTextures(1, &target.textureId);
        target.textureId = 0;
        target.width = 0;
        target.height = 0;
    }

    target = CreateTextTexture(text, fontSize, bold);
}

void UI::RenderTextTexture(const TextTexture& texture, float x, float y, const glm::vec3& color, float alpha) {
    if (!m_textShader || texture.textureId == 0 || m_quadVAO == 0) return;

    float drawX = m_uiOffset.x + x * m_uiScale;
    float drawY = m_uiOffset.y + y * m_uiScale;
    float drawWidth = static_cast<float>(texture.width) * m_uiScale;
    float drawHeight = static_cast<float>(texture.height) * m_uiScale;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(drawX, drawY, 0.0f));
    model = glm::scale(model, glm::vec3(drawWidth, drawHeight, 1.0f));

    m_textShader->Use();
    m_textShader->SetMat4("projection", glm::ortho(0.0f, static_cast<float>(m_viewportWidth),
                                                   static_cast<float>(m_viewportHeight), 0.0f, -1.0f, 1.0f));
    m_textShader->SetMat4("model", model);
    m_textShader->SetVec3("color", color);
    m_textShader->SetFloat("alpha", alpha);
    m_textShader->SetInt("fontTexture", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.textureId);

    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void UI::RenderText(const std::string& text, float x, float y, int fontSize, const glm::vec3& color, float alpha, bool bold) {
    const TextTexture* texture = GetTextTexture(text, fontSize, bold);
    if (!texture) return;
    RenderTextTexture(*texture, x, y, color, alpha);
}

void UI::RenderTextCentered(const std::string& text, float centerX, float centerY, int fontSize, const glm::vec3& color, float alpha, bool bold) {
    const TextTexture* texture = GetTextTexture(text, fontSize, bold);
    if (!texture) return;
    float x = centerX - texture->width * 0.5f;
    float y = centerY - texture->height * 0.5f;
    RenderTextTexture(*texture, x, y, color, alpha);
}

void UI::CreateQuadMesh() {
    // 创建用于UI渲染的四边形
    float vertices[] = {
        // 位置      // 纹理坐标
        0.0f, 1.0f,  0.0f, 1.0f,
        1.0f, 0.0f,  1.0f, 0.0f,
        0.0f, 0.0f,  0.0f, 0.0f,
        
        0.0f, 1.0f,  0.0f, 1.0f,
        1.0f, 1.0f,  1.0f, 1.0f,
        1.0f, 0.0f,  1.0f, 0.0f
    };
    
    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // 位置属�?
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 纹理坐标属�?
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

void UI::RenderQuad(float x, float y, float width, float height, glm::vec3 color, float alpha) {
    if (!m_uiShader || m_quadVAO == 0) return;
    
    float drawX = m_uiOffset.x + x * m_uiScale;
    float drawY = m_uiOffset.y + y * m_uiScale;
    float drawWidth = width * m_uiScale;
    float drawHeight = height * m_uiScale;

    // 创建模型矩阵
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(drawX, drawY, 0.0f));
    model = glm::scale(model, glm::vec3(drawWidth, drawHeight, 1.0f));
    
    // 设置uniform变量
    m_uiShader->Use();
    m_uiShader->SetMat4("model", model);
    m_uiShader->SetVec3("color", color);
    m_uiShader->SetFloat("alpha", alpha);
    
    // 渲染四边�?
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void UI::RenderMenuUI() {
    float t = m_stateTime;

    glm::vec3 base = glm::vec3(0.06f, 0.08f, 0.12f);
    glm::vec3 glow = glm::vec3(0.08f, 0.12f, 0.18f);
    RenderQuad(0, 0, 1280, 720, base, 1.0f);
    RenderQuad(0, 0, 1280, 720, glow, 0.65f);

    float panelWidth = 1000.0f;
    float panelHeight = 560.0f;
    float panelX = (1280.0f - panelWidth) * 0.5f;
    float panelY = 80.0f;

    RenderQuad(panelX, panelY, panelWidth, panelHeight, glm::vec3(0.10f, 0.13f, 0.18f), 0.92f);
    RenderQuad(panelX, panelY, panelWidth, 6, glm::vec3(0.18f, 0.64f, 1.0f), 0.9f);
    RenderQuad(panelX, panelY, 6, panelHeight, glm::vec3(0.18f, 0.64f, 1.0f), 0.9f);

    float startPulse = 1.0f + sin(t * 3.0f) * 0.08f;
    float buttonWidth = 520.0f;
    float buttonX = (1280.0f - buttonWidth) * 0.5f;
    float startY = 290.0f;
    float rhythmY = 350.0f;
    float sceneViewY = 410.0f;  // 场景漫游按钮
    float settingsY = 470.0f;
    float quitY = 530.0f;

    RenderQuad(buttonX, startY, buttonWidth, 52, glm::vec3(0.12f, 0.72f, 0.45f), 0.85f * startPulse);
    RenderQuad(buttonX, rhythmY, buttonWidth, 52, glm::vec3(0.22f, 0.65f, 0.78f), 0.85f);
    RenderQuad(buttonX, sceneViewY, buttonWidth, 52, glm::vec3(0.65f, 0.55f, 0.22f), 0.85f);
    RenderQuad(buttonX, settingsY, buttonWidth, 52, glm::vec3(0.35f, 0.45f, 0.85f), 0.85f);
    RenderQuad(buttonX, quitY, buttonWidth, 52, glm::vec3(0.75f, 0.25f, 0.25f), 0.85f);

    for (int i = 0; i < 7; i++) {
        float x = 160 + i * 150 + sin(t + i) * 30;
        float y = 620 + sin(t * 2.0f + i) * 20;
        float size = 10 + sin(t * 3.0f + i) * 6;
        RenderQuad(x, y, size, size, glm::vec3(0.9f, 0.95f, 1.0f), 0.25f);
    }

    RenderTextCentered(u8"\u97f3\u4e50\u8df3\u8dc33D", 640, 130, 32, glm::vec3(0.85f, 0.95f, 1.0f), 1.0f, true);
    RenderTextCentered(u8"\u6a21\u5f0f\u9009\u62e9", 640, 174, 42, glm::vec3(0.35f, 0.80f, 1.0f), 1.0f, true);
    RenderTextCentered(u8"A/D \u79fb\u52a8  \u7a7a\u683c \u8df3\u8dc3\uff08\u666e\u901a\u6a21\u5f0f\uff09", 640, 220, 18, glm::vec3(0.85f, 0.9f, 0.95f), 0.9f);
    RenderTextCentered(u8"\u8282\u594f\u6a21\u5f0f\uff1aA/D\u79fb\u52a8  \u7a7a\u683c\u8df3  W\u51b2\u523a", 640, 246, 18, glm::vec3(0.85f, 0.9f, 0.95f), 0.9f);

    RenderTextCentered(u8"\u666e\u901a\u6a21\u5f0f", 640, startY + 26, 22, glm::vec3(0.95f, 1.0f, 0.95f), 1.0f, true);
    RenderTextCentered(u8"\u8282\u594f\u6a21\u5f0f", 640, rhythmY + 26, 22, glm::vec3(0.92f, 0.98f, 1.0f), 1.0f, true);
    RenderTextCentered(u8"\u573a\u666f\u6f2b\u6e38", 640, sceneViewY + 26, 22, glm::vec3(1.0f, 0.95f, 0.85f), 1.0f, true);
    RenderTextCentered(u8"\u8bbe\u7f6e", 640, settingsY + 26, 22, glm::vec3(0.95f, 0.97f, 1.0f), 0.95f, true);
    RenderTextCentered(u8"\u9000\u51fa", 640, quitY + 26, 22, glm::vec3(1.0f, 0.95f, 0.95f), 0.95f, true);

    RenderTextCentered(u8"\u6309 1/2/3 \u6216\u70b9\u51fb\u9009\u62e9\u6a21\u5f0f", 640, 605, 18, glm::vec3(0.75f, 0.85f, 0.95f), 0.9f);
}

void UI::RenderSettingsUI() {
    RenderQuad(0, 0, 1280, 720, glm::vec3(0.07f, 0.09f, 0.14f), 0.95f);
    RenderQuad(140, 60, 1000, 600, glm::vec3(0.11f, 0.14f, 0.20f), 0.92f);
    RenderQuad(140, 60, 1000, 6, glm::vec3(0.35f, 0.6f, 1.0f), 0.9f);

    RenderTextCentered(u8"游戏设置", 640, 100, 36, glm::vec3(0.85f, 0.95f, 1.0f), 1.0f, true);
    
    float startY = 140.0f;
    float lineHeight = 45.0f;
    float labelX = 180.0f;
    float sliderX = 420.0f;
    float sliderWidth = 280.0f;
    
    // 设置项列表 - 添加球贴图选择
    const char* labels[] = {
        u8"小球贴图",
        u8"主光源高度",
        u8"主光源强度", 
        u8"环境光颜色",
        u8"漫反射颜色",
        u8"镜面反射颜色",
        u8"补光强度",
        u8"补光颜色",
        u8"补光半径"
    };
    
    int itemCount = 9;
    
    for (int i = 0; i < itemCount; ++i) {
        float y = startY + i * lineHeight;
        bool selected = (m_settingsSelectedItem == i);
        
        // 选中高亮
        if (selected) {
            RenderQuad(160, y - 5, 960, 40, glm::vec3(0.2f, 0.4f, 0.6f), 0.3f);
        }
        
        // 标签
        glm::vec3 textColor = selected ? glm::vec3(0.95f, 0.98f, 1.0f) : glm::vec3(0.7f, 0.8f, 0.9f);
        RenderText(labels[i], labelX, y, 20, textColor, 0.95f, selected);
        
        // 根据不同的设置项渲染不同的控件
        switch (i) {
            case 0: { // 小球贴图选择
                // 显示当前选中的贴图名称
                std::string textureName = "默认";
                if (!m_ballTextureNames.empty() && m_selectedBallTexture >= 0 && 
                    m_selectedBallTexture < static_cast<int>(m_ballTextureNames.size())) {
                    textureName = m_ballTextureNames[m_selectedBallTexture];
                }
                // 左右箭头指示
                RenderText(u8"◀", sliderX, y, 20, selected ? glm::vec3(0.5f, 0.8f, 1.0f) : glm::vec3(0.5f, 0.5f, 0.5f), 0.9f);
                RenderTextCentered(textureName, sliderX + sliderWidth / 2 + 15, y, 18, 
                                   selected ? glm::vec3(1.0f, 0.95f, 0.8f) : glm::vec3(0.8f, 0.8f, 0.8f), 0.95f);
                RenderText(u8"▶", sliderX + sliderWidth, y, 20, selected ? glm::vec3(0.5f, 0.8f, 1.0f) : glm::vec3(0.5f, 0.5f, 0.5f), 0.9f);
                break;
            }
            case 1: { // 主光源高度
                float value = m_lightSettings.mainLightOffset.y;
                RenderSlider(sliderX, y, sliderWidth, value, 5.0f, 50.0f, glm::vec3(0.4f, 0.7f, 1.0f), selected);
                char buf[32];
                snprintf(buf, sizeof(buf), "%.1f", value);
                RenderText(buf, sliderX + sliderWidth + 20, y, 18, textColor, 0.9f);
                break;
            }
            case 2: { // 主光源强度
                float value = m_lightSettings.mainLightIntensity;
                RenderSlider(sliderX, y, sliderWidth, value, 0.0f, 2.0f, glm::vec3(1.0f, 0.9f, 0.4f), selected);
                char buf[32];
                snprintf(buf, sizeof(buf), "%.2f", value);
                RenderText(buf, sliderX + sliderWidth + 20, y, 18, textColor, 0.9f);
                break;
            }
            case 3: { // 环境光颜色
                RenderColorSlider(sliderX, y, sliderWidth, m_lightSettings.mainLightAmbient, 
                                  selected ? m_settingsSelectedComponent : -1, selected);
                break;
            }
            case 4: { // 漫反射颜色
                RenderColorSlider(sliderX, y, sliderWidth, m_lightSettings.mainLightDiffuse,
                                  selected ? m_settingsSelectedComponent : -1, selected);
                break;
            }
            case 5: { // 镜面反射颜色
                RenderColorSlider(sliderX, y, sliderWidth, m_lightSettings.mainLightSpecular,
                                  selected ? m_settingsSelectedComponent : -1, selected);
                break;
            }
            case 6: { // 补光强度
                float value = m_lightSettings.playerLightIntensity;
                RenderSlider(sliderX, y, sliderWidth, value, 0.0f, 1.0f, glm::vec3(0.4f, 0.8f, 1.0f), selected);
                char buf[32];
                snprintf(buf, sizeof(buf), "%.2f", value);
                RenderText(buf, sliderX + sliderWidth + 20, y, 18, textColor, 0.9f);
                break;
            }
            case 7: { // 补光颜色
                RenderColorSlider(sliderX, y, sliderWidth, m_lightSettings.playerLightDiffuse,
                                  selected ? m_settingsSelectedComponent : -1, selected);
                break;
            }
            case 8: { // 补光半径
                float value = m_lightSettings.playerLightRadius;
                RenderSlider(sliderX, y, sliderWidth, value, 1.0f, 10.0f, glm::vec3(0.6f, 0.9f, 0.6f), selected);
                char buf[32];
                snprintf(buf, sizeof(buf), "%.1f", value);
                RenderText(buf, sliderX + sliderWidth + 20, y, 18, textColor, 0.9f);
                break;
            }
        }
    }
    
    // 预览区域
    RenderQuad(760, 150, 340, 240, glm::vec3(0.08f, 0.1f, 0.14f), 0.8f);
    RenderTextCentered(u8"光照预览", 930, 170, 18, glm::vec3(0.7f, 0.8f, 0.9f), 0.9f);
    
    // 绘制简单的预览球体（用颜色块表示）
    float previewCenterX = 930.0f;
    float previewCenterY = 270.0f;
    float previewRadius = 55.0f;
    
    // 模拟光照效果的渐变
    glm::vec3 ambient = m_lightSettings.mainLightAmbient * m_lightSettings.mainLightIntensity;
    glm::vec3 diffuse = m_lightSettings.mainLightDiffuse * m_lightSettings.mainLightIntensity;
    glm::vec3 combined = ambient * 0.3f + diffuse * 0.7f;
    
    RenderQuad(previewCenterX - previewRadius, previewCenterY - previewRadius, 
               previewRadius * 2, previewRadius * 2, combined, 0.9f);
    
    // 补光效果叠加
    glm::vec3 playerColor = m_lightSettings.playerLightDiffuse * m_lightSettings.playerLightIntensity;
    RenderQuad(previewCenterX - previewRadius * 0.5f, previewCenterY - previewRadius * 0.5f,
               previewRadius, previewRadius, playerColor, 0.4f);
    
    // 操作提示
    RenderTextCentered(u8"↑↓选择  ←→调整  鼠标拖动滑块  Tab切换RGB  R重置  ESC返回", 640, 620, 18, glm::vec3(0.75f, 0.85f, 0.95f), 0.85f);
}

void UI::RenderSceneViewUI() {
    // 只渲染帮助提示，不遮挡3D场景
    // 半透明背景条
    RenderQuad(0, 0, 1280, 50, glm::vec3(0.05f, 0.07f, 0.1f), 0.75f);
    RenderQuad(0, 670, 1280, 50, glm::vec3(0.05f, 0.07f, 0.1f), 0.75f);
    
    // 标题
    RenderTextCentered(u8"场景漫游模式", 640, 18, 24, glm::vec3(0.85f, 0.95f, 1.0f), 1.0f, true);
    
    // 操作提示
    RenderTextCentered(u8"WASD/方向键: 移动  Q/E: 上下  鼠标移动: 转向  滚轮: 缩放  F: 重置  ESC: 返回", 
                       640, 693, 18, glm::vec3(0.75f, 0.85f, 0.95f), 0.9f);
}

void UI::RenderSlider(float x, float y, float width, float value, float minVal, float maxVal, const glm::vec3& color, bool selected) {
    // 滑块背景
    RenderQuad(x, y + 8, width, 14, glm::vec3(0.15f, 0.18f, 0.25f), 0.8f);
    
    // 滑块填充
    float percent = (value - minVal) / (maxVal - minVal);
    percent = std::clamp(percent, 0.0f, 1.0f);
    float fillWidth = width * percent;
    RenderQuad(x, y + 8, fillWidth, 14, color, selected ? 0.9f : 0.7f);
    
    // 滑块手柄
    float handleX = x + fillWidth - 6;
    RenderQuad(handleX, y + 4, 12, 22, glm::vec3(0.95f, 0.95f, 1.0f), selected ? 1.0f : 0.8f);
}

void UI::RenderColorSlider(float x, float y, float width, const glm::vec3& color, int selectedComponent, bool isSelected) {
    float componentWidth = (width - 20) / 3.0f;
    
    const char* labels[] = {"R", "G", "B"};
    glm::vec3 colors[] = {
        glm::vec3(1.0f, 0.3f, 0.3f),
        glm::vec3(0.3f, 1.0f, 0.3f),
        glm::vec3(0.3f, 0.6f, 1.0f)
    };
    float values[] = {color.r, color.g, color.b};
    
    for (int i = 0; i < 3; ++i) {
        float compX = x + i * (componentWidth + 10);
        bool compSelected = isSelected && (selectedComponent == i);
        
        // 组件标签
        RenderText(labels[i], compX, y - 2, 14, colors[i], 0.8f);
        
        // 小滑块
        RenderQuad(compX + 15, y + 8, componentWidth - 15, 12, glm::vec3(0.12f, 0.15f, 0.2f), 0.8f);
        float fillWidth = (componentWidth - 15) * std::clamp(values[i], 0.0f, 1.0f);
        RenderQuad(compX + 15, y + 8, fillWidth, 12, colors[i], compSelected ? 0.95f : 0.7f);
        
        // 选中边框
        if (compSelected) {
            RenderQuad(compX + 13, y + 6, componentWidth - 11, 2, glm::vec3(1.0f, 1.0f, 1.0f), 0.8f);
            RenderQuad(compX + 13, y + 18, componentWidth - 11, 2, glm::vec3(1.0f, 1.0f, 1.0f), 0.8f);
        }
    }
    
    // 颜色预览
    RenderQuad(x + width + 10, y + 4, 24, 24, color, 0.95f);
}

void UI::RenderCountdownUI() {
    RenderQuad(0, 0, 1280, 720, glm::vec3(0.02f, 0.03f, 0.05f), 0.35f);

    int count = static_cast<int>(std::ceil(m_countdownTime));
    if (count < 1) {
        count = 1;
    }

    std::string countdownText = std::to_string(count);
    float pulse = 1.0f + sin(m_stateTime * 6.0f) * 0.08f;
    RenderTextCentered(countdownText, 640, 320, 120, glm::vec3(0.85f, 0.95f, 1.0f), pulse, true);
    RenderTextCentered(u8"\u51c6\u5907", 640, 420, 28, glm::vec3(0.75f, 0.85f, 0.95f), 0.9f);
}


void UI::RenderGameUI() {
    RenderQuad(24, 18, 360, 92, glm::vec3(0.02f, 0.03f, 0.05f), 0.75f);
    RenderQuad(24, 18, 360, 4, glm::vec3(0.2f, 0.6f, 0.95f), 0.9f);

    RenderText(u8"\u5206\u6570", 40, 30, 20, glm::vec3(0.8f, 0.9f, 1.0f), 0.9f);
    
    if (m_scoreTexture.textureId != 0) {
        RenderTextTexture(m_scoreTexture, 115, 22, glm::vec3(0.95f, 0.98f, 1.0f), 1.0f);
    } else {
        RenderText(std::to_string(m_currentScore), 115, 22, 26, glm::vec3(0.95f, 0.98f, 1.0f), 1.0f, true);
    }

}


void UI::RenderGameOverUI() {
    float gameOverTime = m_stateTime;

    float fadeAlpha = std::min(gameOverTime * 0.5f, 0.9f);
    RenderQuad(0, 0, 1280, 720, glm::vec3(0.05f, 0.02f, 0.02f), fadeAlpha);

    float panelScale = std::min(gameOverTime * 2.0f, 1.0f);
    float panelWidth = 720 * panelScale;
    float panelHeight = 340 * panelScale;
    float panelX = (1280 - panelWidth) * 0.5f;
    float panelY = (720 - panelHeight) * 0.5f;

    RenderQuad(panelX, panelY, panelWidth, panelHeight, glm::vec3(0.10f, 0.12f, 0.16f), 0.95f);
    RenderQuad(panelX, panelY, panelWidth, 6, glm::vec3(0.9f, 0.35f, 0.35f), 0.9f);
    RenderQuad(panelX, panelY + panelHeight - 4, panelWidth, 4, glm::vec3(0.9f, 0.35f, 0.35f), 0.6f);

    if (panelScale >= 1.0f) {
        RenderTextCentered(u8"\u6e38\u620f\u7ed3\u675f", 640, panelY + 60, 42, glm::vec3(0.95f, 0.9f, 0.9f), 1.0f, true);

        std::string scoreText = std::string(u8"\u6700\u7ec8\u5206\u6570\uff1a") + std::to_string(m_currentScore);
        RenderTextCentered(scoreText, 640, panelY + 135, 26, glm::vec3(0.75f, 0.85f, 0.95f), 0.95f);

        std::string praise;
        if (m_currentScore >= 1000) {
            praise = u8"\u97f3\u4e50\u5927\u5e08\uff01\u8282\u594f\u5b8c\u7f8e\uff01";
        } else if (m_currentScore >= 600) {
            praise = u8"\u8282\u594f\u5f88\u68d2\uff01\u7ee7\u7eed\u52a0\u6cb9\uff01";
        } else if (m_currentScore >= 300) {
            praise = u8"\u4e0d\u9519\u7684\u5f00\u59cb\uff01\u591a\u591a\u7ec3\u4e60\uff01";
        } else {
            praise = u8"\u7ee7\u7eed\u52aa\u529b\uff01\u719f\u80fd\u751f\u5de7\uff01";
        }
        RenderTextCentered(praise, 640, panelY + 205, 22, glm::vec3(0.9f, 0.85f, 0.75f), 0.9f);
    }
}

void UI::RenderSongSelectUI() {
    RenderQuad(0, 0, 1280, 720, glm::vec3(0.06f, 0.08f, 0.12f), 1.0f);

    float panelWidth = 860.0f;
    float panelHeight = 520.0f;
    float panelX = (1280.0f - panelWidth) * 0.5f;
    float panelY = 110.0f;

    RenderQuad(panelX, panelY, panelWidth, panelHeight, glm::vec3(0.10f, 0.13f, 0.18f), 0.95f);
    RenderQuad(panelX, panelY, panelWidth, 6, glm::vec3(0.28f, 0.6f, 0.95f), 0.9f);

    RenderTextCentered(u8"\u9009\u62e9\u6b4c\u66f2", 640, 160, 34, glm::vec3(0.85f, 0.95f, 1.0f), 1.0f, true);

    if (m_songList.empty()) {
        RenderTextCentered(u8"music/ \u4e2d\u672a\u627e\u5230WAV\u6587\u4ef6", 640, 320, 22, glm::vec3(0.75f, 0.85f, 0.95f), 0.9f);
        RenderTextCentered(u8"\u6309 ESC \u8fd4\u56de", 640, 360, 18, glm::vec3(0.7f, 0.8f, 0.9f), 0.9f);
        return;
    }

    int totalCount = static_cast<int>(m_songList.size());
    int visibleCount = std::min(totalCount, 8);
    int startIndex = 0;
    if (m_selectedSongIndex >= visibleCount) {
        startIndex = m_selectedSongIndex - visibleCount + 1;
    }

    float listX = panelX + 80.0f;
    float listY = panelY + 90.0f;
    float listWidth = panelWidth - 160.0f;
    float itemHeight = 42.0f;
    float itemGap = 8.0f;

    for (int i = 0; i < visibleCount; ++i) {
        int index = startIndex + i;
        float y = listY + i * (itemHeight + itemGap);
        bool selected = (index == m_selectedSongIndex);
        glm::vec3 baseColor = selected ? glm::vec3(0.18f, 0.65f, 0.95f) : glm::vec3(0.12f, 0.16f, 0.24f);
        float alpha = selected ? 0.9f : 0.75f;
        RenderQuad(listX, y, listWidth, itemHeight, baseColor, alpha);
        glm::vec3 textColor = selected ? glm::vec3(0.96f, 0.98f, 1.0f) : glm::vec3(0.75f, 0.85f, 0.95f);
        RenderText(m_songList[index], listX + 18.0f, y + 10.0f, 20, textColor, 0.95f, selected);
    }

    RenderTextCentered(u8"\u4e0a\u4e0b\u9009\u62e9\uff0c\u56de\u8f66\u5f00\u59cb\uff0cESC\u8fd4\u56de", 640, panelY + panelHeight - 32.0f, 18,
                       glm::vec3(0.75f, 0.85f, 0.95f), 0.9f);
}

glm::vec2 UI::GetMouseUiPosition(const Input* input) const {
    if (!input || m_uiScale <= 0.0f) {
        return glm::vec2(0.0f);
    }
    glm::vec2 mouse = input->GetMousePosition();
    float uiX = (mouse.x - m_uiOffset.x) / m_uiScale;
    float uiY = (mouse.y - m_uiOffset.y) / m_uiScale;
    return glm::vec2(uiX, uiY);
}
