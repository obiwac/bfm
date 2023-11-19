#version 300 es

precision mediump float;

layout(location = 0) in vec3 node;
layout(location = 1) in vec2 effect;

out vec3 transformed;
out vec3 colour;

uniform mat4 mvp_matrix;
uniform float max_effect;
uniform float anim;

vec3 plasma_colourmap(float t) {
	// more colourmaps can be found here:
	// https://observablehq.com/@flimsyhat/webgl-color-maps

	const vec3 c0 = vec3(0.05873234392399702, 0.02333670892565664, 0.5433401826748754);
	const vec3 c1 = vec3(2.176514634195958, 0.2383834171260182, 0.7539604599784036);
	const vec3 c2 = vec3(-2.689460476458034, -7.455851135738909, 3.110799939717086);
	const vec3 c3 = vec3(6.130348345893603, 42.3461881477227, -28.51885465332158);
	const vec3 c4 = vec3(-11.10743619062271, -82.66631109428045, 60.13984767418263);
	const vec3 c5 = vec3(10.02306557647065, 71.41361770095349, -54.07218655560067);
	const vec3 c6 = vec3(-3.658713842777788, -22.93153465461149, 18.19190778539828);

	return c0 + t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * (c5 + t * c6)))));
}

void main(void) {
	transformed = vec3(node.xy + anim * effect * 3e5, node.z); // displacement

	float t = length(effect) / max_effect;
	colour = plasma_colourmap(anim * t);

	gl_Position = mvp_matrix * vec4(transformed, 1.0);
}
