#type vertex
#version 330 core

layout(location = 0) in vec2 a_Pos;
layout(location = 1) in vec3 a_TexCoords;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in float a_Type;

uniform mat4 proj;
out vec3 v_TexCoords;
out vec4 v_Color;
out float v_Type;

void main() {
    v_TexCoords = a_TexCoords;
    v_Color = a_Color;
    v_Type = a_Type;
    gl_Position = proj * vec4(a_Pos, 0.f, 1.0);
}

#type fragment
#version 330 core

in vec3 v_TexCoords;
in vec4 v_Color;
in float v_Type;

uniform sampler2D u_Texture[32];

void main()
{
    int index = int(v_TexCoords.z);
    vec4 finalColor = vec4(1.0f);
    if(v_Type == 0.0f) finalColor = texture(u_Texture[index], v_TexCoords.xy);
    if(v_Type == 1.0f) finalColor = vec4(1.0, 1.0, 1.0, texture(u_Texture[index], v_TexCoords.xy).r);
    gl_FragColor = finalColor * v_Color;
}