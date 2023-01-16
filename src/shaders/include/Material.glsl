#ifndef MATERIAL_GLSL
#define MATERIAL_GLSL
struct Material {
	uint albedo[6];
	uint materialData[6];
	uint flags;
	uint color;
};

layout (binding = 2, std430) readonly buffer MaterialData { Material materials[]; };
#endif