#version 330

uniform vec3 eyePosition;

in vec3 worldPosition;
in vec3 worldNormal;

uniform float shininess;
uniform float opacity;
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;

out vec4 fragColor;

#pragma include phong.inc.frag

void main(void)
{
    vec3 worldView = normalize(eyePosition - worldPosition);
    fragColor = phongFunction(
                    vec4(ambientColor, opacity),
                    vec4(diffuseColor, opacity),
                    vec4(specularColor, opacity),
                    shininess,
                    worldPosition,
                    worldView,
                    worldNormal);
}
