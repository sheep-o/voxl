#ifndef VOXL_RENDER_H_
#define VOXL_RENDER_H_

#include <vector>
#include <memory>
#include <unordered_map>
#include <queue>
#include <glm/glm.hpp>
#include "Camera.hpp"
#include "Chunk.hpp"

class Chunk;

class Render {
public:
    Render();
    void Draw();
    void UpdateCamera(GLFWwindow *window) { m_camera->CalculateView(window); };
    bool ChunkExists(glm::ivec3 pos);
    std::shared_ptr<Chunk> GetChunk(glm::ivec3 pos);

    static constexpr int RADIUS = 16;
private:
    struct ivec3Hash {
        std::size_t operator()(const glm::ivec3 &c) const {
            std::size_t h1 = std::hash<int>{}(c.x);
            std::size_t h2 = std::hash<int>{}(c.y);
            std::size_t h3 = std::hash<int>{}(c.z);

            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    using ChunkMap 
        = std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>, ivec3Hash>;
    using ChunkQueue
        = std::queue<std::shared_ptr<Chunk>>;

    ChunkMap m_chunks;
    ChunkQueue m_unbuilt_chunks;

    std::shared_ptr<Shader> m_shader;
    std::unique_ptr<Camera> m_camera;
    glm::vec3 m_last_pos;

    void UnloadFarChunks(const glm::ivec3 &cam_chunk);
};

#endif