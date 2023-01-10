#ifndef CAMERA_H
#define CAMERA_H

#include "Window.h"
#include "ShaderProgram.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

class Camera {
public:
        glm::vec3 m_Pos;

        Camera(glm::vec3 pos, float fov, float near, float far, Window *window, ShaderProgram *program);
        ~Camera();

        void CalculateView(float deltaTime);

        float m_FOV;
        Window *m_pWindow;
        ShaderProgram *m_pShader;
};

#endif
