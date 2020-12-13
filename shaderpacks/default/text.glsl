#type vertex
#version 460 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 v_TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    v_TexCoords = vertex.zw;
}  

#type fragment
#version 460 core
in vec2 v_TexCoords;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, v_TexCoords).r);
    gl_FragColor = vec4(textColor, 1.0) * sampled;
}  