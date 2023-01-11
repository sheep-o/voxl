#ifndef WINDOW_H
#define WINDOW_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"

class Window {
public:
        Window(int width, int height, const char *title);
        ~Window();

        void MakeContextCurrent();
        void SwapBuffers();
        void SetInputMode(int mode, int value);
        glm::vec2 GetCursorPos();
        bool WindowShouldClose();
        bool GetKey(int key);
        bool GetMouseBtn(int btn);
        float GetAspectRatio();

        GLFWwindow *m_pWindow;
};

#endif
