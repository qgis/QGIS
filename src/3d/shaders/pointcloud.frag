#version 150

#ifdef STYLE_COLOR_RAMP
in float parameter;
#endif
#ifdef STYLE_CLASSIFICATION
flat in int classParameter;
#endif
#ifdef STYLE_RGB
in vec3 pointColor;
#endif
#ifdef TRIANGULATE
in vec3 worldPosition;
in vec3 vertNorm;
#endif
out vec4 color;

#ifdef STYLE_SINGLE_COLOR
// Sets the unique mesh color
uniform vec3 u_singleColor;
#endif

#if defined(STYLE_COLOR_RAMP) || defined(STYLE_CLASSIFICATION)
// Sets the texture that stores the color ramp or classification color ramp
uniform sampler1D u_colorRampTexture;
#endif

#ifdef STYLE_COLOR_RAMP
// Sets the color ramp type, 0: linear, 1: discrete, 2: exact
uniform int u_colorRampType;
// Sets the color ramp value count, used to check the if not void
uniform int u_colorRampCount;
#endif

#ifdef TRIANGULATE
#pragma include phong.inc.frag
#else
#pragma include light.inc.frag
#endif

#ifdef STYLE_COLOR_RAMP
vec4 linearColorRamp()
{
  int colorRampSize=textureSize(u_colorRampTexture,0);

  if (colorRampSize==1)
  {
    vec4 colorRampLine=texelFetch(u_colorRampTexture,0,0);
    return vec4( colorRampLine.yzw, 1.0f );
  }

  for (int i=0;i<(colorRampSize-1);++i)
  {
    vec4 colorRampLine1=texelFetch(u_colorRampTexture,i,0);
    vec4 colorRampLine2=texelFetch(u_colorRampTexture,i+1,0);

    vec3 color1=colorRampLine1.yzw;
    vec3 color2=colorRampLine2.yzw;

    float value1=colorRampLine1.x;
    float value2=colorRampLine2.x;

    if (parameter <= value1 )
      return vec4( color1, 1.0f );

    if (parameter > value1 && parameter <= value2)
    {
      float mixValue=(parameter - value1)/(value2-value1);
      return vec4( mix(color1,color2,mixValue), 1.0f );
    }
  }

  //last color   if no value is found
  vec4 colorRampLine=texelFetch(u_colorRampTexture,colorRampSize-1,0);
  return vec4( colorRampLine.yzw, 1.0f );
}

vec4 discreteColorRamp()
{
  int colorRampSize=textureSize(u_colorRampTexture,0);

  vec3 color=vec3(0.5,.5,0.5);

  for (int i=0;i<(colorRampSize);++i)
  {
    vec4 colorRampLine=texelFetch(u_colorRampTexture,i,0);
    color=colorRampLine.yzw;
    float value=colorRampLine.x;
    if ( isinf(value) || parameter < value)
      return vec4( color, 1.0f );
  }

  return vec4( color, 1.0f );
}

vec4 exactColorRamp()
{
  int colorRampSize=textureSize(u_colorRampTexture,0);

  for (int i=0;i<(colorRampSize);++i)
  {
    vec4 colorRampLine = texelFetch( u_colorRampTexture, i, 0 );
    vec3 color=colorRampLine.yzw;
    float value=colorRampLine.x;
    if ( abs( float(parameter) - value ) < 0.01 )
      return vec4( color, 1.0f );
  }
  return vec4(0.0, 0.0, 0.0, 1.0f);
}

vec4 colorRamp()
{
  if (u_colorRampCount<=0)
    return vec4(0.0,0.0,0.0,1);

  vec4 colorRampResult = vec4(0.0f, 0.0f, 0.0f, 1.0f);
  switch (u_colorRampType)
  {
  case 0:
    colorRampResult = linearColorRamp();
    break;
  case 1:
    colorRampResult = discreteColorRamp();
    break;
  case 2:
    colorRampResult = exactColorRamp();
    break;
  };

  return colorRampResult;
}
#endif

#ifdef STYLE_CLASSIFICATION
vec4 classification()
{
  vec4 colorRampLine = texelFetch( u_colorRampTexture, classParameter - 1, 0 );
  return vec4(colorRampLine.yzw,1.0);
}
#endif

void main(void)
{
#ifdef STYLE_SINGLE_COLOR
    color = vec4(u_singleColor, 1.0f);
#endif
#ifdef STYLE_COLOR_RAMP
    color = colorRamp();
    // the colors interpolated from the ramp are always SRGB colors, otherwise
    // we get non-visually linear ramp scaling. So now we need to convert
    // to linear for output color and light handling
    color = vec4(pow(color.rgb, vec3(2.2)), color.a);
#endif
#ifdef STYLE_RGB
    // RGB (linear color, color has been linearised in the point data buffer)
    color = vec4(pointColor, 1.0f);
#endif
#ifdef STYLE_CLASSIFICATION
    color = classification();
    // the colors retrieved from the ramp are always SRGB colors. So now we need to convert
    // to linear for output color and light handling
    color = vec4(pow(color.rgb, vec3(2.2)), color.a);
#endif

  //Apply light
#ifdef TRIANGULATE
  float ambianceFactor=0.15; //value defined empirically by visual check to avoid too dark scene
  vec3 diffuseColor;
  adModel(worldPosition, vertNorm, diffuseColor);
  color =vec4( color.xyz * (diffuseColor+ambianceFactor), 1 );
#endif
}
