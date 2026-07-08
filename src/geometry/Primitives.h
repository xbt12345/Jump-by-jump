#pragma once

#include <vector>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

class Primitives {
public:
    // 基本体素生成 - 课程要求
    static std::vector<Vertex> GenerateCube(float size = 1.0f);
    static std::vector<Vertex> GenerateSphere(float radius = 1.0f, int segments = 32);
    static std::vector<Vertex> GenerateCylinder(float radius = 1.0f, float height = 2.0f, int segments = 32);
    static std::vector<Vertex> GenerateCone(float radius = 1.0f, float height = 2.0f, int segments = 32);
    static std::vector<Vertex> GeneratePrism(int sides = 6, float radius = 1.0f, float height = 2.0f);
    static std::vector<Vertex> GenerateFrustum(float topRadius = 0.5f, float bottomRadius = 1.0f, float height = 2.0f, int segments = 32);
    
    // 游戏专用几何体
    static std::vector<Vertex> GeneratePlatform(float width = 2.0f, float depth = 2.0f, float height = 0.2f);
    static std::vector<Vertex> GenerateSlideTrack(float width = 1.0f, float length = 5.0f, float height = 0.1f);
    static std::vector<Vertex> GenerateBouncePlatform(float width = 2.0f, float depth = 2.0f, float height = 0.2f);
    static std::vector<Vertex> GenerateTechPlatform(float width = 2.0f, float depth = 2.0f, float height = 0.2f);
    
    // 获取对应的索引数组
    static std::vector<unsigned int> GetCubeIndices();
    static std::vector<unsigned int> GetSphereIndices(int segments = 32);
    static std::vector<unsigned int> GetCylinderIndices(int segments = 32);
    static std::vector<unsigned int> GetConeIndices(int segments = 32);
    static std::vector<unsigned int> GetPrismIndices(int sides = 6);
    static std::vector<unsigned int> GetFrustumIndices(int segments = 32);

private:
    // 辅助函数
    static glm::vec3 CalculateNormal(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);
    static void AddTriangle(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices,
                           const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3,
                           const glm::vec2& uv1, const glm::vec2& uv2, const glm::vec2& uv3);
};
