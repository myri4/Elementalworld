#pragma shader_stage(vertex)

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec4 a_Color;

#include "include/sceneData.glsl"

layout(location = 0) out vec4 v_Color;

void main() {
    v_Color = a_Color;
    gl_Position = u_ViewProjection * vec4(a_Pos, 1.0);
}