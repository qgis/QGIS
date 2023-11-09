
// copy of light.inc.frag from qt3d extras

const int MAX_LIGHTS = 8;
const int TYPE_POINT = 0;
const int TYPE_DIRECTIONAL = 1;
const int TYPE_SPOT = 2;
struct Light {
    int type;
    vec3 position;
    vec3 color;
    float intensity;
    vec3 direction;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
    float cutOffAngle;
};
uniform Light lights[MAX_LIGHTS];
uniform int lightCount;

// Pre-convolved environment maps
struct EnvironmentLight {
    samplerCube irradiance; // For diffuse contribution
    samplerCube specular; // For specular contribution
};
uniform EnvironmentLight envLight;
uniform int envLightCount = 0;

void adsModelNormalMapped(const in vec3 worldPos,
                          const in vec3 tsNormal,
                          const in vec3 worldEye,
                          const in float shininess,
                          const in mat3 tangentMatrix,
                          out vec3 diffuseColor,
                          out vec3 specularColor)
{
    diffuseColor = vec3(0.0);
    specularColor = vec3(0.0);

    // We perform all work in tangent space, so we convert quantities from world space
    vec3 tsPos = tangentMatrix * worldPos;
    vec3 n = normalize(tsNormal);
    vec3 v = normalize(tangentMatrix * (worldEye - worldPos));
    vec3 s = vec3(0.0);

    for (int i = 0; i < lightCount; ++i) {
        float att = 1.0;
        float sDotN = 0.0;

        if (lights[i].type != TYPE_DIRECTIONAL) {
            // Point and Spot lights

            // Transform the light position from world to tangent space
            vec3 tsLightPos = tangentMatrix * lights[i].position;
            vec3 sUnnormalized = tsLightPos - tsPos;
            s = normalize(sUnnormalized); // Light direction in tangent space

            // Calculate the attenuation factor
            sDotN = dot(s, n);
            if (sDotN > 0.0) {
                if (lights[i].constantAttenuation != 0.0
                 || lights[i].linearAttenuation != 0.0
                 || lights[i].quadraticAttenuation != 0.0) {
                    float dist = length(sUnnormalized);
                    att = 1.0 / (lights[i].constantAttenuation +
                                 lights[i].linearAttenuation * dist +
                                 lights[i].quadraticAttenuation * dist * dist);
                }

                // The light direction is in world space, convert to tangent space
                if (lights[i].type == TYPE_SPOT) {
                    // Check if fragment is inside or outside of the spot light cone
                    vec3 tsLightDirection = tangentMatrix * lights[i].direction;
                    if (degrees(acos(dot(-s, tsLightDirection))) > lights[i].cutOffAngle)
                        sDotN = 0.0;
                }
            }
        } else {
            // Directional lights
            // The light direction is in world space, convert to tangent space
            s = normalize(tangentMatrix * -lights[i].direction);
            sDotN = dot(s, n);
        }

        // Calculate the diffuse factor
        float diffuse = max(sDotN, 0.0);

        // Calculate the specular factor
        float specular = 0.0;
        if (diffuse > 0.0 && shininess > 0.0) {
            float normFactor = (shininess + 2.0) / 2.0;
            vec3 r = reflect(-s, n);   // Reflection direction in tangent space
            specular = normFactor * pow(max(dot(r, v), 0.0), shininess);
        }

        // Accumulate the diffuse and specular contributions
        diffuseColor += att * lights[i].intensity * diffuse * lights[i].color;
        specularColor += att * lights[i].intensity * specular * lights[i].color;
    }
}

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
    vec3 s = vec3(0.0);

    for (int i = 0; i < lightCount; ++i) {
        float att = 1.0;
        float sDotN = 0.0;

        if (lights[i].type != TYPE_DIRECTIONAL) {
            // Point and Spot lights

            // Light position is already in world space
            vec3 sUnnormalized = lights[i].position - worldPos;
            s = normalize(sUnnormalized); // Light direction

            // Calculate the attenuation factor
            sDotN = dot(s, n);
            if (sDotN > 0.0) {
                if (lights[i].constantAttenuation != 0.0
                 || lights[i].linearAttenuation != 0.0
                 || lights[i].quadraticAttenuation != 0.0) {
                    float dist = length(sUnnormalized);
                    att = 1.0 / (lights[i].constantAttenuation +
                                 lights[i].linearAttenuation * dist +
                                 lights[i].quadraticAttenuation * dist * dist);
                }

                // The light direction is in world space already
                if (lights[i].type == TYPE_SPOT) {
                    // Check if fragment is inside or outside of the spot light cone
                    if (degrees(acos(dot(-s, lights[i].direction))) > lights[i].cutOffAngle)
                        sDotN = 0.0;
                }
            }
        } else {
            // Directional lights
            // The light direction is in world space already
            s = normalize(-lights[i].direction);
            sDotN = dot(s, n);
        }

        // Calculate the diffuse factor
        float diffuse = max(sDotN, 0.0);

        // Calculate the specular factor
        float specular = 0.0;
        if (diffuse > 0.0 && shininess > 0.0) {
            float normFactor = (shininess + 2.0) / 2.0;
            vec3 r = reflect(-s, n);   // Reflection direction in world space
            specular = normFactor * pow(max(dot(r, worldView), 0.0), shininess);
        }

        // Accumulate the diffuse and specular contributions
        diffuseColor += att * lights[i].intensity * diffuse * lights[i].color;
        specularColor += att * lights[i].intensity * specular * lights[i].color;
    }
}

void adModel(const in vec3 worldPos,
             const in vec3 worldNormal,
             out vec3 diffuseColor)
{
    diffuseColor = vec3(0.0);

    // We perform all work in world space
    vec3 n = normalize(worldNormal);
    vec3 s = vec3(0.0);

    for (int i = 0; i < lightCount; ++i) {
        float att = 1.0;
        float sDotN = 0.0;

        if (lights[i].type != TYPE_DIRECTIONAL) {
            // Point and Spot lights

            // Light position is already in world space
            vec3 sUnnormalized = lights[i].position - worldPos;
            s = normalize(sUnnormalized); // Light direction

            // Calculate the attenuation factor
            sDotN = dot(s, n);
            if (sDotN > 0.0) {
                if (lights[i].constantAttenuation != 0.0
                 || lights[i].linearAttenuation != 0.0
                 || lights[i].quadraticAttenuation != 0.0) {
                    float dist = length(sUnnormalized);
                    att = 1.0 / (lights[i].constantAttenuation +
                                 lights[i].linearAttenuation * dist +
                                 lights[i].quadraticAttenuation * dist * dist);
                }

                // The light direction is in world space already
                if (lights[i].type == TYPE_SPOT) {
                    // Check if fragment is inside or outside of the spot light cone
                    if (degrees(acos(dot(-s, lights[i].direction))) > lights[i].cutOffAngle)
                        sDotN = 0.0;
                }
            }
        } else {
            // Directional lights
            // The light direction is in world space already
            s = normalize(-lights[i].direction);
            sDotN = dot(s, n);
        }

        // Calculate the diffuse factor
        float diffuse = max(sDotN, 0.0);

        // Accumulate the diffuse contributions
        diffuseColor += att * lights[i].intensity * diffuse * lights[i].color;
    }
}
