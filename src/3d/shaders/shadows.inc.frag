uniform sampler2DArray shadowTexture;
uniform float shadowBias;

// if you change the number of cascades, you also need to update Qgs3D::NUM_SHADOW_CASCADES
const int NUMBER_CASCADES = 4;
uniform mat4 csmMatrices[NUMBER_CASCADES];
// uncomment if using interval based selection
//uniform float csmSplits[NUMBER_CASCADES];

bool isInsideCascade(mat4 lightMatrix, vec3 worldPos)
{
  // map based cascade selection -- see
  // https://learn.microsoft.com/en-us/windows/win32/dxtecharts/cascaded-shadow-maps#map-based-cascade-selection
  vec4 projCoords = lightMatrix * vec4(worldPos, 1.0);
  vec3 ndc = projCoords.xyz / projCoords.w;
  bool insideXY = all(lessThan(abs(ndc.xy), vec2(1.0)));
  bool insideZ = ndc.z > -1.0 && ndc.z < 1.0;
  return insideXY && insideZ;
}

int calcCascadeIndexMapBased(vec3 worldPosition)
{
  // Determine which cascade this pixel falls into
  int cascadeIndex = NUMBER_CASCADES-1;
  for (int i = 0; i < NUMBER_CASCADES; ++i)
  {
    if (isInsideCascade(csmMatrices[i], worldPosition))
    {
      cascadeIndex = i;
      break;
    }
  }
  return cascadeIndex;
}

#if 0
int calcCascadeIndexIntervalBased(float viewZ)
{
  // Interval-Based Selection -- see
  // https://learn.microsoft.com/en-us/windows/win32/dxtecharts/cascaded-shadow-maps#map-based-cascade-selection
  int cascadeIndex = NUMBER_CASCADES-1;
  for (int i = 0; i < NUMBER_CASCADES; ++i)
  {
    if (viewZ < csmSplits[i]) {
      cascadeIndex = i;
      break;
    }
  }
  return cascadeIndex;
}
#endif

float calcShadowFactor(int cascadeIndex, vec3 worldPosition)
{
  float layerIndex = float(cascadeIndex);
  mat4 lightMatrix = csmMatrices[cascadeIndex];
  vec2 texelSize = 1.0 / vec2(textureSize(shadowTexture, 0).xy);

  vec4 LightSpacePos = lightMatrix * vec4(worldPosition, 1.0);
  vec3 ProjCoords = LightSpacePos.xyz / LightSpacePos.w;

  vec2 UVCoords;
  UVCoords.x = 0.5 * ProjCoords.x + 0.5;
  UVCoords.y = 0.5 * ProjCoords.y + 0.5;
  float z = 0.5 * ProjCoords.z + 0.5;

  if ( UVCoords.x < texelSize.x || UVCoords.x > 1.0 - texelSize.x ||
       UVCoords.y < texelSize.y || UVCoords.y > 1.0 - texelSize.y || z > 1.0 )
  {
    return 1.0;
  }

  // percentage close filtering of the shadow map
  float shadow = 0.0;
  int k = 1;
  for(int x = -k; x <= k; ++x)
  {
    for(int y = -k; y <= k; ++y)
    {
      vec3 arrayCoord = vec3(UVCoords + vec2(x, y) * texelSize, layerIndex);
      float pcfDepth = texture(shadowTexture, arrayCoord).r;
      shadow += z - shadowBias > pcfDepth ? 0.0 : 1.0;
    }
  }
  return shadow / (2 * k + 1) / (2 * k + 1);
}

#ifdef TINT_CASCADES
vec3 cascadeTint(int cascadeIndex)
{
  // returns a unique color representing a cascade index
  const vec3 colors[9] = vec3[9](
    vec3(1.0, 0.2, 0.2),
    vec3(0.2, 1.0, 0.2),
    vec3(0.2, 0.4, 1.0),
    vec3(1.0, 1.0, 0.0),
    vec3(1.0, 0.0, 1.0),
    vec3(0.0, 1.0, 1.0),
    vec3(1.0, 0.5, 0.0),
    vec3(0.5, 0.0, 1.0),
    vec3(1.0, 1.0, 1.0)
  );
  return colors[clamp(cascadeIndex, 0, 8)];
}
#endif
