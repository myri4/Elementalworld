#version 450
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_TexCoord;
layout (location = 2) in vec3 a_Normal;
layout (location = 3) in uint a_MaterialID;

layout(location = 0) out vec3 v_TexCoords;
layout(location = 1) out vec4 v_Color;
layout(location = 2) out vec3 v_Normal;
layout(location = 3) out vec3 p0;
layout(location = 4) out uint type;

#include "include/sceneData.glsl"
#include "include/Material.glsl"
#include "include/constants.glsl"

layout (binding = 6, std430) readonly buffer Transform { vec4 transforms[]; };

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
    v_Normal = a_Normal;

	v_TexCoords = a_TexCoord;
    type = materials[a_MaterialID].flags;
    //if (type == 11) 
    currentVertex += transforms[gl_InstanceID + transformOffset].xyz;

    p0 = currentVertex;
	gl_Position = u_ViewProjection * vec4(currentVertex, 1.f);
}