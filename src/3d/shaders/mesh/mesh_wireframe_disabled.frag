#version 150 core

out vec4 fragColor;

uniform sampler2D lightedColorTexture;
uniform int colorTextureWidth;
uniform int colorTextureHeight;


void main()
{
    vec2 colorCoord=vec2(gl_FragCoord.x/colorTextureWidth,gl_FragCoord.y/colorTextureHeight);
    //vec4 color=texture(lightedColorTexture,colorCoord);

    float depthValue=gl_FragCoord.z;

    float l=-1/log(1-depthValue*1000+int(depthValue*1000));

    fragColor = vec4(l,1-l,0,1.0) ;
}
