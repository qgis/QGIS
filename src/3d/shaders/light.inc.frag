
// light related utilities and shading functions

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

struct LightParams
{
  vec3 s; // light direction vector (from surface to light source)
  float sDotN; // NoL (filament). Cosine of the angle between light direction and surface normal. Clamped to >= 0.0
  float att; // Distance-based attenuation and spot light cone falloff multiplier.
  float visibilityFactor; // visibility after shadowing applied. 0 = no visibility, completely shadowed. 1 = completely visible, no shadowing
  vec3 h; // Half-vector between the light direction (s) and the view direction.
  float sDotH; // Cosine of the angle between light direction and half-vector (L dot H). Clamped to >= 0.0.
  float nDotH; // Cosine of the angle between surface normal and half-vector (N dot H). Clamped to >= 0.0.
};

#pragma include shadows.inc.frag


/**
 * \param lightIndex index of light to consider
 * \param wPosition world position
 * \param n NORMALIZED world normal
 * \param wView world view
 */
LightParams calculateLightParams(const in int lightIndex,
                                   const in vec3 wPosition,
                                   const in vec3 n,
                                   const in vec3 wView)
{
    LightParams res;
    res.s = vec3(0.0);
    res.att = 1.0;
    res.sDotN = 0.0;
    res.visibilityFactor = 1.0;

    if (lights[lightIndex].type != TYPE_DIRECTIONAL)
    {
        // Point and Spot lights

        // Light position is already in world space
        vec3 sUnnormalized = vec3(lights[lightIndex].position) - wPosition;
        res.s = normalize(sUnnormalized); // Light direction

        // Calculate the attenuation factor
        res.sDotN = dot(res.s, n);
        if (res.sDotN > 0.0) {
            if (lights[lightIndex].constantAttenuation != 0.0
             || lights[lightIndex].linearAttenuation != 0.0
             || lights[lightIndex].quadraticAttenuation != 0.0) {
                float dist = length(sUnnormalized);
                res.att = 1.0 / (lights[lightIndex].constantAttenuation +
                             lights[lightIndex].linearAttenuation * dist +
                             lights[lightIndex].quadraticAttenuation * dist * dist);
            }

            // The light direction is in world space already
            if (lights[lightIndex].type == TYPE_SPOT) {
                // Check if fragment is inside or outside of the spot light cone
                if (degrees(acos(dot(-res.s, lights[lightIndex].direction))) > lights[lightIndex].cutOffAngle)
                    res.sDotN = 0.0;
            }
        }
    } else {
        // Directional lights
        // The light direction is in world space already
        res.s = normalize(-lights[lightIndex].direction);
        res.sDotN = dot(res.s, n);

        if (renderShadows == 1 && lightIndex == shadowLightIndex)
        {
            res.visibilityFactor = calcVisibilityAfterShadowing(wPosition);
        }
    }
    res.h = normalize(res.s + wView);
    res.sDotH = max(dot(res.s, res.h), 0.0);
    res.sDotN = max(res.sDotN, 0.0);
    res.nDotH = max(dot(n, res.h), 0.0);
    return res;
}
