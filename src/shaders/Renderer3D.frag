#pragma shader_stage(fragment)
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec2 v_TexCoords;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec3 p0;

layout(location = 3) in flat uint albedoID;
layout(location = 4) in flat uint materialID;
layout(location = 5) in flat uint flags;
layout(location = 6) in vec4 color;

#include "include/pbrUtil.glsl"

layout(location = 0) out vec4 FragColor;

layout(binding = 3, set = 0) uniform sampler2D u_Textures[];

void main() {
    vec4 albedo = texture(u_Textures[int(albedoID)], v_TexCoords) * color;
	vec3 N = v_Normal;
	if (!gl_FrontFacing) N = -N;
    //float emmision = texture(u_MaterialData, vec3(v_TexCoords, materialID)).r; // remove and change

    //if (emmision > 0.f){
    //    FragColor = albedo * vec4(vec3(emmision), 1.f) + albedo;
    //    FragColor.a = 1.f;
    //}
    //else 

    vec2 materialInfo = vec2(1.f, 0.f);
    if (materialID != 0) materialInfo = texture(u_Textures[int(materialID)], v_TexCoords).rg;
    
    FragColor = albedo * vec4(rayTrace(N, materialInfo, albedo.rgb), 1.f);
}