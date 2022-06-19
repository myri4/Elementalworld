#version 450 

layout(location = 0) in vec3 v_TexCoords;
layout(location = 1) in vec4 v_Color;
layout(location = 2) in vec3 v_Normal;
layout(location = 3) in vec3 p0;
layout(location = 4) in flat uint type;

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2DArray u_Texture;
layout(binding = 1) uniform sampler2DArray u_ModelTexture;
//layout(binding = 1) uniform sampler2DArray u_EmmisionMap;

#include "include/pbrUtil.glsl"

void main() {
    vec4 textureColor = vec4(1.f);
    if (type == 11) textureColor = texture(u_ModelTexture, v_TexCoords);
    else textureColor = texture(u_Texture, v_TexCoords);
    //vec4 materialData = texture(u_EmmisionMap, v_TexCoords);
    //if ((!true && v_Type != FLUID_CONNECT && v_Type != X_CONNECT) || textureColor.a <= 0.f) discard;
    
	vec3 N = v_Normal;
	if (gl_FrontFacing) N = -N;
  
    FragColor = textureColor * vec4(rayTrace(textureColor.rgb, N), textureColor.a) * v_Color;
}