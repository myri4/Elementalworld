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

layout(location = 3) out float albedo;
layout(location = 4) out float materialData;
layout(location = 5) out uint flags;
layout(location = 6) out vec4 color;

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
    Material material = materials[a_MaterialID];

    v_Normal = a_Normal;

    albedo = material.albedo[int(a_TexCoord.z)];
    materialData = material.materialData[int(a_TexCoord.z)];
    flags = material.flags;
    color = decompress(material.color);

	v_TexCoords = a_TexCoord.xy;
    if (bool(material.flags & WC_MODEL_BIT)) currentVertex += transforms[gl_InstanceID + transformOffset].xyz;

    p0 = currentVertex;
	gl_Position = u_ViewProjection * vec4(currentVertex, 1.f);
}