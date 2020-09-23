#version 330 core          
out vec4 Result;

in vec2 v_TexCoords;
in float v_TexIndex;
in vec4 posit;

uniform sampler2D u_Textures[32];

void main()
{
    int index = int(v_TexIndex);

    vec4 texColor = texture(u_Textures[1], v_TexCoords);
    if(texColor.a < 0.1) discard;

     //  Lately for framebuffer
     // if(gl_FragCoord.x < 600)
     //  Result = texColor; /* vec4(1.0, 0.0, 0.0, 1.0);*/
     // else
       Result = texColor;/* * posit;*/

    //Result = texColor;
}
/*
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

void main()
{
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 result = ambient * objectColor;
    Result = vec4(result, 1.0);
}  
*/