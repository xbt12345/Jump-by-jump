#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

enum class TouchInputType {
    NONE,
    SHORT_TAP,      // 短按跳跃
    LONG_PRESS,     // 长按滑行
    RELEASE,        // 松开
    PERFECT_TAP,    // 完美节拍点击
    EARLY_TAP,      // 提前点击
    LATE_TAP        // 延迟点击
};

struct TouchInput {
    TouchInputType type;
    glm::vec2 position;
    float duration;
    bool isActive;
    float beatAccuracy;     // 节拍准确度 [-1, 1]
    float timingOffset;     // 与节拍的时间偏移
};

// 节拍同步评分
enum class BeatRating {
    MISS,       // 错过
    GOOD,       // 良好 (±150ms)
    GREAT,      // 很好 (±100ms)
    PERFECT,    // 完美 (±50ms)
    FLAWLESS    // 无瑕 (±25ms)
};

class Input {
public:
    Input(GLFWwindow* window);
    ~Input();
    
    void Update();
    void SetCurrentBeatTime(float beatTime) { m_currentBeatTime = beatTime; }
    void SetBeatWindow(float window) { m_beatWindow = window; } // 节拍窗口大小
    
    // 键盘输入
    bool IsKeyDown(int key) const;
    bool IsKeyPressed(int key) const;
    bool IsKeyReleased(int key) const;
    
    // 鼠标输入
    bool IsMouseButtonDown(int button) const;
    bool IsMouseButtonPressed(int button) const;
    bool IsMouseButtonReleased(int button) const;
    
    glm::vec2 GetMousePosition() const;
    glm::vec2 GetMouseDelta() const;
    float GetScrollDelta() const;
    void ResetScrollDelta() { m_scrollDelta = 0.0f; }
    
    // 触摸输入模拟（使用鼠标）
    TouchInput GetTouchInput() const;
    bool IsTouchActive() const;
    float GetTouchDuration() const;
    
    // 节拍同步功能
    BeatRating GetLastBeatRating() const { return m_lastBeatRating; }
    float GetBeatAccuracy() const { return m_lastBeatAccuracy; }
    bool IsOnBeat() const;
    float GetTimingOffset() const { return m_timingOffset; }
    
    // 输入历史记录 - 用于分析玩家节奏感
    std::vector<float> GetRecentInputTimings() const { return m_inputHistory; }
    float GetAverageAccuracy() const;
    
    // 振动反馈（如果支持手柄）
    void TriggerHapticFeedback(float intensity, float duration);

private:
    GLFWwindow* m_window;
    
    // 键盘状态
    bool m_keyStates[GLFW_KEY_LAST + 1];
    bool m_prevKeyStates[GLFW_KEY_LAST + 1];
    
    // 鼠标状态
    bool m_mouseStates[GLFW_MOUSE_BUTTON_LAST + 1];
    bool m_prevMouseStates[GLFW_MOUSE_BUTTON_LAST + 1];
    
    glm::vec2 m_mousePos;
    glm::vec2 m_prevMousePos;
    float m_scrollDelta;
    
    // 触摸输入状态
    TouchInput m_touchInput;
    float m_touchStartTime;
    float m_longPressThreshold;
    
    // 节拍同步
    float m_currentBeatTime;
    float m_beatWindow;
    BeatRating m_lastBeatRating;
    float m_lastBeatAccuracy;
    float m_timingOffset;
    
    // 输入历史
    std::vector<float> m_inputHistory;
    static const size_t MAX_HISTORY_SIZE = 50;
    
    void UpdateTouchInput();
    BeatRating CalculateBeatRating(float timingOffset);
    void RecordInputTiming(float currentTime);
    
    // 滚轮回调
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};