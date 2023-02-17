// copied from qt3d/src/extras/shaders/gl3/phong.inc.frag

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
