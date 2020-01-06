#version 330 core

out vec4 fragColor;

uniform sampler2D lightedColorTexture;
uniform int colorTextureWidth;
uniform int colorTextureHeight;


void main()
{
    vec2 colorCoord=vec2(gl_FragCoord.x/colorTextureWidth,gl_FragCoord.y/colorTextureHeight);
    vec4 color=texture(lightedColorTexture,colorCoord);

    fragColor = color ;
}
