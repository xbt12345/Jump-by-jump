#include "ParticleSystem.h"
#include <iostream>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

ParticleSystem::ParticleSystem() : m_VAO(0), m_VBO(0) {
}

ParticleSystem::~ParticleSystem() {
    Cleanup();
}

bool ParticleSystem::Initialize() {
    if (!LoadParticleShader()) {
        std::cout << "Failed to load particle shader" << std::endl;
        return false;
    }

    CreateParticleMesh();
    return true;
}

void ParticleSystem::Update(float deltaTime) {
    // 更新粒子
    for (auto it = m_particles.begin(); it != m_particles.end();) {
        it->life -= deltaTime;
        it->position += it->velocity * deltaTime;

        // 添加重力效果
        it->velocity.y -= 9.8f * deltaTime * 0.5f;

        if (it->life <= 0.0f) {
            it = m_particles.erase(it);
        } else {
            ++it;
        }
    }
}

void ParticleSystem::Render(Renderer* renderer) {
    if (m_particles.empty() || !m_particleShader) return;

    // 保存当前状态
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_DEPTH_TEST);

    m_particleShader->Use();

    // 设置矩阵
    glm::mat4 view = renderer->GetViewMatrix();
    glm::mat4 projection = renderer->GetProjectionMatrix();
    m_particleShader->SetMat4("view", view);
    m_particleShader->SetMat4("projection", projection);

    glBindVertexArray(m_VAO);

    for (const auto& particle : m_particles) {
        // 计算粒子透明度
        float alpha = particle.life / particle.maxLife;

        // 创建模型矩阵
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, particle.position);
        model = glm::scale(model, glm::vec3(particle.size));

        m_particleShader->SetMat4("model", model);
        m_particleShader->SetVec3("color", particle.color);
        m_particleShader->SetFloat("alpha", alpha);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBindVertexArray(0);

    // 恢复状态
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void ParticleSystem::Cleanup() {
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        m_VAO = m_VBO = 0;
    }
    m_particleShader.reset();
    m_particles.clear();
}

void ParticleSystem::Emit(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color, float speed, float life) {
    Particle particle;
    particle.position = position;
    particle.velocity = direction * speed;
    particle.color = color;
    particle.life = life;
    particle.maxLife = life;
    particle.size = 0.1f + (rand() % 100) / 1000.0f; // 随机大小

    m_particles.push_back(particle);
}

void ParticleSystem::EmitBurst(const glm::vec3& position, int count, const glm::vec3& color, float speed, float life) {
    for (int i = 0; i < count; ++i) {
        // 随机方向
        float angle1 = (rand() % 360) * 3.14159f / 180.0f;
        float angle2 = (rand() % 180) * 3.14159f / 180.0f;

        glm::vec3 direction(
            sin(angle2) * cos(angle1),
            cos(angle2),
            sin(angle2) * sin(angle1)
        );

        Emit(position, direction, color, speed * (0.5f + (rand() % 100) / 100.0f), life);
    }
}

void ParticleSystem::CreateParticleMesh() {
    // 创建简单的四边形用于粒子，包含纹理坐标
    float vertices[] = {
        // 位置        // 纹理坐标
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f
    };

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

bool ParticleSystem::LoadParticleShader() {
    std::string vertexShader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        
        out vec2 TexCoord;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
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
            // 创建圆形粒子效果
            vec2 center = TexCoord - vec2(0.5);
            float dist = length(center);
            if (dist > 0.5) discard;
            
            float smoothAlpha = 1.0 - smoothstep(0.0, 0.5, dist);
            FragColor = vec4(color, alpha * smoothAlpha);
        }
    )";

    m_particleShader = std::make_unique<Shader>();
    return m_particleShader->LoadFromString(vertexShader, fragmentShader);
}