#pragma once
#include <glad/glad.h>
#include <string>

class Texture
{
public:
    GLuint ID;
    GLenum type;

    Texture(const char* image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType);
    void TexUnit(class ShaderClass& shader, const char* uniform, GLuint unit);
    void Bind();
    void Unbind();
    void Delete();
};
