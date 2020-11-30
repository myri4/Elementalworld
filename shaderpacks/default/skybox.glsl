#type vertex
#version 460 core
layout (location = 0) in vec3 a_Pos;

out vec3 v_TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    v_TexCoords = a_Pos;
    gl_Position = projection * model * view * vec4(a_Pos, 1.0);
}  

#type fragment
#version 460 core
layout (location = 0) out vec4 Result;

in vec3 v_TexCoords;

uniform samplerCube skybox;
uniform vec3 fogColor = vec3(0.2f, 0.7f, 1.0f);

const float lowerLimit = -90.0f;
const float upperLimit = 30.0f;

void main()
{    
    vec4 finalColor = vec4(0.0f, 0.5f, 0.75f, 1.0f); 
    //vec4 finalColor = texture(skybox, v_TexCoords);

    float factor = (v_TexCoords.y - lowerLimit) / (upperLimit - lowerLimit);
    factor = clamp(factor, 0.0f, 1.0f);
    Result = mix(finalColor, vec4(fogColor, 1.0f), factor);
}