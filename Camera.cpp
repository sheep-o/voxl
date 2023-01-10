#include "Camera.h"

Camera::Camera(glm::vec3 pos, float fov, float near, float far, Window *window, ShaderProgram *program) {
    m_Pos = pos;
    m_FOV = fov;
    m_pWindow = window;
    m_pShader = program;

    glm::mat4 projection = glm::perspective(m_FOV, window->GetAspectRatio(), near, far);
    program->SendMat4("projection", projection);
}

Camera::~Camera() {
}

void Camera::CalculateView(float deltaTime) {
    glm::mat4 view = glm::lookAt(m_Pos, glm::vec3(0), glm::vec3(0, 1, 0));
    m_pShader->SendMat4("view", view);
}
