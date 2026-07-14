#include "Render.hpp"
#include "Game.hpp"
#include "Chunk.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Render::Render() {
    glEnable(GL_DEPTH_TEST);

    m_shader = std::make_unique<Shader>(
        "C:\\Users\\ramik\\OneDrive\\Documentos\\voxl\\src\\vertex.glsl", 
        "C:\\Users\\ramik\\OneDrive\\Documentos\\voxl\\src\\fragment.glsl"
    );
    m_camera = std::make_unique<Camera>(60, glm::vec3{0, 0, 20}, glm::vec3{0, 0, -1});

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, channels;
    unsigned char *data = stbi_load("C:\\Users\\ramik\\OneDrive\\Documentos\\voxl\\src\\atlas.jpg", &width, &height, &channels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Failed to load texture" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    stbi_image_free(data);

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

    chunk.Draw();
}