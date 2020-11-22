#type vertex
#version 330 core

layout(location = 0) in vec2 a_Pos;
layout(location = 1) in vec2 a_TexCoords;

uniform mat4 proj;
out vec2 v_texCoords;

void main(){
    v_texCoords = a_TexCoords;
    gl_Position = proj * vec4(a_Pos, 0.0f, 1.0);
}

#type fragment
#version 330 core

out vec4 Result;
in vec2 v_texCoords;

uniform sampler2D u_Texture;

void main()
{
    Result = texture(u_Texture, v_texCoords);
}