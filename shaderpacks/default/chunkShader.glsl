#type vertex
#version 450 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_TexCoord;
layout (location = 2) in vec4 a_NormalType;
layout (location = 3) in uint a_Color;

const float PI = 3.1415926535897932384626433832795;

layout(location = 0) out vec3 v_TexCoords;
layout(location = 1) flat out uint v_Type;
layout(location = 2) out vec4 v_Color;
layout(location = 3) out float v_visibility;
layout(location = 4) out vec3 v_Normal;
layout(location = 5) out vec3 p0;

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

void main()
{
    vec3 currentVertex = a_Pos;

    const float c = 1.f / 255.f;
    v_Color.r = float((a_Color & uint(0xff000000)) >> 24) * c;
    v_Color.g = float((a_Color & uint(0x00ff0000)) >> 16) * c;
    v_Color.b = float((a_Color & uint(0x0000ff00)) >> 8) * c;
    v_Color.a = float((a_Color & uint(0x000000ff))) * c;

    //Normal
    //vec3 normal;
    //normal.x = float((a_NormalType & uint(0xff000000)) >> 24) * c;
    //normal.y = float((a_NormalType & uint(0x00ff0000)) >> 16) * c;
    //normal.z = float((a_NormalType & uint(0x0000ff00)) >> 8) * c;
    //v_Type = a_NormalType & uint(0x000000ff);
    v_Type = uint(a_NormalType.a);
    v_Normal = normalize(a_NormalType.xyz);

    if (v_Type == 1) currentVertex = vec3(a_Pos.x, a_Pos.y - 0.2f, a_Pos.z); // fluid

    vec4 PosRelativeToCam = u_View * vec4(currentVertex, 1.f); // * u_Model

	gl_Position = u_Projection * PosRelativeToCam;

	v_TexCoords = a_TexCoord;

    // Fog
    float dist = length(PosRelativeToCam.xyz);
    v_visibility = exp(-pow(dist * u_Density, u_Gradient));
    v_visibility = clamp(v_visibility, 0.f, 1.f);

    p0 = currentVertex;
}

#type fragment
#version 450 core 

layout(location = 0) in vec3 v_TexCoords;
layout(location = 1) flat in uint v_Type;
layout(location = 2) in vec4 v_Color;
layout(location = 3) in float v_visibility;
layout(location = 4) in vec3 v_Normal;
layout(location = 5) in vec3 p0;

layout(binding = 0) uniform sampler2DArray u_Texture;
layout(binding = 1) uniform sampler2DArray u_NormalMap;

#include "shaderpacks/default/pbrUtil.glsl"

void main() {
    vec4 textureColor = texture(u_Texture, v_TexCoords) * v_Color;
    if ((!true && v_Type != 1u && v_Type != 2u) || textureColor.a < 0.1) discard;
    
	vec3 N = getNormalFromMap(texture(u_NormalMap, v_TexCoords).rgb, v_Normal, p0, v_TexCoords.xy);
	if (!gl_FrontFacing) N = -N;
    vec4 finalColor = textureColor * vec4(rayTrace(textureColor.rgb, N), textureColor.a);
  
    gl_FragColor = mix(vec4(fogColor, 1.0f), finalColor, v_visibility);
}