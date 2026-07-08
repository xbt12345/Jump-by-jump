# Jump-by-jump / Music Jump 3D

一个基于 OpenGL 的 3D 音乐节奏跳跃小游戏。

## 项目简介

本项目使用 C++17 + OpenGL 3.3 编写，玩家控制角色在不断生成的平台上跳跃，同时配合音乐节奏获得更丰富的游戏体验。

## 功能特性

- 3D 场景渲染（OpenGL 3.3 Core Profile）
- 自定义着色器系统（Shader）
- 粒子特效系统（ParticleSystem）
- 音乐可视化（MusicVisualizer）
- 音频分析（AudioAnalyzer）
- 程序化关卡生成（LevelGenerator）
- 支持普通模式与节奏模式切换

## 技术栈

- C++17
- OpenGL 3.3
- GLFW（窗口与输入）
- GLAD（OpenGL 函数加载）
- GLM（数学库）
- stb_image（纹理加载）

## 项目结构

```
.
├── Balls/              # 球体纹理资源
├── include/            # 头文件（GLFW、GLAD、GLM、KHR）
├── lib/                # 预编译库（GLFW、GLAD）
├── music/              # 音乐资源
├── output/             # 编译输出目录
├── shaders/            # GLSL 着色器
├── src/                # 源代码
│   ├── audio/          # 音频分析
│   ├── core/           # 引擎核心（Renderer、Camera、Input、UI、Engine）
│   ├── game/           # 游戏逻辑（Player、Platform、LevelGenerator）
│   ├── geometry/       # 几何体生成
│   ├── graphics/       # 图形特效（Shader、ParticleSystem、MusicVisualizer）
│   ├── main/           # 程序入口
│   └── third_party/    # 第三方源码（glad_stub）
├── Makefile            # 构建脚本
└── README.md           # 本文件
```

## 构建与运行

### Windows（MinGW）

1. 确保已安装 MinGW-w64 或 MSYS2，并将 `g++` 加入环境变量。
2. 在项目根目录执行：

```bash
make
```

3. 编译完成后，可执行文件位于 `output/main.exe`。双击运行或在终端执行：

```bash
./output/main.exe
```

### macOS / Linux

```bash
make
./output/main
```

> 注：macOS/Linux 需要自行安装 GLFW 开发库。

## 操作说明

- `A` / `D`：左右移动
- `SPACE`：开始游戏 / 跳跃

## 依赖说明

- 仓库已包含 Windows 下所需的 GLFW 与 GLAD 预编译库（`lib/`）。
- `include/glm/` 为 GLM 数学库源码，已作为普通文件提交，无需额外配置。
- 音乐文件 `music/experience.wav` 已包含在仓库中。

## 许可证

本项目为个人学习作品，相关第三方库归各自原作者所有。
