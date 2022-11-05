#pragma shader_stage(vertex)

layout(location = 0) out vec2 v_TexCoords;

void main()
{
	v_TexCoords = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(v_TexCoords * 2.0f + -1.0f, 0.0f, 1.0f);
    v_TexCoords.y = 1.f - v_TexCoords.y;
}