#version 330

in vec3 worldPosition;
in vec3 worldNormal;
in vec2 texCoord;

uniform vec3 eyePosition;
uniform vec4 ambientColor;
uniform vec4 specularColor;
uniform float shininess;
uniform sampler2D diffuseTexture;
uniform float opacity;

out vec4 fragColor;

#pragma include phong.inc.frag

void main(void)
{
    vec4 diffuseTextureColor = vec4(texture(diffuseTexture, texCoord).rgb, opacity);
    vec3 worldView = normalize(eyePosition - worldPosition);
    fragColor = phongFunction(ambientColor, diffuseTextureColor, specularColor,
                              shininess,
                              worldPosition, worldView, worldNormal);
}
