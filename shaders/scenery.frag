#version 300 es

precision mediump float;

out vec4 out_colour;

in vec3 colour;

void main(void) {
	out_colour = vec4(colour, 1.0);
}
