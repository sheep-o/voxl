#include "Render.hpp"
#include "Game.hpp"
#include "Chunk.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Render::Render() {
    /*
    GLfloat verts[] = {
        0, 0, 1.f,  // 0: Bottom-Front-Left
        1.f, 0, 1.f,   // 1: Bottom-Front-Right
        1.f, 1.f, 1.f,    // 2: Top-Front-Right
        0, 1.f, 1.f,   // 3: Top-Front-Left
        0, 0, 0, // 4: Bottom-Back-Left
        1.f, 0, 0,  // 5: Bottom-Back-Right
        1.f, 1.f, 0,   // 6: Top-Back-Right
        0, 1.f, 0   // 7: Top-Back-Left
    };

    GLuint indices[] = {
        // Front face
        0, 1, 2, 0, 2, 3,
        // Back face
        4, 6, 5, 4, 7, 6,
        // Top face
        3, 2, 6, 3, 6, 7,
        // Bottom face
        4, 5, 1, 4, 1, 0,
        // Right face
        1, 5, 6, 1, 6, 2,
        // Left face
        4, 0, 3, 4, 3, 7
    };

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), nullptr);
    */

    glEnable(GL_DEPTH_TEST);

    m_shader = std::make_unique<Shader>(
        "C:\\Users\\ramik\\OneDrive\\Documentos\\voxl\\src\\vertex.glsl", 
        "C:\\Users\\ramik\\OneDrive\\Documentos\\voxl\\src\\fragment.glsl"
    );
    m_camera = std::make_unique<Camera>(60, glm::vec3{0, 0, 20}, glm::vec3{0, 0, -1});

    chunk.GenTerrain();
    chunk.BuildMesh();
    chunk.Upload();
}

void Render::Draw() {
    m_shader->Use();
    m_shader->UniformMat4("projection", m_camera->GetProj());
    m_shader->UniformMat4("view", m_camera->GetView());

    glClearColor(1, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    chunk.Draw();
}