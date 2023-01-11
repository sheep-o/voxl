#ifndef CAMERA_H
#define CAMERA_H

#include "Window.h"
#include "ShaderProgram.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

class Camera {
public:
        glm::vec3 m_Pos;
        glm::vec3 m_ChunkPos;
        int CHUNK_WIDTH = 4;
        glm::vec3 m_Front;
        glm::vec3 m_Up;

        Camera(glm::vec3 pos, float fov, float near, float far, Window *window, ShaderProgram *program);
        ~Camera();

        void CalculateView(float deltaTime);

        float m_FOV;
        Window *m_pWindow;
        ShaderProgram *m_pShader;

        float m_WalkSpeed;
        float m_LookSpeed;

        glm::vec2 lastCursorPos;

        glm::vec2 m_ViewAngles;
};

#endif
