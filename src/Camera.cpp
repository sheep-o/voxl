#include "Camera.hpp"
#include <GLFW/glfw3.h>
#include <cstdio>

#include "MeshingEngine.hpp"

bool press_m1 = false, press_m2 = false;

void Camera::CalculateView(GLFWwindow *window, std::shared_ptr<MeshingEngine> mesher) {
    float curr_time = static_cast<float>(glfwGetTime());
    float delta_time = curr_time - m_prev_time;
    m_prev_time = curr_time;

    static constexpr int TPS = 5;
    static constexpr float SPT = 1.f / TPS;
    static float acc = 0;
    acc += delta_time;
    if (acc >= SPT) {
        acc -= SPT;
        mesher->Tick();
        //printf("Pos: %f, %f, %f\n", m_pos.x, m_pos.y, m_pos.z);
    }


    glm::vec3 delta{};
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) delta += m_front*m_speed*delta_time;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) delta -= m_front*m_speed*delta_time;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) delta -= glm::normalize(glm::cross(m_front, m_up))*m_speed*delta_time;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) delta += glm::normalize(glm::cross(m_front, m_up))*m_speed*delta_time;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) delta += m_up*m_speed*delta_time;
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) delta -= m_up*m_speed*delta_time;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (!press_m1) {
            press_m1 = true;
            if (glm::vec3 end,prev; mesher->Raycast(m_eye, m_front, end, prev)) {
                mesher->SetBlock(end, {});
                mesher->RemoveActive(prev);
                mesher->Request(glm::ivec3{
                    static_cast<int>(std::floor(end.x / CHUNK_WIDTH)),
                    static_cast<int>(std::floor(end.y / CHUNK_HEIGHT)),
                    static_cast<int>(std::floor(end.z / CHUNK_DEPTH))
                });
            }
        }
    } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE) {
        press_m1 = false;
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
        if (!press_m2) {
            press_m2 = true;
            if (glm::vec3 end,prev; mesher->Raycast(m_eye, m_front, end, prev)) {
                auto active = mesher->GetActive(end);
                if (active) {
                    active->OnInteract(end, mesher.get());
                } else {
                    Chunk::Block b;
                    b.SetID(Chunk::Block::ID::DRILL);
                    mesher->SetBlock(prev, b);
                    mesher->MakeActive(prev, Chunk::Block::ID::DRILL);
                    mesher->Request(glm::ivec3{
                        static_cast<int>(std::floor(end.x / CHUNK_WIDTH)),
                        static_cast<int>(std::floor(end.y / CHUNK_HEIGHT)),
                        static_cast<int>(std::floor(end.z / CHUNK_DEPTH))
                    });
                }
            }
        }
    } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE) {
        press_m2 = false;
    }

            /*
            auto mesher = static_cast<MeshingEngine *>(glfwGetWindowUserPointer(window));
            */


    m_pos.x += delta.x;
    if (mesher->CheckCollision(m_pos, m_size)) {
        if (delta.x > 0) m_pos.x = std::floor(m_pos.x + m_size.x / 2.0f) - m_size.x / 2.0f - 0.001f;
        if (delta.x < 0) m_pos.x = std::ceil(m_pos.x - m_size.x / 2.0f) + m_size.x / 2.0f + 0.001f;
    }

    m_pos.y += delta.y;
    if (mesher->CheckCollision(m_pos, m_size)) {
        if (delta.y > 0) m_pos.y = std::floor(m_pos.y + m_size.y) - m_size.y - 0.001f; // Hit ceiling
        if (delta.y < 0) {
            m_pos.y = std::ceil(m_pos.y) + 0.001f; // Hit floor
        }
    }

    m_pos.z += delta.z;
    if (mesher->CheckCollision(m_pos, m_size)) {
        if (delta.z > 0) m_pos.z = std::floor(m_pos.z + m_size.z / 2.0f) - m_size.z / 2.0f - 0.001f;
        if (delta.z < 0) m_pos.z = std::ceil(m_pos.z - m_size.z / 2.0f) + m_size.z / 2.0f + 0.001f;
    }

    double x, y;
    glfwGetCursorPos(window, &x, &y);
    glm::vec2 cursor{x, y};
    if (cursor != m_prev_cur) {
        glm::vec2 delta = cursor - m_prev_cur;
        m_view_angles += delta * m_look_speed * delta_time;

        if (m_view_angles.y > 89.f)
            m_view_angles.y = 89.f;
        if (m_view_angles.y < -89.f)
            m_view_angles.y = -89.f;
        if (m_view_angles.x >= 360 || m_view_angles.x <= -360)
            m_view_angles.x = 0;

        glm::vec3 direction;
        direction.x = cos(glm::radians(m_view_angles.x)) * cos(glm::radians(m_view_angles.y));
        direction.y = -sin(glm::radians(m_view_angles.y));
        direction.z = sin(glm::radians(m_view_angles.x)) * cos(glm::radians(m_view_angles.y));

        m_front = glm::normalize(direction);
    }

    m_eye = m_pos + glm::vec3{0, m_size.y, 0};
    m_view = glm::lookAt(m_eye, m_eye + m_front, m_up);
    m_prev_cur = cursor;
}

Camera::Frustum Camera::GetFrustum() {
    Frustum frustum;
    glm::mat4 vp = m_proj * m_view;

    // Left plane
    frustum.planes[0].normal = glm::vec3(vp[0][3] + vp[0][0], vp[1][3] + vp[1][0], vp[2][3] + vp[2][0]);
    frustum.planes[0].distance = vp[3][3] + vp[3][0];

    // Right plane
    frustum.planes[1].normal = glm::vec3(vp[0][3] - vp[0][0], vp[1][3] - vp[1][0], vp[2][3] - vp[2][0]);
    frustum.planes[1].distance = vp[3][3] - vp[3][0];

    // Bottom plane
    frustum.planes[2].normal = glm::vec3(vp[0][3] + vp[0][1], vp[1][3] + vp[1][1], vp[2][3] + vp[2][1]);
    frustum.planes[2].distance = vp[3][3] + vp[3][1];

    // Top plane
    frustum.planes[3].normal = glm::vec3(vp[0][3] - vp[0][1], vp[1][3] - vp[1][1], vp[2][3] - vp[2][1]);
    frustum.planes[3].distance = vp[3][3] - vp[3][1];

    // Near plane
    frustum.planes[4].normal = glm::vec3(vp[0][3] + vp[0][2], vp[1][3] + vp[1][2], vp[2][3] + vp[2][2]);
    frustum.planes[4].distance = vp[3][3] + vp[3][2];

    // Far plane
    frustum.planes[5].normal = glm::vec3(vp[0][3] - vp[0][2], vp[1][3] - vp[1][2], vp[2][3] - vp[2][2]);
    frustum.planes[5].distance = vp[3][3] - vp[3][2];

    for (int i = 0; i < 6; i++) {
        frustum.planes[i].Normalize();
    }

    return frustum;
}