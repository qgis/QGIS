#version 330 core

in vec3 worldPosition;
in vec3 worldNormal;
in float magnitude;
in vec3 barycentric;

out vec4 fragColor;

//Uniforms
// Sets if the wireframe is enabled
uniform bool wireframeEnabled;
// Sets if the line width of the wireframe
uniform float lineWidth;
// Sets the color of the wireframe
uniform vec4 lineColor;
// Sets if triangles should use per face normal instead of the per vertex normal
uniform bool flatTriangles;

// Sets the redering style, 0: unique color, 1: color ramp shader of terrain, 2: color ramp shader of 2D rendering
uniform int textureType;

// Sets the color ramp type, 0: linear, 1: discrete, 2: exact
uniform int colorRampType;
// Sets the unique mesh color
uniform vec4 meshColor;
// Sets the texture that stores the color ramp
uniform sampler1D colorRampTexture;
// Sets the color ramp value count, used to check the if not void
uniform int colorRampCount;

// Sets if the arrow rendering is enabled
uniform bool arrowsEnabled;
// Sets the texture used to store vector values
uniform sampler2D arrowsGridTexture;
// Sets the location of the minimum corner of the grid
uniform vec2 arrowsMinCorner;
// Sets the spacing between arrows in map unit
uniform float arrowsSpacing;
// Sets the texture used for drawing arrows
uniform sampler2D arrowTexture;
// Sets the arrows color
uniform vec4 arrowsColor;

#pragma include ../phong.inc.frag


float edgeFactor()
{
    vec3 dBdx = dFdx( barycentric );
    vec3 dBdy = dFdy( barycentric );
    vec3 gradientLength = sqrt( dBdx * dBdx + dBdy * dBdy );
    vec3 d = barycentric / gradientLength;
    float dist = min( min( d.x, d.y ), d.z );

    if ( dist < lineWidth - 1.0 )
        return 0.0;
    if ( dist > lineWidth + 1.0 )
        return 1.0;

    float x = dist - ( lineWidth - 1.0 );
    return 1.0 - exp2( -2.0 * x * x );
}


vec3 linearColorRamp()
{
   int colorRampSize=textureSize(colorRampTexture,0);

   if (colorRampSize==1)
   {
     vec4 colorRampLine=texelFetch(colorRampTexture,0,0);
     return colorRampLine.yzw;
   }


   for (int i=0;i<(colorRampSize-1);++i)
   {
      vec4 colorRampLine1=texelFetch(colorRampTexture,i,0);
      vec4 colorRampLine2=texelFetch(colorRampTexture,i+1,0);

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
   vec4 colorRampLine=texelFetch(colorRampTexture,colorRampSize-1,0);
   return colorRampLine.yzw;



  return vec3(0.5,0.5,0.5);
}

vec3 discreteColorRamp()
{
   int colorRampSize=textureSize(colorRampTexture,0);

   vec3 color=vec3(0.5,.5,0.5);

    for (int i=0;i<(colorRampSize);++i)
    {
        vec4 colorRampLine=texelFetch(colorRampTexture,i,0);

        color=colorRampLine.yzw;
        float value=colorRampLine.x;

        if ( isinf(value) || magnitude<value)
        {
            return color;
        }
    }

    return color;
}

vec3 exactColorRamp()
{
   int colorRampSize=textureSize(colorRampTexture,0);

    for (int i=0;i<(colorRampSize);++i)
    {
        vec4 colorRampLine=texelFetch(colorRampTexture,i,0);

        vec3 color=colorRampLine.yzw;
        float value=colorRampLine.x;

        if ( abs(magnitude-value)<0.01)
        {
            return color;
        }
    }

    return vec3(0.5,0.5,0.5);
}


vec4 colorRamp()
{
    if (colorRampCount<=0)
      return vec4(0.5,0.5,0.5,1);

    vec3 colorRampResult;

    switch (colorRampType)
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

float arrows()
{
    ivec2 size=textureSize(arrowsGridTexture,0);

    float posX=(worldPosition.x-arrowsMinCorner.x)/arrowsSpacing+0.5;
    float posY=(worldPosition.y+arrowsMinCorner.y)/arrowsSpacing+0.5;
    int gridPosX=int(posX);
    int gridPosY=int(posY);

    if (gridPosX<0 || gridPosX>size.x || gridPosY<0 || gridPosY>size.y)
        return 0.0;

    ivec2 textureGridPosition=ivec2(gridPosX,gridPosY);
    vec4 gridValue=texelFetch(arrowsGridTexture,textureGridPosition,0);
    float scale=gridValue.x;
    float angle=gridValue.y-1.570796;

    vec2 textureArrowPosition=vec2(posX-gridPosX,posY-gridPosY);

    float s = sin(angle);
    float c = cos(angle);
    vec2 pivot=vec2(0.5,0.5);
    mat2 rotationMatrix = mat2( c, -s, s,  c);
    textureArrowPosition=textureArrowPosition-pivot;
    textureArrowPosition=rotationMatrix*(textureArrowPosition);
    textureArrowPosition/=scale;
    textureArrowPosition=textureArrowPosition+pivot;

    return texture(arrowTexture,textureArrowPosition).a;
}

void main()
{
    vec4 color;
    switch (textureType)
    {
      case 0:
      color=meshColor;
      break;
      case 1:
      case 2:
      color=colorRamp();
      // the colors interpolated from the ramp are always SRGB colors, otherwise
      // we get non-visually linear ramp scaling. So now we need to convert
      // to linear for output color and light handling
      color = vec4(pow(color.rgb, vec3(2.2)), color.a);
      break;
    };

    float ambianceFactor=0.15;
    vec3 diffuseColor;

    vec3 shadingNormal;
    if (flatTriangles)
        shadingNormal = normalize( cross( dFdx( worldPosition ), dFdy( worldPosition ) ) );
    else
        shadingNormal = worldNormal;

    //Apply light
    adModel(worldPosition, shadingNormal, diffuseColor);
    color = vec4(  color.xyz * (diffuseColor+ambianceFactor), 1 );

    if (wireframeEnabled)
      color = mix( lineColor, color, edgeFactor() );

    if (arrowsEnabled)
    {
      float a=arrows();
      color=a*arrowsColor+(1-a)*color;
    }

    fragColor=color;
}
