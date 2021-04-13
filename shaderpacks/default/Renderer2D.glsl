#type vertex
#version 330 core

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_TexCoords;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in float a_Type;

uniform mat4 u_Model = mat4(1.f);
uniform mat4 u_View = mat4(1.f);
uniform mat4 u_Projection = mat4(1.f);

out vec3 v_TexCoords;
out vec4 v_Color;
out float v_Type;

void main() {
    v_TexCoords = a_TexCoords;
    v_Color = a_Color;
    v_Type = a_Type;
    gl_Position = u_Projection * u_View * u_Model * vec4(a_Pos, 1.0);
}

#type fragment

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
    if(v_Type == 2.0f) finalColor = vec4(1.f);
    gl_FragColor = finalColor * v_Color;
}