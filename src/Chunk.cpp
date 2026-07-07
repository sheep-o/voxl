#include "Chunk.hpp"

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

void Chunk::BuildMesh() {
    m_verts.clear();
    m_indices.clear();

    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_DEPTH; z++) {

                if (m_blocks[Index(x, y, z)] == Block::AIR) 
                    continue;

                int base = m_verts.size();
                m_verts.push_back({x, y, z+1});
                m_verts.push_back({x+1, y, z+1});
                m_verts.push_back({x+1, y+1, z+1});
                m_verts.push_back({x, y+1, z+1});
                m_verts.push_back({x, y, z});
                m_verts.push_back({x+1, y, z});
                m_verts.push_back({x+1, y+1, z});
                m_verts.push_back({x, y+1, z});

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

                for (int b : indices)
                    m_indices.push_back(base + b);

            }
        }
    }
}

void Chunk::Upload() {
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_verts.size()*sizeof(glm::vec3), m_verts.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size()*sizeof(GLuint), m_indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*3, nullptr);
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
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_DEPTH; z++) {
                m_blocks[Index(x, y, z)] = Block::STONE;
            }
        }
    }
}