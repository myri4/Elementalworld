#type vertex
#version 430 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in ivec4 boneIds; 
layout(location = 4) in vec4 weights;

uniform mat4 model;
layout (std140, binding = 0) uniform Transforms
{
    mat4 u_Projection;
    mat4 u_View;
    float deltaTime;
};

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec2 v_TexCoords;

void main()
{
    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1) 
            continue;
        if(boneIds[i] >= MAX_BONES) 
        {
            totalPosition = vec4(pos, 1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(pos, 1.0f);
        totalPosition += localPosition * weights[i];
        vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * a_Normal;
   }
	
    gl_Position =  u_Projection * u_View * model * totalPosition;
    //gl_Position = u_Projection * u_View * model * vec4(pos, 1.0);
	v_TexCoords = a_TexCoords;
}

#type fragment
#version 430 core
#include "shaderpacks/default/pbrUtil.glsl"

in vec2 v_TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{    
    gl_FragColor = texture(texture_diffuse1, v_TexCoords);
}