#include "Render.hpp"
#include "Game.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <unordered_map>
#include <queue>
#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Render::Render() {
    glEnable(GL_DEPTH_TEST);

    m_shader = std::make_shared<Shader>(
        "../../../src/vertex.glsl",
        "../../../src/fragment.glsl"
    );

    m_ui_shader = std::make_shared<Shader>(
        "../../../src/ui_vertex.glsl",
        "../../../src/ui_fragment.glsl"
    );

    m_sky_shader = std::make_shared<Shader>(
        "../../../src/sky_vertex.glsl",
        "../../../src/sky_fragment.glsl"
    );

    m_ui_shader->Use();
    float crosshairVerts[] = {
        -0.02f,  0.00f,  // Left
         0.02f,  0.00f,  // Right
         0.00f, -0.035f, // Bottom
         0.00f,  0.035f  // Top
    };

    glGenVertexArrays(1, &m_crosshair_vao);
    glGenBuffers(1, &m_crosshair_vbo);

    glBindVertexArray(m_crosshair_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_crosshair_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVerts), crosshairVerts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    m_cubemap = load_cubemap({
        "../../../src/skybox/skybox2_right1.jpg",
        "../../../src/skybox/skybox2_left2.jpg",
        "../../../src/skybox/skybox2_top3.jpg",
        "../../../src/skybox/skybox2_bottom4.jpg",
        "../../../src/skybox/skybox2_front5.jpg",
        "../../../src/skybox/skybox2_back6.jpg",
    });

    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &m_skybox_vao);
    glBindVertexArray(m_skybox_vao);

    glGenBuffers(1, &m_skybox_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_skybox_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    m_camera = std::make_shared<Camera>(60, glm::vec3{0, 80, 0}, glm::vec3{0, 0, -1});
    m_last_pos = m_camera->GetPos();
    m_mesher = std::make_shared<MeshingEngine>(8, this);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, channels;
    unsigned char *data = stbi_load("../../../src/atlas.png", &width, &height, &channels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Failed to load texture" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    stbi_image_free(data);

    int x = std::floor(m_camera->GetPos().x) / CHUNK_WIDTH;
    int z = std::floor(m_camera->GetPos().z) / CHUNK_WIDTH;
    int y = std::floor(m_camera->GetPos().y) / CHUNK_HEIGHT;
    for (int i = x - RADIUS; i <= x + RADIUS; i++) {
        for (int j = z - RADIUS; j <= z + RADIUS; j++) {
            for (int k = y - 2; k <= y + 2; k++) {
                m_mesher->Request(glm::ivec3{i, k, j});
            }
        }
    }
}

void Render::Draw() {
    m_shader->Use();
    glEnable(GL_DEPTH_TEST);

    int lx = static_cast<int>(std::floor(m_last_pos.x / CHUNK_WIDTH));
    int ly = static_cast<int>(std::floor(m_last_pos.y / CHUNK_HEIGHT));
    int lz = static_cast<int>(std::floor(m_last_pos.z / CHUNK_DEPTH));

    int x = static_cast<int>(std::floor(m_camera->GetPos().x / CHUNK_WIDTH));
    int y = static_cast<int>(std::floor(m_camera->GetPos().y / CHUNK_HEIGHT));
    int z = static_cast<int>(std::floor(m_camera->GetPos().z / CHUNK_DEPTH));

    m_mesher->Upload();

    if (x != lx || y != ly || z != lz) {
        m_mesher->UnloadFarChunks(glm::ivec3{x, y, z});

        int dx = x - lx;
        int dy = y - ly; 
        int dz = z - lz;

        if (dx != 0) {
            int target_x = (dx > 0) ? (x + RADIUS) : (x - RADIUS);
            for (int k = y - 2; k <= y + 2; k++) {
                for (int j = z - RADIUS; j <= z + RADIUS; j++) {
                    m_mesher->Request(glm::ivec3{target_x, k, j});
                }
            }
        }

        if (dy != 0) {
            int target_y = (dy > 0) ? (y + 2) : (y - 2);
            for (int i = x - RADIUS; i <= x + RADIUS; i++) {
                for (int j = z - RADIUS; j <= z + RADIUS; j++) {
                    m_mesher->Request(glm::ivec3{i, target_y, j});
                }
            }
        }

        if (dz != 0) {
            int target_z = (dz > 0) ? (z + RADIUS) : (z - RADIUS);
            for (int i = x - RADIUS; i <= x + RADIUS; i++) {
                for (int k = y - 2; k <= y + 2; k++) {
                    m_mesher->Request(glm::ivec3{i, k, target_z});
                }
            }
        }
    }

    m_shader->UniformMat4("projection", m_camera->GetProj());
    m_shader->UniformMat4("view", m_camera->GetView());

    glClearColor(1, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_mesher->Draw(m_shader, m_camera);

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    m_sky_shader->Use();
    m_sky_shader->UniformMat4("projection", m_camera->GetProj());
    glm::mat4 view = glm::mat4(glm::mat3(m_camera->GetView()));
    m_sky_shader->UniformMat4("view", view);
    glBindVertexArray(m_skybox_vao);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    m_ui_shader->Use();
    glBindVertexArray(m_crosshair_vao);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_LINES, 0, 4);

    m_last_pos = m_camera->GetPos();
}


unsigned int Render::load_cubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                        0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}  