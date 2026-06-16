#include "Game.hpp"
#include <iostream>

Game::Game(int width, int height, const char *title) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create a window" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(m_window);

    if (!glewInit()) {
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