#type vertex
#version 330

layout (location = 0) in vec3 a_Pos;

uniform mat4 u_Model = mat4(1.0f);
uniform mat4 u_Projection = mat4(1.0f);

void main()
{
	gl_Position = u_Projection * u_Model * vec4(a_Pos, 1.0);
}

#type fragment
#version 330

layout (location = 0) out vec4 o_Color;

uniform vec4 u_Color;

void main()
{
	o_Color = u_Color;
}