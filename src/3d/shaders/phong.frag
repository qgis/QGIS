#version 330

uniform vec3 eyePosition;
uniform float shininess;

in vec3 worldPosition;
in vec3 worldNormal;

#ifdef DATA_DEFINED
in DataColor {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
} vs_in;
#else
uniform float opacity;
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
#endif

out vec4 fragColor;

#pragma include phong.inc.frag

void main(void)
{
    vec3 worldView = normalize(eyePosition - worldPosition);

    #ifdef DATA_DEFINED
      fragColor = phongFunction(
                      vs_in.ambient,
                      vs_in.diffuse,
                      vs_in.specular,
                      shininess,
                      worldPosition,
                      worldView,
                      worldNormal);
    #else
      fragColor = phongFunction(
                      vec4(ambientColor, opacity),
                      vec4(diffuseColor, opacity),
                      vec4(specularColor, opacity),
                      shininess,
                      worldPosition,
                      worldView,
                      worldNormal);
    #endif
}
