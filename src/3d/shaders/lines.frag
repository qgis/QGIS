#version 150

uniform vec4 lineColor;
uniform bool useTex;
uniform bool useVertexColors;
uniform sampler2D tex0;

in VertexData{
    vec2 mTexCoord;
    vec3 mColor;
} VertexIn;

out vec4 oColor;

void main(void)
{
    if (useVertexColors)
    {
        // option 1: vertex color
        oColor = vec4(VertexIn.mColor, 1.0);
    }
    else if ( !useTex )
    {
        // option 2: uniform color
        oColor = lineColor;
    }
    else
    {
        // option 3: textured color
        oColor = texture(tex0, VertexIn.mTexCoord.xy );
    }

    //vec4 clr = texture( tex0, VertexIn.mTexCoord.xy );
    //oColor.rgb = VertexIn.mColor * clr.rgb;
    //oColor.a = clr.a;
}
