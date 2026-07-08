#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// 材质属�?
struct Material {
    sampler2D diffuse;
    sampler2D specularMap;
    float shininess;
    vec3 ambient;
    vec3 diffuseColor;
    vec3 specular;
};

// 光源属�?
struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    // 衰减参数
    float constant;
    float linear;
    float quadratic;
};

uniform Material material;
uniform Light light;        // 主光�?
uniform Light playerLight;  // 小球头上的光�?
uniform vec3 viewPos;
uniform bool useTexture;
uniform float time;
uniform bool isPlayer;
uniform bool isPlatform;
uniform int platformType;   // 新增：平台类�?(0=NORMAL, 1=SLIDE, 2=MOVING, 3=BOUNCE, 4=BOOST)

// 计算单个光源的贡�?- 纯光照，无假阴影
vec3 calculateLight(Light lightSource, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(lightSource.position - fragPos);
    
    // 环境�?
    vec3 ambient = lightSource.ambient * material.ambient;
    if (useTexture) {
        ambient *= texture(material.diffuse, TexCoord).rgb;
    } else {
        ambient *= material.diffuseColor;
    }
    
    // 漫反�?
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = lightSource.diffuse * diff;
    if (useTexture) {
        diffuse *= texture(material.diffuse, TexCoord).rgb;
    } else {
        diffuse *= material.diffuseColor;
    }
    
    // 镜面反射
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = lightSource.specular * spec;
    if (useTexture) {
        specular *= texture(material.specularMap, TexCoord).rgb;
    } else {
        specular *= material.specular;
    }
    
    // 计算衰减
    float distance = length(lightSource.position - fragPos);
    float attenuation = 1.0 / (lightSource.constant + lightSource.linear * distance + 
                              lightSource.quadratic * (distance * distance));
    
    // 应用衰减
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return ambient + diffuse + specular;
}

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // 计算主光源贡�?
    vec3 mainLightResult = calculateLight(light, norm, FragPos, viewDir);
    
    // 计算玩家光源贡献（作为补光）
    vec3 playerLightResult = calculateLight(playerLight, norm, FragPos, viewDir) * 0.4;
    
    // 合并光源结果
    vec3 result = mainLightResult + playerLightResult;
    
    // 添加特殊效果
    if (isPlayer) {
        // 玩家发光效果
        float glow = sin(time * 3.0) * 0.15 + 0.85;
        vec3 glowColor = vec3(0.3, 0.7, 1.0) * glow * 0.3;
        result += glowColor;
        
        // 边缘光效�?
        float fresnel = 1.0 - max(dot(viewDir, norm), 0.0);
        fresnel = pow(fresnel, 2.0);
        result += vec3(0.4, 0.8, 1.0) * fresnel * 0.4;

        // 软阴影与表面细节
        float bottomShade = smoothstep(-0.2, 0.6, norm.y);
        result *= mix(0.55, 1.0, bottomShade);
        float playerGrain = sin(FragPos.y * 10.0 + FragPos.x * 4.0) * 0.04;
        result *= (1.0 - playerGrain);
    }
    
    if (isPlatform) {
        // 基础平台材质增强效果
        float pulse = sin(time * 2.0 + FragPos.x * 0.2 + FragPos.z * 0.2) * 0.15 + 0.85;
        float secondaryPulse = sin(time * 4.0 + FragPos.y * 0.3) * 0.08 + 0.92;
        
        // 根据平台类型应用不同的视觉效�?
        if (platformType == 0) { // NORMAL - 蓝色水晶效果
            // 水晶内部光芒效果
            float crystalGlow = sin(time * 3.0 + FragPos.x * 0.5 + FragPos.z * 0.5) * 0.3 + 0.7;
            vec3 crystalColor = vec3(0.2, 0.5, 1.0) * crystalGlow;
            result += crystalColor * 0.2;
            
            // 水晶折射效果
            float fresnel = pow(1.0 - max(dot(viewDir, norm), 0.0), 3.0);
            result += vec3(0.4, 0.7, 1.0) * fresnel * 0.4;
            
            // 内部闪烁
            float sparkle = sin(time * 8.0 + FragPos.x * 2.0) * sin(time * 6.0 + FragPos.z * 2.0);
            if (sparkle > 0.7) {
                result += vec3(0.8, 0.9, 1.0) * (sparkle - 0.7) * 2.0;
            }
            
        } else if (platformType == 1) { // SLIDE - 冰块滑轨效果
            float frost = sin(time * 2.0 + FragPos.x * 2.0 + FragPos.z * 1.5) * 0.04 + 0.96;
            result *= frost;
            float edgeGlow = pow(1.0 - max(dot(viewDir, norm), 0.0), 2.0);
            result += vec3(0.8, 0.8, 0.6) * edgeGlow * 0.2;
            float iceSheen = pow(max(dot(viewDir, reflect(-normalize(light.position - FragPos), norm)), 0.0), material.shininess);
            result += vec3(0.9, 0.85, 0.6) * iceSheen * 0.25;
            
        } else if (platformType == 2) { // MOVING - 科技能量效果
            // 能量脉冲线条
            float energyPulse = sin(time * 4.0 + FragPos.x * 1.5) * 0.5 + 0.5;
            float circuitPattern = sin(FragPos.x * 8.0) * sin(FragPos.z * 8.0);
            if (circuitPattern > 0.3) {
                vec3 energyColor = vec3(0.2, 1.0, 0.6) * energyPulse;
                result += energyColor * 0.4;
            }
            
            // 全息效果
            float hologram = sin(time * 6.0 + FragPos.y * 2.0) * 0.2 + 0.8;
            result *= hologram;
            
            // 科技边缘�?
            float techGlow = pow(1.0 - max(dot(viewDir, norm), 0.0), 1.5);
            result += vec3(0.3, 0.9, 0.7) * techGlow * 0.5;
            
            // 数据流效�?
            float dataStream = fract(time * 2.0 + FragPos.x * 0.3);
            if (dataStream > 0.8) {
                result += vec3(0.5, 1.0, 0.8) * (dataStream - 0.8) * 5.0;
            }
            
        } else if (platformType == 4) { // BOOST - directional boost effect
            float flow = sin(FragPos.z * 3.0 + time * 2.0) * 0.06 + 0.94;
            result *= flow;
            float edgeGlow = pow(1.0 - max(dot(viewDir, norm), 0.0), 2.0);
            result += vec3(0.25, 0.15, 0.05) * edgeGlow * 0.12;
        } else if (platformType == 3) { // BOUNCE - 魔法水晶效果
            // 魔法能量波动
            float magicWave = sin(time * 3.0 + length(FragPos.xz) * 2.0) * 0.4 + 0.6;
            float magicPulse = cos(time * 5.0) * 0.3 + 0.7;
            vec3 magicColor = vec3(0.8, 0.3, 1.0) * magicWave * magicPulse;
            result += magicColor * 0.35;
            
            // 水晶内部星光效果
            float starlight = sin(time * 7.0 + FragPos.x * 3.0) * sin(time * 9.0 + FragPos.z * 3.0);
            if (starlight > 0.6) {
                result += vec3(1.0, 0.8, 1.0) * (starlight - 0.6) * 3.0;
            }
            
            // 魔法光环
            float magicRing = 1.0 - smoothstep(0.3, 0.7, length(FragPos.xz - floor(FragPos.xz + 0.5)));
            result += vec3(0.9, 0.4, 1.0) * magicRing * 0.3;
            
            // 超高光泽
            float superSpec = pow(max(dot(viewDir, reflect(-normalize(light.position - FragPos), norm)), 0.0), material.shininess * 2.0);
            result += vec3(1.0, 0.9, 1.0) * superSpec * 0.8;
        }
        
        // 通用增强效果
        result *= pulse * secondaryPulse;

        // 简单软阴影：在平台顶部投射玩家的模糊阴�?
        float upFactor = clamp(dot(norm, vec3(0.0, 1.0, 0.0)), 0.0, 1.0);
        if (upFactor > 0.5) {
            float height = max(playerLight.position.y - FragPos.y, 0.0);
            float radius = 0.6 + height * 0.25;
            float dist = length(FragPos.xz - playerLight.position.xz);
            float shadow = 1.0 - smoothstep(radius * 0.35, radius, dist);
            float shadowStrength = shadow * 0.7 * upFactor;
            result *= (1.0 - shadowStrength);
        }

        // 平台表面细节纹理
        float detail = sin(FragPos.x * 6.0) * sin(FragPos.z * 6.0);
        float grain = 0.9 + 0.1 * (detail * 0.5 + 0.5);
        result *= grain;
        
        // 玩家光源在平台上的额外照明效�?
        float playerLightDistance = length(playerLight.position - FragPos);
        if (playerLightDistance < 4.0) {
            float lightInfluence = 1.0 - smoothstep(0.0, 4.0, playerLightDistance);
            // 根据平台类型调整光源颜色
            vec3 lightColor = vec3(0.15, 0.3, 0.6);
            if (platformType == 1) {
                lightColor = vec3(0.3, 0.2, 0.1);
            } else if (platformType == 2) {
                lightColor = vec3(0.1, 0.3, 0.2);
            } else if (platformType == 3) {
                lightColor = vec3(0.3, 0.1, 0.3);
            } else if (platformType == 4) {
                lightColor = vec3(0.35, 0.2, 0.05);
            }
            result += lightColor * lightInfluence * 0.3;
        }
    }
    
    // 添加全局色调映射，让颜色更自�?
    result = result / (result + vec3(1.0)); // 简单的Reinhard色调映射
    result = pow(result, vec3(1.0/2.2));    // 伽马校正
    
    float alpha = 1.0;
    if (isPlatform && platformType == 3) {
        alpha = 0.78;
    }
    FragColor = vec4(result, alpha);
}
