#include "Render.hpp"
#include "Game.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <unordered_map>
#include <queue>
#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Render::Render() {
    glEnable(GL_DEPTH_TEST);

    m_shader = std::make_shared<Shader>(
        "../../../src/vertex.glsl",
        "../../../src/fragment.glsl"
    );
    m_camera = std::make_shared<Camera>(60, glm::vec3{0, 10, 0}, glm::vec3{0, 0, -1});
    m_last_pos = m_camera->GetPos();
    m_mesher = std::make_unique<MeshingEngine>(8, this);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, channels;
    unsigned char *data = stbi_load("../../../src/atlas.jpg", &width, &height, &channels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Failed to load texture" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    stbi_image_free(data);

    int x = std::floor(m_camera->GetPos().x) / CHUNK_WIDTH;
    int z = std::floor(m_camera->GetPos().z) / CHUNK_WIDTH;
    for (int i = x - RADIUS; i <= x + RADIUS; i++) {
        for (int j = z - RADIUS; j <= z + RADIUS; j++) {
            m_mesher->Request(glm::ivec3{i, 0, j});
        }
    }
}

void Render::Draw() {
    m_shader->Use();

    int lx = std::floor(m_last_pos.x) / CHUNK_WIDTH;
    int lz = std::floor(m_last_pos.z) / CHUNK_WIDTH;
    int x = std::floor(m_camera->GetPos().x) / CHUNK_WIDTH;
    int z = std::floor(m_camera->GetPos().z) / CHUNK_WIDTH;

    m_mesher->Upload();

    if (x != lx || z != lz) {
        m_mesher->UnloadFarChunks(glm::ivec3{x, 0, z});

        for (int i = x - RADIUS; i <= x + RADIUS; i++) {
            for (int j = z - RADIUS; j <= z + RADIUS; j++) {
                m_mesher->Request(glm::ivec3{i, 0, j});
            }
        }
    }

    m_shader->UniformMat4("projection", m_camera->GetProj());
    m_shader->UniformMat4("view", m_camera->GetView());

    glClearColor(1, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_mesher->Draw(m_shader, m_camera);

    m_last_pos = m_camera->GetPos();
}