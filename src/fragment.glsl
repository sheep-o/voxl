#version 330 core

out vec4 FragColor;

in vec2 texCoords;
in float shade;

uniform sampler2D tex;

void main() {
	vec4 color = texture(tex, texCoords);
	FragColor = vec4(color.rgb * shade * 0.9, color.a);
}