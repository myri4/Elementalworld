#pragma shader_stage(vertex)

layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_TexCoord;
layout (location = 2) in vec3 a_Normal;
layout (location = 3) in uint a_MaterialID;

#include "include/sceneData.glsl"
#include "include/Material.glsl"
#include "include/constants.glsl"

layout(location = 0) out vec2 v_TexCoords;
layout(location = 1) out vec3 v_Normal;
layout(location = 2) out vec3 p0;

layout(location = 3) out flat uint albedoID;
layout(location = 4) out flat uint materialID;
layout(location = 5) out uint flags;
layout(location = 6) out vec4 color;

vec4 decompress(const in uint num) {
    vec4 Output;
    Output.r = float((num & uint(0x000000ff))) / 255.f;
    Output.g = float((num & uint(0x0000ff00)) >> 8) / 255.f;
    Output.b = float((num & uint(0x00ff0000)) >> 16) / 255.f;
    Output.a = float((num & uint(0xff000000)) >> 24) / 255.f;
    return Output;
}

void main()
{
    vec3 currentVertex = a_Pos;
    Material material = materials[a_MaterialID];

    v_Normal = a_Normal;

    albedoID = material.albedo[int(a_TexCoord.z)];
    materialID = material.materialData[int(a_TexCoord.z)];
    flags = material.flags;
    color = decompress(material.color);

	v_TexCoords = a_TexCoord.xy;

    p0 = currentVertex;
	gl_Position = u_ViewProjection * vec4(currentVertex, 1.f);
}