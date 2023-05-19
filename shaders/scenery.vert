#version 330

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 normal;

out vec3 colour;

uniform mat4 mvp_matrix;

void main(void) {
	colour = normal;
	gl_Position = mvp_matrix * vec4(vert, 1.0);
}
