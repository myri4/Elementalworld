#pragma shader_stage(fragment)
#include "include/sky.glsl"

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 uv;

void main()
{    
    vec3 camPos = cameraPos;
    vec3 rayDir = normalize(lower_left_corner + uv.x * horizontal + uv.y * vertical - cameraPos);    

    FragColor = vec4(ComputeSky(camPos, rayDir), 0.f);
}