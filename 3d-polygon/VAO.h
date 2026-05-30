#pragma once
#include <glad/glad.h>

class VAO
{
public:
    GLuint ID;
    VAO();
    void LinkVBO(class VBO& vbo, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
    void Bind();
    void Unbind();
    void Delete();
};
