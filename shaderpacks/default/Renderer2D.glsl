#type vertex
#version 430 core

layout(location = 0) in vec2 a_Pos;
layout(location = 1) in vec3 a_TexCoords;
layout(location = 2) in uint a_Color;
layout(location = 3) in float a_Type;

layout(location = 0) uniform mat4 u_Projection = mat4(1.f);

out vec3 v_TexCoords;
out vec4 v_Color;
out float v_Type;

void main() {
    v_TexCoords = a_TexCoords;

    const float c = 1.f / 255.f;
    v_Color.r = float((a_Color & uint(0xff000000)) >> 24) * c;
    v_Color.g = float((a_Color & uint(0x00ff0000)) >> 16) * c;
    v_Color.b = float((a_Color & uint(0x0000ff00)) >> 8) * c;
    v_Color.a = float((a_Color & uint(0x000000ff))) * c;

    v_Type = a_Type;
    gl_Position = u_Projection * vec4(a_Pos, 1., 1.);
}

#type fragment
#version 430 core

in vec3 v_TexCoords;
in vec4 v_Color;
in float v_Type;

layout(binding = 0) uniform sampler2D u_Texture[32];

layout(location = 0) out vec4 FragColor;
void main()
{
    int index = int(v_TexCoords.z);
    vec4 finalColor = vec4(1.f);
    if(v_Type == 0.f) finalColor = texture(u_Texture[index], v_TexCoords.xy);
    else if(v_Type == 1.f) finalColor = vec4(1.f, 1.f, 1.f, texture(u_Texture[index], v_TexCoords.xy).r);

    FragColor = finalColor * v_Color;
}