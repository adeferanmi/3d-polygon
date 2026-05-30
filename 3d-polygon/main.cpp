#include <iostream>
#include <cstdint>
#include <cmath>
#include <array>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaderClass.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "Texture.h"

// ---------------------------------------------------------------------------
// Icosahedron geometry
// ---------------------------------------------------------------------------
// Golden ratio
static const float PHI = (1.0f + std::sqrt(5.0f)) / 2.0f;

// 12 unique vertices of a unit icosahedron (normalised)
static glm::vec3 icoVerts[12] = {
    glm::normalize(glm::vec3(-1,  PHI,  0)),
    glm::normalize(glm::vec3(1,  PHI,  0)),
    glm::normalize(glm::vec3(-1, -PHI,  0)),
    glm::normalize(glm::vec3(1, -PHI,  0)),
    glm::normalize(glm::vec3(0, -1,  PHI)),
    glm::normalize(glm::vec3(0,  1,  PHI)),
    glm::normalize(glm::vec3(0, -1, -PHI)),
    glm::normalize(glm::vec3(0,  1, -PHI)),
    glm::normalize(glm::vec3(PHI,  0, -1)),
    glm::normalize(glm::vec3(PHI,  0,  1)),
    glm::normalize(glm::vec3(-PHI,  0, -1)),
    glm::normalize(glm::vec3(-PHI,  0,  1)),
};

// 20 triangular faces (CCW winding)
static GLuint icoFaces[20][3] = {
    {0,11, 5},{0, 5, 1},{0, 1, 7},{0, 7,10},{0,10,11},
    {1, 5, 9},{5,11, 4},{11,10, 2},{10, 7, 6},{7, 1, 8},
    {3, 9, 4},{3, 4, 2},{3, 2, 6},{3, 6, 8},{3, 8, 9},
    {4, 9, 5},{2, 4,11},{6, 2,10},{8, 6, 7},{9, 8, 1},
};

// Build flat-normal vertex buffer (each face gets its own 3 verts so normals
// are per-face, giving the hard-edge look seen in the reference image).
// Layout per vertex: pos(3) + texCoord(2) + normal(3) = 8 floats
static void buildIcosahedron(std::vector<GLfloat>& verts, std::vector<GLuint>& indices)
{
    verts.clear();
    indices.clear();

    GLuint idx = 0;
    for (auto& face : icoFaces)
    {
        glm::vec3 a = icoVerts[face[0]];
        glm::vec3 b = icoVerts[face[1]];
        glm::vec3 c = icoVerts[face[2]];

        // Face normal (flat shading)
        glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));

        // Simple planar UV projection per face
        // Map the three corners to UV (0,0),(1,0),(0.5,1) for an equilateral feel
        std::array<glm::vec2, 3> uv = {
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.5f, 1.0f)
        };

        for (int i = 0; i < 3; ++i)
        {
            glm::vec3& p = (i == 0) ? a : (i == 1) ? b : c;
            verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z);
            verts.push_back(uv[i].x); verts.push_back(uv[i].y);
            verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z);
            indices.push_back(idx++);
        }
    }
}

// ---------------------------------------------------------------------------
// Window resize callback
// ---------------------------------------------------------------------------
static void framebuffer_size_callback(GLFWwindow*, int w, int h)
{
    glViewport(0, 0, w, h);
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
auto main() -> std::int32_t
{
    // --- Init GLFW ---
    if (!glfwInit()) { std::cerr << "GLFW init failed\n"; return -1; }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const int WIN_W = 800, WIN_H = 800;
    GLFWwindow* window = glfwCreateWindow(WIN_W, WIN_H, "Icosahedron", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glViewport(0, 0, WIN_W, WIN_H);
    glEnable(GL_DEPTH_TEST);

    // --- Build geometry ---
    std::vector<GLfloat> verts;
    std::vector<GLuint>  indices;
    buildIcosahedron(verts, indices);

    // --- GPU objects ---
    VAO vao;
    vao.Bind();

    VBO vbo(verts.data(), (GLsizeiptr)(verts.size() * sizeof(GLfloat)));
    EBO ebo(indices.data(), (GLsizeiptr)(indices.size() * sizeof(GLuint)));

    constexpr GLsizeiptr stride = 8 * sizeof(GLfloat);
    vao.LinkVBO(vbo, 0, 3, GL_FLOAT, stride, (void*)0);                          // position
    vao.LinkVBO(vbo, 1, 2, GL_FLOAT, stride, (void*)(3 * sizeof(GLfloat)));      // texCoord
    vao.LinkVBO(vbo, 2, 3, GL_FLOAT, stride, (void*)(5 * sizeof(GLfloat)));      // normal

    vao.Unbind();
    vbo.Unbind();
    ebo.Unbind();

    // --- Shader ---
    // Paths are relative to where the executable runs (project root / working dir).
    // Adjust if your VS working directory differs.
    ShaderClass shader("Shaders/default.vert", "Shaders/default.frag");

    // --- Texture ---
    // Place concrete.png in the same folder as the executable (or adjust path).
    Texture concrete("Textures/concrete.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    concrete.TexUnit(shader, "tex0", 0);

    // --- Uniforms ---
    shader.Activate();
    glUniform3f(glGetUniformLocation(shader.ID, "lightColor"), 1.0f, 1.0f, 1.0f);
    glUniform3f(glGetUniformLocation(shader.ID, "lightPos"), 4.0f, 6.0f, 4.0f);

    // Projection (fixed)
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)WIN_W / (float)WIN_H,
        0.1f, 100.0f
    );
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE,
        glm::value_ptr(projection));

    // View (fixed camera)
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.5f),   // camera position
        glm::vec3(0.0f, 0.0f, 0.0f),   // look-at target
        glm::vec3(0.0f, 1.0f, 0.0f)    // up vector
    );
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE,
        glm::value_ptr(view));

    // --- Render loop ---
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Activate();
        concrete.Bind();

        // Rotate over time
        float time = (float)glfwGetTime();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, time * 0.6f, glm::vec3(0.0f, 1.0f, 0.0f)); // Y axis spin
        model = glm::rotate(model, time * 0.25f, glm::vec3(1.0f, 0.0f, 0.0f)); // slight X tilt
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE,
            glm::value_ptr(model));

        vao.Bind();
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, nullptr);
        vao.Unbind();

        concrete.Unbind();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Cleanup ---
    vao.Delete();
    vbo.Delete();
    ebo.Delete();
    concrete.Delete();
    shader.Delete();
    glfwTerminate();
    return 0;
}