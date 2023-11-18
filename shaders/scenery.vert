#version 300 es

precision mediump float;

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 normal;

out vec3 colour;

uniform mat4 mvp_matrix;

void main(void) {
	const vec3 sunlight = vec3(2.0, 1.0, 0.0);
	float facing = dot(normalize(normal), normalize(sunlight));
	colour = vec3((1.0 + facing) / 2.0);

	gl_Position = mvp_matrix * vec4(vert, 1.0);
}
