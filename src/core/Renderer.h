#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "../graphics/Shader.h"

struct Light {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    
    float constant;
    float linear;
    float quadratic;
};

struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
    
    unsigned int diffuseTexture;
    unsigned int specularTexture;
    bool useTexture;
};

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    bool Initialize();
    void Cleanup();
    
    // 矩阵设置
    void SetViewMatrix(const glm::mat4& view);
    void SetProjectionMatrix(const glm::mat4& projection);
    
    // 光照设置
    void SetLight(const Light& light);
    void SetPlayerLight(const Light& playerLight); // 新增：小球头上的光源
    void SetViewPosition(const glm::vec3& viewPos);
    void UpdateLightPosition(const glm::vec3& playerPos);
    void UpdatePlayerLight(const glm::vec3& playerPos); // 新增：更新小球光源位置
    
    // 渲染方法
    void DrawMesh(unsigned int VAO, const glm::mat4& model, const Material& material, int indexCount = 36, bool isPlayer = false, bool isPlatform = false, int platformType = 0);
    void DrawWireframe(unsigned int VAO, const glm::mat4& model, int indexCount = 36);
    
    // 获取着色器
    Shader* GetBasicShader() { return m_basicShader.get(); }
    
    // 获取矩阵
    const glm::mat4& GetViewMatrix() const { return m_viewMatrix; }
    const glm::mat4& GetProjectionMatrix() const { return m_projectionMatrix; }
    
    // 时间管理
    void UpdateTime(float currentTime) { m_currentTime = currentTime; }

private:
    std::unique_ptr<Shader> m_basicShader;
    std::unique_ptr<Shader> m_wireframeShader;
    
    glm::mat4 m_viewMatrix;
    glm::mat4 m_projectionMatrix;
    
    Light m_light;
    Light m_playerLight; // 新增：小球头上的光源
    glm::vec3 m_viewPosition;
    float m_currentTime;
    
    bool LoadShaders();
};