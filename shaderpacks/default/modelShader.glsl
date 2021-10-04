#type vertex
#version 450 core
layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in ivec4 boneIds; 
layout(location = 4) in vec4 weights;

layout(location = 0) uniform mat4 model;
layout (std140, binding = 0) uniform Transforms
{
    mat4 u_Projection;
    mat4 u_View;
    float deltaTime;
	int u_numLights;

	vec3 fogColor;
	vec3 cameraPos;
	vec3 ambientColor;
	float u_Density;
	float u_Gradient;
};

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
layout(location = 1) uniform mat4 finalBonesMatrices[MAX_BONES];

layout(location = 0) out vec2 v_TexCoords;
layout(location = 1) out float v_visibility;
layout(location = 2) out vec3 v_Normal;
layout(location = 3) out vec3 p0;

void main()
{
     vec4 totalPosition = vec4(0.f);
     vec3 totalNormal = vec3(0.f);
     for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
     {
         if(boneIds[i] == -1) 
             continue;
         if(boneIds[i] >= MAX_BONES) 
         {
             totalPosition = vec4(a_Pos, 1.0f);
             break;
         }
         vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(a_Pos, 1.f);
         totalPosition += localPosition * weights[i];         
         vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * a_Normal;
         totalNormal += localNormal * weights[i];
    }
    totalNormal = a_Normal;
    totalPosition = vec4(a_Pos, 1.f);
    v_Normal = mat3(transpose(inverse(model))) * totalNormal;
    v_Normal = normalize(v_Normal);
	vec4 currentVertex = model * totalPosition;
    p0 = currentVertex.xyz;
    v_TexCoords = a_TexCoord;
    

    vec4 PosRelativeToCam = u_View * currentVertex; // * u_Model
    // Fog
    float dist = length(PosRelativeToCam.xyz);
    v_visibility = exp(-pow(dist * u_Density, u_Gradient));
    v_visibility = clamp(v_visibility, 0.f, 1.f);    

    gl_Position = u_Projection * PosRelativeToCam;
}

#type fragment
#version 450 core

layout(location = 0) in vec2 v_TexCoords;
layout(location = 1) in float v_visibility;
layout(location = 2) in vec3 v_Normal;
layout(location = 3) in vec3 p0;

#include "shaderpacks/default/pbrUtil.glsl"

layout(binding = 0) uniform sampler2D u_Texture;
layout(binding = 1) uniform sampler2D u_NormalMap;

void main()
{    
    vec4 textureColor = texture(u_Texture, v_TexCoords);    
    
	vec3 N = getNormalFromMap(texture(u_NormalMap, v_TexCoords).rgb, v_Normal, p0, v_TexCoords.xy);
	if (!gl_FrontFacing) N = -N;

    vec4 finalColor = textureColor * vec4(rayTrace(textureColor.rgb, N), textureColor.a);
    gl_FragColor = mix(vec4(fogColor, 1.0f), finalColor, v_visibility);
}