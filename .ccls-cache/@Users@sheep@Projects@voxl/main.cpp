#include "Window.h"
#include "VAO.h"
#include "VBO.h"
#include "ShaderProgram.h"
#include "Camera.h"

int main() {
    Window window(1280, 720, "Window");

    ShaderProgram program("./shaders/vertex.glsl", "./shaders/fragment.glsl");
    program.Use();

    float fov = 30.f * (M_PI / 180.f);

    Camera cam(glm::vec3(0, 0, 5), fov, 0.1f, 1000.f, &window, &program);

    float vertices[] = {
    -0.5f, -0.5f,  0.5f, //0
    0.5f, -0.5f,  0.5f, //1
    -0.5f,  0.5f,  0.5f, //2
    0.5f,  0.5f,  0.5f, //3
    -0.5f, -0.5f, -0.5f, //4
    0.5f, -0.5f, -0.5f, //5
    -0.5f,  0.5f, -0.5f, //6
    0.5f,  0.5f, -0.5f  //7
    };

    unsigned short indices[] = {
        //Top
        2, 6, 7,
        2, 3, 7,

        //Bottom
        0, 4, 5,
        0, 1, 5,

        //Left
        0, 2, 6,
        0, 4, 6,

        //Right
        1, 3, 7,
        1, 5, 7,

        //Front
        0, 2, 3,
        0, 1, 3,

        //Back
        4, 6, 7,
        4, 5, 7
    };

    float offsets[] = {
    0, 0, 0,
    10, 0, 0,
    20, 0, 0,
    };

    VAO vao;
    vao.Bind();

    VBO vbo;
    vbo.Bind(GL_ARRAY_BUFFER);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribDivisor(0, 0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

    VBO pbo;
    pbo.Bind(GL_ARRAY_BUFFER);
    glBufferData(GL_ARRAY_BUFFER, sizeof(offsets), offsets, GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

    VBO ebo;
    ebo.Bind(GL_ELEMENT_ARRAY_BUFFER);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnable(GL_DEPTH_TEST);
    float currentFrame, lastFrame, deltaTime;
    while(!window.WindowShouldClose()) {
        currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.5f, 0, 0.5f, 1.f);

        cam.CalculateView(deltaTime);

        glDrawElementsInstanced(GL_TRIANGLES, sizeof(indices) / sizeof(unsigned short), GL_UNSIGNED_SHORT, 0, 3);

        window.SwapBuffers();
        glfwPollEvents();
    }
}
