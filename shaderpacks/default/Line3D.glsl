#type vertex
#version 330 core

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec4 a_Color;

uniform mat4 u_View = mat4(1.f);
uniform mat4 u_Projection = mat4(1.f);

out vec4 v_Color;

void main() {
    v_Color = a_Color;
    gl_Position = u_Projection * u_View * vec4(a_Pos, 1.0);
}

#type fragment

in vec4 v_Color;

void main()
{
    gl_FragColor = v_Color;
}