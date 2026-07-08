#include "Primitives.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

std::vector<Vertex> Primitives::GenerateCube(float size) {
    std::vector<Vertex> vertices;
    float half = size * 0.5f;
    
    // 立方体的6个面，每个面4个顶点
    // 前面
    vertices.push_back({{-half, -half,  half}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}});
    vertices.push_back({{ half, -half,  half}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}});
    vertices.push_back({{ half,  half,  half}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}});
    vertices.push_back({{-half,  half,  half}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}});
    
    // 后面
    vertices.push_back({{-half, -half, -half}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}});
    vertices.push_back({{-half,  half, -half}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}});
    vertices.push_back({{ half,  half, -half}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}});
    vertices.push_back({{ half, -half, -half}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}});
    
    // 左面
    vertices.push_back({{-half,  half,  half}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}});
    vertices.push_back({{-half,  half, -half}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}});
    vertices.push_back({{-half, -half, -half}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}});
    vertices.push_back({{-half, -half,  half}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}});
    
    // 右面
    vertices.push_back({{ half,  half,  half}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}});
    vertices.push_back({{ half, -half,  half}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}});
    vertices.push_back({{ half, -half, -half}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}});
    vertices.push_back({{ half,  half, -half}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}});
    
    // 底面
    vertices.push_back({{-half, -half, -half}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}});
    vertices.push_back({{ half, -half, -half}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}});
    vertices.push_back({{ half, -half,  half}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}});
    vertices.push_back({{-half, -half,  half}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}});
    
    // 顶面
    vertices.push_back({{-half,  half, -half}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}});
    vertices.push_back({{-half,  half,  half}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}});
    vertices.push_back({{ half,  half,  half}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}});
    vertices.push_back({{ half,  half, -half}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}});
    
    return vertices;
}

std::vector<Vertex> Primitives::GenerateSphere(float radius, int segments) {
    std::vector<Vertex> vertices;
    
    for (int i = 0; i <= segments; ++i) {
        float phi = M_PI * float(i) / float(segments);
        
        for (int j = 0; j <= segments; ++j) {
            float theta = 2.0f * M_PI * float(j) / float(segments);
            
            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);
            
            glm::vec3 position(x, y, z);
            glm::vec3 normal = glm::normalize(position);
            glm::vec2 texCoord(float(j) / float(segments), float(i) / float(segments));
            
            vertices.push_back({position, normal, texCoord});
        }
    }
    
    return vertices;
}

std::vector<Vertex> Primitives::GenerateCylinder(float radius, float height, int segments) {
    std::vector<Vertex> vertices;
    float halfHeight = height * 0.5f;
    
    // 侧面顶点
    for (int i = 0; i <= segments; ++i) {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        
        // 底部顶点
        vertices.push_back({{x, -halfHeight, z}, {x/radius, 0.0f, z/radius}, {float(i)/segments, 0.0f}});
        // 顶部顶点
        vertices.push_back({{x, halfHeight, z}, {x/radius, 0.0f, z/radius}, {float(i)/segments, 1.0f}});
    }
    
    // 底面中心
    vertices.push_back({{0.0f, -halfHeight, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 0.5f}});
    // 顶面中心
    vertices.push_back({{0.0f, halfHeight, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 0.5f}});
    
    // 底面和顶面的边缘顶点
    for (int i = 0; i <= segments; ++i) {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        
        // 底面边缘
        vertices.push_back({{x, -halfHeight, z}, {0.0f, -1.0f, 0.0f}, {0.5f + 0.5f*cos(theta), 0.5f + 0.5f*sin(theta)}});
        // 顶面边缘
        vertices.push_back({{x, halfHeight, z}, {0.0f, 1.0f, 0.0f}, {0.5f + 0.5f*cos(theta), 0.5f + 0.5f*sin(theta)}});
    }
    
    return vertices;
}

std::vector<Vertex> Primitives::GenerateCone(float radius, float height, int segments) {
    std::vector<Vertex> vertices;
    
    // 顶点
    vertices.push_back({{0.0f, height, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 1.0f}});
    
    // 底面中心
    vertices.push_back({{0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 0.5f}});
    
    // 底面边缘顶点
    for (int i = 0; i <= segments; ++i) {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        
        // 侧面法向量计算
        glm::vec3 sideNormal = glm::normalize(glm::vec3(x, radius, z));
        
        // 侧面顶点
        vertices.push_back({{x, 0.0f, z}, sideNormal, {float(i)/segments, 0.0f}});
        
        // 底面顶点
        vertices.push_back({{x, 0.0f, z}, {0.0f, -1.0f, 0.0f}, {0.5f + 0.5f*cos(theta), 0.5f + 0.5f*sin(theta)}});
    }
    
    return vertices;
}

std::vector<Vertex> Primitives::GeneratePlatform(float width, float depth, float height) {
    std::vector<Vertex> vertices;
    
    // 增加平台厚度，让它看起来更有体积感
    float thickHeight = height * 1.8f; // 增加厚度
    float halfWidth = width * 0.5f;
    float halfDepth = depth * 0.5f;
    float halfHeight = thickHeight * 0.5f;
    
    // 顶面 - 添加细节和倒角效果
    float bevelSize = 0.05f; // 倒角大小
    
    // 主顶面
    vertices.push_back({{-halfWidth + bevelSize,  halfHeight, -halfDepth + bevelSize}, {0.0f, 1.0f, 0.0f}, {0.1f, 0.9f}});
    vertices.push_back({{-halfWidth + bevelSize,  halfHeight,  halfDepth - bevelSize}, {0.0f, 1.0f, 0.0f}, {0.1f, 0.1f}});
    vertices.push_back({{ halfWidth - bevelSize,  halfHeight,  halfDepth - bevelSize}, {0.0f, 1.0f, 0.0f}, {0.9f, 0.1f}});
    vertices.push_back({{ halfWidth - bevelSize,  halfHeight, -halfDepth + bevelSize}, {0.0f, 1.0f, 0.0f}, {0.9f, 0.9f}});
    
    // 底面
    vertices.push_back({{-halfWidth, -halfHeight, -halfDepth}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}});
    vertices.push_back({{ halfWidth, -halfHeight, -halfDepth}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}});
    vertices.push_back({{ halfWidth, -halfHeight,  halfDepth}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}});
    vertices.push_back({{-halfWidth, -halfHeight,  halfDepth}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}});
    
    // 前面 - 添加层次感
    vertices.push_back({{-halfWidth, -halfHeight,  halfDepth}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}});
    vertices.push_back({{ halfWidth, -halfHeight,  halfDepth}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}});
    vertices.push_back({{ halfWidth,  halfHeight - bevelSize,  halfDepth}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.8f}});
    vertices.push_back({{-halfWidth,  halfHeight - bevelSize,  halfDepth}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.8f}});
    
    // 后面
    vertices.push_back({{-halfWidth, -halfHeight, -halfDepth}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}});
    vertices.push_back({{-halfWidth,  halfHeight - bevelSize, -halfDepth}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.8f}});
    vertices.push_back({{ halfWidth,  halfHeight - bevelSize, -halfDepth}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.8f}});
    vertices.push_back({{ halfWidth, -halfHeight, -halfDepth}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}});
    
    // 左面
    vertices.push_back({{-halfWidth,  halfHeight - bevelSize,  halfDepth}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.8f}});
    vertices.push_back({{-halfWidth,  halfHeight - bevelSize, -halfDepth}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.8f}});
    vertices.push_back({{-halfWidth, -halfHeight, -halfDepth}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}});
    vertices.push_back({{-halfWidth, -halfHeight,  halfDepth}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}});
    
    // 右面
    vertices.push_back({{ halfWidth,  halfHeight - bevelSize,  halfDepth}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.8f}});
    vertices.push_back({{ halfWidth, -halfHeight,  halfDepth}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}});
    vertices.push_back({{ halfWidth, -halfHeight, -halfDepth}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}});
    vertices.push_back({{ halfWidth,  halfHeight - bevelSize, -halfDepth}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.8f}});
    
    // 添加倒角面 - 前倒角
    glm::vec3 bevelNormal = glm::normalize(glm::vec3(0.0f, 0.7f, 0.7f));
    vertices.push_back({{-halfWidth + bevelSize,  halfHeight,  halfDepth - bevelSize}, bevelNormal, {0.1f, 1.0f}});
    vertices.push_back({{ halfWidth - bevelSize,  halfHeight,  halfDepth - bevelSize}, bevelNormal, {0.9f, 1.0f}});
    vertices.push_back({{ halfWidth,  halfHeight - bevelSize,  halfDepth}, bevelNormal, {1.0f, 0.8f}});
    vertices.push_back({{-halfWidth,  halfHeight - bevelSize,  halfDepth}, bevelNormal, {0.0f, 0.8f}});
    
    // 后倒角
    bevelNormal = glm::normalize(glm::vec3(0.0f, 0.7f, -0.7f));
    vertices.push_back({{-halfWidth + bevelSize,  halfHeight, -halfDepth + bevelSize}, bevelNormal, {0.1f, 1.0f}});
    vertices.push_back({{-halfWidth,  halfHeight - bevelSize, -halfDepth}, bevelNormal, {0.0f, 0.8f}});
    vertices.push_back({{ halfWidth,  halfHeight - bevelSize, -halfDepth}, bevelNormal, {1.0f, 0.8f}});
    vertices.push_back({{ halfWidth - bevelSize,  halfHeight, -halfDepth + bevelSize}, bevelNormal, {0.9f, 1.0f}});
    
    // 左倒角
    bevelNormal = glm::normalize(glm::vec3(-0.7f, 0.7f, 0.0f));
    vertices.push_back({{-halfWidth + bevelSize,  halfHeight, -halfDepth + bevelSize}, bevelNormal, {0.1f, 1.0f}});
    vertices.push_back({{-halfWidth + bevelSize,  halfHeight,  halfDepth - bevelSize}, bevelNormal, {0.1f, 1.0f}});
    vertices.push_back({{-halfWidth,  halfHeight - bevelSize,  halfDepth}, bevelNormal, {0.0f, 0.8f}});
    vertices.push_back({{-halfWidth,  halfHeight - bevelSize, -halfDepth}, bevelNormal, {0.0f, 0.8f}});
    
    // 右倒角
    bevelNormal = glm::normalize(glm::vec3(0.7f, 0.7f, 0.0f));
    vertices.push_back({{ halfWidth - bevelSize,  halfHeight, -halfDepth + bevelSize}, bevelNormal, {0.9f, 1.0f}});
    vertices.push_back({{ halfWidth,  halfHeight - bevelSize, -halfDepth}, bevelNormal, {1.0f, 0.8f}});
    vertices.push_back({{ halfWidth,  halfHeight - bevelSize,  halfDepth}, bevelNormal, {1.0f, 0.8f}});
    vertices.push_back({{ halfWidth - bevelSize,  halfHeight,  halfDepth - bevelSize}, bevelNormal, {0.9f, 1.0f}});
    
    return vertices;
}

std::vector<Vertex> Primitives::GenerateBouncePlatform(float width, float depth, float height) {
    // Reuse the base platform geometry for bounce platforms.
    return GeneratePlatform(width, depth, height);
}

std::vector<Vertex> Primitives::GenerateTechPlatform(float width, float depth, float height) {
    // Reuse the base platform geometry for tech platforms.
    return GeneratePlatform(width, depth, height);
}

std::vector<Vertex> Primitives::GenerateSlideTrack(float width, float length, float height) {
    // 生成滑轨平台（长条形）
    std::vector<Vertex> vertices = GenerateCube(1.0f);
    
    // 缩放到滑轨尺寸
    for (auto& vertex : vertices) {
        vertex.Position.x *= length;  // 长度方向
        vertex.Position.y *= height;
        vertex.Position.z *= width;   // 宽度方向
    }
    
    return vertices;
}

std::vector<unsigned int> Primitives::GetCubeIndices() {
    return {
        // 主顶面
        0, 1, 2, 2, 3, 0,
        // 底面
        4, 5, 6, 6, 7, 4,
        // 前面
        8, 9, 10, 10, 11, 8,
        // 后面
        12, 13, 14, 14, 15, 12,
        // 左面
        16, 17, 18, 18, 19, 16,
        // 右面
        20, 21, 22, 22, 23, 20,
        // 前倒角
        24, 25, 26, 26, 27, 24,
        // 后倒角
        28, 29, 30, 30, 31, 28,
        // 左倒角
        32, 33, 34, 34, 35, 32,
        // 右倒角
        36, 37, 38, 38, 39, 36
    };
}

std::vector<unsigned int> Primitives::GetSphereIndices(int segments) {
    std::vector<unsigned int> indices;
    
    for (int i = 0; i < segments; ++i) {
        for (int j = 0; j < segments; ++j) {
            int first = i * (segments + 1) + j;
            int second = first + segments + 1;
            
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
    
    return indices;
}

glm::vec3 Primitives::CalculateNormal(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) {
    glm::vec3 v1 = p2 - p1;
    glm::vec3 v2 = p3 - p1;
    return glm::normalize(glm::cross(v1, v2));
}
