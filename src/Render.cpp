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
        "../../src/vertex.glsl", 
        "../../src/fragment.glsl"
    );
    m_camera = std::make_unique<Camera>(60, glm::vec3{0, 10, 0}, glm::vec3{0, 0, -1});
    m_last_pos = m_camera->GetPos();
    m_mesher = std::make_unique<MeshingEngine>(4, this);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, channels;
    unsigned char *data = stbi_load("../../src/atlas.jpg", &width, &height, &channels, 0);
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
            /*
            auto chunk = std::make_shared<Chunk>(glm::ivec3{i, 0, j});
            chunk->GenTerrain();
            m_unbuilt_chunks.push(chunk);
            m_chunks.emplace(chunk->GetPos(), chunk);
            */

            m_mesher->Request(glm::ivec3{i, 0, j});
        }
    }
}

/*
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
            glm::ivec3 pos = it->first;
            it = m_chunks.erase(it);

            glm::ivec3 offsets[4] = {{1,0,0}, {-1,0,0}, {0,0,1}, {0,0,-1}};
            for (auto& offset : offsets) {
                auto neighbor = GetChunk(pos + offset);
                if (neighbor && neighbor->IsBuilt()) {
                    m_unbuilt_chunks.push(neighbor);
                }
            }
        } else {
            ++it;
        }
    }
}
*/

void Render::Draw() {
    m_shader->Use();

    int lx = std::floor(m_last_pos.x) / CHUNK_WIDTH;
    int lz = std::floor(m_last_pos.z) / CHUNK_WIDTH;
    int x = std::floor(m_camera->GetPos().x) / CHUNK_WIDTH;
    int z = std::floor(m_camera->GetPos().z) / CHUNK_WIDTH;

    m_mesher->Upload();

    if (x != lx || z != lz) {
        //UnloadFarChunks(glm::ivec3{x, 0, z});
        m_mesher->UnloadFarChunks(glm::ivec3{x, 0, z});

        for (int i = x - RADIUS; i <= x + RADIUS; i++) {
            for (int j = z - RADIUS; j <= z + RADIUS; j++) {
                /*
                if (m_chunks.find(glm::ivec3{i, 0, j}) == m_chunks.end()) {
                    auto chunk = std::make_shared<Chunk>(glm::ivec3{i, 0, j});
                    chunk->GenTerrain();
                    m_unbuilt_chunks.push(chunk);
                    m_chunks.emplace(chunk->GetPos(), chunk);

                    glm::ivec3 offsets[4] = {{1,0,0}, {-1,0,0}, {0,0,1}, {0,0,-1}};
                    for (auto& offset : offsets) {
                        auto neighbor = GetChunk(glm::ivec3{i, 0, j} + offset);
                        if (neighbor && neighbor->IsBuilt()) {
                            m_unbuilt_chunks.push(neighbor);
                        }
                    }
                }
                */

                std::cout << "Requesting chunk at (" << i << ", 0, " << j << ")" << std::endl;
                m_mesher->Request(glm::ivec3{i, 0, j});
            }
        }
    }

    /*
    for (int i = 0; i < 4; i++) {
        if (m_unbuilt_chunks.empty()) break;
        std::shared_ptr<Chunk> chunk = m_unbuilt_chunks.front();
        m_unbuilt_chunks.pop();

        chunk->BuildMesh(this);
        chunk->Upload();
    }
    */

    m_shader->UniformMat4("projection", m_camera->GetProj());
    m_shader->UniformMat4("view", m_camera->GetView());

    glClearColor(1, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /*
    for (auto &[coord, c] : m_chunks) {
        if (c->IsBuilt()) {
            c->Draw(*m_shader);
        }
    }
    */
    m_mesher->Draw(*m_shader);

    m_last_pos = m_camera->GetPos();
}

/*
bool Render::ChunkExists(glm::ivec3 pos) {
    return m_chunks.find(pos) != m_chunks.end();
}

std::shared_ptr<Chunk> Render::GetChunk(glm::ivec3 pos) {
    auto it = m_chunks.find(pos);
    if (it == m_chunks.end()) {
        return nullptr;
    }

    return it->second;
}

// hmmmmmm
Block Render::GetBlock(glm::ivec3 pos) {
    auto int_floor_div = [](int num, int denom) {
        int res = num / denom;
        int rem = num % denom;
        if (rem != 0 && ((num < 0) ^ (denom < 0))) {
            res -= 1;
        }
        return res;
    };

    glm::ivec3 chunk_coord{
        int_floor_div(pos.x, CHUNK_WIDTH),
        int_floor_div(pos.y, CHUNK_HEIGHT),
        int_floor_div(pos.z, CHUNK_DEPTH)
    };

    auto it = m_chunks.find(chunk_coord);
    if (it == m_chunks.end()) {
        return Block::AIR;
    }

    auto wrapIndex = [](int i, int i_max) {
        return ((i % i_max) + i_max) % i_max;
    };

    glm::ivec3 local_pos{
        wrapIndex(pos.x, CHUNK_WIDTH),
        wrapIndex(pos.y, CHUNK_HEIGHT),
        wrapIndex(pos.z, CHUNK_DEPTH)
    };

    return it->second->GetBlock(local_pos);
}
*/