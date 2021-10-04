#type vertex
#version 450 core
layout (location = 0) in vec3 a_Pos;

layout (location = 0) out vec3 v_TexCoords;
layout (location = 1) out vec3 v_Position;

layout (std140) uniform Matrices
{
    mat4 u_Projection;
    mat4 u_View;
    float deltaTime;
};
layout(location = 0) uniform mat4 u_Model = mat4(1.f);

void main()
{
    v_Position = a_Pos;
    v_TexCoords = vec4(vec4(a_Pos, 1.f) * u_Model).xyz;
    gl_Position = u_Projection * mat4(mat3(u_View)) * vec4(a_Pos, 1.0);
}  

#type fragment
#version 450 core

layout (location = 0) in vec3 v_TexCoords;
layout (location = 1) in vec3 v_Position;

layout (binding = 0) uniform samplerCube skybox;
/*
night = vec3(0.f, 0.f, 4.f / 255.f);
day = vec3(115.f, 211.f, 255.f) / 255.f;
sunset = vec3(255, 178, 79) / 255.f;
*/
layout(location = 1) uniform vec3 skyColor = vec3(115.f, 211.f, 255.f) / 255.f;
layout(location = 2) uniform vec3 voidColor = vec3(0.f, 0.5f, 0.75f);

const float lowerLimit = -140.0f;
const float upperLimit = 10.0f;

void main()
{    
    vec4 finalColor = texture(skybox, v_TexCoords * 2.f);
    float factor = (v_Position.y - lowerLimit) / (upperLimit - lowerLimit);
    if(finalColor.a > 0.5f) factor = 0.f;    
    else finalColor = vec4(voidColor, 1.f);
    factor = clamp(factor, 0.f, 1.f);
    gl_FragColor = mix(finalColor, vec4(skyColor, 1.f), factor);
}