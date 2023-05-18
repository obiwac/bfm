#version 330

layout(location = 0) in vec3 node;
layout(location = 1) in vec2 effect;

out vec3 transformed;
out vec3 colour;

uniform mat4 mvp_matrix;

void main(void) {
	transformed = vec3(node.xy + effect * 1e5, node.z * 1.01); // displacement
	colour = vec3(0.0);

	gl_Position = mvp_matrix * vec4(transformed, 1.0);
}
