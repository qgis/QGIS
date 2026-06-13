uniform sampler2DArray shadowTexture;
uniform float shadowBias;
uniform float maxShadowDistance;
uniform vec3 eyePosition;

uniform int renderShadows;
uniform int shadowLightIndex;

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

// from https://github.com/Delt06/unity-pcf-poisson/blob/master/Assets/Shaders/LitPoissonSampling.shader
const int POISSON_SAMPLES = 16;
// how far to spread the shadow samples, can make shadows softer/sharper
// values < 2 look pixelated!
// We could potentially expose this if we want control over shadow softness
const float filterRadius = 2.0;
const vec2 poissonDisk[16] = vec2[](
    vec2( -0.94201624, -0.39906216 ),
    vec2( 0.94558609, -0.76890725 ),
    vec2( -0.094184101, -0.92938870 ),
    vec2( 0.34495938, 0.29387760 ),
    vec2( -0.91588581, 0.45771432 ),
    vec2( -0.81544232, -0.87912464 ),
    vec2( -0.38277543, 0.27676845 ),
    vec2( 0.97484398, 0.75648379 ),
    vec2( 0.44323325, -0.97511554 ),
    vec2( 0.53742981, -0.47373420 ),
    vec2( -0.26496911, -0.41893023 ),
    vec2( 0.79197514, 0.19090188 ),
    vec2( -0.24188840, 0.99706507 ),
    vec2( -0.81409955, 0.91437590 ),
    vec2( 0.19984126, 0.78641367 ),
    vec2( 0.14383161, -0.14100790 )
);

float rand(vec2 co)
{
  // https://stackoverflow.com/a/4275343
  return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

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

  float shadow = 0.0;

  // random angle (in radians) based on the fragment's world position
  float angle = rand(worldPosition.xy) * 6.28318530718;
  float s = sin(angle);
  float c = cos(angle);
  mat2 rot = mat2(c, -s, s, c);

  // Poisson disk filtering of the shadow map
  for(int i = 0; i < POISSON_SAMPLES; ++i)
  {
    vec2 offset = rot * poissonDisk[i] * filterRadius;
    vec3 arrayCoord = vec3(UVCoords + offset * texelSize, layerIndex);

    float pcfDepth = texture(shadowTexture, arrayCoord).r;
    shadow += z - shadowBias > pcfDepth ? 0.0 : 1.0;
  }
  return shadow / float(POISSON_SAMPLES);
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
  float distToCamera = length(worldPosition - eyePosition);
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

vec3 calcCascadeTint(vec3 worldPosition, vec3 cameraPosition) {
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
  float distToCamera = length(worldPosition - cameraPosition);
  float fadeStartDistance = maxShadowDistance * (1.0-MAX_SHADOW_DISTANCE_FADE_OVER);
  float distanceFade = smoothstep(fadeStartDistance, maxShadowDistance, distToCamera);

  return mix(finalColor, vec3(1.0), distanceFade);
}
#endif
