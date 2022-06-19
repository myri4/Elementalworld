struct Light {
	vec3 vector;
	uint color;
};

layout (binding = 1, std140) uniform Lighting { Light lights[1]; };