#pragma shader_stage(fragment)

layout(location = 0) in vec2 v_TexCoords;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec3 p0;

layout(location = 3) in float albedo;
layout(location = 4) in float materialData;
layout(location = 5) in flat uint flags;
layout(location = 6) in vec4 color;

#include "include/pbrUtil.glsl"

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2DArray u_Albedo;
layout(binding = 1) uniform sampler2DArray u_MaterialData;

void main() {
    vec4 albedo = texture(u_Albedo, vec3(v_TexCoords, albedo)) * color;
    
	vec3 N = v_Normal;
	if (gl_FrontFacing) N = -N; // @TODO: Fix
    float emmision = texture(u_MaterialData, vec3(v_TexCoords, materialData)).r; // remove and change
    
    vec4 materialData = vec4(0.f, 1.f, 1.f, 0.f);

    if (emmision > 0.f)
        FragColor = albedo * vec4(vec3(emmision), 1.f) + albedo;
    else 
        FragColor = albedo * vec4(rayTrace(N, materialData, albedo.rgb), albedo.a);
}