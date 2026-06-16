#include "Render.hpp"
#include "Game.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Render::Render() {
    GLfloat verts[] = {
        0.5, 0.5, 0,
        0.5, -0.5, 0,
        -0.5, -0.5, 0,

        -0.5, -0.5, 0,
        -0.5, 0.5, 0,
        0.5, 0.5, 0,
    };

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), nullptr);

    glEnable(GL_DEPTH_TEST);

    m_shader = std::make_unique<Shader>("src/vertex.glsl", "src/fragment.glsl");
    m_camera = std::make_unique<Camera>(60, glm::vec3{0, 0, 3}, glm::vec3{0, 0, -1});
}

void Render::Draw() {
    m_shader->Use();
    m_shader->UniformMat4("projection", m_camera->GetProj());
    m_shader->UniformMat4("view", m_camera->GetView());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1, 0, 1, 1);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}