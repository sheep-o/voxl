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

    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_DEPTH; z++) {

                if (m_blocks[Index(x, y, z)] == Block::AIR) 
                    continue;

                int base = m_verts.size();

                float xf = static_cast<GLfloat>(x);
                float yf = static_cast<GLfloat>(y);
                float zf = static_cast<GLfloat>(z);

                for (const Vertex &v : cubeVerts) {
                    m_verts.push_back({v.x+xf, v.y+yf, v.z+zf, v.w, v.h});
                }

                for (int b : cubeIndices)
                    m_indices.push_back(base + b);

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
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_DEPTH; z++) {
                m_blocks[Index(x, y, z)] = Block::STONE;
            }
        }
    }
}