#ifndef VOXL_RENDER_H_
#define VOXL_RENDER_H_

#include <memory>
#include <mutex>
#include <glm/glm.hpp>
#include "MeshingEngine.hpp"
#include "Camera.hpp"
#include "Chunk.hpp"

enum class Block;
class Chunk;
class MeshingEngine;

class Render {
public:
    Render();
    void Draw();
    void UpdateCamera(GLFWwindow *window) const { m_camera->CalculateView(window, m_mesher); };

    static constexpr int RADIUS = 8;
private:
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<Camera> m_camera;
    glm::vec3 m_last_pos;

    std::mutex m_chunks_mutex;

    std::shared_ptr<MeshingEngine> m_mesher;
};


#endif