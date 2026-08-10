#ifndef VOXL_CAMERA_H_
#define VOXL_CAMERA_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.hpp"
#include <GLFW/glfw3.h>

class Camera {
public:
    struct Plane {
        glm::vec3 normal;
        float distance;

        void Normalize() {
            float mag = glm::length(normal);
            normal /= mag;
            distance /= mag;
        }
    };

    struct Frustum {
        Plane planes[6];

        bool IsBoxVisible(const glm::vec3& min, const glm::vec3& max) const {
            glm::vec3 center = (min + max) * 0.5f;
            glm::vec3 extents = max - center;

            for (int i = 0; i < 6; i++) {
                const Plane& p = planes[i];
                
                float r = extents.x * std::abs(p.normal.x) + 
                        extents.y * std::abs(p.normal.y) + 
                        extents.z * std::abs(p.normal.z);
                
                float d = glm::dot(p.normal, center) + p.distance;

                if (d < -r) {
                    return false; 
                }
            }
            return true;
        }
    };

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
    Frustum GetFrustum();

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
    float m_look_speed = 10.f;
};

#endif