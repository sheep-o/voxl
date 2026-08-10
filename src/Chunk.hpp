#ifndef VOXL_CHUNK_HPP_
#define VOXL_CHUNK_HPP_

#include <vector>
#include <array>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "Shader.hpp"
#include "Render.hpp"

class Render;

static constexpr int CHUNK_WIDTH = 16;
static constexpr int CHUNK_HEIGHT = 16;
static constexpr int CHUNK_DEPTH = 16;
static constexpr int CHUNK_SIZE = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH;

enum class Block {
    AIR,
    STONE,
    DIRT,
    GRASS
};

class Chunk {
public:
    Chunk();
    Chunk(glm::ivec3 pos);
    ~Chunk();

    void BuildMesh();
    void Upload();
    void Draw(Shader &shader);
    void Place(glm::ivec3 &pos, Block type);
    void GenTerrain();
    glm::ivec3 GetPos() { return m_pos; }
    Block GetBlock(glm::ivec3 pos);
    bool IsBuilt() { return m_built; }
    bool IsUploaded() { return m_uploaded; }

    struct Vertex {
        GLfloat x,y,z,w,h;
    };

    enum class State {
        UNLOADED,
        GENERATED,
        BUILDING,
        BUILT,
        UPLOADED
    } m_state = State::UNLOADED;
    
    std::vector<Vertex> &GetVerts() { return m_verts; }
    std::vector<GLuint> &GetIndices() { return m_indices; }
private:
    glm::ivec3 m_pos;
    GLuint m_vao = 0, m_vbo = 0, m_ebo = 0;
    bool m_built = false;
    bool m_uploaded = false;

    std::vector<Vertex> m_verts;
    std::vector<GLuint> m_indices;
    std::array<Block, CHUNK_SIZE> m_blocks;

    int Index(int x, int y, int z) {
        return x + CHUNK_WIDTH * (z + CHUNK_DEPTH * y);
    }
};

#endif