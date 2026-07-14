#include "Chunk.hpp"
#include <cmath>
#include "PerlinNoise.hpp"

Chunk::Chunk() {
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

/*
const Vertex cubeVerts[] = {
    // Front (+Z)
    {0, 0, 1, 0, 0},
    {1, 0, 1, 1, 0},
    {1, 1, 1, 1, 1},
    {0, 1, 1, 0, 1},

    // Back (-Z)
    {1, 0, 0, 0, 0},
    {0, 0, 0, 1, 0},
    {0, 1, 0, 1, 1},
    {1, 1, 0, 0, 1},

    // Top (+Y)
    {0, 1, 1, 0, 0},
    {1, 1, 1, 1, 0},
    {1, 1, 0, 1, 1},
    {0, 1, 0, 0, 1},

    // Bottom (-Y)
    {0, 0, 0, 0, 0},
    {1, 0, 0, 1, 0},
    {1, 0, 1, 1, 1},
    {0, 0, 1, 0, 1},

    // Right (+X)
    {1, 0, 1, 0, 0},
    {1, 0, 0, 1, 0},
    {1, 1, 0, 1, 1},
    {1, 1, 1, 0, 1},

    // Left (-X)
    {0, 0, 0, 0, 0},
    {0, 0, 1, 1, 0},
    {0, 1, 1, 1, 1},
    {0, 1, 0, 0, 1}
};

const GLuint cubeIndices[] = {
    0, 1, 2, 0, 2, 3,
    4, 5, 6, 4, 6, 7,
    8, 9, 10, 8, 10, 11,
    12, 13, 14, 12, 14, 15,
    16, 17, 18, 16, 18, 19,
    20, 21, 22, 20, 22, 23
};
*/

void Chunk::BuildMesh() {
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

                float t = 0.5;
                float du = (b == Block::STONE) ? 0.5f : 0;


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

void Chunk::Draw() {
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
}

void Chunk::Place(glm::vec3 &pos, Block type) {
    int i = Index(pos.x, pos.y, pos.z);
    if (i >= m_blocks.size()) return;

    m_blocks[i] = type;
}

void Chunk::GenTerrain() {
    constexpr int base_height = 10;
    constexpr float amplitude = 8.0f;

    siv::PerlinNoise perlin{ std::random_device{} };

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            int terrain_height =
                base_height + perlin.octave2D(x, z, 10);

            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                Block block = Block::AIR;

                if (y == 0) {
                    block = Block::STONE;
                } else if (y < terrain_height - 3) {
                    block = Block::STONE;
                } else if (y < terrain_height) {
                    block = Block::DIRT;
                } else if (y == terrain_height) {
                    block = Block::GRASS;
                }

                m_blocks[Index(x, y, z)] = block;
            }
        }
    }
}