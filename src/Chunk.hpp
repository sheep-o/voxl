#ifndef VOXL_CHUNK_HPP_
#define VOXL_CHUNK_HPP_

#include <vector>
#include <array>
#include <glm/glm.hpp>
#include <GL/glew.h>

static constexpr int CHUNK_WIDTH = 16;
static constexpr int CHUNK_HEIGHT = 16;
static constexpr int CHUNK_DEPTH = 16;

enum class Block {
    AIR,
    STONE
};

class Chunk {
public:
    Chunk();
    ~Chunk();

    void BuildMesh();
    void Upload();
    void Draw();
    void Place(glm::vec3 &pos, Block type);
    void GenTerrain();
private:
    GLuint m_vao = 0, m_vbo = 0, m_ebo = 0;

    struct Vertex {
        GLfloat x,y,z,w,h;
    };

    std::vector<Vertex> m_verts;
    std::vector<GLuint> m_indices;
    std::array<Block, CHUNK_WIDTH*CHUNK_HEIGHT*CHUNK_DEPTH> m_blocks;

    int Index(int x, int y, int z) {
        return x + CHUNK_WIDTH * (z + CHUNK_DEPTH * y);
    }
};

#endif