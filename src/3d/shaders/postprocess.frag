#version 330

uniform sampler2D colorTexture;

#ifdef ENABLE_EFFECTS
uniform sampler2D depthTexture;
uniform sampler2D ssaoTexture;

// view camera uniforms
uniform mat4 invertedCameraProj;
uniform float farPlane;
uniform float nearPlane;

uniform int edlEnabled;
uniform float edlStrength;
uniform int edlDistance;

uniform int ssaoEnabled;

uniform sampler2D bloomTexture;
uniform int bloomEnabled;
uniform float bloomFactor;
#endif

#ifdef ENABLE_EFFECTS
uniform float exposureAdjustment;
uniform int toneMapping;
#else
uniform float exposureAdjustment = 0;
uniform int toneMapping = 0;
#endif

in vec2 texCoord;

out vec4 fragColor;

uniform mat4 invertedCameraView;

#ifdef ENABLE_EFFECTS

#pragma include shadows.inc.frag

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
#endif

vec3 aces_approx(vec3 v)
{
  v *= 0.6f;
  float a = 2.51f;
  float b = 0.03f;
  float c = 2.43f;
  float d = 0.59f;
  float e = 0.14f;
  return clamp((v*(a*v+b))/(v*(c*v+d)+e), 0.0f, 1.0f);
}


void main()
{
  vec3 linearColor = texture(colorTexture, texCoord).rgb;

  vec3 finalColor = linearColor;

#ifdef ENABLE_EFFECTS

#ifdef TINT_CASCADES
  float depth = texture(depthTexture, texCoord).r;
  if ( renderShadows != 0 && depth < 1.0 )
  {
    vec3 worldPosition = WorldPosFromDepth( depth );
    vec3 cameraPosition = invertedCameraView[3].xyz;
    // for debugging: shade pixels by cascade index, to visualise cascade breaks
    finalColor = mix(finalColor, calcCascadeTint(worldPosition, cameraPosition), 0.5);
  }
#endif
  
  if (edlEnabled != 0)
  {
    float shade = exp(-edlFactor(texCoord) * edlStrength);
    finalColor = finalColor * shade;
  }
  if ( ssaoEnabled != 0 )
  {
    finalColor = finalColor.rgb * texture( ssaoTexture, texCoord ).r;
  }
#endif

  // Apply exposure correction
  finalColor *= exp2(exposureAdjustment);

#ifdef ENABLE_EFFECTS
  // bloom comes AFTER exposure, but must be BEFORE tone mapping (we require HDR ranges for
  // bloom)
  if ( bloomEnabled != 0 )
  {
    vec3 bloom = texture(bloomTexture, texCoord).rgb;
    finalColor += bloom * bloomFactor;
  }
#endif

  // Apply tonemap transform to get into LDR range [0, 1]
  // (aces looks great with exposure ~0.5, but maybe not wanted for GIS applications? could be an option...)
  if ( toneMapping == 1 )
  {
    finalColor = aces_approx(finalColor);
  }
  else
  {
    // just hard clamp instead. we lose detail in bright areas, but retain exact match for colors in the 0-1 range,
    // which is more appropriate for mapping anyway.
    finalColor = min(finalColor, 1);
  }

  // gamma correction is hardcoded to 2.2
  vec3 sRgbColor = pow(finalColor, vec3(1.0 / 2.2));

  fragColor = vec4(sRgbColor, 1.0f);
}
