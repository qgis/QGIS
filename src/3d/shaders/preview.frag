#version 330

uniform sampler2D previewTexture;
uniform bool isDepth;
uniform mat4 projectionMatrix;
uniform mat4 inverseProjectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 inverseViewMatrix;
uniform vec3 eyePosition;
uniform float nearPlane;
uniform float farPlane;

in vec3 position;
in vec2 texCoord;

out vec4 fragColor;

float linearizeDepth(float depth)
{
  float ndc = depth * 2.0 - 1.0;
  return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - ndc * (farPlane - nearPlane));
}

void main()
{
  if (isDepth)
  {
      float z = texture2D(previewTexture, texCoord).r;
      fragColor.b = float( int(z * 255) ) / 255.0;
      z = z * 255.0 - fragColor.b * 255.0;
      fragColor.g = float( int(z * 255) ) / 255.0;
      z = z * 255.0 - fragColor.g * 255.0;
      fragColor.r = float( int(z * 255) ) / 255.0;
      z = z * 255.0 - fragColor.r * 255.0;
      fragColor.a = 1;
  }
  else
    fragColor = vec4(texture(previewTexture, texCoord).rgb, 1.0f);
}
