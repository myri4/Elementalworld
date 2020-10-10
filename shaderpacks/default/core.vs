#version 330 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_TexCoord;
layout (location = 2) in float a_TexIndex;
layout (location = 3) in vec3 a_Normal;

out vec2 v_TexCoords;
out float v_TexIndex;
out vec3 v_Normal;
out vec3 v_FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
	gl_Position = projection * view * model * vec4(a_Pos, 1.0);
	v_TexCoords = a_TexCoord;
	v_TexIndex = a_TexIndex;
	v_Normal = a_Normal;
	v_FragPos = vec3(model * vec4(a_Pos, 1.0));
}