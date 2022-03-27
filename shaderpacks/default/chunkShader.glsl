#type vertex
#version 450 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in uint a_TexCoord;
layout (location = 2) in vec3 a_Normal;
layout (location = 3) in uint a_Color;
layout (location = 4) in uint a_MaterialID;

const float PI = 3.1415926535897932384626433832795;

layout(location = 0) out vec3 v_TexCoords;
layout(location = 1) flat out uint v_Type;
layout(location = 2) out vec4 v_Color;
layout(location = 4) out vec3 v_Normal;
layout(location = 5) out vec3 p0;

layout (std140, binding = 0) uniform Transforms
{
    mat4 u_Projection;
    mat4 u_View;

	vec3 cameraPos;
    vec3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
	vec2 windowSize;
	uint u_numLights;
};

struct MaterialCompressed {
	//uint albedo[6];
	//uint normal[6];
	//uint MRA[6];
	uint flags;
	uint color;
};

struct Material {
	uint albedo;
	uint normal;
	uint MRA;
    bool cullFace;
    float ior;
	vec4 color;
};

layout (binding = 3, std140) uniform MaterialData
{
    MaterialCompressed materials[1];
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

    uint c = materials[a_MaterialID].color;
    v_Color = decompress(c);
    vec4 decompressedData = decompress(a_TexCoord);
    //Normal
    v_Normal = a_Normal;

	gl_Position = u_Projection * u_View * vec4(currentVertex, 1.f);

	v_TexCoords.xyz = decompressedData.xyz * 255.f;

    p0 = currentVertex;
}

#type fragment
#version 450 core 

layout(location = 0) in vec3 v_TexCoords;
layout(location = 1) flat in uint v_Type;
layout(location = 2) in vec4 v_Color;

layout(location = 4) in vec3 v_Normal;
layout(location = 5) in vec3 p0;

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2DArray u_Texture;
layout(binding = 1) uniform sampler2DArray u_NormalMap;

#include "shaderpacks/default/pbrUtil.glsl"

void main() {
    vec4 textureColor = texture(u_Texture, v_TexCoords) * v_Color;
    //if ((!true && v_Type != FLUID_CONNECT && v_Type != X_CONNECT) || textureColor.a <= 0.f) discard;
    
	vec3 N = getNormalFromMap(texture(u_NormalMap, v_TexCoords).rgb, v_Normal, p0, v_TexCoords.xy);
	if (gl_FrontFacing) N = -N;
  
    FragColor = textureColor * vec4(rayTrace(textureColor.rgb, N), textureColor.a);
}