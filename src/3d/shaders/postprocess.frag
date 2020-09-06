#version 150 core

uniform sampler2D colorTexture;
uniform sampler2D depthTexture;
//uniform sampler2DShadow shadowTexture;
uniform sampler2D shadowTexture;

// light camera uniforms
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform float lightFarPlane;
uniform float lightNearPlane;

uniform float shadowMinX;
uniform float shadowMaxX;
uniform float shadowMinZ;
uniform float shadowMaxZ;

uniform vec3 lightPosition;
uniform vec3 lightDirection;

// view camera uniforms
uniform mat4 invertedCameraView;
uniform mat4 invertedCameraProj;
uniform float farPlane;
uniform float nearPlane;

uniform int renderShadows;
uniform float shadowBias;

in vec2 texCoord;

out vec4 fragColor;

vec3 WorldPosFromDepth(float depth) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(texCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = invertedCameraProj * clipSpacePosition;

    viewSpacePosition /= viewSpacePosition.w;
    vec4 worldSpacePosition =  invertedCameraView * viewSpacePosition;
    worldSpacePosition /= worldSpacePosition.w;

    return worldSpacePosition.xyz;
}

float CalcShadowFactor(vec4 LightSpacePos)
{
  vec2 texelSize = 1.0 / textureSize(shadowTexture, 0);
  vec3 ProjCoords = LightSpacePos.xyz / LightSpacePos.w;
  vec2 UVCoords;
  UVCoords.x = 0.5 * ProjCoords.x + 0.5;
  UVCoords.y = 0.5 * ProjCoords.y + 0.5;
  float z = 0.5 * ProjCoords.z + 0.5;

  // percentage close filtering of the shadow map
  float shadow = 0.0;
  int k = 1;
  for(int x = -k; x <= k; ++x)
  {
    for(int y = -k; y <= k; ++y)
    {
      float pcfDepth = texture(shadowTexture, UVCoords + vec2(x, y) * texelSize).r;
      shadow += z - shadowBias >= pcfDepth ? 0.5 : 1.0;
    }
  }

  return shadow / (2 * k + 1) / (2 * k + 1);
}

void main()
{
  vec3 worldPosition = WorldPosFromDepth(texture(depthTexture, texCoord).r);
  vec4 positionInLightSpace = projectionMatrix * viewMatrix * vec4(worldPosition, 1.0f);
  positionInLightSpace /= positionInLightSpace.w;
  vec3 color = texture(colorTexture, texCoord).rgb;
  // if shadow rendering is disabled or the pixel is outside the shadow rendering distance don't render shadows
  if (renderShadows == 0 || worldPosition.x > shadowMaxX || worldPosition.x < shadowMinX || worldPosition.z > shadowMaxZ || worldPosition.z < shadowMinZ) {
    fragColor = vec4(color.rgb, 1.0f);
  } else {
    float visibilityFactor = CalcShadowFactor(positionInLightSpace);
    fragColor = vec4(visibilityFactor * color, 1.0f);
  }
}
