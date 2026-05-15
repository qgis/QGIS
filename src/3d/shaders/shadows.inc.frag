uniform sampler2DArray shadowTexture;
uniform float shadowBias;
uniform float maxShadowDistance;
uniform mat4 invertedCameraView;

// if you change the number of cascades, you also need to update Qgs3D::NUM_SHADOW_CASCADES
const int NUMBER_CASCADES = 4;
uniform mat4 csmMatrices[NUMBER_CASCADES];
uniform mat4 csmBoundsMatrices[NUMBER_CASCADES];

struct CascadeInfo {
  int index1;
  int index2;
  float blend; // blend factor between cascades, where 0 = use only index1 and 1 = use only index2
};

const float CASCADE_BLEND_BAND = 0.1;
// band size to fade all shadows over as the max shadow distance is approached (as proportion of max shadow distance)
const float MAX_SHADOW_DISTANCE_FADE_OVER = 0.05;

CascadeInfo calcCascadeInfo(vec3 worldPosition)
{
  CascadeInfo info;
  info.index1 = -1;
  info.index2 = -1;
  info.blend = 1.0;

  for (int i = 0; i < NUMBER_CASCADES; ++i)
  {
    // map based cascade selection -- see
    // https://learn.microsoft.com/en-us/windows/win32/dxtecharts/cascaded-shadow-maps#map-based-cascade-selection
    vec4 projCoords = csmBoundsMatrices[i] * vec4(worldPosition, 1.0);
    vec3 ndc = projCoords.xyz / projCoords.w;
    bool insideXY = all(lessThan(abs(ndc.xy), vec2(1.0)));
    bool insideZ = ndc.z > -1.0 && ndc.z < 1.0;
    if (insideXY && insideZ)
    {
      info.index1 = i;
      if ( i < NUMBER_CASCADES - 1 )
      {
        // if we aren't using the final cascade, then smoothly blend into the next one at the
        // seams
        float maxDist = max(max(abs(ndc.x), abs(ndc.y)), abs(ndc.z));
        info.blend = smoothstep(1.0 - CASCADE_BLEND_BAND, 1.0, maxDist);
        if (info.blend > 0.0)
        {
          // blend into the next cascade
          info.index2 = i + 1;
        }
      }
      break;
    }
  }

  return info;
}

float calcSingleCascadeShadowFactor(int cascadeIndex, vec3 worldPosition)
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

float calcVisibilityAfterShadowing(vec3 worldPosition)
{
  CascadeInfo info = calcCascadeInfo(worldPosition);
  if ( info.index1 < 0 )
  {
    return 1.0;
  }

  float shadow1 = calcSingleCascadeShadowFactor(info.index1, worldPosition);

  float finalShadow = shadow1;
  if (info.index2 >= 0){
    float shadow2 = calcSingleCascadeShadowFactor(info.index2, worldPosition);
    finalShadow = mix(shadow1, shadow2, info.blend);
  }

  // fade off as global distance to shadow approaches maximum shadow rendering distance
  vec3 cameraPos = invertedCameraView[3].xyz;
  float distToCamera = length(worldPosition - cameraPos);
  float fadeStartDistance = maxShadowDistance * (1.0 - MAX_SHADOW_DISTANCE_FADE_OVER);
  float distanceFade = smoothstep(fadeStartDistance, maxShadowDistance, distToCamera);

  return mix(finalShadow, 1.0, distanceFade);
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

vec3 calcCascadeTint(vec3 worldPosition) {
  CascadeInfo info = calcCascadeInfo(worldPosition);
  if ( info.index1 < 0 )
  {
    return vec3(1.0);
  }
  vec3 color1 = cascadeTint(info.index1);
  vec3 finalColor = color1;

  if (info.index2 >= 0) {
    vec3 color2 = cascadeTint(info.index2);
    finalColor = mix(color1, color2, info.blend);
  }

  // fade off as global distance to shadow approaches maximum shadow rendering distance
  vec3 cameraPos = invertedCameraView[3].xyz;
  float distToCamera = length(worldPosition - cameraPos);
  float fadeStartDistance = maxShadowDistance * (1.0-MAX_SHADOW_DISTANCE_FADE_OVER);
  float distanceFade = smoothstep(fadeStartDistance, maxShadowDistance, distToCamera);

  return mix(finalColor, vec3(1.0), distanceFade);
}
#endif
