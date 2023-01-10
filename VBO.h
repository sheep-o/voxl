#ifndef VBO_H
#define VBO_H

#include <GL/glew.h>

class VBO {
public:
        VBO();
        ~VBO();

        void Bind(GLenum target);

        unsigned int m_ID;
};

#endif
