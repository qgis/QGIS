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
uniform mat4 cameraView;
uniform mat4 cameraProj;
uniform float farPlane;
uniform float nearPlane;

uniform int renderShadows;

in vec2 texCoord;

out vec4 fragColor;

vec3 WorldPosFromDepth(float depth) {
    float z = depth * 2.0 - 1.0;
    mat4 projMatrixInv = inverse(cameraProj);
    mat4 viewMatrixInv = inverse(cameraView);

    vec4 clipSpacePosition = vec4(texCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = projMatrixInv * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;
//    return viewSpacePosition.xyz;
    vec4 worldSpacePosition =  viewMatrixInv * viewSpacePosition;
//    worldSpacePosition = inverseModelMatrix * worldSpacePosition;
    worldSpacePosition /= worldSpacePosition.w;

    return worldSpacePosition.xyz;
}

float LinearizeDepth(float depth)
{
  float nearPlane = lightNearPlane;
  float farPlane = lightFarPlane;
    float z = depth * 2.0 - 1.0; // back to NDC
    float d = (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
    return d / farPlane;
}

float CalcShadowFactor(vec4 LightSpacePos)
{
  float bias = 0.00000f;
  vec2 texelSize = 1.0 / textureSize(shadowTexture, 0);
  vec3 ProjCoords = LightSpacePos.xyz / LightSpacePos.w;
  vec2 UVCoords;
  UVCoords.x = 0.5 * ProjCoords.x + 0.5;
  UVCoords.y = 0.5 * ProjCoords.y + 0.5;
  float z = 0.5 * ProjCoords.z + 0.5;

  // percentage close filtering of the shadow map
  float shadow = 0.0;
  int k = 2;
  for(int x = -k; x <= k; ++x)
  {
    for(int y = -k; y <= k; ++y)
    {
      float pcfDepth = texture(shadowTexture, UVCoords + vec2(x, y) * texelSize).r;
      shadow += z - bias > pcfDepth ? 0.5 : 1.0;
    }
  }

  return shadow / (2 * k + 1) / (2 * k + 1);
}

vec3 to_color(vec3 v)
{
  return (normalize(v) + 1.0f) / 2.0f;
}

vec3 sharpenFilter() {
  vec2 texelSize = 1.0 / textureSize(shadowTexture, 0);
  vec3 color = 5.0f * texture(colorTexture, texCoord).rgb;
  color -= texture(colorTexture, texCoord - texelSize * vec2(0.0f, 1.0f)).rgb;
  color -= texture(colorTexture, texCoord - texelSize * vec2(1.0f, 0.0f)).rgb;
  color -= texture(colorTexture, texCoord - texelSize * vec2(0.0f, -1.0f)).rgb;
  color -= texture(colorTexture, texCoord - texelSize * vec2(-1.0f, 0.0f)).rgb;
  return color;
}

vec3 edgeDetectionFilter() {
  vec2 texelSize = 1.0 / textureSize(shadowTexture, 0);
  vec3 color = 8.0f * texture(colorTexture, texCoord).rgb;
  color -= texture(colorTexture, texCoord - texelSize * vec2(0.0f, 1.0f)).rgb;
  color -= texture(colorTexture, texCoord - texelSize * vec2(1.0f, 0.0f)).rgb;
  color -= texture(colorTexture, texCoord - texelSize * vec2(0.0f, -1.0f)).rgb;
  color -= texture(colorTexture, texCoord - texelSize * vec2(-1.0f, 0.0f)).rgb;
  color -= texture(colorTexture, texCoord - texelSize * vec2(1.0f, 1.0f)).rgb;
  color -= texture(colorTexture, texCoord - texelSize * vec2(1.0f, -1.0f)).rgb;
  color -= texture(colorTexture, texCoord - texelSize * vec2(-1.0f, -1.0f)).rgb;
  color -= texture(colorTexture, texCoord - texelSize * vec2(-1.0f, 1.0f)).rgb;
  return color;
}

vec3 applyKernel(mat3 kernel) {
  vec2 texelSize = 1.0 / textureSize(shadowTexture, 0);
  vec2 pixels[9] = vec2[9](
    vec2(-1.0f, -1.0f), vec2(-1.0f,  0.0f), vec2(-1.0f,  1.0f),
    vec2( 0.0f, -1.0f), vec2( 0.0f,  0.0f), vec2( 0.0f,  1.0f),
    vec2( 1.0f, -1.0f), vec2( 1.0f,  0.0f), vec2( 1.0f,  1.0f)
  );
  vec3 color = vec3(0.0f);
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      color += kernel[i][j] * texture(colorTexture, texCoord + pixels[3 * i + j] * texelSize).rgb;
    }
  }
  return color;
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
