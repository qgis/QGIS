#version 330 core

in treatedVertex {
    vec3 position;
    vec3 wordPosition;
    vec3 wordNormal;
} fs_in;

out vec4 fragColor;

uniform sampler2D colorTexture;
uniform int colorTextureWidth;
uniform int colorTextureHeight;
uniform float ambianceFactor;

#pragma include ../light.inc.frag

void main()
{
    vec3 diffuseColor;
    vec3 specularColor;
    vec2 colorCoord=vec2(gl_FragCoord.x/colorTextureWidth,gl_FragCoord.y/colorTextureHeight);
    vec4 color=texture(colorTexture,colorCoord);
    adsModel(fs_in.wordPosition, fs_in.wordNormal, fs_in.position, 0, diffuseColor, specularColor);

    fragColor = vec4(  color.xyz * (diffuseColor+ambianceFactor) + vec3(1.0,1.0,1.0) *specularColor, 1 );
}
