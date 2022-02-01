#version 330

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

uniform int edlEnabled;
uniform float edlStrength;
uniform int edlDistance;

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

vec4 EyeCoordsFromDepth(vec2 textureCoord, float depth) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(textureCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = invertedCameraProj * clipSpacePosition;
    return viewSpacePosition;
}

float CalcShadowFactor(vec4 LightSpacePos)
{
  vec2 texelSize = 1.0 / textureSize(shadowTexture, 0);
  vec3 ProjCoords = LightSpacePos.xyz / LightSpacePos.w;
  vec2 UVCoords;
  UVCoords.x = 0.5 * ProjCoords.x + 0.5;
  UVCoords.y = 0.5 * ProjCoords.y + 0.5;
  float z = 0.5 * ProjCoords.z + 0.5;

  if ( UVCoords.x < 0 || UVCoords.x > 1 || UVCoords.y < 0 || UVCoords.y > 1 )
   return 1.0;

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

float linearizeDepth(float depth)
{
  float ndc = depth * 2.0 - 1.0;
  return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - ndc * (farPlane - nearPlane));
}

float edlFactor(vec2 coords)
{
  vec2 texelSize = 2.0 / textureSize(depthTexture, 0);
  vec2 neighbours[4] = vec2[4](vec2(-1.0f, 0.0f), vec2(1.0f, 0.0f), vec2(0.0f, -1.0f), vec2(0.0f, 1.0f) );
  float factor = 0.0f;
  float centerDepth = linearizeDepth( texture(depthTexture, coords).r ) / farPlane;
  for (int i = 0; i < 4; i++)
  {
    vec2 neighbourCoords = coords + edlDistance * texelSize * neighbours[i];
    float neighbourDepth = linearizeDepth( texture(depthTexture, neighbourCoords).r ) / farPlane;
    neighbourDepth = (neighbourDepth == 1.0) ? 0.0 : neighbourDepth;
    if (neighbourDepth != 0.0f)
    {
      if (centerDepth == 0.0f) factor += 1.0f;
      else factor += max(0, centerDepth - neighbourDepth);
    }
  }
  return factor / 4.0f;
}

void main()
{
  float depth = texture(depthTexture, texCoord).r;
  vec3 worldPosition = WorldPosFromDepth( depth );
  vec4 positionInLightSpace = projectionMatrix * viewMatrix * vec4(worldPosition, 1.0f);
  positionInLightSpace /= positionInLightSpace.w;
  vec3 color = texture(colorTexture, texCoord).rgb;
  // if shadow rendering is disabled or the pixel is outside the shadow rendering distance don't render shadows
  if (renderShadows == 0 || depth >= 1 || worldPosition.x > shadowMaxX || worldPosition.x < shadowMinX || worldPosition.z > shadowMaxZ || worldPosition.z < shadowMinZ)
  {
    fragColor = vec4(color, 1.0f);
  } else
  {
    float visibilityFactor = CalcShadowFactor(positionInLightSpace);
    fragColor = vec4(visibilityFactor * color, 1.0f);
  }
  if (edlEnabled != 0)
  {
    float shade = exp(-edlFactor(texCoord) * edlStrength);
    fragColor = vec4(fragColor.rgb * shade, fragColor.a);
  }
}
