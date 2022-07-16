struct Material {
	uint albedo[6];
	uint materialData[6];
	uint flags;
	uint color;
};

layout (binding = 3, std430) readonly buffer MaterialData { Material materials[]; };