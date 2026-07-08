#define GLAPI
#include <glad/glad.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

static void* glad_get_proc(GLADloadproc load, const char* name) {
    void* proc = load ? load(name) : nullptr;
#if defined(_WIN32)
    if (!proc) {
        static HMODULE module = GetModuleHandleA("opengl32.dll");
        if (!module) {
            module = LoadLibraryA("opengl32.dll");
        }
        if (module) {
            proc = reinterpret_cast<void*>(GetProcAddress(module, name));
        }
    }
#endif
    return proc;
}

extern "C" int gladLoadGLLoader(GLADloadproc load) {
    if (!load) {
        return 0;
    }

    int ok = 1;

#define GLAD_LOAD(name) \
    glad_gl##name = reinterpret_cast<decltype(glad_gl##name)>(glad_get_proc(load, "gl" #name)); \
    ok = ok && (glad_gl##name != nullptr)

    GLAD_LOAD(ActiveTexture);
    GLAD_LOAD(AttachShader);
    GLAD_LOAD(BindBuffer);
    GLAD_LOAD(BindTexture);
    GLAD_LOAD(BindVertexArray);
    GLAD_LOAD(BlendFunc);
    GLAD_LOAD(BufferData);
    GLAD_LOAD(Clear);
    GLAD_LOAD(ClearColor);
    GLAD_LOAD(CompileShader);
    GLAD_LOAD(CreateProgram);
    GLAD_LOAD(CreateShader);
    GLAD_LOAD(DeleteBuffers);
    GLAD_LOAD(DeleteProgram);
    GLAD_LOAD(DeleteShader);
    GLAD_LOAD(DeleteTextures);
    GLAD_LOAD(DeleteVertexArrays);
    GLAD_LOAD(Disable);
    GLAD_LOAD(DrawArrays);
    GLAD_LOAD(DrawElements);
    GLAD_LOAD(Enable);
    GLAD_LOAD(EnableVertexAttribArray);
    GLAD_LOAD(GenBuffers);
    GLAD_LOAD(GenTextures);
    GLAD_LOAD(GenVertexArrays);
    GLAD_LOAD(GetIntegerv);
    GLAD_LOAD(GetProgramInfoLog);
    GLAD_LOAD(GetProgramiv);
    GLAD_LOAD(GetShaderInfoLog);
    GLAD_LOAD(GetShaderiv);
    GLAD_LOAD(GetString);
    GLAD_LOAD(GetUniformLocation);
    GLAD_LOAD(LinkProgram);
    GLAD_LOAD(PolygonMode);
    GLAD_LOAD(ShaderSource);
    GLAD_LOAD(TexImage2D);
    GLAD_LOAD(TexParameteri);
    GLAD_LOAD(Uniform1f);
    GLAD_LOAD(Uniform1i);
    GLAD_LOAD(Uniform2fv);
    GLAD_LOAD(Uniform3fv);
    GLAD_LOAD(Uniform4fv);
    GLAD_LOAD(UniformMatrix2fv);
    GLAD_LOAD(UniformMatrix3fv);
    GLAD_LOAD(UniformMatrix4fv);
    GLAD_LOAD(UseProgram);
    GLAD_LOAD(VertexAttribPointer);
    GLAD_LOAD(Viewport);

#undef GLAD_LOAD

    return ok;
}
