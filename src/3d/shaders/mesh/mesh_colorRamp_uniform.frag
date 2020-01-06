

in MeshVertex {
    vec3 position;
    vec3 wordPosition;
    vec3 wordNormal;
    float magnitude;
} fs_in;


uniform vec4 colorRampValues[COLOR_RAMP_SIZE];

uniform int colorRampType;
uniform float verticaleScale;

out vec4 fragColor;

vec4 linear()
{
   int colorRampSize=COLOR_RAMP_SIZE;

    for (int i=0;i<(colorRampSize-1);++i)
    {
        vec4 color1=vec4(colorRampValues[i].xyz,1.0);
        vec4 color2=vec4(colorRampValues[i+1].xyz,1.0);

        float value1=colorRampValues[i].w*verticaleScale;
        float value2=colorRampValues[i+1].w*verticaleScale;

        if (fs_in.magnitude>value1 && fs_in.magnitude<=value2)
        {
            float mixValue=(fs_in.magnitude-value1)/(value2-value1);
            return mix(color1,color2,mixValue);
        }
    }

    return vec4(0.5,0.5,0.5,1);
}

vec4 discrete()
{
   int colorRampSize=COLOR_RAMP_SIZE;

    for (int i=0;i<(colorRampSize-1);++i)
    {
        vec4 color=vec4(colorRampValues[i].xyz,1.0);

        float value1=colorRampValues[i].w*verticaleScale;

        if (fs_in.magnitude<value1)
        {
            return color;
        }
    }

    return vec4(colorRampValues[colorRampSize-1].xyz,1.0);
}

vec4 exact()
{
   int colorRampSize=COLOR_RAMP_SIZE;

    for (int i=0;i<(colorRampSize-1);++i)
    {
        vec4 color=vec4(colorRampValues[i].xyz,1.0);

        float value1=colorRampValues[i].w*verticaleScale;

        if (fs_in.magnitude==value1)
        {
            return color;
        }
    }

    return vec4(0.5,0.5,0.5,1);
}

void main()
{
    if (colorRampType==0)
    {
        fragColor=linear();
    }

    if (colorRampType==1)
    {
        fragColor=discrete();
    }

    if (colorRampType==2)
    {
        fragColor=exact();
    }
}
