#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../core/Engine.h"

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#endif

// 窗口大小
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// 回调函数声明
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

static int FailAndWait(const char* message) {
    std::cerr << message << std::endl;
    std::cerr << "Press Enter to exit..." << std::endl;
    std::cin.get();
    return -1;
}

// 设置工作目录为exe所在目录的上一级（项目根目录）
static void SetWorkingDirectory() {
#ifdef _WIN32
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    
    // 找到最后一个反斜杠，截断得到exe所在目录
    char* lastSlash = strrchr(exePath, '\\');
    if (lastSlash) {
        *lastSlash = '\0';
        // 再向上一级（从output目录到项目根目录）
        lastSlash = strrchr(exePath, '\\');
        if (lastSlash) {
            *lastSlash = '\0';
            _chdir(exePath);
        }
    }
#endif
}

int main() {
    // 设置工作目录，确保双击exe也能正确加载资源
    SetWorkingDirectory();
    
    std::cout << "Starting Music Jump 3D..." << std::endl;
    
    // 初始化GLFW
    if (!glfwInit()) {
        return FailAndWait("Failed to initialize GLFW");
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Music Jump 3D", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return FailAndWait("Failed to create GLFW window (OpenGL 3.3 required)");
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return FailAndWait("Failed to initialize OpenGL functions");
    }

    // 启用深度测试
    glEnable(GL_DEPTH_TEST);
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // 创建游戏引擎
    Engine engine;
    if (!engine.Initialize(window)) {
        std::cout << "Failed to initialize engine" << std::endl;
        glfwTerminate();
        return FailAndWait("Failed to initialize engine");
    }

    std::cout << "Game initialized successfully! Controls:" << std::endl;
    std::cout << "- A/D: Move left/right" << std::endl;
    std::cout << "- SPACE: Start game / Jump" << std::endl;
    std::cout << "Press SPACE to start the game!" << std::endl;

    double startTime = glfwGetTime();
    bool suppressedClose = false;
    int frameCount = 0;

    // 游戏主循环
    while (true) {
        // 避免启动瞬间的异常关闭信号
        if (glfwWindowShouldClose(window)) {
            double aliveTime = glfwGetTime() - startTime;
            if (aliveTime < 0.5) {
                glfwSetWindowShouldClose(window, GLFW_FALSE);
                suppressedClose = true;
            } else {
                break;
            }
        }

        processInput(window);

        // 更新游戏逻辑
        engine.Update();

        // 渲染
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        engine.Render();

        glfwSwapBuffers(window);
        glfwPollEvents();
        frameCount++;
    }

    // 清理资源
    engine.Cleanup();
    glfwTerminate();
    
    if (suppressedClose) {
        std::cout << "Startup close request suppressed." << std::endl;
    }
    std::cout << "Game loop exited after " << frameCount << " frames." << std::endl;
    std::cout << "Game ended successfully!" << std::endl;
    return 0;
}

void processInput(GLFWwindow* window) {
    (void)window;
}

void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height) {
    glViewport(0, 0, width, height);
}
