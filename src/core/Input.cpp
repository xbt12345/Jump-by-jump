#include "Input.h"
#include <cstring>
#include <algorithm>
#include <cmath>

Input::Input(GLFWwindow* window) : m_window(window), m_scrollDelta(0.0f), 
    m_touchStartTime(0.0f), m_longPressThreshold(0.3f),
    m_currentBeatTime(0.0f), m_beatWindow(0.15f), m_lastBeatRating(BeatRating::MISS),
    m_lastBeatAccuracy(0.0f), m_timingOffset(0.0f) {
    
    // 初始化状态数组
    memset(m_keyStates, false, sizeof(m_keyStates));
    memset(m_prevKeyStates, false, sizeof(m_prevKeyStates));
    memset(m_mouseStates, false, sizeof(m_mouseStates));
    memset(m_prevMouseStates, false, sizeof(m_prevMouseStates));
    
    // 初始化触摸输入
    m_touchInput.type = TouchInputType::NONE;
    m_touchInput.position = glm::vec2(0.0f);
    m_touchInput.duration = 0.0f;
    m_touchInput.isActive = false;
    m_touchInput.beatAccuracy = 0.0f;
    m_touchInput.timingOffset = 0.0f;
    
    // 设置滚轮回调 (在窗口创建后安全设置)
    if (window) {
        glfwSetWindowUserPointer(window, this);
        glfwSetScrollCallback(window, ScrollCallback);
        
        // 获取初始鼠标位置
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        m_mousePos = m_prevMousePos = glm::vec2(x, y);
    } else {
        // 如果窗口无效，使用默认值
        m_mousePos = m_prevMousePos = glm::vec2(0.0f, 0.0f);
    }
}

Input::~Input() {
}

void Input::Update() {
    // 保存上一帧状态
    memcpy(m_prevKeyStates, m_keyStates, sizeof(m_keyStates));
    memcpy(m_prevMouseStates, m_mouseStates, sizeof(m_mouseStates));
    m_prevMousePos = m_mousePos;
    
    // 注意：滚轮值由回调设置，在帧结束时重置（见 ResetScrollDelta）
    // 不在这里重置，以便滚轮值能在本帧被使用
    
    // 更新键盘状态
    for (int i = 0; i <= GLFW_KEY_LAST; ++i) {
        m_keyStates[i] = glfwGetKey(m_window, i) == GLFW_PRESS;
    }
    
    // 更新鼠标按钮状态
    for (int i = 0; i <= GLFW_MOUSE_BUTTON_LAST; ++i) {
        m_mouseStates[i] = glfwGetMouseButton(m_window, i) == GLFW_PRESS;
    }
    
    // 更新鼠标位置
    double x, y;
    glfwGetCursorPos(m_window, &x, &y);
    m_mousePos = glm::vec2(x, y);
    
    // 更新触摸输入
    UpdateTouchInput();
    
    // 注意：滚轮增量在下一帧开始时重置，不在这里重置
    // 因为滚轮回调可能在glfwPollEvents后触发
}

bool Input::IsKeyDown(int key) const {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return m_keyStates[key];
}

bool Input::IsKeyPressed(int key) const {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return m_keyStates[key] && !m_prevKeyStates[key];
}

bool Input::IsKeyReleased(int key) const {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return !m_keyStates[key] && m_prevKeyStates[key];
}

bool Input::IsMouseButtonDown(int button) const {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return m_mouseStates[button];
}

bool Input::IsMouseButtonPressed(int button) const {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return m_mouseStates[button] && !m_prevMouseStates[button];
}

bool Input::IsMouseButtonReleased(int button) const {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return !m_mouseStates[button] && m_prevMouseStates[button];
}

glm::vec2 Input::GetMousePosition() const {
    return m_mousePos;
}

glm::vec2 Input::GetMouseDelta() const {
    return m_mousePos - m_prevMousePos;
}

float Input::GetScrollDelta() const {
    return m_scrollDelta;
}

TouchInput Input::GetTouchInput() const {
    return m_touchInput;
}

bool Input::IsTouchActive() const {
    return m_touchInput.isActive;
}

float Input::GetTouchDuration() const {
    return m_touchInput.duration;
}

void Input::UpdateTouchInput() {
    bool currentlyPressed = IsMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT);
    bool justPressed = IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
    bool justReleased = IsMouseButtonReleased(GLFW_MOUSE_BUTTON_LEFT);
    
    // 重置触摸输入状态
    m_touchInput.type = TouchInputType::NONE;
    m_touchInput.beatAccuracy = 0.0f;
    m_touchInput.timingOffset = 0.0f;
    
    if (justPressed) {
        m_touchStartTime = glfwGetTime();
        m_touchInput.isActive = true;
        m_touchInput.position = m_mousePos;
        
        // 记录输入时间用于节拍分析
        RecordInputTiming(m_touchStartTime);
        
        // 计算与最近节拍的时间偏移
        float beatInterval = 60.0f / 120.0f; // 假设120 BPM
        float timeSinceLastBeat = fmod(m_touchStartTime - m_currentBeatTime, beatInterval);
        float timeToBeat = timeSinceLastBeat;
        if (timeToBeat > beatInterval * 0.5f) {
            timeToBeat -= beatInterval; // 调整为最近的节拍
        }
        
        m_timingOffset = timeToBeat;
        m_lastBeatRating = CalculateBeatRating(abs(timeToBeat));
        m_lastBeatAccuracy = 1.0f - (abs(timeToBeat) / (m_beatWindow * 0.5f));
        m_lastBeatAccuracy = std::max(0.0f, std::min(1.0f, m_lastBeatAccuracy));
        
        m_touchInput.beatAccuracy = m_lastBeatAccuracy;
        m_touchInput.timingOffset = m_timingOffset;
        
        // 根据节拍准确度设置输入类型
        if (m_lastBeatRating >= BeatRating::GREAT) {
            m_touchInput.type = TouchInputType::PERFECT_TAP;
        } else if (timeToBeat < 0) {
            m_touchInput.type = TouchInputType::EARLY_TAP;
        } else if (timeToBeat > 0) {
            m_touchInput.type = TouchInputType::LATE_TAP;
        } else {
            m_touchInput.type = TouchInputType::SHORT_TAP;
        }
    }
    
    if (currentlyPressed && m_touchInput.isActive) {
        float currentTime = glfwGetTime();
        m_touchInput.duration = currentTime - m_touchStartTime;
        
        // 长按检测
        if (m_touchInput.duration >= m_longPressThreshold) {
            m_touchInput.type = TouchInputType::LONG_PRESS;
        }
    }
    
    if (justReleased && m_touchInput.isActive) {
        m_touchInput.type = TouchInputType::RELEASE;
        m_touchInput.isActive = false;
    }
}

BeatRating Input::CalculateBeatRating(float timingOffset) {
    float absOffset = abs(timingOffset);
    
    if (absOffset <= 0.025f) return BeatRating::FLAWLESS;  // ±25ms
    if (absOffset <= 0.05f)  return BeatRating::PERFECT;   // ±50ms
    if (absOffset <= 0.1f)   return BeatRating::GREAT;     // ±100ms
    if (absOffset <= 0.15f)  return BeatRating::GOOD;      // ±150ms
    
    return BeatRating::MISS;
}

void Input::RecordInputTiming(float currentTime) {
    m_inputHistory.push_back(currentTime);
    if (m_inputHistory.size() > MAX_HISTORY_SIZE) {
        m_inputHistory.erase(m_inputHistory.begin());
    }
}

float Input::GetAverageAccuracy() const {
    if (m_inputHistory.size() < 2) return 0.5f;
    
    float totalAccuracy = 0.0f;
    int validInputs = 0;
    
    for (size_t i = 1; i < m_inputHistory.size(); ++i) {
        float interval = m_inputHistory[i] - m_inputHistory[i-1];
        float expectedInterval = 60.0f / 120.0f; // 假设120 BPM
        float accuracy = 1.0f - abs(interval - expectedInterval) / expectedInterval;
        accuracy = std::max(0.0f, std::min(1.0f, accuracy));
        
        totalAccuracy += accuracy;
        validInputs++;
    }
    
    return validInputs > 0 ? totalAccuracy / validInputs : 0.5f;
}

bool Input::IsOnBeat() const {
    return m_lastBeatRating >= BeatRating::GOOD;
}

void Input::TriggerHapticFeedback(float intensity, float duration) {
    // 预留接口，未来可以集成手柄振动
    (void)intensity;
    (void)duration;
}

void Input::ScrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset) {
    Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (input) {
        input->m_scrollDelta = static_cast<float>(yoffset);
    }
}