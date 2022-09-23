#pragma shader_stage(vertex)

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in ivec4 boneIds; 
layout(location = 4) in vec4 weights;

#include "include/sceneData.glsl"
#include "include/constants.glsl"

layout (std140, binding = 4, set = 0) uniform ModelMatrices
{
    mat4 finalBonesMatrices[MAX_BONES];
    mat4 model;
};

layout(location = 0) out vec2 v_TexCoords;
layout(location = 1) out vec3 v_Normal;
layout(location = 2) out vec3 p0;

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

    // remove when animation
    totalNormal = a_Normal;
    totalPosition = vec4(a_Pos, 1.f);

    v_Normal = mat3(transpose(inverse(model))) * totalNormal;
    v_Normal = normalize(v_Normal);
	vec4 currentVertex = model * totalPosition;
    p0 = currentVertex.xyz;
    v_TexCoords = a_TexCoord;

    gl_Position = u_ViewProjection * currentVertex;
}