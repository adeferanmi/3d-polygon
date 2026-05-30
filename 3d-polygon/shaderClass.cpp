#include "shaderClass.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>

std::string ShaderClass::get_file_contents(const char* filename)
{
    std::ifstream in(filename, std::ios::binary);
    if (!in) throw std::runtime_error(std::string("Cannot open shader file: ") + filename);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

ShaderClass::ShaderClass(const char* vertexFile, const char* fragmentFile)
{
    std::string vertCode = get_file_contents(vertexFile);
    std::string fragCode = get_file_contents(fragmentFile);
    const char* vertSrc = vertCode.c_str();
    const char* fragSrc = fragCode.c_str();

    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vertSrc, nullptr);
    glCompileShader(vert);
    compileErrors(vert, "VERTEX");

    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fragSrc, nullptr);
    glCompileShader(frag);
    compileErrors(frag, "FRAGMENT");

    ID = glCreateProgram();
    glAttachShader(ID, vert);
    glAttachShader(ID, frag);
    glLinkProgram(ID);
    compileErrors(ID, "PROGRAM");

    glDeleteShader(vert);
    glDeleteShader(frag);
}

void ShaderClass::Activate() { glUseProgram(ID); }
void ShaderClass::Delete()   { glDeleteProgram(ID); }

void ShaderClass::compileErrors(GLuint shader, const char* type)
{
    GLint hasCompiled;
    char infoLog[1024];
    if (std::string(type) != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE)
        {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cout << "SHADER_COMPILATION_ERROR for: " << type << "\n" << infoLog << "\n";
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE)
        {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cout << "SHADER_LINKING_ERROR for: " << type << "\n" << infoLog << "\n";
        }
    }
}
