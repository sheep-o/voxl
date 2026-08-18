#ifndef VOXL_CHUNK_HPP_
#define VOXL_CHUNK_HPP_

#include <vector>
#include <array>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <atomic>
#include <memory>
#include "Shader.hpp"

static constexpr int CHUNK_WIDTH = 32;
static constexpr int CHUNK_HEIGHT = 32;
static constexpr int CHUNK_DEPTH = 32;
static constexpr int CHUNK_SIZE = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH;

class Chunk {
public:
    enum class State {UNLOADED, GENERATED, BUILDING, BUILT, UPLOADED};
    struct Vertex { GLfloat x, y, z, w, h, shade; };

    struct Block {
        enum class ID : uint8_t { AIR, STONE, DIRT, GRASS, DRILL };
        enum class Dir : uint8_t { UP, DOWN, NORTH, SOUTH, EAST, WEST };
        
        uint16_t data = 0;

        ID GetID() const {
            return static_cast<ID>(data & 0xff);
        }

        Dir GetDir() const {
            return static_cast<Dir>(data & 0b11100000000);
        }

        uint8_t GetState() const {
            return static_cast<uint8_t>(data >> 11);
        }

        void SetID(ID id) {
            data = (data & 0xff00) | static_cast<uint16_t>(id);
        }

        void SetDir(Dir dir) {
            data = (data & 0b00011111111) | (static_cast<uint16_t>(dir) << 8);
        }

        void SetState(uint8_t state) {
            data = (data & 0b11111111111) | (state << 11);
        }
    };

    Chunk();
    explicit Chunk(glm::ivec3 pos);
    ~Chunk();

    void Upload();
    void Draw(std::shared_ptr<Shader> shader) const;
    void Place(const glm::ivec3 &pos, Block type);
    void GenTerrain();
    void SetState(const State state) { m_state.store(state); }
    bool TryLock() {
        auto expected = State::GENERATED;
        return m_state.compare_exchange_strong(expected, State::BUILDING);
    }

    [[nodiscard]] Block GetBlock(glm::ivec3 pos) const;
    [[nodiscard]] glm::ivec3 GetPos() const { return m_pos; }
    [[nodiscard]] State GetState() const { return m_state.load(); }
    [[nodiscard]] bool IsUploaded() const { return m_uploaded; }

    [[nodiscard]] Block &GetBlock(glm::ivec3 pos);

    // Remove soon
    std::vector<Vertex> &GetVerts() { return m_verts; }
    std::vector<GLuint> &GetIndices() { return m_indices; }

private:
    glm::ivec3 m_pos;
    GLuint m_vao = 0, m_vbo = 0, m_ebo = 0;

    std::vector<Vertex> m_verts;
    std::vector<GLuint> m_indices;
    std::array<Block, CHUNK_SIZE> m_blocks;

    static int Index(const int x, const int y, const int z) {
        return x + CHUNK_WIDTH * (z + CHUNK_DEPTH * y);
    }

    std::atomic<State> m_state = State::UNLOADED;
    bool m_uploaded = false;
};

#endif