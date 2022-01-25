#type vertex
#version 450 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in uint a_TexCoord;
layout (location = 2) in vec3 a_Normal;
layout (location = 3) in uint a_Color;

const float PI = 3.1415926535897932384626433832795;

layout(location = 0) out vec3 v_TexCoords;
layout(location = 1) flat out uint v_Type;
layout(location = 2) out vec4 v_Color;
layout(location = 3) out float v_visibility;
layout(location = 4) out vec3 v_Normal;
layout(location = 5) out vec3 p0;
layout(location = 6) out vec3 eye_relative_pos;

layout (std140, binding = 0) uniform Transforms
{
    mat4 u_Projection;
    mat4 u_View;
    float deltaTime;
	int u_numLights;

	vec3 cameraPos;
    vec3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
	vec2 windowSize;
	vec3 fogColor;
	float u_Density;
	float u_Gradient;
};

vec4 decompress(const in uint num) {
    vec4 Output;
    const float c = 1.f / 255.f;
    Output.r = float((num & uint(0x000000ff))) * c;
    Output.g = float((num & uint(0x0000ff00)) >> 8) * c;
    Output.b = float((num & uint(0x00ff0000)) >> 16) * c;
    Output.a = float((num & uint(0xff000000)) >> 24) * c;
    return Output;
}

void main()
{
    vec3 currentVertex = a_Pos;

    v_Color = decompress(a_Color);
    vec4 decompressedData = decompress(a_TexCoord);
    //Normal
    v_Normal = normalize(a_Normal);
    v_Type = uint(decompressedData.a);

    if (v_Type == 1) currentVertex = vec3(a_Pos.x, a_Pos.y - 0.2f, a_Pos.z); // fluid

    vec4 PosRelativeToCam = u_View * vec4(currentVertex, 1.f); // * u_Model

	gl_Position = u_Projection * PosRelativeToCam;

	v_TexCoords.xyz = decompressedData.xyz * 255.f;

    // Fog
    float dist = length(PosRelativeToCam.xyz);
    v_visibility = exp(-pow(dist * u_Density, u_Gradient));
    v_visibility = clamp(v_visibility, 0.f, 1.f);

    p0 = currentVertex;
    eye_relative_pos = p0 - cameraPos;
}

#type fragment
#version 450 core 

layout(location = 0) in vec3 v_TexCoords;
layout(location = 1) flat in uint v_Type;
layout(location = 2) in vec4 v_Color;
layout(location = 3) in float v_visibility;
layout(location = 4) in vec3 v_Normal;
layout(location = 5) in vec3 p0;
layout(location = 6) in vec3 eye_relative_pos;

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2DArray u_Texture;
layout(binding = 1) uniform sampler2DArray u_NormalMap;

#include "shaderpacks/default/pbrUtil.glsl"

void main() {    
    vec3 dFdxPos = dFdx(eye_relative_pos);
    vec3 dFdyPos = dFdy(eye_relative_pos);
    vec3 Normal = normalize(cross(dFdxPos,dFdyPos)) * -0.5 - 0.5;

    vec4 textureColor = texture(u_Texture, v_TexCoords) * v_Color;
    //textureColor.rgb = pow(textureColor.rgb, vec3(2.2f));
    if ((!true && v_Type != FLUID_CONNECT && v_Type != X_CONNECT) || textureColor.a < 0.1) discard;
    
	vec3 N = getNormalFromMap(texture(u_NormalMap, v_TexCoords).rgb, v_Normal, p0, v_TexCoords.xy);
	if (gl_FrontFacing) N = -N;
  
    FragColor = mix(vec4(fogColor, 1.f), textureColor * vec4(rayTrace(textureColor.rgb, N), textureColor.a), v_visibility);

    
    //float brightness = dot(fragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    //if(brightness > 1.0)
    //    FragColor = vec4(FragColor.rgb, 1.0);
    //else
    //    FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}