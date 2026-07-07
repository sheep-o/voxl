#include "Camera.hpp"
#include <GLFW/glfw3.h>
#include <cstdio>

void Camera::CalculateView(GLFWwindow *window) {
    float curr_time = static_cast<float>(glfwGetTime());
    float delta_time = curr_time - m_prev_time;
    m_prev_time = curr_time;

    glm::vec3 delta{};
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) m_pos += m_front*m_speed*delta_time;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) m_pos -= m_front*m_speed*delta_time;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) m_pos -= glm::normalize(glm::cross(m_front, m_up))*m_speed*delta_time;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) m_pos += glm::normalize(glm::cross(m_front, m_up))*m_speed*delta_time;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) m_pos += m_up*m_speed*delta_time;
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) m_pos -= m_up*m_speed*delta_time;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    //printf("Pos: %f, %f, %f\n", m_pos.x, m_pos.y, m_pos.z); 
    
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

    m_view = glm::lookAt(m_pos, m_pos + m_front, m_up);
    m_prev_cur = cursor;
}