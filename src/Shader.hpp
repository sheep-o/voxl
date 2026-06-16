#ifndef VOXL_SHADER_H_
#define VOXL_SHADER_H_

#include <GL/glew.h>
#include <string>
#include "glm/glm.hpp"

class Shader {
public:
    Shader(const std::string &vertex_path, const std::string &frag_path);

    void Use() { glUseProgram(m_id); }
    void UniformMat4(const std::string &loc, const glm::mat4 &mat) { 
        glUniformMatrix4fv(glGetUniformLocation(m_id, loc.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
private:
    GLuint m_id;
};

#endif