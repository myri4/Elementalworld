#version 330 core
out vec4 Result;

in vec2 TexCoords;

uniform sampler2D screenTexture;

const float offset = 1.0 / 300.0;  

void main()
{
   // if(gl_FragCoord.y < 360)
  //  Result = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);
     Result = texture(screenTexture, TexCoords);
}