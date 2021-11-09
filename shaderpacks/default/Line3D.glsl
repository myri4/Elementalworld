#type vertex
#version 450 core

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec4 a_Color;

layout (std140, binding = 0) uniform Transforms
{
    mat4 u_Projection;
    mat4 u_View;
};

layout(location = 0) out vec4 v_Color;

void main() {
    v_Color = a_Color;
    gl_Position = u_Projection * u_View * vec4(a_Pos, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) in vec4 v_Color;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = v_Color;
}