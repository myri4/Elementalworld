#type vertex
#version 430 core
layout (location = 0) in vec2 a_Pos;
layout (location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main()
{
    gl_Position = vec4(a_Pos, 0.0, 1.0); 
    v_TexCoords = a_TexCoords;
}  

#type fragment
#version 430 core

in vec2 v_TexCoords;

layout(binding = 0) uniform sampler2D screenTexture;

//vec3 GetKernelEffect(float kernel[9]){
//
//        const float offset = 1.0 / 300.0;    
//        
//        vec2 offsets[9] = vec2[](
//        vec2(-offset,  offset), // top-left
//        vec2( 0.0f,    offset), // top-center
//        vec2( offset,  offset), // top-right
//        vec2(-offset,  0.0f),   // center-left
//        vec2( 0.0f,    0.0f),   // center-center
//        vec2( offset,  0.0f),   // center-right
//        vec2(-offset, -offset), // bottom-left
//        vec2( 0.0f,   -offset), // bottom-center
//        vec2( offset, -offset)  // bottom-right    
//    );
//    vec3 sampleTex[9];
//    for(int i = 0; i < 9; i++)
//    {
//        sampleTex[i] = vec3(texture(screenTexture, v_TexCoords.st + offsets[i]));
//    }
//     vec3 col = vec3(0.0);
//    for(int i = 0; i < 9; i++)
//        col += sampleTex[i] * kernel[i];
//
//    return col;
//}

void main()
{    
    //float kernel[9] = float[](
    //    1,  1, 1,
    //    1, -8, 1,
    //    1,  1, 1
    //);
    //
    //vec3 hdrColor = texture(screenTexture, v_TexCoords).rgb;
    //vec3 result;
    //if(hdr)
    //{
    //    // exposure
    //    result = vec3(1.0) - exp(-hdrColor * exposure);
    //    // also gamma correct while we're at it       
    //    result = pow(result, vec3(1.0 / gamma));
    //}
    //else
    //{
    //    result = pow(hdrColor, vec3(1.0 / gamma));
    //}
    //result = (result - 0.5f) * (1.0f + contrast) + 0.5f;

    //gl_FragColor = vec4(result, 1.0f);

    gl_FragColor = texture(screenTexture, v_TexCoords);
}