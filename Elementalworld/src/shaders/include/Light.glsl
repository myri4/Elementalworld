#ifndef LIGHT_GLSL
#define LIGHT_GLSL

struct Light {
	vec3 vector;
	float intensity;
	vec3 color;
	float radius;
};

layout (std430, binding = 0) readonly buffer LightData { Light lights[]; };
#endif