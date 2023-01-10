#ifndef WINDOW_H
#define WINDOW_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Window {
public:
        Window(int width, int height, const char *title);
        ~Window();

        void MakeContextCurrent();
        void SwapBuffers();
        bool WindowShouldClose();
        bool GetKey(int key);
        float GetAspectRatio();

        GLFWwindow *m_pWindow;
};

#endif
