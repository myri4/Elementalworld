#type vertex
#version 330 core

layout(location = 0) in vec2 a_Pos;
layout(location = 1) in vec3 a_TexCoords;
layout(location = 2) in vec4 a_Color;

uniform mat4 proj;
out vec3 v_TexCoords;
out vec4 v_Color;

void main() {
    v_TexCoords = a_TexCoords;
    v_Color = a_Color;
    gl_Position = proj * vec4(a_Pos, 0.f, 1.0);
}

#type fragment
#version 330 core

in vec3 v_TexCoords;
in vec4 v_Color;

uniform sampler2D u_Texture[32];

void main()
{
    int index = int(v_TexCoords.z);
    gl_FragColor = texture(u_Texture[index], v_TexCoords.xy) * v_Color;
}