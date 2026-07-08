#include "Renderer.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <filesystem>

namespace {
std::string ResolveShaderPath(const std::string& filename) {
    namespace fs = std::filesystem;
    const fs::path candidates[] = {
        fs::path("shaders") / filename,
        fs::path("../shaders") / filename,
        fs::path("../../shaders") / filename
    };

    for (const auto& path : candidates) {
        if (fs::exists(path)) {
            return path.string();
        }
    }

    return (fs::path("shaders") / filename).string();
}
} // namespace

Renderer::Renderer() 
    : m_viewMatrix(1.0f)
    , m_projectionMatrix(1.0f)
    , m_viewPosition(0.0f) {
    
    // 主光源设置 - 模拟太阳光，位置更高更远
    m_light.position = glm::vec3(0.0f, 25.0f, 10.0f);
    m_light.ambient = glm::vec3(0.4f, 0.4f, 0.5f);  // 稍微偏蓝的环境光
    m_light.diffuse = glm::vec3(1.0f, 0.95f, 0.8f); // 暖色调的主光
    m_light.specular = glm::vec3(1.0f, 1.0f, 1.0f);
    m_light.constant = 1.0f;
    m_light.linear = 0.007f;     // 更小的线性衰减
    m_light.quadratic = 0.0002f; // 更小的二次衰减，模拟远距离光源
    
    // 玩家光源设置 - 作为补光，填充阴影
    m_playerLight.position = glm::vec3(0.0f, 0.0f, 0.0f);
    m_playerLight.ambient = glm::vec3(0.1f, 0.15f, 0.2f);  // 冷色调补光
    m_playerLight.diffuse = glm::vec3(0.3f, 0.5f, 0.8f);   // 蓝色调
    m_playerLight.specular = glm::vec3(0.2f, 0.3f, 0.5f);
    m_playerLight.constant = 1.0f;
    m_playerLight.linear = 0.35f;    // 较快衰减，局部照明
    m_playerLight.quadratic = 0.44f; // 快速衰减
}

Renderer::~Renderer() {
    Cleanup();
}

bool Renderer::Initialize() {
    if (!LoadShaders()) {
        std::cout << "Failed to load shaders" << std::endl;
        return false;
    }
    
    std::cout << "Renderer initialized successfully" << std::endl;
    return true;
}

void Renderer::Cleanup() {
    m_basicShader.reset();
    m_wireframeShader.reset();
}

bool Renderer::LoadShaders() {
    // 加载基本着色器 - 修正路径
    m_basicShader = std::make_unique<Shader>();
    const std::string vertexPath = ResolveShaderPath("basic.vert");
    const std::string fragmentPath = ResolveShaderPath("basic.frag");
    if (!m_basicShader->LoadFromFile(vertexPath, fragmentPath)) {
        std::cout << "Failed to load basic shader" << std::endl;
        return false;
    }
    
    // 创建简单的线框着色器
    std::string wireframeVert = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";
    
    std::string wireframeFrag = R"(
        #version 330 core
        out vec4 FragColor;
        
        uniform vec3 color;
        
        void main() {
            FragColor = vec4(color, 1.0);
        }
    )";
    
    m_wireframeShader = std::make_unique<Shader>();
    if (!m_wireframeShader->LoadFromString(wireframeVert, wireframeFrag)) {
        std::cout << "Failed to load wireframe shader" << std::endl;
        return false;
    }
    
    return true;
}

void Renderer::SetViewMatrix(const glm::mat4& view) {
    m_viewMatrix = view;
}

void Renderer::SetProjectionMatrix(const glm::mat4& projection) {
    m_projectionMatrix = projection;
}

void Renderer::SetLight(const Light& light) {
    m_light = light;
}

void Renderer::SetPlayerLight(const Light& playerLight) {
    m_playerLight = playerLight;
}

void Renderer::SetViewPosition(const glm::vec3& viewPos) {
    m_viewPosition = viewPos;
}

void Renderer::UpdateLightPosition(const glm::vec3& playerPos) {
    // 主光源保持相对固定的位置，但会缓慢跟随玩家
    // 这样可以产生更自然的阴影方向变化
    glm::vec3 targetPos = playerPos + glm::vec3(15.0f, 25.0f, 15.0f);
    
    // 使用插值让光源平滑移动，而不是瞬间跳跃
    float lerpFactor = 0.02f; // 很慢的跟随速度
    m_light.position = glm::mix(m_light.position, targetPos, lerpFactor);
}

void Renderer::UpdatePlayerLight(const glm::vec3& playerPos) {
    // 玩家光源紧贴玩家，用于填充阴影
    m_playerLight.position = playerPos + glm::vec3(0.0f, 1.2f, 0.0f);
}

void Renderer::DrawMesh(unsigned int VAO, const glm::mat4& model, const Material& material, int indexCount, bool isPlayer, bool isPlatform, int platformType) {
    if (!m_basicShader) return;
    
    m_basicShader->Use();
    
    // 设置矩阵
    m_basicShader->SetMat4("model", model);
    m_basicShader->SetMat4("view", m_viewMatrix);
    m_basicShader->SetMat4("projection", m_projectionMatrix);
    
    // 计算法线矩阵
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    m_basicShader->SetMat3("normalMatrix", normalMatrix);
    
    // 设置材质
    m_basicShader->SetVec3("material.ambient", material.ambient);
    m_basicShader->SetVec3("material.diffuseColor", material.diffuse);
    m_basicShader->SetVec3("material.specular", material.specular);
    m_basicShader->SetFloat("material.shininess", material.shininess);
    m_basicShader->SetBool("useTexture", material.useTexture);
    
    // 设置主光源
    m_basicShader->SetVec3("light.position", m_light.position);
    m_basicShader->SetVec3("light.ambient", m_light.ambient);
    m_basicShader->SetVec3("light.diffuse", m_light.diffuse);
    m_basicShader->SetVec3("light.specular", m_light.specular);
    m_basicShader->SetFloat("light.constant", m_light.constant);
    m_basicShader->SetFloat("light.linear", m_light.linear);
    m_basicShader->SetFloat("light.quadratic", m_light.quadratic);
    
    // 设置小球光源
    m_basicShader->SetVec3("playerLight.position", m_playerLight.position);
    m_basicShader->SetVec3("playerLight.ambient", m_playerLight.ambient);
    m_basicShader->SetVec3("playerLight.diffuse", m_playerLight.diffuse);
    m_basicShader->SetVec3("playerLight.specular", m_playerLight.specular);
    m_basicShader->SetFloat("playerLight.constant", m_playerLight.constant);
    m_basicShader->SetFloat("playerLight.linear", m_playerLight.linear);
    m_basicShader->SetFloat("playerLight.quadratic", m_playerLight.quadratic);
    
    // 设置观察位置
    m_basicShader->SetVec3("viewPos", m_viewPosition);
    
    // 设置时间和对象类型用于特效
    m_basicShader->SetFloat("time", m_currentTime);
    m_basicShader->SetBool("isPlayer", isPlayer);
    m_basicShader->SetBool("isPlatform", isPlatform);
    m_basicShader->SetInt("platformType", platformType); // 新增：传递平台类型
    
    // 绑定纹理
    if (material.useTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material.diffuseTexture);
        m_basicShader->SetInt("material.diffuse", 0);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, material.specularTexture);
        m_basicShader->SetInt("material.specularMap", 1);
    }
    
    bool useBlend = isPlatform && platformType == 3;
    if (useBlend) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    // 绘制
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    if (useBlend) {
        glDisable(GL_BLEND);
    }
}

void Renderer::DrawWireframe(unsigned int VAO, const glm::mat4& model, int indexCount) {
    if (!m_wireframeShader) return;
    
    m_wireframeShader->Use();
    
    // 设置矩阵
    m_wireframeShader->SetMat4("model", model);
    m_wireframeShader->SetMat4("view", m_viewMatrix);
    m_wireframeShader->SetMat4("projection", m_projectionMatrix);
    m_wireframeShader->SetVec3("color", glm::vec3(1.0f, 1.0f, 1.0f));
    
    // 启用线框模式
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    // 绘制
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    // 恢复填充模式
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


