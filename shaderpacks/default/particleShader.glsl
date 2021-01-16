#type vertex
#version 460

layout (location = 0) in vec3 a_Pos;

uniform mat4 u_Model = mat4(1.0f);
uniform mat4 u_View = mat4(1.0f);
uniform mat4 u_Projection = mat4(1.0f);

void main()
{
	gl_Position = u_Projection * u_Model * u_View * vec4(a_Pos, 1.0);
}

#type fragment
#version 460

uniform vec4 u_Color;

void main()
{
	gl_FragColor = u_Color;
}