#type vertex
#version 330 core
layout (location = 0) in vec2 a_Pos;
layout (location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main()
{
    v_TexCoords = a_TexCoords;
    gl_Position = vec4(a_Pos, 0.0, 1.0); 
}  

#type fragment
#version 330 core
layout (location = 0) out vec4 Result;

in vec2 v_TexCoords;

uniform sampler2D screenTexture;

void main()
{
    Result = vec4(1,1,3,1);
}