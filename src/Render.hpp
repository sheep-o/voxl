#ifndef VOXL_RENDER_H_
#define VOXL_RENDER_H_

#include <vector>
#include <memory>
#include <unordered_map>
#include <queue>
#include <glm/glm.hpp>
#include "MeshingEngine.hpp"
#include "Camera.hpp"
#include "Chunk.hpp"
#include "Types.hpp"

enum class Block;
class Chunk;
class MeshingEngine;

class Render {
public:
    Render();
    void Draw();
    void UpdateCamera(GLFWwindow *window) { m_camera->CalculateView(window); };
    bool ChunkExists(glm::ivec3 pos);
    std::shared_ptr<Chunk> GetChunk(glm::ivec3 pos);

    static constexpr int RADIUS = 2;
private:

    std::shared_ptr<Shader> m_shader;
    std::unique_ptr<Camera> m_camera;
    glm::vec3 m_last_pos;

    std::mutex m_chunks_mutex;

    std::unique_ptr<MeshingEngine> m_mesher;

    //void UnloadFarChunks(const glm::ivec3 &cam_chunk);
};


#endif