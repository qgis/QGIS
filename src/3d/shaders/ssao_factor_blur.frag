#version 330

uniform sampler2D colorTexture;
uniform sampler2D depthTexture;
uniform sampler2D ssaoTexture;

in vec2 texCoord;

out vec4 fragColor;

void main()
{
  vec2 texelSize = 1.0 / vec2( textureSize( ssaoTexture, 0 ) );
  float result = 0.0;
  for (int x = -2; x < 2; ++x)
  {
      for (int y = -2; y < 2; ++y)
      {
          vec2 offset = vec2(float(x), float(y)) * texelSize;
          result += texture( ssaoTexture, texCoord + offset ).r;
      }
  }
  fragColor = vec4( vec3( result / (4.0 * 4.0) ), 1.0 );
}
