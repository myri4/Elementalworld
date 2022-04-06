#type vertex
#version 450 core

layout (location = 0) in vec2 a_Pos;
layout (location = 1) in uint a_Color;
layout (location = 2) in vec3 a_TexCoords;

layout(location = 0) out vec3 v_TexCoords;
layout(location = 1) out vec4 v_Color;

void main() {
    const float c = 1.f / 255.f;
    v_Color.r = float((a_Color & uint(0xff000000)) >> 24) * c;
    v_Color.g = float((a_Color & uint(0x00ff0000)) >> 16) * c;
    v_Color.b = float((a_Color & uint(0x0000ff00)) >> 8) * c;
    v_Color.a = float((a_Color & uint(0x000000ff))) * c;

    v_TexCoords = a_TexCoords;

    gl_Position = vec4(a_Pos, 1.f, 1.f);
}

#type fragment
#version 450 core

layout(binding = 0) uniform sampler2D u_Textures[32];

layout(location = 0) in vec3 v_TexCoords;
layout(location = 1) in vec4 v_Color;

layout(location = 0) out vec4 FragColor;

void main() {
    FragColor = v_Color * texture(u_Textures[int(v_TexCoords.z)], v_TexCoords.xy);
}