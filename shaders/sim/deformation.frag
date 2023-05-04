#version 330

out vec4 out_colour;

in vec3 transformed;
in vec3 colour;

void main(void) {
	out_colour = colour;
}
