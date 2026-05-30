#pragma once
#include <glad/glad.h>
#include <string>

class ShaderClass
{
public:
    GLuint ID;
    ShaderClass(const char* vertexFile, const char* fragmentFile);
    void Activate();
    void Delete();

private:
    static std::string get_file_contents(const char* filename);
    static void compileErrors(GLuint shader, const char* type);
};
