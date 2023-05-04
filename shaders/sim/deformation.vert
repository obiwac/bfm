#version 330

layout(location = 0) in vec2 node;
layout(location = 1) in vec2 effect;

out vec3 transformed;
out vec3 colour;

void main(void) {
	transformed = node + effect; // displacement
	colour = effect;

	gl_Position = vec4(transformed, 1.0, 1.0);
}
