#include "VBO.h"

VBO::VBO() {
    glGenBuffers(1, &m_ID);
}

VBO::~VBO() {
    glDeleteBuffers(1, &m_ID);
}

void VBO::Bind(GLenum target) {
    glBindBuffer(target, m_ID);
}
