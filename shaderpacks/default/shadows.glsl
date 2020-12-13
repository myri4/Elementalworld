#type vertex
#version 330 core
layout (location = 0) in vec3 a_Pos;

out vec2 v_TexCoords;

uniform mat4 lightSpaceMatrix;
uniform mat4 u_Model;

void main()
{
    gl_Position = lightSpaceMatrix * u_Model * vec4(a_Pos, 1.0); 
}  

#type fragment
#version 330 core

void main()
{
 gl_FragDepth = gl_FragCoord.z;

}