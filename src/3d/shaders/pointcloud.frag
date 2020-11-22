#version 150

in float magnitude;
in float clsid;

out vec4 color;

// Sets the redering style, 0: unique color, 1: color ramp shader of terrain, 2: color ramp shader of 2D rendering
uniform int u_textureType;
// Sets the unique mesh color
uniform vec3 u_singleColor; //
// Sets the color ramp type, 0: linear, 1: discrete, 2: exact
uniform int u_colorRampType; //
// Sets the texture that stores the color ramp
uniform sampler1D u_colorRampTexture; //
// Sets the color ramp value count, used to check the if not void
uniform int u_colorRampCount; //

vec4 clsidBasedRendering()
{
  vec4 color;
  if ( abs(clsid-2) < 0.1 )         // ground
    color = vec4(1,1,0,1);
  else if ( abs( clsid - 3 ) < 0.1 )      // low vegetation
    color = vec4(0,0.4,0,1);
  else if ( abs( clsid - 4 ) < 0.1 )      // medium vegetation
    color = vec4(0,0.6,0,1);
  else if ( abs( clsid - 5 ) < 0.1 )     // high vegetation
    color = vec4(0,1,0,1);
  else if ( abs( clsid - 12 ) < 0.1 )   // overlaps
  {
    color = vec4(1,0,0,1);
    discard;  // skip overlaps
  }
  else
  {
    color = vec4(0,1,1,1);
    //discard;
  }
  return color;
}

vec3 linearColorRamp()
{
  int colorRampSize=textureSize(u_colorRampTexture,0);

  if (colorRampSize==1)
  {
    vec4 colorRampLine=texelFetch(u_colorRampTexture,0,0);
    return colorRampLine.yzw;
  }


  for (int i=0;i<(colorRampSize-1);++i)
  {
    vec4 colorRampLine1=texelFetch(u_colorRampTexture,i,0);
    vec4 colorRampLine2=texelFetch(u_colorRampTexture,i+1,0);

    vec3 color1=colorRampLine1.yzw;
    vec3 color2=colorRampLine2.yzw;

    float value1=colorRampLine1.x;
    float value2=colorRampLine2.x;

    if (magnitude<=value1 )
      return color1;

    if (magnitude>value1 && magnitude<=value2)
    {
      float mixValue=(magnitude-value1)/(value2-value1);
      return mix(color1,color2,mixValue);
    }
  }

  //last color if no value is found
  vec4 colorRampLine=texelFetch(u_colorRampTexture,colorRampSize-1,0);
  return colorRampLine.yzw;
  return vec3(0.5,0.5,0.5);
}

vec3 discreteColorRamp()
{
  int colorRampSize=textureSize(u_colorRampTexture,0);

  vec3 color=vec3(0.5,.5,0.5);

  for (int i=0;i<(colorRampSize);++i)
  {
    vec4 colorRampLine=texelFetch(u_colorRampTexture,i,0);
    color=colorRampLine.yzw;
    float value=colorRampLine.x;
    if ( isinf(value) || magnitude<value)
      return color;
  }

  return color;
}

vec3 exactColorRamp()
{
  int colorRampSize=textureSize(u_colorRampTexture,0);

  for (int i=0;i<(colorRampSize);++i)
  {
    vec4 colorRampLine = texelFetch( u_colorRampTexture, i, 0 );
    vec3 color=colorRampLine.yzw;
    float value=colorRampLine.x;
    if ( abs( magnitude - value ) < 0.01 )
      return color;
  }

  return vec3(0.5, 0.5, 0.5);
}

vec4 colorRamp()
{
  if (u_colorRampCount<=0)
    return vec4(0.5,0.5,0.5,1);

  vec3 colorRampResult;

  switch (u_colorRampType)
  {
  case 0:
    colorRampResult=linearColorRamp();
    break;
  case 1:
    colorRampResult=discreteColorRamp();
    break;
  case 2:
    colorRampResult=exactColorRamp();
    break;
  };

  return vec4(colorRampResult,1);
}

void main(void)
{
  switch (u_textureType)
  {
  case 0: // single color
    color = vec4(u_singleColor, 1.0f);
    break;
  case 1: // color ramp
    color=colorRamp();
    break;
  }
}
