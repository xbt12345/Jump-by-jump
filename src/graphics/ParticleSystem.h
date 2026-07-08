#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "../core/Renderer.h"

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 color;
    float life;
    float maxLife;
    float size;
};

class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();

    bool Initialize();
    void Update(float deltaTime);
    void Render(Renderer* renderer);
    void Cleanup();

    // 发射粒子
    void Emit(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color, float speed, float life);
    void EmitBurst(const glm::vec3& position, int count, const glm::vec3& color, float speed, float life);

private:
    std::vector<Particle> m_particles;
    unsigned int m_VAO;
    unsigned int m_VBO;
    std::unique_ptr<Shader> m_particleShader;

    void CreateParticleMesh();
    bool LoadParticleShader();
};