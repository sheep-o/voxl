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
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) m_pos -= glm::normalize(glm::cross(m_front, glm::vec3(0, 1, 0)))*m_speed*delta_time;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) m_pos += glm::normalize(glm::cross(m_front, glm::vec3(0, 1, 0)))*m_speed*delta_time;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    //printf("Pos: %f, %f, %f\n", m_pos.x, m_pos.y, m_pos.z); 

    m_view = glm::lookAt(m_pos, m_pos + m_front, {0,1,0});
}