#include "Window.h"
#include "VAO.h"
#include "VBO.h"
#include "ShaderProgram.h"
#include "Camera.h"

int main() {
    Window window(1280, 720, "Window");

    glfwSetWindowSizeCallback(window.m_pWindow, [](GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    });

    ShaderProgram program("./shaders/vertex.glsl", "./shaders/fragment.glsl");
    program.Use();

    float fov = 30.f * (M_PI / 180.f);

    Camera cam(glm::vec3(0, 0, 5), fov, 0.1f, 1000.f, &window, &program);

    float vertices[] = {
    -0.5f, 0.5f, 0,
    -0.5f, -0.5f, 0,
    0.5f, -0.5f, 0,
    };

    VAO vao;
    vao.Bind();

    VBO vbo;
    vbo.Bind(GL_ARRAY_BUFFER);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

    glEnable(GL_DEPTH_TEST);
    while(!window.WindowShouldClose()) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.5f, 0, 0.5f, 1.f);

        cam.CalculateView(0);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        window.SwapBuffers();
        glfwPollEvents();
    }
}
