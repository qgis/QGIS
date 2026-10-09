// copied from qt3d/src/extras/shaders/gl3/phong.inc.frag

#pragma include light.inc.frag


void adsModel(const in vec3 worldPos,
              const in vec3 worldNormal,
              const in vec3 worldView,
              const in float shininess,
              out vec3 diffuseColor,
              out vec3 specularColor)
{
    diffuseColor = vec3(0.0);
    specularColor = vec3(0.0);

    // We perform all work in world space
    vec3 n = normalize(worldNormal);

    for (int i = 0; i < lightCount; ++i) {
        LightParams light = calculateLightParams(i, worldPos, n, worldView);

        // Calculate the diffuse factor
        float diffuse = max(light.sDotN, 0.0);

        // Calculate the specular factor
        float specular = 0.0;
        if (diffuse > 0.0 && shininess > 0.0) {
            float normFactor = (shininess + 2.0) / (2.0 * 3.14159);
            vec3 r = reflect(-light.s, n);   // Reflection direction in world space
            specular = normFactor * pow(max(dot(r, worldView), 0.0), shininess);
        }

        // Accumulate the diffuse and specular contributions
        diffuseColor += light.visibilityFactor * light.att * lights[i].intensity * diffuse * lights[i].color;
        specularColor += light.visibilityFactor * light.att * lights[i].intensity * specular * lights[i].color;
    }
}

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
    vec3 color = ambient.rgb
               + diffuseColor * diffuse.rgb
               + specularColor * specular.rgb;

    return vec4(color, diffuse.a);
}

void adModel(const in vec3 worldPos,
             const in vec3 worldNormal,
             out vec3 diffuseColor)
{
    vec3 worldView = normalize(eyePosition - worldPos);
    diffuseColor = vec3(0.0);

    // We perform all work in world space
    vec3 n = normalize(worldNormal);
    vec3 s = vec3(0.0);

    for (int i = 0; i < lightCount; ++i) {
        float att = 1.0;
        float sDotN = 0.0;
        float visibilityFactor = 1.0;
        LightParams light = calculateLightParams(i, worldPos, n, worldView);

        // Calculate the diffuse factor
        float diffuse = max(light.sDotN, 0.0);

        // Accumulate the diffuse contributions
        diffuseColor += light.visibilityFactor * light.att * lights[i].intensity * diffuse * lights[i].color;
    }
}
