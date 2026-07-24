#include "Render.hpp"
#include "Game.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <unordered_map>
#include <queue>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Render::Render() {
    glEnable(GL_DEPTH_TEST);

    m_shader = std::make_shared<Shader>(
        "C:\\Users\\ramik\\OneDrive\\Documentos\\voxl\\src\\vertex.glsl", 
        "C:\\Users\\ramik\\OneDrive\\Documentos\\voxl\\src\\fragment.glsl"
    );
    m_camera = std::make_unique<Camera>(60, glm::vec3{0, 10, 0}, glm::vec3{0, 0, -1});
    m_last_pos = m_camera->GetPos();

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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

    int x = std::floor(m_camera->GetPos().x) / CHUNK_WIDTH;
    int z = std::floor(m_camera->GetPos().z) / CHUNK_WIDTH;
    for (int i = x - RADIUS; i <= x + RADIUS; i++) {
        for (int j = z - RADIUS; j <= z + RADIUS; j++) {
            m_unbuilt_chunks.push(
                std::make_shared<Chunk>(glm::vec3{i, 0, j})
            );
        }
    }
}

void Render::UnloadFarChunks(const glm::ivec3 &cam_chunk) {
    for (auto it = m_chunks.begin(); it != m_chunks.end(); ) {
        const glm::ivec3& coord = it->first;

        int dx = coord.x - cam_chunk.x;
        int dy = coord.y - cam_chunk.y;
        int dz = coord.z - cam_chunk.z;

        bool tooFar =
            std::abs(dx) > RADIUS ||
            std::abs(dz) > RADIUS;
        if (tooFar) {
            it = m_chunks.erase(it);
        } else {
            ++it;
        }
    }
}

void Render::Draw() {
    int lx = std::floor(m_last_pos.x) / CHUNK_WIDTH;
    int lz = std::floor(m_last_pos.z) / CHUNK_WIDTH;
    int x = std::floor(m_camera->GetPos().x) / CHUNK_WIDTH;
    int z = std::floor(m_camera->GetPos().z) / CHUNK_WIDTH;

    if (x != lx || z != lz) {
        UnloadFarChunks(glm::ivec3{x, 0, z});

        for (int i = x - RADIUS; i <= x + RADIUS; i++) {
            for (int j = z - RADIUS; j <= z + RADIUS; j++) {
                if (m_chunks.find(glm::ivec3{i, 0, j}) == m_chunks.end()) {
                    m_unbuilt_chunks.push(
                        std::make_shared<Chunk>(glm::vec3{i, 0, j})
                    );
                }
            }
        }
    }

    for (int i = 0; i < 4; i++) {
        if (m_unbuilt_chunks.empty()) break;
        std::shared_ptr<Chunk> chunk = m_unbuilt_chunks.front();
        m_unbuilt_chunks.pop();

        chunk->GenTerrain();
        chunk->BuildMesh(this);
        chunk->Upload();
        m_chunks.emplace(chunk->GetPos(), chunk);
    }

    m_shader->Use();
    m_shader->UniformMat4("projection", m_camera->GetProj());
    m_shader->UniformMat4("view", m_camera->GetView());

    glClearColor(1, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto &[coord, c] : m_chunks) {
        c->Draw(*m_shader);
    }

    m_last_pos = m_camera->GetPos();
}

bool Render::ChunkExists(glm::ivec3 pos) {
    return m_chunks.find(pos) != m_chunks.end();
}

std::shared_ptr<Chunk> Render::GetChunk(glm::ivec3 pos) {
    auto it = m_chunks.find(pos);
    assert(it != m_chunks.end());

    return it->second;
}