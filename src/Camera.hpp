#ifndef VOXL_CAMERA_H_
#define VOXL_CAMERA_H_

#include <complex.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.hpp"
#include <GLFW/glfw3.h>
#include <memory>

class MeshingEngine;


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
          m_size({1, 2, 1}),
          m_eye(m_pos + glm::vec3{0, m_size.y, 0}),
          m_view(glm::lookAt(pos, target, m_up)),
          m_proj(glm::perspective(glm::radians(fov), 1280.f/720.f, 0.1f, 1000.f)),
          m_view_angles({-89, 0}),
          m_prev_cur({0, 0})
          {}

    [[nodiscard]] glm::mat4 GetView() const { return m_view; }
    [[nodiscard]] glm::mat4 GetProj() const { return m_proj; }
    [[nodiscard]] glm::vec3 GetPos() const { return m_pos; }
    [[nodiscard]] glm::vec3 GetSize() const { return m_size; }
    Frustum GetFrustum();

    void CalculateView(GLFWwindow* window, std::shared_ptr<MeshingEngine> mesher);
private:
    glm::vec3 m_pos;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_size;
    glm::vec3 m_eye;
    glm::mat4 m_view;
    glm::mat4 m_proj;

    glm::vec2 m_view_angles;
    glm::vec2 m_prev_cur;

    float m_speed = 12.f;
    float m_prev_time = static_cast<float>(glfwGetTime());
    float m_look_speed = 10.f;
};

#endif