#type vertex
#version 330 core
layout (location = 0) in vec2 a_Pos;
layout (location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    gl_Position = vec4(a_Pos, 0.0, 1.0); 
    v_TexCoords = a_TexCoords;
}  

#type fragment
#version 330 core
layout (location = 0) out vec4 Result;

in vec2 v_TexCoords;

uniform sampler2D depthMap;

void main()
{

    float depthValue = texture(depthMap, v_TexCoords).r;
    Result = vec4(vec3(depthValue), 1.0);

}