#include "Chunk.hpp"
#include <cmath>
#include "PerlinNoise.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Chunk::Chunk() {
    m_pos = {0,0,0};
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    m_blocks.fill(Block::AIR);
}

Chunk::Chunk(glm::vec3 &pos) {
    m_pos = pos;
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    m_blocks.fill(Block::AIR);
}

Chunk::~Chunk() {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
}

void Chunk::BuildMesh(Render *render) {
    m_verts.clear();
    m_indices.clear();

    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_DEPTH; z++) {

                auto b = m_blocks[Index(x,y,z)];
                if (b == Block::AIR) 
                    continue;

                float xf = static_cast<GLfloat>(x);
                float yf = static_cast<GLfloat>(y);
                float zf = static_cast<GLfloat>(z);

                float xr = xf + m_pos.x*CHUNK_WIDTH;
                float yr = yf + m_pos.y*CHUNK_WIDTH;
                float zr = zf + m_pos.z*CHUNK_WIDTH;

                float t = 0.5;
                float du = (b == Block::STONE) ? 0.5f : 0;

                bool draw_PZ = 
                    // Edge of render distance
                    ((z == CHUNK_DEPTH - 1) && (!render->ChunkExists(m_pos + glm::vec3{0,0,1}))) || 
                    // Non-edge next to air
                    ((z != CHUNK_DEPTH - 1) && m_blocks[Index(x, y, z + 1)] == Block::AIR) || 
                    // Edge next to air 
                    ((z == CHUNK_DEPTH - 1) && render->GetChunk(m_pos + glm::vec3{0,0,1})->GetBlock(glm::vec3{x, y, 0}) == Block::AIR);

                // Front (+Z)
                if (z == CHUNK_DEPTH - 1 ||
                    m_blocks[Index(x, y, z + 1)] == Block::AIR) {
                    
                    
                    GLuint base = static_cast<GLuint>(m_verts.size());

                    m_verts.push_back({xf,     yf,     zf + 1, du, 0});
                    m_verts.push_back({xf + 1, yf,     zf + 1, t+du, 0});
                    m_verts.push_back({xf + 1, yf + 1, zf + 1, t+du, t});
                    m_verts.push_back({xf,     yf + 1, zf + 1, du, t});

                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 1);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 3);
                }

                // Back (-Z)
                if (z == 0 ||
                    m_blocks[Index(x, y, z - 1)] == Block::AIR) {
                    
                    GLuint base = static_cast<GLuint>(m_verts.size());

                    m_verts.push_back({xf + 1, yf,     zf, du, 0});
                    m_verts.push_back({xf,     yf,     zf, t+du, 0});
                    m_verts.push_back({xf,     yf + 1, zf, t+du, t});
                    m_verts.push_back({xf + 1, yf + 1, zf, du, t});

                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 1);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 3);
                }

                // Top (+Y)
                if (y == CHUNK_HEIGHT - 1 ||
                    m_blocks[Index(x, y + 1, z)] == Block::AIR) {
                    
                    GLuint base = static_cast<GLuint>(m_verts.size());

                    if (b == Block::GRASS) {
                        m_verts.push_back({xf,     yf + 1, zf + 1, 0, 0.5f});
                        m_verts.push_back({xf + 1, yf + 1, zf + 1, 0.5f, 0.5f});
                        m_verts.push_back({xf + 1, yf + 1, zf,     0.5f, 1.f});
                        m_verts.push_back({xf,     yf + 1, zf,     0, 1.f});
                    } else {
                        m_verts.push_back({xf,     yf + 1, zf + 1, du, 0});
                        m_verts.push_back({xf + 1, yf + 1, zf + 1, t+du, 0});
                        m_verts.push_back({xf + 1, yf + 1, zf,     t+du, t});
                        m_verts.push_back({xf,     yf + 1, zf,     du, t});
                    }

                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 1);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 3);
                }

                // Bottom (-Y)
                if (y == 0 ||
                    m_blocks[Index(x, y - 1, z)] == Block::AIR) {
                    
                    GLuint base = static_cast<GLuint>(m_verts.size());

                    m_verts.push_back({xf,     yf, zf,     du, 0});
                    m_verts.push_back({xf + 1, yf, zf,     t+du, 0});
                    m_verts.push_back({xf + 1, yf, zf + 1, t+du, t});
                    m_verts.push_back({xf,     yf, zf + 1, du, t});

                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 1);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 3);
                }

                // Right (+X)
                if (x == CHUNK_WIDTH - 1 ||
                    m_blocks[Index(x + 1, y, z)] == Block::AIR) {
                    
                    GLuint base = static_cast<GLuint>(m_verts.size());

                    m_verts.push_back({xf + 1, yf,     zf + 1, du, 0});
                    m_verts.push_back({xf + 1, yf,     zf,     t+du, 0});
                    m_verts.push_back({xf + 1, yf + 1, zf,     t+du, t});
                    m_verts.push_back({xf + 1, yf + 1, zf + 1, du, t});

                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 1);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 3);
                }

                // Left (-X)
                if (x == 0 ||
                    m_blocks[Index(x - 1, y, z)] == Block::AIR) {
                    
                    GLuint base = static_cast<GLuint>(m_verts.size());

                    m_verts.push_back({xf, yf,     zf,     du, 0});
                    m_verts.push_back({xf, yf,     zf + 1, t+du, 0});
                    m_verts.push_back({xf, yf + 1, zf + 1, t+du, t});
                    m_verts.push_back({xf, yf + 1, zf,     du, t});

                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 1);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 3);
                }
            }
        }
    }
}

void Chunk::Upload() {
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_verts.size()*sizeof(Vertex), m_verts.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size()*sizeof(GLuint), m_indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*5, nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*5, reinterpret_cast<void *>(sizeof(GLfloat)*3));
}

void Chunk::Draw(Shader &shader) {
    glm::mat4 model{1.f};
    model = glm::translate(model, m_pos*static_cast<float>(CHUNK_WIDTH));
    shader.UniformMat4("model", model);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
}

void Chunk::Place(glm::vec3 &pos, Block type) {
    int i = Index(pos.x, pos.y, pos.z);
    if (i >= m_blocks.size()) return;

    m_blocks[i] = type;
}

void Chunk::GenTerrain() {
    constexpr int baseHeight = 7;
    constexpr double amplitude = 10.0;
    constexpr double frequency = 0.035;

    static const siv::PerlinNoise perlin{12345u};

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            int worldX = x + m_pos.x * CHUNK_WIDTH;
            int worldZ = z + m_pos.z * CHUNK_DEPTH;

            double noise = perlin.octave2D_01(
                worldX * frequency,
                worldZ * frequency,
                4
            );

            int terrainHeight =
                baseHeight + static_cast<int>(noise * amplitude);

            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                Block block = Block::AIR;

                if (y == 0) {
                    block = Block::STONE;
                } else if (y < terrainHeight - 3) {
                    block = Block::STONE;
                } else if (y < terrainHeight) {
                    block = Block::DIRT;
                } else if (y == terrainHeight) {
                    block = Block::GRASS;
                }

                m_blocks[Index(x, y, z)] = block;
            }
        }
    }
}

Block Chunk::GetBlock(glm::ivec3 pos) {
    assert(pos.x < CHUNK_WIDTH && pos.y < CHUNK_WIDTH && pos.z < CHUNK_WIDTH);

    return m_blocks[Index(pos.x, pos.y, pos.z)];
}