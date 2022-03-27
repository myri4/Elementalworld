#type vertex
#version 450 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 tex_coord;

layout (location = 0) uniform vec2 viewportSize;

out VS_OUT {
    vec4 color;
    vec2 tex_coord;
} vs_out;

void main() {
    float x =  ((pos.x / viewportSize.x) * 2.f - 1.f);
    float y = -((pos.y / viewportSize.y) * 2.f - 1.f);
    gl_Position = vec4(x, y, -1.f, 1.f);
    vs_out.color = color / 255.f;
    vs_out.tex_coord = tex_coord;
}

#type fragment
#version 450 core

layout(binding = 0) uniform sampler2D fontTexture;

in VS_OUT {
    vec4 color;
    vec2 tex_coord;
} fs_in;

layout(location = 0) out vec4 FragColor;

void main() {
    FragColor = fs_in.color * texture(fontTexture, fs_in.tex_coord);
}