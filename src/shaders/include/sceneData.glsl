layout (std140, binding = 0) uniform SceneData
{
    mat4 u_ViewProjection;

	vec3 cameraPos;
    vec3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
	vec2 windowSize;
	uint u_numLights;
    uint bvhCounter;
};