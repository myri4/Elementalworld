#pragma shader_stage(fragment)

layout(binding = 0, set = 0) uniform sampler2D u_Textures[32];

layout(location = 0) in vec3 v_TexCoords;
layout(location = 1) in vec4 v_Color;

layout(location = 0) out vec4 FragColor;

void main() {
    FragColor = v_Color * texture(u_Textures[int(v_TexCoords.z)], v_TexCoords.xy);
}