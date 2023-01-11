#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include "glm/glm.hpp"

class ShaderProgram {
public:
        ShaderProgram(const char *vpath, const char *fpath);
        ~ShaderProgram();

        void Use();
        void SendMat4(const char *name, glm::mat4 matrix);

        unsigned int m_ID;
};

#endif
