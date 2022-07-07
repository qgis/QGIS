#version 330

uniform vec3 eyePosition;
uniform float shininess;

in vec3 worldPosition;
in vec3 worldNormal;

in DataColor {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
} vs_in;

out vec4 fragColor;

#pragma include phong.inc.frag

void main(void)
{
    vec3 worldView = normalize(eyePosition - worldPosition);
    fragColor = phongFunction(vs_in.ambient,vs_in.diffuse,vs_in.specular, 1.0, worldPosition, worldView, worldNormal);
}
