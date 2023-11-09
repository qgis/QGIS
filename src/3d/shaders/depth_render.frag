#version 330 core

uniform sampler2D depthTexture;

in vec3 position;
in vec2 texCoord;

out vec4 fragColor;

void main()
{
  float z = texture2D( depthTexture, texCoord ).r;
  fragColor.b = float( int(z * 255) ) / 255.0;
  z = z * 255.0 - fragColor.b * 255.0;
  fragColor.g = float( int(z * 255) ) / 255.0;
  z = z * 255.0 - fragColor.g * 255.0;
  fragColor.r = float( int(z * 255) ) / 255.0;
  z = z * 255.0 - fragColor.r * 255.0;
  fragColor.a = 1;
}
