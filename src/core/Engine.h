#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <memory>
#include "Renderer.h"
#include "Camera.h"
#include "Input.h"
#include "UI.h"
#include "../game/Player.h"
#include "../game/LevelGenerator.h"
#include "../audio/AudioAnalyzer.h"
#include "../graphics/ParticleSystem.h"
#include "../graphics/MusicVisualizer.h"

enum class GameMode {
    NORMAL,
    RHYTHM
};

class Engine {
public:
    Engine();
    ~Engine();

    bool Initialize(GLFWwindow* window);
    void Update();
    void Render();
    void Cleanup();

private:
    GLFWwindow* m_window;
    
    // 核心系统
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<Input> m_input;
    std::unique_ptr<UI> m_ui;
    
    // 游戏对象
    std::unique_ptr<Player> m_player;
    std::unique_ptr<LevelGenerator> m_levelGenerator;
    std::unique_ptr<AudioAnalyzer> m_audioAnalyzer;
    std::unique_ptr<ParticleSystem> m_particleSystem;
    std::unique_ptr<MusicVisualizer> m_musicVisualizer;
    Platform* m_lastPlatform;
    Platform* m_lastScoredPlatform;
    
    // 时间管理
    float m_deltaTime;
    float m_lastFrame;
    float m_gameplayTime;
    GameMode m_gameMode;
    bool m_rhythmJumpQueued;
    float m_rhythmJumpQueuedTime;
    int m_nextPlatformType;
    
    void UpdateDeltaTime();
    void CheckPlatformCollisions();
    void FindNextPlatform();
    void ResetGame();
    void SetupLighting(); // 新增：设置光源
};
