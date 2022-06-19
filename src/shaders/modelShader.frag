#version 450

layout(location = 0) in vec2 v_TexCoords;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec3 p0;

#include "include/pbrUtil.glsl"

layout(binding = 0) uniform sampler2D u_Texture;

layout(location = 0) out vec4 FragColor;
void main()
{    
    vec4 textureColor = texture(u_Texture, v_TexCoords);    
    
	vec3 N = v_Normal;
	if (!gl_FrontFacing) N = -N;

    vec4 finalColor = textureColor * vec4(rayTrace(textureColor.rgb, N), textureColor.a);
    FragColor = finalColor;
}