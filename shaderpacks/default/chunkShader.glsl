#type vertex
#version 460 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoords;
out float v_visibility;

uniform const float u_Density = 0.01;
uniform const float u_Gradient = 1.5;

uniform mat4 u_Model = mat4(1.0f);
uniform mat4 u_View = mat4(1.0f);
uniform mat4 u_Projection = mat4(1.0f);

void main()
{
    vec4 PosRelativeToCam = u_View * u_Model * vec4(a_Pos, 1.0);
	gl_Position = u_Projection * PosRelativeToCam;
	v_TexCoords = a_TexCoord;

    // Fog
    float dist = length(PosRelativeToCam.xyz);
    v_visibility = exp(-pow((dist * u_Density), u_Gradient));
    v_visibility = clamp(v_visibility, 0.0f, 1.0f);    
}

#type fragment
#version 460 core 

layout (location = 0) out vec4 Result;

in vec2 v_TexCoords;
in vec3 v_FragPos;
in float v_visibility;

uniform sampler2D u_Texture;
uniform vec3 fogColor = vec3(0.5f);

void main()
{
    vec4 texColor = texture(u_Texture, vec2(v_TexCoords));

    if(texColor.a < 0.1) discard;
  
    Result = texColor; // Fog mix(vec4(fogColor, 1.0f), texColor, v_visibility)
}