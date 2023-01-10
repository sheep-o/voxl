#include "VAO.h"
#include <GL/glew.h>

VAO::VAO() {
    glGenVertexArrays(1, &m_ID);
}

VAO::~VAO() {
    glDeleteVertexArrays(1, &m_ID);
}

void VAO::Bind() {
    glBindVertexArray(m_ID);
}
