#include "Game.hpp"
#include <iostream>

Game::Game(int width, int height, const char *title) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create a window" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(m_window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    m_render = std::make_unique<Render>();
}

void Game::Run() {
    while (!glfwWindowShouldClose(m_window)) {
        m_render->UpdateCamera(m_window);
        m_render->Draw();

        glfwPollEvents();
        glfwSwapBuffers(m_window);
    }
}