#version 150

uniform vec4 lineColor;
uniform bool useTex;
uniform sampler2D tex0;

in VertexData{
    vec2 mTexCoord;
//	vec3 mColor;
} VertexIn;

out vec4 oColor;

void main(void)
{
    if (!useTex)
    {
        // option 1: plain color
        oColor = lineColor;
    }
    else
    {
        // option 2: textured color
        oColor = texture(tex0, VertexIn.mTexCoord.xy );
    }

    //vec4 clr = texture( tex0, VertexIn.mTexCoord.xy );
    //oColor.rgb = VertexIn.mColor * clr.rgb;
    //oColor.a = clr.a;
}
