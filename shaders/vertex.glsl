#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aOff;

out int id;

uniform mat4 projection;
uniform mat4 view;

void main() {
    gl_Position = projection * view * vec4(aPos + aOff, 1);
    id = gl_InstanceID;
}
