#ifndef VOXL_GAME_H_
#define VOXL_GAME_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include "Render.hpp"

constexpr int GAME_WIDTH  = 1280;
constexpr int GAME_HEIGHT = 720;

class Game {
public:
    Game(int width, int height, const char *title);
    void Run();
private:
    GLFWwindow *m_window;
    std::unique_ptr<Render> m_render;
};

#endif