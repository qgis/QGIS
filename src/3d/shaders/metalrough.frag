// Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#version 140

// defines are added here as a pre-processing step

uniform vec3 eyePosition;
in vec3 worldPosition;

#ifndef FLAT_SHADING
in vec3 worldNormal;
#endif

#ifdef BASE_COLOR_MAP
uniform sampler2D baseColorMap;
#else
uniform vec4 baseColor;
#endif

#ifdef METALNESS_MAP
uniform sampler2D metalnessMap;
#else
uniform float metalness;
#endif

#ifdef ROUGHNESS_MAP
uniform sampler2D roughnessMap;
#else
uniform float roughness;
#endif

#ifdef AMBIENT_OCCLUSION_MAP
uniform sampler2D ambientOcclusionMap;
#endif

#ifdef NORMAL_MAP
uniform sampler2D normalMap;
#endif

#if defined(NORMAL_MAP) || defined(HEIGHT_MAP)
in vec4 worldTangent;
#endif

#ifdef HEIGHT_MAP
uniform sampler2D heightMap;
uniform float parallaxScale = 0.1;
#endif

#ifdef EMISSION_MAP
uniform sampler2D emissionMap;
uniform float emissiveFactor = 1;
#endif

#if defined(BASE_COLOR_MAP) || defined(METALNESS_MAP) || defined(ROUGHNESS_MAP) || defined(AMBIENT_OCCLUSION_MAP) || defined(NORMAL_MAP)|| defined(HEIGHT_MAP) || defined(EMISSION_MAP)
in vec2 texCoord;
#endif

uniform float opacity;

const float PI = 3.14159265359;

#pragma include light.inc.frag

#if defined(NORMAL_MAP) || defined(HEIGHT_MAP)
mat3 calcTangentToWorldSpaceMatrix(const in vec3 wNormal, const in vec4 wTangent)
{
    vec3 N = normalize(wNormal);

    // Make the tangent truly orthogonal to the normal by using Gram-Schmidt.
    // This allows building the tangentMatrix below by simply transposing the
    // tangent -> eyespace matrix (which would now be orthogonal)
    vec3 wFixedTangent = normalize(wTangent.xyz - dot(wTangent.xyz, N) * N);

    // Calculate binormal vector. No "real" need to renormalize it,
    // as built by crossing two normal vectors.
    // To orient the binormal correctly, use the fourth coordinate of the tangent,
    // which is +1 for a right hand system, and -1 for a left hand system.
    vec3 wBinormal = cross(N, wFixedTangent.xyz) * wTangent.w;

    // Construct matrix to transform from tangent space to world space
    mat3 tangentToWorldMatrix = mat3(wFixedTangent, wBinormal, N);
    return tangentToWorldMatrix;
}

mat3 calcTangentSpace(const in vec3 wNormal, const in vec3 wPosition, const in vec2 uv)
{
    vec3 Q1 = dFdx(wPosition);
    vec3 Q2 = dFdy(wPosition);
    vec2 st1 = dFdx(uv);
    vec2 st2 = dFdy(uv);

    vec3 N = normalize(wNormal);
    vec3 T = normalize(Q1*st2.t - Q2*st1.t);
    T = normalize(T - dot(T, N) * N);
    vec3 B = -normalize(cross(N, T));

    return mat3(T, B, N);
}
#endif

#ifdef HEIGHT_MAP
// https://www.artstation.com/blogs/andreariccardi/3VPo/a-new-approach-for-parallax-mapping-presenting-the-contact-refinement-parallax-mapping-technique
// adapted from https://github.com/panthuncia/webgl_test/blob/main/index.html
vec3 applyContactRefinementParallaxCoordsAndHeight(const in vec2 uv, const in vec3 viewDirTangent)
{
    float maxHeight = parallaxScale;
    float minHeight = maxHeight * 0.5;

    int numSteps = 15;

    float viewCorrection = (-viewDirTangent.z) + 2.0;
    float stepSize = 1.0 / (float(numSteps) + 1.0);

    vec2 tsOffset = viewDirTangent.xy * vec2(1.0, -1.0)* viewCorrection;

    vec2 stepOffset = tsOffset * vec2(maxHeight, maxHeight) * stepSize;
    vec2 lastOffset = tsOffset * vec2(minHeight, minHeight) + uv;

    float lastRayDepth = 1.0;
    float lastHeight = 1.0;
    vec2 p1;
    vec2 p2;
    bool refine = false;

    while (numSteps > 0)
    {
        vec2 candidateOffset = lastOffset - stepOffset;
        float currentRayDepth = lastRayDepth - stepSize;

        float currentHeight = texture(heightMap, candidateOffset).r;

        if (currentHeight > currentRayDepth)
        {
            p1 = vec2(currentRayDepth, currentHeight);
            p2 = vec2(lastRayDepth, lastHeight);

            if (refine)
            {
                lastHeight = currentHeight;
                break;
            } else {
                refine = true;
                lastRayDepth = p2.x;
                stepSize /= float(numSteps);
                stepOffset /= float(numSteps);
                continue;
            }
        }
        lastOffset = candidateOffset;
        lastRayDepth = currentRayDepth;
        lastHeight = currentHeight;
        numSteps -= 1;
    }

    float diff1 = p1.x - p1.y;
    float diff2 = p2.x - p2.y;
    float denominator = diff2 - diff1;

    float parallaxAmount = 0.0;
    if(denominator != 0.0)
    {
        parallaxAmount = (p1.x * diff2 - p2.x * diff1) / denominator;
    }

    float offsetDepth = ((1.0 - parallaxAmount) * -maxHeight) + minHeight;
    return vec3(tsOffset * offsetDepth + uv, lastHeight);
}
#endif


float remapRoughness(const in float roughness)
{
    // As per page 14 of
    // http://www.frostbite.com/wp-content/uploads/2014/11/course_notes_moving_frostbite_to_pbr.pdf
    // we remap the roughness to give a more perceptually linear response
    // of "bluriness" as a function of the roughness specified by the user.
    // r = roughness^2
    const float maxSpecPower = 999999.0;
    const float minRoughness = sqrt(2.0 / (maxSpecPower + 2));
    return max(roughness * roughness, minRoughness);
}

float alphaToMipLevel(float alpha)
{
    float specPower = 2.0 / (alpha * alpha) - 2.0;

    // We use the mip level calculation from Lys' default power drop, which in
    // turn is a slight modification of that used in Marmoset Toolbag. See
    // https://docs.knaldtech.com/doku.php?id=specular_lys for details.
    // For now we assume a max specular power of 999999 which gives
    // maxGlossiness = 1.
    const float k0 = 0.00098;
    const float k1 = 0.9921;
    float glossiness = (pow(2.0, -10.0 / sqrt(specPower)) - k0) / k1;

    // Lookup the number of mips in the specular envmap
    int mipLevels = envLight.specularMipLevels;

    // Offset of smallest miplevel we should use (corresponds to specular
    // power of 1). I.e. in the 32x32 sized mip.
    const float mipOffset = 5.0;

    // The final factor is really 1 - g / g_max but as mentioned above g_max
    // is 1 by definition here so we can avoid the division. If we make the
    // max specular power for the spec map configurable, this will need to
    // be handled properly.
    float mipLevel = (mipLevels - 1.0 - mipOffset) * (1.0 - glossiness);
    return mipLevel;
}

float normalDistribution(const in vec3 n, const in vec3 h, const in float alpha)
{
    // Trowbridge-Reitz GGX
    // see https://google.github.io/filament/Filament.md.html, "Normal distribution function (specular D)"
    // https://learnopengl.com/PBR/Theory, "Normal distribution function"
    float alpha2 = alpha * alpha;
    float nDotH = max(dot(n, h), 0.0);
    float nDotH2 = nDotH * nDotH;

    float denom = (nDotH2 * (alpha2 - 1.0) + 1.0);

    // our alpha is already clamped by remapRoughness, so denom won't perfectly hit 0
    return alpha2 / (PI * denom * denom);
}

vec3 fresnelFactor(const in vec3 color, const in float cosineFactor)
{
    // Calculate the Fresnel effect value
    vec3 f = color;
    vec3 F = f + (1.0 - f) * pow(clamp(1.0 - cosineFactor, 0.0, 1.0), 5.0);
    return clamp(F, f, vec3(1.0));
}

// A modified Fresnel function that respects surface roughness
// It explicitly injects the roughness value into the Fresnel equation to forcefully clamp
// the maximum reflectivity at grazing angles based on how rough the material is.
// ** Only applicable for image based lighting **
vec3 fresnelSchlickRoughness(const in vec3 F0, const in float cosTheta, const in float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float geometrySchlickGGX(const in float nDotV, const in float k)
{
    // see https://learnopengl.com/PBR/Theory, "Geometry function"
    float nom = nDotV;
    float denom = nDotV * (1.0 - k) + k;
    return nom / denom;
}

float geometricModel(const in float lDotN,
                     const in float vDotN,
                     const in float roughness,
                     const in bool isIBL)
{
    // see https://learnopengl.com/PBR/Theory, "Geometry function"

    float k;
    if ( isIBL)
    {
        // for image based lighting we do NOT apply the Epic hotness fix
        k = roughness * roughness / 2.0;
    }
    else
    {
        // calculate k for direct lighting
        // uses Epic Unreal Engine 4 fix of k = (roughness + 1)^2 / 8.
        // see https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
        // "Specular G"
        k = ((roughness + 1) * (roughness + 1)) / 8.0;
    }

    // note that geometrySchlickGGX uses "nDotV", but dot product is commutative, so
    // that's not an issue
    float ggx1 = geometrySchlickGGX(vDotN, k);
    float ggx2 = geometrySchlickGGX(lDotN, k);

    return ggx1 * ggx2;
}

vec3 specularModel(const in vec3 F0,
                   const in float sDotH,
                   const in float sDotN,
                   const in float vDotN,
                   const in vec3 n,
                   const in vec3 h,
                   const in float roughness,
                   const in bool isIBL)
{
    // Clamp sDotN and vDotN to small positive value to prevent the
    // denominator in the reflection equation going to infinity. Balance this
    // by using the clamped values in the geometric factor function to
    // avoid ugly seams in the specular lighting.
    float sDotNPrime = max(sDotN, 0.001);
    float vDotNPrime = max(vDotN, 0.001);

    vec3 F = isIBL ? fresnelSchlickRoughness(F0, sDotH, roughness) : fresnelFactor(F0, sDotH);
    float G = geometricModel(sDotNPrime, vDotNPrime, roughness, isIBL);

    vec3 cSpec = F * G / (4.0 * sDotNPrime * vDotNPrime);
    return clamp(cSpec, vec3(0.0), vec3(1.0));
}

vec3 pbrModel(const in int lightIndex,
              const in vec3 wPosition,
              const in vec3 wNormal,
              const in vec3 wView,
              const in vec3 baseColor,
              const in float metalness,
              const in float roughness,
              const in float alpha,
              const in float ambientOcclusion)
{
    // Calculate some useful quantities
    vec3 n = wNormal;
    vec3 s = vec3(0.0);
    vec3 v = wView;
    vec3 h = vec3(0.0);

    float vDotN = dot(v, n);
    float sDotN = 0.0;
    float sDotH = 0.0;
    float att = 1.0;

    if (lights[lightIndex].type != TYPE_DIRECTIONAL) {
        // Point and Spot lights
        vec3 sUnnormalized = vec3(lights[lightIndex].position) - wPosition;
        s = normalize(sUnnormalized);

        // Calculate the attenuation factor
        sDotN = dot(s, n);
        if (sDotN > 0.0) {
            if (lights[lightIndex].constantAttenuation != 0.0
             || lights[lightIndex].linearAttenuation != 0.0
             || lights[lightIndex].quadraticAttenuation != 0.0) {
                float dist = length(sUnnormalized);
                att = 1.0 / (lights[lightIndex].constantAttenuation +
                             lights[lightIndex].linearAttenuation * dist +
                             lights[lightIndex].quadraticAttenuation * dist * dist);
            }

            // The light direction is in world space already
            if (lights[lightIndex].type == TYPE_SPOT) {
                // Check if fragment is inside or outside of the spot light cone
                if (degrees(acos(dot(-s, lights[lightIndex].direction))) > lights[lightIndex].cutOffAngle)
                    sDotN = 0.0;
            }
        }
    } else {
        // Directional lights
        // The light direction is in world space already
        s = normalize(-lights[lightIndex].direction);
        sDotN = dot(s, n);
    }

    h = normalize(s + v);
    sDotH = dot(s, h);

    // Calculate diffuse component
    vec3 diffuseColor = (1.0 - metalness) * baseColor * lights[lightIndex].color;
    vec3 diffuse = diffuseColor * max(sDotN, 0.0) / PI;

    // Calculate specular component
    vec3 dielectricColor = vec3(0.04);
    vec3 F0 = mix(dielectricColor, baseColor, metalness);
    vec3 specularFactor = vec3(0.0);
    if (sDotN > 0.0) {
        specularFactor = specularModel(F0, sDotH, sDotN, vDotN, n, h, roughness, false);
        specularFactor *= normalDistribution(n, h, alpha);
    }
    vec3 specularColor = lights[lightIndex].color;
    vec3 specular = specularColor * specularFactor * max(sDotN, 0.0);

    // Blend between diffuse and specular to conserve energy
    // see https://learnopengl.com/PBR/Theory, "Energy conservation"
    vec3 kS = fresnelFactor(F0, sDotH);
    vec3 color = att * lights[lightIndex].intensity * (specular + diffuse * (vec3(1.0) - kS));

    // Reduce by ambient occlusion amount
    color *= ambientOcclusion;

    return color;
}

vec3 pbrIblModel(const in vec3 wNormal,
                 const in vec3 wView,
                 const in vec3 baseColor,
                 const in float metalness,
                 const in float roughness,
                 const in float alpha,
                 const in float ambientOcclusion)
{
    // Calculate reflection direction of view vector about surface normal
    // vector in world space. This is used in the fragment shader to sample
    // from the environment textures for a light source. This is equivalent
    // to the l vector for punctual light sources. Armed with this, calculate
    // the usual factors needed
    vec3 n = wNormal;
    vec3 l = reflect(-wView, n);
    vec3 v = wView;
    vec3 h = normalize(l + v);
    float vDotN = dot(v, n);
    float lDotN = dot(l, n);
    float lDotH = dot(l, h);

    // Calculate diffuse component
    vec3 diffuseColor = (1.0 - metalness) * baseColor;
    vec3 diffuse = diffuseColor * texture(envLight.irradiance, n).rgb;

    // Calculate specular component
    vec3 dielectricColor = vec3(0.04);
    vec3 F0 = mix(dielectricColor, baseColor, metalness);
    vec3 specularFactor = specularModel(F0, lDotH, lDotN, vDotN, n, h, roughness, true);

    float lod = alphaToMipLevel(alpha);
//#define DEBUG_SPECULAR_LODS
#ifdef DEBUG_SPECULAR_LODS
    if (lod > 7.0)
        return vec3(1.0, 0.0, 0.0);
    else if (lod > 6.0)
        return vec3(1.0, 0.333, 0.0);
    else if (lod > 5.0)
        return vec3(1.0, 1.0, 0.0);
    else if (lod > 4.0)
        return vec3(0.666, 1.0, 0.0);
    else if (lod > 3.0)
        return vec3(0.0, 1.0, 0.666);
    else if (lod > 2.0)
        return vec3(0.0, 0.666, 1.0);
    else if (lod > 1.0)
        return vec3(0.0, 0.0, 1.0);
    else if (lod > 0.0)
        return vec3(1.0, 0.0, 1.0);
#endif
    vec3 specularSkyColor = textureLod(envLight.specular, l, lod).rgb;
    vec3 specular = specularSkyColor * specularFactor;

    // Blend between diffuse and specular to conserve energy
    // see https://learnopengl.com/PBR/Theory, "Energy conservation"
    vec3 kS = fresnelSchlickRoughness(F0, max(vDotN, 0.0), roughness);
    vec3 color = specular + diffuse * (vec3(1.0) - kS);

    // Reduce by ambient occlusion amount
    color *= ambientOcclusion;

    return color;
}

vec4 metalRoughFunction(const in vec4 baseColor,
                        const in float metalness,
                        const in float roughness,
                        const in float ambientOcclusion,
                        const in vec3 worldPosition,
                        const in vec3 worldView,
                        const in vec3 worldNormal,
                        const in vec2 activeTexCoord)
{
    vec3 cLinear = vec3(0.0);

    // Remap roughness for a perceptually more linear correspondence
    float alpha = remapRoughness(roughness);

    for (int i = 0; i < envLightCount; ++i) {
        cLinear += pbrIblModel(worldNormal,
                               worldView,
                               baseColor.rgb,
                               metalness,
                               roughness,
                               alpha,
                               ambientOcclusion);
    }

    for (int i = 0; i < lightCount; ++i) {
        cLinear += pbrModel(i,
                            worldPosition,
                            worldNormal,
                            worldView,
                            baseColor.rgb,
                            metalness,
                            roughness,
                            alpha,
                            ambientOcclusion);
    }

#ifdef EMISSION_MAP
    vec3 emission = texture(emissionMap, activeTexCoord).rgb * emissiveFactor;
    cLinear += emission;
#endif
    return vec4(cLinear, 1.0);
}


out vec4 fragColor;

void main()
{
#if defined(BASE_COLOR_MAP) || defined(METALNESS_MAP) || defined(ROUGHNESS_MAP) || defined(AMBIENT_OCCLUSION_MAP) || defined(NORMAL_MAP) || defined(HEIGHT_MAP) || defined(EMISSION_MAP)
    vec2 activeTexCoord = texCoord;
#else
    // unused
    vec2 activeTexCoord = vec2(0.0f, 0.0f);
#endif

    vec3 worldView = normalize(eyePosition - worldPosition);

#if defined(NORMAL_MAP) || defined(HEIGHT_MAP)
    mat3 tangentToWorld;
    if (length(worldTangent.xyz) > 0.001)
    {
        // use model tangents if they exist
        tangentToWorld = calcTangentToWorldSpaceMatrix(worldNormal, worldTangent);
    }
    else
    {
        // fall back to derivative tangents if we don't have model tangents (worse quality)
        tangentToWorld = calcTangentSpace(worldNormal, worldPosition, activeTexCoord);
    }
#endif

#ifdef HEIGHT_MAP
    // need the view vector in tangent space
    mat3 worldToTangent = transpose(tangentToWorld);
    vec3 tangentView = normalize(worldToTangent * worldView);
    // apply parallax and then use adjusted coordinates from here
    activeTexCoord = applyContactRefinementParallaxCoordsAndHeight(texCoord, tangentView).xy;
#endif

#ifdef BASE_COLOR_MAP
    vec4 c = texture(baseColorMap, activeTexCoord);
#else
    vec4 c = baseColor;
#endif

#ifdef METALNESS_MAP
    float m = texture(metalnessMap, activeTexCoord).r;
#else
    float m = metalness;
#endif

#ifdef ROUGHNESS_MAP
    float r = texture(roughnessMap, activeTexCoord).r;
#else
    float r = roughness;
#endif

#ifdef AMBIENT_OCCLUSION_MAP
    float ao = texture(ambientOcclusionMap, activeTexCoord).r;
#else
    float ao = 1.0;
#endif

#ifdef NORMAL_MAP
    vec3 mapN = texture(normalMap, activeTexCoord).rgb * 2.0 - 1.0;
    vec3 n = normalize(tangentToWorld * mapN);
#else
#ifdef FLAT_SHADING
 vec3 fdx = dFdx(worldPosition);
 vec3 fdy = dFdy(worldPosition);
 vec3 n = normalize(cross(fdx, fdy));
#else
 vec3 n = normalize(worldNormal);
#endif
#endif

    fragColor = vec4(metalRoughFunction(c, m, r, ao,
                                   worldPosition,
                                   worldView,
                                   n, activeTexCoord).rgb, opacity);
}
