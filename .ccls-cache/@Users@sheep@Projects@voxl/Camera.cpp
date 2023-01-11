#include "Camera.h"
#include <cstdio>

Camera::Camera(glm::vec3 pos, float fov, float near, float far, Window *window, ShaderProgram *program) {
    m_Pos = pos;
    m_Front = glm::vec3(0, 0, -1);
    m_Up = glm::vec3(0, 1, 0);

    m_FOV = fov;
    m_pWindow = window;
    m_pShader = program;
    m_WalkSpeed = 1.f;
    m_LookSpeed = 3.f;

    lastCursorPos = m_pWindow->GetCursorPos();
    m_ViewAngles = glm::vec2(-89, 0);

    glm::mat4 projection = glm::perspective(m_FOV, window->GetAspectRatio(), near, far);
    program->SendMat4("projection", projection);

    window->SetInputMode(GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

Camera::~Camera() {
}

void Camera::CalculateView(float deltaTime) {
    if (m_pWindow->GetKey(GLFW_KEY_W)) m_Pos += m_Front * m_WalkSpeed * deltaTime;
    if (m_pWindow->GetKey(GLFW_KEY_S)) m_Pos -= m_Front * m_WalkSpeed * deltaTime;
    if (m_pWindow->GetKey(GLFW_KEY_D)) m_Pos += glm::normalize(glm::cross(m_Front, glm::vec3(0, 1, 0))) * m_WalkSpeed * deltaTime;
    if (m_pWindow->GetKey(GLFW_KEY_A)) m_Pos -= glm::normalize(glm::cross(m_Front, glm::vec3(0, 1, 0))) * m_WalkSpeed * deltaTime;
    if (m_pWindow->GetKey(GLFW_KEY_ESCAPE)) m_pWindow->SetInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    if (m_pWindow->GetMouseBtn(0)) m_pWindow->SetInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (m_pWindow->GetCursorPos() != lastCursorPos) {
        glm::vec2 mouseDelta = m_pWindow->GetCursorPos() - lastCursorPos;
        m_ViewAngles += mouseDelta * m_LookSpeed * deltaTime;

        if (m_ViewAngles.y > 89.0f)
            m_ViewAngles.y = 89.0f;
        if (m_ViewAngles.y < -89.0f)
            m_ViewAngles.y = -89.0f;
        if (m_ViewAngles.x >= 360 || m_ViewAngles.x <= -360)
            m_ViewAngles.x = 0;

        glm::vec3 direction;
        direction.x = cos(glm::radians(m_ViewAngles.x)) * cos(glm::radians(m_ViewAngles.y));
        direction.y = -sin(glm::radians(m_ViewAngles.y));
        direction.z = sin(glm::radians(m_ViewAngles.x)) * cos(glm::radians(m_ViewAngles.y));

        m_Front = glm::normalize(direction);
    }

    m_ChunkPos.x = floor(m_Pos.x / CHUNK_WIDTH);
    m_ChunkPos.y = floor(m_Pos.y / CHUNK_WIDTH);
    m_ChunkPos.z = floor(m_Pos.z / CHUNK_WIDTH);

    printf("%f, %f, %f\n", m_ChunkPos.x, m_ChunkPos.y, m_ChunkPos.z);

    glm::mat4 view = glm::lookAt(m_Pos, m_Front + m_Pos, m_Up);
    m_pShader->SendMat4("view", view);

    lastCursorPos = m_pWindow->GetCursorPos();
}
