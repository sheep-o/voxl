#ifndef VOXL_CAMERA_H_
#define VOXL_CAMERA_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.hpp"
#include <GLFW/glfw3.h>

class Camera {
public:
    Camera(float fov, const glm::vec3 &pos, const glm::vec3 &target)
        : m_pos(pos), 
          m_front(glm::normalize(target)),
          m_up({0,1,0}),
          m_view(glm::lookAt(pos, target, m_up)), 
          m_proj(glm::perspective(glm::radians(fov), 1280.f/720.f, 0.1f, 1000.f)),
          m_view_angles({-89, 0}),
          m_prev_cur({0, 0}) {}

    glm::mat4 GetView() { return m_view; }
    glm::mat4 GetProj() { return m_proj; }
    glm::vec3 GetPos()  { return m_pos; }

    void CalculateView(GLFWwindow *window);
private:
    glm::vec3 m_pos;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::mat4 m_view;
    glm::mat4 m_proj;

    glm::vec2 m_view_angles;
    glm::vec2 m_prev_cur;

    float m_speed = 12.f;
    float m_prev_time = static_cast<float>(glfwGetTime());
    float m_look_speed = 3.f;
};

#endif