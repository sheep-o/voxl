#include "Shader.hpp"
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>

Shader::Shader(const std::string &vertex_path, const std::string &frag_path) {
    GLuint vertex_id, frag_id;

    {
        std::string vertex_src;
        std::ifstream f(vertex_path);
        if (f.is_open()) {
            std::stringstream buf;
            buf << f.rdbuf();
            vertex_src = buf.str();
        } else {
            std::cerr << "Failed to find vertex shader" << std::endl;
            exit(EXIT_FAILURE);
        }
        char *data = vertex_src.data();
        f.close();

        vertex_id = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_id, 1, &data, 0);
        glCompileShader(vertex_id);
        GLint is_compiled = GL_FALSE;
        glGetShaderiv(vertex_id, GL_COMPILE_STATUS, &is_compiled);
        if (is_compiled == GL_FALSE) {
            GLint max_length = 0;
            glGetShaderiv(vertex_id, GL_INFO_LOG_LENGTH, &max_length);

            std::vector<GLchar> error_log(max_length);
            glGetShaderInfoLog(vertex_id, max_length, &max_length, &error_log[0]);

            std::cerr << "Failed to compile vertex shader: " << std::endl
                << std::string(error_log.begin(), error_log.end()) << std::endl;

            glDeleteShader(vertex_id);
            exit(EXIT_FAILURE);
        }
    }

    {
        std::string frag_src;
        std::ifstream f(frag_path);
        if (f.is_open()) {
            std::stringstream buf;
            buf << f.rdbuf();
            frag_src = buf.str();
        } else {
            std::cerr << "Failed to find fragment shader" << std::endl;
            exit(EXIT_FAILURE);
        }
        char *data = frag_src.data();
        f.close();

        frag_id = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag_id, 1, &data, 0);
        glCompileShader(frag_id);
        GLint is_compiled = GL_FALSE;
        glGetShaderiv(frag_id, GL_COMPILE_STATUS, &is_compiled);
        if (is_compiled == GL_FALSE) {
            GLint max_length = 0;
            glGetShaderiv(frag_id, GL_INFO_LOG_LENGTH, &max_length);

            std::vector<GLchar> error_log(max_length);
            glGetShaderInfoLog(frag_id, max_length, &max_length, &error_log[0]);

            std::cerr << "Failed to compile frag shader: " << std::endl
                << std::string(error_log.begin(), error_log.end()) << std::endl;

            glDeleteShader(frag_id);
            exit(EXIT_FAILURE);
        }
    }

    m_id = glCreateProgram();
    glAttachShader(m_id, vertex_id);
    glAttachShader(m_id, frag_id);
    glLinkProgram(m_id);
    glDeleteShader(vertex_id);
    glDeleteShader(frag_id);
}