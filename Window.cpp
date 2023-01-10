#include "Window.h"
#include <cstdlib>

Window::Window(int width, int height, const char *title) {
    if (!glfwInit()) exit(-1);

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_pWindow = glfwCreateWindow(width, height, title, 0, 0);
    if (!m_pWindow) exit(-1);

    glfwMakeContextCurrent(m_pWindow);

    if (glewInit() != GLEW_OK) exit(-1);
}

Window::~Window() {
    glfwDestroyWindow(m_pWindow);
    glfwTerminate();
}

void Window::MakeContextCurrent() {
    glfwMakeContextCurrent(m_pWindow);
}

void Window::SwapBuffers() {
    glfwSwapBuffers(m_pWindow);
}

bool Window::WindowShouldClose() {
    return glfwWindowShouldClose(m_pWindow);
}

bool Window::GetKey(int key) {
    return glfwGetKey(m_pWindow, key) == GLFW_PRESS;
}

float Window::GetAspectRatio() {
    int width, height;
    glfwGetWindowSize(m_pWindow, &width, &height);
    return (float) width / height;
}
