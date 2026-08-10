#include "Chunk.hpp"
#include <cmath>
#include "PerlinNoise.hpp"
#include <glm/gtc/matrix_transform.hpp>

Chunk::Chunk() {
    m_pos = {0,0,0};
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    m_blocks.fill(Block::AIR);
}

Chunk::Chunk(const glm::ivec3 pos) {
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

    m_uploaded = true;
}

void Chunk::Draw(Shader &shader) const {
    glm::mat4 model{1.f};
    model = glm::translate(
        model, 
        glm::vec3(m_pos)*static_cast<float>(CHUNK_WIDTH)
    );
    shader.UniformMat4("model", model);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
}

void Chunk::Place(const glm::ivec3 &pos, const Block type) {
    const int i = Index(pos.x, pos.y, pos.z);
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

    m_state = State::GENERATED;
}

Chunk::Block Chunk::GetBlock(const glm::ivec3 pos) const {
    assert(pos.x < CHUNK_WIDTH && pos.y < CHUNK_WIDTH && pos.z < CHUNK_WIDTH);

    return m_blocks[Index(pos.x, pos.y, pos.z)];
}
