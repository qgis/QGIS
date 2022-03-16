#version 150

uniform bool triangulate;

in float parameter;
flat in int classParameter;

in vec3 pointColor;
in vec3 worldPosition; //used when points are triangulated
in vec3 vertNorm; //used when points are triangulated
out vec4 color;

// Sets the redering style, 0: unique color, 1: color ramp shader of terrain, 2: color ramp shader of 2D rendering, 3 : RGB, 4 : Classification
uniform int u_renderingStyle;
// Sets the unique mesh color
uniform vec3 u_singleColor;
// Sets the color ramp type, 0: linear, 1: discrete, 2: exact
uniform int u_colorRampType;
// Sets the texture that stores the color ramp
uniform sampler1D u_colorRampTexture; //
// Sets the color ramp value count, used to check the if not void
uniform int u_colorRampCount;

#pragma include light.inc.frag

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

vec4 classification()
{
  vec4 colorRampLine = texelFetch( u_colorRampTexture, classParameter - 1, 0 );
  return vec4(colorRampLine.yzw,1.0);
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

void main(void)
{
  switch (u_renderingStyle)
  {
  case 0: //  no rendering
    color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    break;
  case 1: // single color
    color = vec4(u_singleColor, 1.0f);
    break;
  case 2: // color ramp
    color = colorRamp();
    break;
  case 3: // RGB
    color = vec4(pointColor, 1.0f);
    break;
  case 4: // classification
    color = classification();
    break;
  }

  //Apply light
  if (triangulate)
  {
      float ambianceFactor=0.15; //value defined empircally by visual check to avoid too dark scene
      vec3 diffuseColor;
      adModel(worldPosition, vertNorm, diffuseColor);
      color =vec4( color.xyz * (diffuseColor+ambianceFactor), 1 );
  }

}
