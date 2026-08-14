#ifndef VOXL_MESHING_ENGINE_HPP_
#define VOXL_MESHING_ENGINE_HPP_

#include <iostream>
#include <glm/glm.hpp>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "Types.hpp"
#include "Chunk.hpp"
#include "Camera.hpp"

class Render;
class Shader;

class MeshingEngine {
public:
    MeshingEngine(size_t num_threads, Render *render) 
        : m_num_threads(num_threads), m_render(render)
    {
        for (size_t i = 0; i < m_num_threads; ++i) {
            std::thread(&MeshingEngine::worker, this).detach();
        }
    }

    ~MeshingEngine() = default; // implement later to gracefully stop threads

    void Request(glm::ivec3 pos);
    void UnloadFarChunks(const glm::ivec3 &cam_chunk);
    void Draw(std::shared_ptr<Shader> shader, std::shared_ptr<Camera> camera);
    void Upload();
    bool CheckCollision(glm::vec3 pos, glm::vec3 size);

private:
    struct Req {
        enum class Type {TERRAIN, MESH} type;
        glm::ivec3 pos;
    };

    size_t m_num_threads;
    ChunkMap m_chunks;
    std::mutex m_chunks_mutex;
    Render *m_render;

    std::queue<Req> m_requests;
    std::mutex m_requests_mutex;
    std::condition_variable m_requests_cv;

    std::queue<std::shared_ptr<Chunk>> m_results;
    std::mutex m_results_mutex;

    void worker();
    void req_internal(glm::ivec3 pos);
    void build_mesh(std::shared_ptr<Chunk> chunk);
    std::shared_ptr<Chunk> get_chunk(glm::ivec3 pos);

    Chunk::Block get_block(glm::vec3 pos);
};

#endif