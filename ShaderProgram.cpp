#include "ShaderProgram.h"
#include <GL/glew.h>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "glm/gtc/type_ptr.hpp"

ShaderProgram::ShaderProgram(const char *vpath, const char *fpath) {
    // -> read vertex shader file
    FILE *fp;
    int size;
    fp = fopen(vpath, "rb");
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);
    char *buffer = (char *)malloc(size + 1);
    fread(buffer, 1, size + 1, fp);
    fclose(fp);
    // -> compile vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &buffer, 0);
    free(buffer);
    glCompileShader(vertexShader);
    // -> log errors
    int infoLogLen;
    glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLen);
    if (infoLogLen > 0) {
        std::vector<char> infoLog(infoLogLen);
        glGetShaderInfoLog(vertexShader, infoLogLen, 0, &infoLog[0]);
        fprintf(stderr, "%s\n", &infoLog[0]);
    }
    // -> read fragment shader file
    fp = fopen(fpath, "rb");
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);
    buffer = (char *)malloc(size + 1);
    fread(buffer, 1, size + 1, fp);
    fclose(fp);
    // -> compile fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &buffer, 0);
    free(buffer);
    glCompileShader(fragmentShader);
    // -> log errors
    glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLogLen);
    if (infoLogLen > 0) {
        std::vector<char> infoLog(infoLogLen);
        glGetShaderInfoLog(fragmentShader, infoLogLen, 0, &infoLog[0]);
        fprintf(stderr, "%s\n", &infoLog[0]);
    }
    // -> link shader program
    m_ID = glCreateProgram();
    glAttachShader(m_ID, vertexShader);
    glAttachShader(m_ID, fragmentShader);
    glLinkProgram(m_ID);
    // -> clean up
    glDetachShader(m_ID, vertexShader);
    glDetachShader(m_ID, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

ShaderProgram::~ShaderProgram() {
    glDeleteProgram(m_ID);
}

void ShaderProgram::Use() {
    glUseProgram(m_ID);
}

void ShaderProgram::SendMat4(const char *name, glm::mat4 matrix) {
    int location = glGetUniformLocation(m_ID, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}
