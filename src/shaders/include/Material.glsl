struct MaterialCompressed {
	uint albedo[6];
	uint MRH[6];
	uint flags;
	uint color;
};

struct Material {
	uint albedo;
	uint normal;
	uint MRH;
    bool cullFace;
    float ior;
	vec4 color;
};

layout (binding = 3, std430) readonly buffer MaterialData { MaterialCompressed materials[]; };