#version 330 core

in treatedVertex {
    vec3 worldPosition;
    vec3 worldNormal;
    float magnitude;
    noperspective vec4 edgeA;
    noperspective vec4 edgeB;
    flat int configuration;
} fs_in;

out vec4 fragColor;

uniform bool wireframeEnabled;
uniform float lineWidth;
uniform vec4 lineColor;

uniform int textureType;
uniform int colorRampType;
uniform vec4 meshColor;
uniform float verticaleScale;

uniform sampler1D colorRampTexture;
uniform int colorRampCount;

#pragma include ../light.inc.frag

// modified copy from Qt source : examples/qt3d/wireframe/robustwireframe.frag
vec4 wireframeShadeLine( const in vec4 color )
{
    // Find the smallest distance between the fragment and a triangle edge
    float d;
    if ( fs_in.configuration == 0 )
    {
        // Common configuration
        d = min( fs_in.edgeA.x, fs_in.edgeA.y );
        d = min( d, fs_in.edgeA.z );
    }
    else
    {
        // Handle configuration where screen space projection breaks down
        // Compute and compare the squared distances
        vec2 AF = gl_FragCoord.xy - fs_in.edgeA.xy;
        float sqAF = dot( AF, AF );
        float AFcosA = dot( AF, fs_in.edgeA.zw );
        d = abs( sqAF - AFcosA * AFcosA );

        vec2 BF = gl_FragCoord.xy - fs_in.edgeB.xy;
        float sqBF = dot( BF, BF );
        float BFcosB = dot( BF, fs_in.edgeB.zw );
        d = min( d, abs( sqBF - BFcosB * BFcosB ) );

        // Only need to care about the 3rd edge for some configurations.
        if ( fs_in.configuration == 1 || fs_in.configuration == 2 || fs_in.configuration == 4 )
        {
            float AFcosA0 = dot( AF, normalize( fs_in.edgeB.xy - fs_in.edgeA.xy ) );
            d = min( d, abs( sqAF - AFcosA0 * AFcosA0 ) );
        }

        d = sqrt( d );
    }

    // Blend between line color and phong color
    float mixVal;
    if ( d < lineWidth - 1.0 )
    {
        mixVal = 1.0;
    }
    else if ( d > lineWidth + 1.0 )
    {
        mixVal = 0.0;
    }
    else
    {
        float x = d - ( lineWidth - 1.0 );
        mixVal = exp2( -2.0 * ( x * x ) );
    }

    return mix( color, lineColor, mixVal );
}

vec3 linearColorRamp()
{
   int colorRampSize=textureSize(colorRampTexture,0);

   if (colorRampSize==1)
   {
     vec4 colorRampLine=texture(colorRampTexture,(0.5)/colorRampSize);
     return colorRampLine.yzw;
   }


   for (int i=0;i<(colorRampSize-1);++i)
   {
      vec4 colorRampLine1=texture(colorRampTexture,(i+0.5)/colorRampSize);
      vec4 colorRampLine2=texture(colorRampTexture,(i+1.5)/colorRampSize);

      vec3 color1=colorRampLine1.yzw;
      vec3 color2=colorRampLine2.yzw;

      float value1=colorRampLine1.x*verticaleScale;
      float value2=colorRampLine2.x*verticaleScale;

        if (fs_in.magnitude>value1 && fs_in.magnitude<=value2)
        {
            float mixValue=(fs_in.magnitude-value1)/(value2-value1);
            return mix(color1,color2,mixValue);
        }
  }

  return vec3(0.5,0.5,0.5);
}

vec3 discreteColorRamp()
{
   int colorRampSize=textureSize(colorRampTexture,0);

   vec3 color=vec3(0.5,.5,0.5);

    for (int i=0;i<(colorRampSize);++i)
    {
        vec4 colorRampLine=texture(colorRampTexture,(i+0.5)/colorRampSize);

        color=colorRampLine.yzw;
        float value=colorRampLine.x*verticaleScale;

        if ( isinf(value) || fs_in.magnitude<value)
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
        vec4 colorRampLine=texture(colorRampTexture,(i+0.5)/colorRampSize);

        vec3 color=colorRampLine.yzw;
        float value=colorRampLine.x*verticaleScale;

        if ( abs(fs_in.magnitude-value)<0.01)
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

void main()
{
    vec4 color;
    switch (textureType)
    {
      case 0:
      color=meshColor;
      break;
      case 1:
      color=colorRamp();
      break;
    };

    float ambianceFactor=0.15;
    vec3 diffuseColor;

    //Apply light
    adModel(fs_in.worldPosition, fs_in.worldNormal,diffuseColor);
    color = vec4(  color.xyz * (diffuseColor+ambianceFactor), 1 );

    if (wireframeEnabled)
      color = wireframeShadeLine( color );

    fragColor=color;
}
