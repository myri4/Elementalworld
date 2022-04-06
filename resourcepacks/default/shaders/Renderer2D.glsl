#type vertex
#version 450 core

layout(location = 0) in vec2 a_Pos;
layout(location = 1) in vec3 a_TexCoords;

layout(location = 0) out vec3 v_TexCoords;

void main() {
    v_TexCoords = a_TexCoords;

    gl_Position = vec4(a_Pos, 1., 1.);
}

#type fragment
#version 450 core

layout(location = 0) in vec3 v_TexCoords;

layout(binding = 0) uniform sampler2D u_Texture[32];

layout(location = 0) out vec4 FragColor;
void main()
{
    FragColor = texture(u_Texture[int(v_TexCoords.z)], v_TexCoords.xy);
}