#include "Engine.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include <cmath>

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

Engine::Engine() 
    : m_window(nullptr)
    , m_lastPlatform(nullptr)
    , m_lastScoredPlatform(nullptr)
    , m_deltaTime(0.0f)
    , m_lastFrame(0.0f)
    , m_gameplayTime(0.0f)
    , m_gameMode(GameMode::NORMAL)
    , m_rhythmJumpQueued(false)
    , m_rhythmJumpQueuedTime(0.0f)
    , m_nextPlatformType(-1) {
}

Engine::~Engine() {
    Cleanup();
}

bool Engine::Initialize(GLFWwindow* window) {
    m_window = window;
    
    std::cout << "Initializing Engine..." << std::endl;
    
    // 初�化核心系�?
    std::cout << "Creating renderer..." << std::endl;
    m_renderer = std::make_unique<Renderer>();
    
    std::cout << "Creating camera..." << std::endl;
    m_camera = std::make_unique<Camera>();
    
    std::cout << "Creating input..." << std::endl;
    m_input = std::make_unique<Input>(window);
    
    std::cout << "Creating UI..." << std::endl;
    m_ui = std::make_unique<UI>();
    
    // 初�化游戏对�?
    std::cout << "Creating game objects..." << std::endl;
    try {
        std::cout << " - Creating player..." << std::endl;
        m_player = std::make_unique<Player>();
        std::cout << " - Player created" << std::endl;

        std::cout << " - Creating level generator..." << std::endl;
        m_levelGenerator = std::make_unique<LevelGenerator>();
        std::cout << " - Level generator created" << std::endl;

        std::cout << " - Creating audio analyzer..." << std::endl;
        m_audioAnalyzer = std::make_unique<AudioAnalyzer>();
        std::cout << " - Audio analyzer created" << std::endl;

        std::cout << " - Creating particle system..." << std::endl;
        m_particleSystem = std::make_unique<ParticleSystem>();
        std::cout << " - Particle system created" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Failed to create game objects: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cout << "Failed to create game objects: unknown error" << std::endl;
        return false;
    }
    
    // 初�化音乐�视化系统
    std::cout << "Creating music visualizer..." << std::endl;
    m_musicVisualizer = std::make_unique<MusicVisualizer>();
    
    // 初�化渲染�
    std::cout << "Initializing renderer..." << std::endl;
    if (!m_renderer->Initialize()) {
        std::cout << "Failed to initialize renderer" << std::endl;
        return false;
    }
    
    // 初�化UI系统
    std::cout << "Initializing UI..." << std::endl;
    if (!m_ui->Initialize()) {
        std::cout << "Failed to initialize UI" << std::endl;
        return false;
    }
    
    // 初�化粒子系�?
    std::cout << "Initializing particle system..." << std::endl;
    if (!m_particleSystem->Initialize()) {
        std::cout << "Failed to initialize particle system" << std::endl;
        return false;
    }

    std::cout << "Initializing audio analyzer..." << std::endl;
    if (!m_audioAnalyzer->Initialize()) {
        std::cout << "Failed to initialize audio analyzer" << std::endl;
        return false;
    }
    m_ui->SetSongList(m_audioAnalyzer->GetTrackNames());
    
    // 初�化音乐�视化
    std::cout << "Initializing music visualizer..." << std::endl;
    if (!m_musicVisualizer->Initialize()) {
        std::cout << "Failed to initialize music visualizer" << std::endl;
        return false;
    }
    
    // 设置相机初�位� - 在小球后面和上方
    m_camera->SetPosition(glm::vec3(-8.0f, 6.0f, 0.0f));
    m_camera->SetTarget(glm::vec3(0.0f, 1.0f, 0.0f));
    
    // 设置投影矩阵
    m_camera->SetPerspective(60.0f, (float)1280 / (float)720, 0.1f, 100.0f);
    
    // 初始化游戏对象
    m_player->Initialize();
    m_player->SetParticleSystem(m_particleSystem.get());
    
    // 加载球贴图
    m_player->LoadBallTextures("Balls");
    m_ui->SetBallTextureNames(m_player->GetTextureNames());
    m_ui->SetSelectedBallTexture(m_player->GetCurrentTextureIndex());
    
    m_levelGenerator->Initialize();
    
    // 设置光源
    SetupLighting();
    
    std::cout << "Engine initialized successfully" << std::endl;
    return true;
}

void Engine::Update() {
    UpdateDeltaTime();

    m_input->Update();
    m_ui->HandleInput(m_input.get());
    m_ui->Update(m_deltaTime);

    switch (m_ui->GetGameState()) {
        case GameState::MENU: {
            if (m_ui->IsStartButtonPressed()) {
                m_gameMode = GameMode::NORMAL;
                m_audioAnalyzer->EnableSimulation(true);
                m_audioAnalyzer->StopPlayback();
                m_levelGenerator->SetRhythmMode(false);
                m_levelGenerator->SetStartPlatformLength(8.0f);
                m_player->SetAllowJumpInput(true);
                m_player->SetRhythmMode(false);
                ResetGame();
                m_ui->StartCountdown(3.0f);
                m_ui->SetGameState(GameState::COUNTDOWN);
                m_ui->ResetButtons();
            }
            if (m_ui->IsRhythmButtonPressed()) {
                m_gameMode = GameMode::RHYTHM;
                m_audioAnalyzer->RefreshTracks();
                m_ui->SetSongList(m_audioAnalyzer->GetTrackNames());
                m_ui->SetGameState(GameState::SONG_SELECT);
                m_ui->ResetButtons();
            }
            if (m_ui->IsSettingsButtonPressed()) {
                m_ui->SetGameState(GameState::SETTINGS);
                m_ui->ResetButtons();
            }
            if (m_ui->IsSceneViewButtonPressed()) {
                // 进入场景漫游模式
                m_ui->SetGameState(GameState::SCENE_VIEW);
                m_camera->SetMode(CameraMode::FREE);
                // 生成固定演示场景
                m_levelGenerator->GenerateDemoScene();
                // 设置相机初始位置 - 在起始平台上方，看向场景
                m_camera->SetPerspective(45.0f, 16.0f / 9.0f, 0.1f, 500.0f); // 增大远剪裁平面
                m_camera->SetPosition(glm::vec3(-5.0f, 5.0f, 8.0f));
                m_camera->SetTarget(glm::vec3(15.0f, 0.0f, 0.0f));
                m_camera->SetMoveSpeed(12.0f);
                m_ui->ResetSceneViewButton();
            }
            if (m_ui->IsQuitButtonPressed()) {
                glfwSetWindowShouldClose(m_window, GLFW_TRUE);
                m_ui->ResetButtons();
            }
            break;
        }
        case GameState::SCENE_VIEW: {
            // 场景漫游模式 - 键盘控制位置，鼠标移动控制视角，滚轮控制缩放
            glm::vec2 mouseDelta = m_input->GetMouseDelta();
            float scrollDelta = m_input->GetScrollDelta();
            
            // 碰撞检测函数 - 检查相机位置是否与平台碰撞
            auto collisionCheck = [this](const glm::vec3& pos, float radius) -> bool {
                // 获取附近的平台进行碰撞检测
                auto platforms = m_levelGenerator->GetNearbyPlatforms(pos, radius + 2.0f);
                for (auto* platform : platforms) {
                    if (platform && platform->CheckCollision(pos, radius)) {
                        return true; // 发生碰撞
                    }
                }
                // 地面碰撞检测 - 不允许低于地面
                if (pos.y < radius) {
                    return true;
                }
                return false;
            };
            
            // 方向键/WASD控制相机移动（带碰撞检测）
            if (m_input->IsKeyDown(GLFW_KEY_UP) || m_input->IsKeyDown(GLFW_KEY_W)) {
                m_camera->TryMoveForward(m_deltaTime, collisionCheck);
            }
            if (m_input->IsKeyDown(GLFW_KEY_DOWN) || m_input->IsKeyDown(GLFW_KEY_S)) {
                m_camera->TryMoveBackward(m_deltaTime, collisionCheck);
            }
            if (m_input->IsKeyDown(GLFW_KEY_LEFT) || m_input->IsKeyDown(GLFW_KEY_A)) {
                m_camera->TryMoveLeft(m_deltaTime, collisionCheck);
            }
            if (m_input->IsKeyDown(GLFW_KEY_RIGHT) || m_input->IsKeyDown(GLFW_KEY_D)) {
                m_camera->TryMoveRight(m_deltaTime, collisionCheck);
            }
            // Q/E 控制上下
            if (m_input->IsKeyDown(GLFW_KEY_Q)) {
                m_camera->TryMoveDown(m_deltaTime, collisionCheck);
            }
            if (m_input->IsKeyDown(GLFW_KEY_E)) {
                m_camera->TryMoveUp(m_deltaTime, collisionCheck);
            }
            
            // 鼠标移动 - 直接控制视角（FPS风格）
            if (std::abs(mouseDelta.x) > 0.1f || std::abs(mouseDelta.y) > 0.1f) {
                m_camera->RotateByMouse(mouseDelta.x, mouseDelta.y);
            }
            
            // 滚轮 - 控制FOV缩放
            if (std::abs(scrollDelta) > 0.01f) {
                float currentFov = m_camera->GetFOV();
                currentFov -= scrollDelta * 3.0f; // 滚轮向上缩小FOV（放大画面）
                currentFov = std::clamp(currentFov, 20.0f, 90.0f);
                m_camera->SetFOV(currentFov);
            }
            
            // F键 - 重置视角到起点
            if (m_input->IsKeyPressed(GLFW_KEY_F)) {
                m_camera->SetPosition(glm::vec3(-5.0f, 5.0f, 8.0f));
                m_camera->SetTarget(glm::vec3(15.0f, 0.0f, 0.0f));
                m_camera->SetFOV(45.0f); // 重置FOV
            }
            
            // ESC - 返回菜单
            if (m_input->IsKeyPressed(GLFW_KEY_ESCAPE)) {
                m_ui->SetGameState(GameState::MENU);
                m_camera->SetMode(CameraMode::FOLLOW);
                // 重置相机位置和FOV
                m_camera->SetPosition(glm::vec3(-8.0f, 6.0f, 0.0f));
                m_camera->SetTarget(glm::vec3(0.0f, 1.0f, 0.0f));
                m_camera->SetFOV(45.0f);
                // 清除演示场景，准备游戏
                m_levelGenerator->Reset();
            }
            
            // 更新渲染矩阵
            m_renderer->SetViewMatrix(m_camera->GetViewMatrix());
            m_renderer->SetProjectionMatrix(m_camera->GetProjectionMatrix());
            m_renderer->SetViewPosition(m_camera->GetPosition());
            break;
        }
        case GameState::SONG_SELECT: {
            if (m_ui->IsBackPressed()) {
                m_ui->SetGameState(GameState::MENU);
                m_ui->ResetButtons();
                break;
            }
            if (m_ui->IsSongConfirmPressed()) {
                int index = m_ui->GetSelectedSongIndex();
                if (m_audioAnalyzer->SelectTrack(static_cast<size_t>(index))) {
                    m_gameMode = GameMode::RHYTHM;
                    m_audioAnalyzer->EnableSimulation(false);
                    m_levelGenerator->SetRhythmMode(true);
                    m_player->SetAllowJumpInput(false);
                    m_player->SetRhythmMode(true);
                    float leadTime = m_audioAnalyzer->GetFirstJumpTime();
                    float startLength = std::max(9.0f, std::max(0.8f, leadTime) * 3.4f * 2.0f + 3.0f);
                    m_levelGenerator->SetStartPlatformLength(startLength);
                    ResetGame();
                    const RhythmSequence* sequence = m_audioAnalyzer->GetRhythmSequence();
                    if (sequence) {
                        m_levelGenerator->GenerateLevelFromSequence(*sequence);
                    }
                    m_ui->StartCountdown(3.0f);
                    m_ui->SetGameState(GameState::COUNTDOWN);
                }
                m_ui->ResetButtons();
            }
            break;
        }
        case GameState::COUNTDOWN: {
            m_audioAnalyzer->Update(m_deltaTime);
            BeatInfo beatInfo = m_audioAnalyzer->GetBeatInfo();
            m_input->SetCurrentBeatTime(beatInfo.currentTime);

            m_musicVisualizer->Update(m_deltaTime, beatInfo);
            m_levelGenerator->Update(m_deltaTime, beatInfo);
            m_player->Update(m_deltaTime, m_input.get());
            m_particleSystem->Update(m_deltaTime);

            if (m_ui->IsCountdownFinished()) {
                m_player->StartGame();
                if (m_gameMode == GameMode::RHYTHM) {
                    m_audioAnalyzer->StartPlayback();
                }
                m_ui->SetGameState(GameState::PLAYING);
            }
            break;
        }
        case GameState::PLAYING: {
            if (m_gameMode == GameMode::NORMAL) {
                m_gameplayTime += m_deltaTime;
                float speedScale = 1.0f + std::min(m_gameplayTime * 0.01f, 0.8f);
                m_player->SetSpeedScale(speedScale);
                m_levelGenerator->SetSpeedScale(speedScale);
            } else {
                m_player->SetSpeedScale(1.0f);
                m_levelGenerator->SetSpeedScale(1.0f);
            }

            m_audioAnalyzer->Update(m_deltaTime);
            BeatInfo beatInfo = m_audioAnalyzer->GetBeatInfo();
            m_input->SetCurrentBeatTime(beatInfo.currentTime);

            m_musicVisualizer->Update(m_deltaTime, beatInfo);
            m_levelGenerator->Update(m_deltaTime, beatInfo);
            m_player->Update(m_deltaTime, m_input.get());
            m_particleSystem->Update(m_deltaTime);

            FindNextPlatform();
            CheckPlatformCollisions();

            if (m_gameMode == GameMode::RHYTHM) {
                float interval = (beatInfo.jumpInterval > 0.05f) ? beatInfo.jumpInterval : 0.6f;
                float diffPrev = std::abs(beatInfo.currentTime - beatInfo.lastJumpTime);
                float diffNext = std::abs(beatInfo.nextJumpTime - beatInfo.currentTime);
                float diff = std::min(diffPrev, diffNext);
                float window = std::clamp(interval * 0.55f, 0.18f, 0.38f);
                float cue = std::clamp(1.0f - diff / window, 0.0f, 1.0f);
                float edgeFactor = 1.0f;
                if (m_lastPlatform) {
                    glm::vec3 platformPos = m_lastPlatform->GetPosition();
                    glm::vec3 platformSize = m_lastPlatform->GetCollisionSize();
                    float frontEdge = platformPos.x + platformSize.x * 0.5f;
                    float dist = frontEdge - m_player->GetPosition().x;
                    edgeFactor = std::clamp(1.0f - dist / 2.8f, 0.0f, 1.0f);
                }
                cue *= edgeFactor;
                bool strong = beatInfo.isStrongBeat || beatInfo.isJumpBeat;
                m_ui->SetRhythmCue(cue, strong, m_nextPlatformType);
            } else {
                m_ui->SetRhythmCue(0.0f, false, -1);
            }

            if (m_gameMode == GameMode::RHYTHM) {
                if (m_input->IsKeyPressed(GLFW_KEY_SPACE)) {
                    m_rhythmJumpQueued = true;
                    m_rhythmJumpQueuedTime = beatInfo.currentTime;
                }

                if (m_rhythmJumpQueued) {
                    float elapsed = beatInfo.currentTime - m_rhythmJumpQueuedTime;
                    float interval = (beatInfo.jumpInterval > 0.05f) ? beatInfo.jumpInterval : 0.6f;
                    if (beatInfo.nextJumpTime <= 0.01f) {
                        if (m_player->CanJump()) {
                            m_player->Jump(JumpType::NORMAL);
                            m_rhythmJumpQueued = false;
                        }
                    } else {
                    float maxQueue = std::clamp(interval * 1.2f, 0.45f, 1.1f);
                    if (elapsed > maxQueue) {
                        m_rhythmJumpQueued = false;
                    } else {
                        bool edgeJump = false;
                        if (m_player->CanJump() && m_lastPlatform) {
                            glm::vec3 platformPos = m_lastPlatform->GetPosition();
                            glm::vec3 platformSize = m_lastPlatform->GetCollisionSize();
                            glm::vec3 playerPos = m_player->GetPosition();
                            float frontEdge = platformPos.x + platformSize.x * 0.5f - 0.1f;
                            if (playerPos.x >= frontEdge - 0.35f) {
                                edgeJump = true;
                            }
                        }

                        if (edgeJump) {
                            m_player->Jump(JumpType::NORMAL);
                            m_rhythmJumpQueued = false;
                        } else {
                            float earlyWindow = std::clamp(interval * 0.55f, 0.18f, 0.36f);
                            float lateWindow = std::clamp(interval * 0.40f, 0.12f, 0.28f);
                            bool inLast = (beatInfo.currentTime >= beatInfo.lastJumpTime - earlyWindow &&
                                           beatInfo.currentTime <= beatInfo.lastJumpTime + lateWindow);
                            bool inNext = (beatInfo.currentTime >= beatInfo.nextJumpTime - earlyWindow &&
                                           beatInfo.currentTime <= beatInfo.nextJumpTime + lateWindow);
                            if ((inLast || inNext) && m_player->CanJump()) {
                                float diffPrev = std::abs(beatInfo.currentTime - beatInfo.lastJumpTime);
                                float diffNext = std::abs(beatInfo.nextJumpTime - beatInfo.currentTime);
                                float diff = std::min(diffPrev, diffNext);
                                float perfectWindow = std::clamp(interval * 0.12f, 0.04f, 0.09f);
                                JumpType jumpType = (diff <= perfectWindow) ? JumpType::PERFECT : JumpType::NORMAL;
                                m_player->Jump(jumpType);
                                m_rhythmJumpQueued = false;
                            }
                        }
                    }
                    }
                }
            }

            glm::vec3 targetPos = m_player->GetPosition();
            glm::vec3 shakeOffset = m_musicVisualizer->GetCameraShakeOffset();
            m_camera->FollowTarget(targetPos + shakeOffset, m_deltaTime);

            m_ui->UpdateScore(m_player->GetScore(), m_player->GetCombo());

            if (m_player->GetPosition().y < -10.0f) {
                m_ui->SetGameState(GameState::GAME_OVER);
                if (m_gameMode == GameMode::RHYTHM) {
                    m_audioAnalyzer->StopPlayback();
                }
            }
            break;
        }
        case GameState::GAME_OVER:
            if (m_ui->IsStartButtonPressed()) {
                m_audioAnalyzer->StopPlayback();
                m_audioAnalyzer->EnableSimulation(true);
                m_gameMode = GameMode::NORMAL;
                m_levelGenerator->SetRhythmMode(false);
                m_player->SetAllowJumpInput(true);
                m_player->SetRhythmMode(false);
                ResetGame();
                m_ui->SetGameState(GameState::MENU);
                m_ui->ResetButtons();
            }
            break;
        case GameState::SETTINGS:
            // 如果光源设置发生变化，重新设置光源
            if (m_ui->HasLightSettingsChanged()) {
                SetupLighting();
                m_ui->ClearLightSettingsChanged();
            }
            // 如果球贴图发生变化，更新玩家贴图
            if (m_ui->HasBallTextureChanged()) {
                int newIndex = m_ui->GetSelectedBallTexture();
                m_player->SetBallTexture(newIndex);
                m_ui->ClearBallTextureChanged();
            }
            // 返回菜单
            if (m_ui->IsBackPressed()) {
                m_ui->SetGameState(GameState::MENU);
                m_ui->ResetButtons();
            }
            break;
        default:
            break;
    }
    
    // 帧结束时重置滚轮值
    m_input->ResetScrollDelta();
}

void Engine::Render() {
    // 在倒计时、游戏进行、场景漫游时渲染3D场景
    GameState state = m_ui->GetGameState();
    if (state == GameState::PLAYING || state == GameState::COUNTDOWN || state == GameState::SCENE_VIEW) {
        // 设置相机矩阵
        m_renderer->SetViewMatrix(m_camera->GetViewMatrix());
        m_renderer->SetProjectionMatrix(m_camera->GetProjectionMatrix());
        m_renderer->SetViewPosition(m_camera->GetPosition());
        
        // 更新光源位置跟随玩家（场景漫游时使用固定位置）
        if (state == GameState::SCENE_VIEW) {
            m_renderer->UpdateLightPosition(glm::vec3(0.0f, 1.0f, 0.0f));
            m_renderer->UpdatePlayerLight(glm::vec3(0.0f, 1.0f, 0.0f));
        } else {
            m_renderer->UpdateLightPosition(m_player->GetPosition());
            m_renderer->UpdatePlayerLight(m_player->GetPosition());
        }
        
        // 渲染关卡
        m_levelGenerator->Render(m_renderer.get());
        
        // 场景漫游模式下不渲染玩家和粒子
        if (state != GameState::SCENE_VIEW) {
            // 获取地面高度用于阴影渲染
            float groundY = 0.0f;
            if (m_lastPlatform) {
                groundY = m_lastPlatform->GetPosition().y + m_lastPlatform->GetSize().y * 0.5f;
            }
            (void)groundY; // 避免未使用警告

            // 渲染玩家
            m_player->Render(m_renderer.get());
            
            // 渲染粒子效果
            m_particleSystem->Render(m_renderer.get());
        }
    }
    
    // 渲染UI
    m_ui->Render(m_renderer.get());
    
}

void Engine::Cleanup() {
    if (m_ui) m_ui->Cleanup();
    if (m_player) m_player->Cleanup();
    if (m_levelGenerator) m_levelGenerator->Cleanup();
    if (m_particleSystem) m_particleSystem->Cleanup();
    if (m_renderer) m_renderer->Cleanup();
}

void Engine::SetupLighting() {
    // 从UI获取光源设置
    const LightSettings& settings = m_ui->GetLightSettings();
    
    // 设置主光源
    Light mainLight;
    mainLight.position = glm::vec3(0.0f, settings.mainLightOffset.y, 0.0f);
    mainLight.ambient = settings.mainLightAmbient * settings.mainLightIntensity;
    mainLight.diffuse = settings.mainLightDiffuse * settings.mainLightIntensity;
    mainLight.specular = settings.mainLightSpecular * settings.mainLightIntensity;
    mainLight.constant = 1.0f;
    mainLight.linear = 0.014f;
    mainLight.quadratic = 0.0007f;
    m_renderer->SetLight(mainLight);
    
    // 设置玩家补光
    Light playerLight;
    playerLight.position = glm::vec3(0.0f, 1.0f, 0.0f);
    playerLight.ambient = settings.playerLightAmbient * settings.playerLightIntensity;
    playerLight.diffuse = settings.playerLightDiffuse * settings.playerLightIntensity;
    playerLight.specular = settings.playerLightSpecular * settings.playerLightIntensity;
    playerLight.constant = 1.0f;
    // 根据半径调整衰减
    float radiusFactor = 4.0f / settings.playerLightRadius;
    playerLight.linear = 0.35f * radiusFactor;
    playerLight.quadratic = 0.44f * radiusFactor * radiusFactor;
    m_renderer->SetPlayerLight(playerLight);
}

void Engine::UpdateDeltaTime() {
    float currentFrame = glfwGetTime();
    m_deltaTime = currentFrame - m_lastFrame;
    m_lastFrame = currentFrame;
}

void Engine::ResetGame() {
    m_player->Reset(true);
    m_player->SetSpeedScale(1.0f);
    m_levelGenerator->Reset();
    m_levelGenerator->SetSpeedScale(1.0f);
    m_audioAnalyzer->StopPlayback();
    m_audioAnalyzer->Reset();
    m_ui->UpdateScore(0, 0);
    m_camera->SetPosition(glm::vec3(-8.0f, 6.0f, 0.0f));
    m_camera->SetTarget(glm::vec3(0.0f, 1.0f, 0.0f));
    m_lastPlatform = nullptr;
    m_lastScoredPlatform = nullptr;
    m_gameplayTime = 0.0f;
    m_rhythmJumpQueued = false;
    m_rhythmJumpQueuedTime = 0.0f;
    m_nextPlatformType = -1;
}

void Engine::FindNextPlatform() {
    // 获取玩�前方的平�?
    glm::vec3 playerPos = m_player->GetPosition();
    auto nearbyPlatforms = m_levelGenerator->GetNearbyPlatforms(playerPos, 20.0f); // 增加搜索范围
    
    Platform* nextPlatform = nullptr;
    float closestDistance = std::numeric_limits<float>::max();
    
    // 找到玩�前方最近的平台
    for (auto* platform : nearbyPlatforms) {
        glm::vec3 platformPos = platform->GetPosition();
        
        // �考虑玩�前方的平台（X坐标更大�?
        if (platformPos.x > playerPos.x + 0.5f) { // 添加小的偏移量避免�择当前平台
            float distance = glm::length(platformPos - playerPos);
            
            if (distance < closestDistance) {
                closestDistance = distance;
                nextPlatform = platform;
            }
        }
    }
    
    // 告诉玩�下��平台的位�
    if (nextPlatform) {
        glm::vec3 nextPos = nextPlatform->GetPosition();
        m_player->SetNextPlatformPosition(nextPos);
        m_nextPlatformType = static_cast<int>(nextPlatform->GetType());
    } else {
        m_player->ClearNextPlatform();
        m_nextPlatformType = -1;
    }
}

void Engine::CheckPlatformCollisions() {
    glm::vec3 playerPos = m_player->GetPosition();
    bool wasGrounded = m_player->IsGrounded();
    bool foundCollision = false;
    Platform* candidate = nullptr;

    // 首先检查侧面碰撞并推开小球
    auto nearbyPlatforms = m_levelGenerator->GetNearbyPlatforms(playerPos, 5.0f);
    for (auto* platform : nearbyPlatforms) {
        glm::vec3 platformPos = platform->GetPosition();
        glm::vec3 platformSize = platform->GetCollisionSize();
        glm::vec3 pushOut;
        
        if (m_player->CheckSideCollision(platformPos, platformSize, pushOut)) {
            // 推开小球，防止穿透
            glm::vec3 newPos = m_player->GetPosition() + pushOut;
            m_player->SetPosition(newPos);
            
            // 如果碰到侧面，根据推开方向调整速度
            glm::vec3 velocity = m_player->GetVelocity();
            if (std::abs(pushOut.x) > 0.01f) {
                velocity.x = std::max(0.0f, velocity.x); // 不能向后
            }
            if (std::abs(pushOut.z) > 0.01f) {
                velocity.z = 0.0f; // 停止侧向移动
            }
        }
    }
    
    // 更新玩家位置（可能被侧面碰撞修改了）
    playerPos = m_player->GetPosition();

    if (wasGrounded && m_lastPlatform) {
        if (m_player->CheckPlatformCollision(m_lastPlatform->GetPosition(), m_lastPlatform->GetCollisionSize())) {
            candidate = m_lastPlatform;
        }
    }

    if (!candidate) {
        float bestScore = std::numeric_limits<float>::max();
        
        for (auto* platform : nearbyPlatforms) {
            glm::vec3 platformPos = platform->GetPosition();
            glm::vec3 platformSize = platform->GetCollisionSize();
            if (!m_player->CheckPlatformCollision(platformPos, platformSize)) {
                continue;
            }

            float dx = playerPos.x - platformPos.x;
            float dz = playerPos.z - platformPos.z;
            float score = dx * dx + dz * dz;
            if (score < bestScore) {
                bestScore = score;
                candidate = platform;
            }
        }
    }

    if (candidate) {
        bool isNewPlatform = (m_lastPlatform != candidate);
        bool notScoredYet = (m_lastScoredPlatform != candidate);
        bool awardScore = isNewPlatform && !wasGrounded && notScoredYet;

        if (isNewPlatform && m_lastPlatform) {
            m_lastPlatform->OnPlayerLeave();
        }

        m_player->OnLandOnPlatform(candidate->GetPosition(),
            candidate->GetCollisionSize(),
            static_cast<int>(candidate->GetType()),
            awardScore);

        if (isNewPlatform || !wasGrounded) {
            candidate->OnPlayerLand();
        }
        m_lastPlatform = candidate;
        if (awardScore) {
            m_lastScoredPlatform = candidate;
        }
        m_player->SetPlatformVelocity(candidate->GetVelocity());
        m_player->SetPlatformCenter(candidate->GetPosition());
        foundCollision = true;
    }
    
    // 如果玩家离开了所有平台
    if (!foundCollision && m_lastPlatform) {
        m_lastPlatform->OnPlayerLeave();
        m_lastPlatform = nullptr;
    }

    if (!foundCollision) {
        m_player->SetGrounded(false);
        m_player->SetPlatformVelocity(glm::vec3(0.0f));
    }
    
    // 如果玩家掉得太低且没有碰撞，进入游戏结束
    if (!foundCollision && playerPos.y < -10.0f) {
        m_ui->SetGameState(GameState::GAME_OVER);
    }
}
