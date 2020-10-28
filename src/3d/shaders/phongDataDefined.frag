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

#pragma include light.inc.frag

vec4 phongFunction(const in vec4 ambient,
                   const in vec4 diffuse,
                   const in vec4 specular,
                   const in float shin,
                   const in vec3 worldPosition,
                   const in vec3 worldView,
                   const in vec3 worldNormal)
{
    // Calculate the lighting model, keeping the specular component separate
    vec3 diffuseColor, specularColor;
    adsModel(worldPosition, worldNormal, worldView, shin, diffuseColor, specularColor);

    // Combine spec with ambient+diffuse for final fragment color
    vec3 color = (ambient.rgb + diffuseColor) * diffuse.rgb
               + specularColor * specular.rgb;

    return vec4(color, diffuse.a);
}

void main(void)
{
    fragColor = phongFunction(vs_in.ambient,vs_in.diffuse,vs_in.specular, 1.0, worldPosition, eyePosition, worldNormal);
}
