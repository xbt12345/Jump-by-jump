#pragma once

#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include "Renderer.h"
#include "Input.h"
#include "../graphics/Shader.h"

enum class GameState {
    MENU,
    SETTINGS,
    SCENE_VIEW,  // 场景漫游模式
    SONG_SELECT,
    COUNTDOWN,
    PLAYING,
    GAME_OVER
};

// 光源设置结构
struct LightSettings {
    // 主光源
    glm::vec3 mainLightOffset = glm::vec3(15.0f, 25.0f, 15.0f);
    glm::vec3 mainLightAmbient = glm::vec3(0.4f, 0.4f, 0.5f);
    glm::vec3 mainLightDiffuse = glm::vec3(1.0f, 0.95f, 0.8f);
    glm::vec3 mainLightSpecular = glm::vec3(1.0f, 1.0f, 1.0f);
    float mainLightIntensity = 1.0f;
    
    // 玩家补光
    glm::vec3 playerLightAmbient = glm::vec3(0.1f, 0.15f, 0.2f);
    glm::vec3 playerLightDiffuse = glm::vec3(0.3f, 0.5f, 0.8f);
    glm::vec3 playerLightSpecular = glm::vec3(0.2f, 0.3f, 0.5f);
    float playerLightIntensity = 0.4f;
    float playerLightRadius = 4.0f;
};

class UI {
public:
    UI();
    ~UI();
    
    bool Initialize();
    void Update(float deltaTime);
    void Render(Renderer* renderer);
    void Cleanup();
    
    // 游戏状态管�?
    void SetGameState(GameState state) { m_gameState = state; }
    GameState GetGameState() const { return m_gameState; }
    
    // 分数显示
    void UpdateScore(int score, int combo);
    
    // 新增：节拍准确度显示
    void UpdateBeatAccuracy(float accuracy, BeatRating rating);
    void SetRhythmCue(float intensity, bool strong, int platformType);
    
    // 菜单处理
    bool IsStartButtonPressed() const { return m_startPressed; }
    bool IsRhythmButtonPressed() const { return m_rhythmPressed; }
    bool IsQuitButtonPressed() const { return m_quitPressed; }
    bool IsSettingsButtonPressed() const { return m_settingsPressed; }
    bool IsSongConfirmPressed() const { return m_songConfirmPressed; }
    bool IsBackPressed() const { return m_backPressed; }
    int GetSelectedSongIndex() const { return m_selectedSongIndex; }
    void SetSongList(const std::vector<std::string>& songs);
    void ResetButtons();

    void StartCountdown(float seconds);
    bool IsCountdownFinished() const { return m_countdownTime <= 0.0f; }
    float GetCountdownTime() const { return m_countdownTime; }
    
    // 光源设置
    const LightSettings& GetLightSettings() const { return m_lightSettings; }
    void SetLightSettings(const LightSettings& settings) { m_lightSettings = settings; }
    bool HasLightSettingsChanged() const { return m_lightSettingsChanged; }
    void ClearLightSettingsChanged() { m_lightSettingsChanged = false; }
    
    // 场景漫游模式
    bool IsSceneViewMode() const { return m_sceneViewMode; }
    bool IsSceneViewButtonPressed() const { return m_sceneViewButtonPressed; }
    void ResetSceneViewButton() { m_sceneViewButtonPressed = false; }
    
    // 球贴图选择
    void SetBallTextureNames(const std::vector<std::string>& names) { 
        m_ballTextureNames = names;
    }
    void SetSelectedBallTexture(int index) { m_selectedBallTexture = index; }
    int GetSelectedBallTexture() const { return m_selectedBallTexture; }
    bool HasBallTextureChanged() const { return m_ballTextureChanged; }
    void ClearBallTextureChanged() { m_ballTextureChanged = false; }
    
    // 输入处理
    void HandleInput(class Input* input);

private:
    struct TextTexture {
        unsigned int textureId = 0;
        int width = 0;
        int height = 0;
    };
    GameState m_gameState;
    
    // 分数显示
    int m_currentScore;
    int m_currentCombo;

    float m_stateTime;
    GameState m_lastState;
    
    // 节拍准确度显示
    float m_lastBeatAccuracy;
    BeatRating m_lastBeatRating;
    float m_ratingDisplayTime;
    glm::vec3 m_ratingColor;
    float m_rhythmCueIntensity;
    bool m_rhythmCueStrong;
    int m_rhythmCuePlatformType;

    int m_lastScore;
    int m_lastCombo;
    
    TextTexture m_scoreTexture;
    TextTexture m_comboTexture;
    TextTexture m_ratingTexture; // 新增：评级显示纹�?
    
    // Menu state
    bool m_startPressed;
    bool m_quitPressed;
    bool m_settingsPressed;
    bool m_rhythmPressed;
    bool m_songConfirmPressed;
    bool m_backPressed;
    int m_selectedSongIndex;
    std::vector<std::string> m_songList;

    float m_countdownTime;
    
    // 光源设置
    LightSettings m_lightSettings;
    bool m_lightSettingsChanged = false;
    int m_settingsSelectedItem = 0;
    int m_settingsSelectedComponent = 0; // 0=x/r, 1=y/g, 2=z/b
    
    // 鼠标拖动滑块
    bool m_isDraggingSlider = false;
    int m_draggingSliderItem = -1;
    int m_draggingSliderComponent = -1; // 用于颜色滑块的RGB分量
    float m_sliderX = 420.0f;
    float m_sliderWidth = 280.0f;
    
    // 场景漫游模式
    bool m_sceneViewMode = false;
    bool m_sceneViewButtonPressed = false;
    
    // 球贴图选择
    std::vector<std::string> m_ballTextureNames;
    int m_selectedBallTexture = 0;
    bool m_ballTextureChanged = false;
    
    // OpenGL渲染资源
    unsigned int m_quadVAO;
    unsigned int m_quadVBO;
    std::unique_ptr<Shader> m_uiShader;
    std::unique_ptr<Shader> m_textShader;
    std::unordered_map<std::string, TextTexture> m_textCache;

    int m_viewportWidth;
    int m_viewportHeight;
    float m_uiScale;
    glm::vec2 m_uiOffset;
    
    // UI渲染方法
    void CreateUIShader();
    void CreateTextShader();
    void CreateQuadMesh();
    void RenderQuad(float x, float y, float width, float height, glm::vec3 color, float alpha = 1.0f);
    TextTexture CreateTextTexture(const std::string& text, int fontSize, bool bold);
    const TextTexture* GetTextTexture(const std::string& text, int fontSize, bool bold);
    void UpdateDynamicText(TextTexture& target, const std::string& text, int fontSize, bool bold);
    void RenderTextTexture(const TextTexture& texture, float x, float y, const glm::vec3& color, float alpha);
    void RenderText(const std::string& text, float x, float y, int fontSize, const glm::vec3& color, float alpha = 1.0f, bool bold = false);
    void RenderTextCentered(const std::string& text, float centerX, float centerY, int fontSize, const glm::vec3& color, float alpha = 1.0f, bool bold = false);
    
    // 新的UI渲染方法
    void RenderMenuUI();
    void RenderSettingsUI();
    void RenderSceneViewUI();
    void RenderSongSelectUI();
    void RenderCountdownUI();
    void RenderGameUI();
    void RenderGameOverUI();
    void RenderSlider(float x, float y, float width, float value, float minVal, float maxVal, const glm::vec3& color, bool selected);
    void RenderColorSlider(float x, float y, float width, const glm::vec3& color, int selectedComponent, bool isSelected);
    glm::vec2 GetMouseUiPosition(const class Input* input) const;
    
};
