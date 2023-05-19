#version 330

layout(location = 0) in vec3 vert;

out vec3 colour;

uniform mat4 mvp_matrix;

void main(void) {
	colour = vert;
	gl_Position = mvp_matrix * vec4(vert, 1.0);
}
