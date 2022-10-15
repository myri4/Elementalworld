#pragma shader_stage(vertex)

#include "include/sceneData.glsl"

struct Vertex {
	vec4 pos;
	vec4 color;
};

layout(location = 0) out vec4 v_Color;
layout(binding = 1, std430) readonly buffer VertexData { Vertex vertices[]; };

void main() {
    vec3 a_Pos = vertices[gl_VertexIndex].pos.xyz;
    v_Color = vertices[gl_VertexIndex].color;
    gl_Position = u_ViewProjection * vec4(a_Pos, 1.0);
}