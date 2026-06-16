#ifndef VOXL_RENDER_H_
#define VOXL_RENDER_H_

#include <vector>
#include <memory>
#include "Shader.hpp"
#include "Camera.hpp"

class Render {
public:
    Render();
    void Draw();
    void UpdateCamera(GLFWwindow *window) { m_camera->CalculateView(window); };
private:
    std::vector<size_t> m_visible_chunks;
    std::unique_ptr<Shader> m_shader;
    std::unique_ptr<Camera> m_camera;
};

#endif