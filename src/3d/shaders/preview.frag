#version 330

uniform sampler2D previewTexture;
uniform bool isDepth;
uniform mat4 projectionMatrix;
uniform mat4 inverseProjectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 inverseViewMatrix;
uniform vec3 eyePosition;


in vec3 position;
in vec2 texCoord;

out vec4 fragColor;

// this is supposed to get the world position from the depth buffer
vec3 WorldPosFromDepth(float depth) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(texCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = inverseProjectionMatrix * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = inverseViewMatrix * viewSpacePosition;

    return worldSpacePosition.xyz;
}



void main()
{
  if (isDepth)
  {
      float m22 = -projectionMatrix[2][2];
      float m32 = -projectionMatrix[2][3];

      float zNear = (2.0f * m32) / (2.0f * m22 - 2.0f);
      float zFar = ( (m22 - 1.0f) * zNear) / (m22 + 1.0);

      vec3 pos = WorldPosFromDepth( texture2D(previewTexture, texCoord).r );
      float dist = length(pos - eyePosition.xyz);
      float zLinearized = (dist - zNear) / (zFar - zNear);
      int zLinearizedMultiplied = int( 256 * 256 * 256 * zLinearized );
      fragColor = vec4( float(zLinearizedMultiplied % 256) / 256.0, float(zLinearizedMultiplied / 256 % 256) / 256.0, float( zLinearizedMultiplied / 256 / 256 % 256 ) / 256.0, 1 );
  }
  else
    fragColor = vec4(texture(previewTexture, texCoord).rgb, 1.0f);
}
