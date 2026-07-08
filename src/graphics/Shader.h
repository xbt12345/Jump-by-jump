#pragma once

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class Shader {
public:
    unsigned int ID;
    
    Shader();
    ~Shader();
    
    // 从文件加载着色器
    bool LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
    
    // 从字符串加载着色器
    bool LoadFromString(const std::string& vertexSource, const std::string& fragmentSource);
    
    // 使用着色器程序
    void Use() const;
    
    // uniform工具函数
    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec2(const std::string& name, const glm::vec2& value) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    void SetVec4(const std::string& name, const glm::vec4& value) const;
    void SetMat2(const std::string& name, const glm::mat2& mat) const;
    void SetMat3(const std::string& name, const glm::mat3& mat) const;
    void SetMat4(const std::string& name, const glm::mat4& mat) const;

private:
    // 检查编译/链接错误
    void CheckCompileErrors(unsigned int shader, const std::string& type);
    
    // 从文件读取源码
    std::string ReadFile(const std::string& filePath);
};
