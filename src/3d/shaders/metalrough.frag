// Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#version 330

// defines are added here as a pre-processing step
#ifdef ENABLE_IBL
uniform samplerCube globalSpecularMap;
uniform int globalSpecularMipLevels;
uniform vec3 envLightSh[9];
uniform int envLightMode; // 1 = Skybox IBL, 0 = Solid Background
uniform float envLightStrength;
#endif

in vec3 worldPosition;

#ifndef FLAT_SHADING
in vec3 worldNormal;
#endif

#ifdef DATA_DEFINED
in DataColor {
    vec3 base;
    vec3 emission;
} vs_in;
#elif defined(BASE_COLOR_MAP)
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

uniform float reflectance;

#ifdef AMBIENT_OCCLUSION_MAP
uniform sampler2D ambientOcclusionMap;
#endif

#ifdef NORMAL_MAP
uniform sampler2D normalMap;
#endif

#if defined(NORMAL_MAP) || defined(HEIGHT_MAP) || defined(ANISOTROPY)
in vec4 worldTangent;
#endif

#ifdef HEIGHT_MAP
uniform sampler2D heightMap;
uniform float parallaxScale = 0.1;
#endif

#ifdef ANISOTROPY
uniform float anisotropy;
uniform float anisotropyRotation;
#endif

#ifdef CLEAR_COAT
uniform float clearCoatFactor;
uniform float clearCoatRoughness;
#endif

#ifdef DATA_DEFINED
// DataColor has emission color
#elif defined(EMISSION_MAP)
uniform sampler2D emissionMap;
#else
uniform vec3 emissiveColor;
#endif
uniform float emissiveFactor = 1;

#if defined(BASE_COLOR_MAP) || defined(METALNESS_MAP) || defined(ROUGHNESS_MAP) || defined(AMBIENT_OCCLUSION_MAP) || defined(NORMAL_MAP)|| defined(HEIGHT_MAP) || defined(EMISSION_MAP)
in vec2 texCoord;
#endif

uniform float opacity;

const float PI = 3.14159265359;

#pragma include light.inc.frag

#if defined(NORMAL_MAP) || defined(HEIGHT_MAP) || defined(ANISOTROPY)
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

float normalDistribution(const in float nDotH, const in float alpha)
{
    // Trowbridge-Reitz GGX
    // see https://google.github.io/filament/Filament.md.html, "Normal distribution function (specular D)"
    // https://learnopengl.com/PBR/Theory, "Normal distribution function"
    float alpha2 = alpha * alpha;
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

// analytical approximation of the split-sum for environment bidirectional reflectance distribution function
// as per https://www.unrealengine.com/blog/physically-based-shading-on-mobile?lang=en-US
vec2 environmentBrdfApproximation(const in float roughness, const in float viewDotNormal)
{
    vec4 c0 = vec4(-1.0, -0.0275, -0.572, 0.022);
    vec4 c1 = vec4(1.0, 0.0425, 1.04, -0.04);
    vec4 r = roughness * c0 + c1;
    float a004 = min(r.x * r.x, exp2(-9.28 * viewDotNormal)) * r.x + r.y;
    vec2 ab = vec2(-1.04, 1.04) * a004 + r.zw;
    return ab;
}

#ifdef ANISOTROPY
// anisotropic normal distribution function
float normalDistributionAnisotropic(const in float tDotH, const in float bDotH, const in float nDotH, const in float alphaT, const in float alphaB)
{
    float d = tDotH * tDotH / (alphaT * alphaT) + bDotH * bDotH / (alphaB * alphaB) + nDotH * nDotH;
    return 1.0 / max(PI * alphaT * alphaB * d * d, 0.0001);
}

// anisotropic visibility function (replaces geometric and denominator terms)
// as https://google.github.io/filament/Filament.md.html, 4.10.15 Anisotropic specular BRDF, Listing 16
float visibilityAnisotropic(const in float nDotL, const in float nDotV, const in float tDotV, const in float bDotV, const in float tDotL, const in float bDotL, const in float alphaT, const in float alphaB)
{
    float lambdaV = nDotL * length(vec3(alphaT * tDotV, alphaB * bDotV, nDotV));
    float lambdaL = nDotV * length(vec3(alphaT * tDotL, alphaB * bDotL, nDotL));
    return max(0.5 / max(lambdaV + lambdaL, 0.0001), 0.0);
}

// bend reflection vector for anisotropic image based lighting
// as https://google.github.io/filament/Filament.md.html, 5.3.13 Anisotropy, Listing 33
vec3 bentAnisotropicReflection(const in vec3 v, const in vec3 n, const in vec3 t, const in vec3 b, const in float anisotropy)
{
    vec3 anisotropicDirection = anisotropy >= 0.0 ? b : t;
    vec3 anisotropicTangent = cross(anisotropicDirection, v);
    vec3 anisotropicNormal = cross(anisotropicTangent, anisotropicDirection);
    vec3 bentNormal = normalize(mix(n, anisotropicNormal, anisotropy));
    return reflect(-v, bentNormal);
}
#endif

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

#ifdef CLEAR_COAT
float visibilityKelemen(const in float lDotH)
{
  // as per https://google.github.io/filament/Filament.md.html#materialsystem/clearcoatmodel, listing 13
  return 0.25 / max(lDotH * lDotH, 0.00001);
}

float pow5(float x)
{
  float x2 = x * x;
  return x2 * x2 * x;
}

// https://github.com/google/filament/blob/3427685f2fa40de3a5fcaae6d7ffa157780e53a8/shaders/src/surface_brdf.fs#L155
float fresnelSchlick(float f0, float f90, float VoH)
{
  return f0 + (f90 - f0) * pow5(1.0 - VoH);
}

vec3 adjustF0ForClearCoatInterface(vec3 F0)
{
  // adjust F0 based on clear coat-material interface (IOR 1.5) instead of air-material
  // see https://google.github.io/filament/Filament.md.html#materialsystem/clearcoatmodel/clearcoatparameterization section 4.9.14
  vec3 f0Sqrt = sqrt(clamp(F0, vec3(0.0), vec3(1.0)));
  vec3 f0Num = vec3(1.0) - 5.0 * f0Sqrt;
  vec3 f0Den = vec3(5.0) - f0Sqrt;
  vec3 f0Base = (f0Num * f0Num) / (f0Den * f0Den);
  return mix(F0, f0Base, clearCoatFactor);
}

#endif

vec3 pbrModel(const in int lightIndex,
              const in vec3 wPosition,
              const in vec3 n, // NORMALIZED world normal
              const in vec3 wView,
              const in vec3 t,
              const in vec3 b,
              const in vec3 baseColor,
              const in float metalness,
              const in float roughness,
              const in float reflectance,
              const in float alpha,
              const in float ambientOcclusion,
              const in float anisotropy)
{
    // Calculate some useful quantities
    vec3 v = wView;

    float vDotN = dot(v, n);

    LightParams light = calculateLightParams(lightIndex, wPosition, n, wView);

    // Calculate diffuse component
    vec3 diffuseColor = (1.0 - metalness) * baseColor * lights[lightIndex].color;
    vec3 diffuse = diffuseColor * light.sDotN / PI;

    // Calculate specular component
    vec3 dielectricColor = vec3(0.16 * reflectance * reflectance);
    vec3 F0 = mix(dielectricColor, baseColor, metalness);

#ifdef CLEAR_COAT
    F0 = adjustF0ForClearCoatInterface(F0);
#endif

    vec3 specularFactor = vec3(0.0);
    if (light.sDotN > 0.0) {
#ifdef ANISOTROPY
        float alphaT = max(alpha * (1.0 + anisotropy), 0.0001);
        float alphaB = max(alpha * (1.0 - anisotropy), 0.0001);

        float tDotH = dot(t, light.h);
        float bDotH = dot(b, light.h);
        float nDotH = dot(n, light.h);
        float tDotV = dot(t, v);
        float bDotV = dot(b, v);
        float tDotL = dot(t, light.s);
        float bDotL = dot(b, light.s);

        float D = normalDistributionAnisotropic(tDotH, bDotH, nDotH, alphaT, alphaB);
        float V = visibilityAnisotropic(light.sDotN, vDotN, tDotV, bDotV, tDotL, bDotL, alphaT, alphaB);
        vec3 F = fresnelFactor(F0, light.sDotH);

        specularFactor = D * V * F;
#else
        specularFactor = specularModel(F0, light.sDotH, light.sDotN, vDotN, n, light.h, roughness, false);
        specularFactor *= normalDistribution(light.nDotH, alpha);
#endif

        // calculate multi-scatter energy compensation factor for direct light
        // see https://google.github.io/filament/Filament.md.html#materialsystem/energyconservation, section 4.7.2
        vec2 environmentBrdf = environmentBrdfApproximation(roughness, vDotN);
        float directionalAlbedo = environmentBrdf.x + environmentBrdf.y;
        vec3 energyCompensation = vec3(1.0) + F0 * (1.0 / max(directionalAlbedo, 0.001) - 1.0);
        specularFactor *= energyCompensation;
    }
    vec3 specularColor = lights[lightIndex].color;
    vec3 specular = specularColor * specularFactor * light.sDotN;

    // Blend between diffuse and specular to conserve energy
    // see https://learnopengl.com/PBR/Theory, "Energy conservation"
    vec3 kS = fresnelFactor(F0, light.sDotH);
    vec3 Fd = diffuse * (vec3(1.0) - kS);
    vec3 Fr = specular;

#ifdef CLEAR_COAT
    // as per https://google.github.io/filament/Filament.md.html#materialsystem/clearcoatmodel/clearcoatparameterization, Listing 14
    float ccPerceptualRoughness = clamp(clearCoatRoughness, 0.089, 1.0);
    float ccAlpha = ccPerceptualRoughness * ccPerceptualRoughness;

    // clear coat BRDF
    // we don't currently expose control over clear-coat normals, so standard material
    // normal is used
    float Dc = normalDistribution(light.nDotH, ccAlpha); // we use GGX for normalDistribution
    float Vc = visibilityKelemen(light.sDotH);
    // fixed IOR to 1.5 => reflectance = 0.04
    float Fc = fresnelSchlick(0.04, 1.0, light.sDotH) * clearCoatFactor;

    // clear coat specific specular reflection
    vec3 Frc = vec3(Dc * Vc * Fc) * light.sDotN * lights[lightIndex].color;

    // Account for energy loss in the base layer
    vec3 layeredColor = (Fd + Fr * (1.0 - Fc)) * (1.0 - Fc) + Frc;
#else
    vec3 layeredColor = Fd + Fr;
#endif

    vec3 color = light.visibilityFactor * light.att * lights[lightIndex].intensity * layeredColor;

    // Reduce by ambient occlusion amount
    color *= ambientOcclusion;

    return color;
}

#ifdef ENABLE_IBL
float roughnessToMipLevel(float roughness)
{
  // as per https://google.github.io/filament/Filament.md.html, section 5.3.11.11
  float lod = float(globalSpecularMipLevels - 1) * roughness;
  return max(lod, 0.0);
}

vec3 pbrIblModelSphericalHarmonics(const in vec3 wNormal,
                 const in vec3 wView,
                 const in vec3 t,
                 const in vec3 b,
                 const in vec3 baseColor,
                 const in float metalness,
                 const in float roughness,
                 const in float reflectance,
                 const in float alpha,
                 const in float ambientOcclusion,
                 const in float anisotropy)
{
    // Calculate reflection direction of view vector about surface normal
    // vector in world space. This is used in the fragment shader to sample
    // from the environment textures for a light source. This is equivalent
    // to the l vector for punctual light sources. Armed with this, calculate
    // the usual factors needed
    vec3 n = wNormal;
#ifdef ANISOTROPY
    // apply bent reflection for anisotropic ibl
    vec3 l = bentAnisotropicReflection(wView, n, t, b, anisotropy);
#else
    vec3 l = reflect(-wView, n); // r (filament)
#endif
    vec3 v = wView;
    vec3 h = normalize(l + v);
    float vDotN = max(dot(v, n), 0.0); // NoV (filament)
    float lDotN = dot(l, n);
    float lDotH = dot(l, h);

    // Calculate diffuse component
    vec3 diffuseColor = (1.0 - metalness) * baseColor;

    // Calculate environmental irradiance from spherical harmonics
    const float c1 = 0.429043;
    const float c2 = 0.511664;
    const float c3 = 0.743125;
    const float c4 = 0.886227;
    const float c5 = 0.247708;
    vec3 envIrradiance =
        c1 * envLightSh[8] * (wNormal.x * wNormal.x - wNormal.y * wNormal.y) +
        c3 * envLightSh[6] * wNormal.z * wNormal.z +
        c4 * envLightSh[0] -
        c5 * envLightSh[6] +
        2.0 * c1 * (envLightSh[4] * wNormal.x * wNormal.y + envLightSh[7] * wNormal.x * wNormal.z + envLightSh[5] * wNormal.y * wNormal.z) +
        2.0 * c2 * (envLightSh[3] * wNormal.x + envLightSh[1] * wNormal.y + envLightSh[2] * wNormal.z);
    envIrradiance = max(envIrradiance, vec3(0.0));

    vec3 diffuse = diffuseColor * envIrradiance;

    // Calculate specular component
    vec3 dielectricColor = vec3(0.16 * reflectance * reflectance);
    vec3 F0 = mix(dielectricColor, baseColor, metalness);

    float lod = roughnessToMipLevel(roughness);

#ifdef CLEAR_COAT
    F0 = adjustF0ForClearCoatInterface(F0);
#endif

    // convert Z-up reflection to Y-up for OpenGL cubemap lookup
    vec3 yUpReflect = vec3(l.x, l.z, -l.y);
    vec3 indirectSpecular = textureLod(globalSpecularMap, yUpReflect, lod).rgb;

    // apply split-sum approximation for image based lighting specular
    vec2 environmentBrdf = environmentBrdfApproximation(roughness, vDotN);
    vec3 specular = indirectSpecular * (F0 * environmentBrdf.x + environmentBrdf.y);

    // multi-scatter energy compensation factor for indirect light
    // see https://google.github.io/filament/Filament.md.html#materialsystem/energyconservation, section 4.7.2
    float directionalAlbedo = environmentBrdf.x + environmentBrdf.y;
    vec3 energyCompensation = vec3(1.0) + F0 * (1.0 / max(directionalAlbedo, 0.001) - 1.0);
    specular *= energyCompensation;

    // Blend between diffuse and specular to conserve energy
    // see https://learnopengl.com/PBR/Theory, "Energy conservation"
    vec3 kS = fresnelSchlickRoughness(F0, vDotN, roughness);
    vec3 Fd = diffuse * (vec3(1.0) - kS);
    vec3 Fr = specular;

#ifdef CLEAR_COAT
    // as per https://google.github.io/filament/Filament.md.html#materialsystem/clearcoatmodel/clearcoatparameterization, Listing 14
    float ccPerceptualRoughness = clamp(clearCoatRoughness, 0.089, 1.0);
    float ccLod = roughnessToMipLevel(ccPerceptualRoughness);

    vec3 ccIndirectSpecular = textureLod(globalSpecularMap, yUpReflect, ccLod).rgb;
    // fixed IOR to 1.5 => reflectance = 0.04
    float Fc = fresnelSchlick(0.04, 1.0, vDotN) * clearCoatFactor;
    vec2 ccEnvBrdf = environmentBrdfApproximation(ccPerceptualRoughness, vDotN);
    vec3 Frc = ccIndirectSpecular * (0.04 * ccEnvBrdf.x + ccEnvBrdf.y) * clearCoatFactor;
    // Account for energy loss in the base layer
    vec3 layeredColor = (Fd + Fr * (1.0 - Fc)) * (1.0 - Fc) + Frc;
#else
    vec3 layeredColor = Fd + Fr;
#endif

    // Reduce by ambient occlusion amount
    vec3 color = layeredColor * ambientOcclusion;

    return color;
}
#endif

vec4 metalRoughFunction(const in vec4 baseColor,
                        const in float metalness,
                        const in float roughness,
                        const in float reflectance,
                        const in float ambientOcclusion,
                        const in float anisotropy,
                        const in vec3 worldPosition,
                        const in vec3 worldView,
                        const in vec3 worldNormal,
                        const in vec3 t,
                        const in vec3 b,
                        const in vec2 activeTexCoord)
{
    vec3 cLinear = vec3(0.0);

    // Remap roughness for a perceptually more linear correspondence
    float alpha = remapRoughness(roughness);
#ifdef ENABLE_IBL
    if ( envLightMode == 1 && envLightStrength > 0 )
    {
        cLinear += pbrIblModelSphericalHarmonics(worldNormal,
                               worldView,
                               t, b,
                               baseColor.rgb,
                               metalness,
                               roughness,
                               reflectance,
                               alpha,
                               ambientOcclusion,
                               anisotropy) * envLightStrength;
    }
#endif

    for (int i = 0; i < lightCount; ++i) {
        cLinear += pbrModel(i,
                            worldPosition,
                            worldNormal,
                            worldView,
                            t, b,
                            baseColor.rgb,
                            metalness,
                            roughness,
                            reflectance,
                            alpha,
                            ambientOcclusion,
                            anisotropy);
    }

#ifdef DATA_DEFINED
    cLinear += vs_in.emission * emissiveFactor;
#elif defined(EMISSION_MAP)
    vec3 emission = texture(emissionMap, activeTexCoord).rgb * emissiveFactor;
    cLinear += emission;
#else
    cLinear += emissiveColor * emissiveFactor;
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

#if defined(NORMAL_MAP) || defined(HEIGHT_MAP) || defined(ANISOTROPY)
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

#ifdef DATA_DEFINED
    vec4 c = vec4(vs_in.base, 1.0);
#elif defined(BASE_COLOR_MAP)
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

#ifdef ANISOTROPY
    vec3 anisotropyDirection = vec3(cos(anisotropyRotation), sin(anisotropyRotation), 0.0);
    vec3 t = normalize(tangentToWorld * anisotropyDirection);
    t = normalize(t - dot(t, n) * n);
    vec3 b = normalize(cross(n, t));
    float aniso = anisotropy;
#else
    vec3 t = vec3(0.0);
    vec3 b = vec3(0.0);
    float aniso = 0;
#endif

    fragColor = vec4(metalRoughFunction(c, m, r, reflectance, ao, aniso,
                                   worldPosition,
                                   worldView,
                                   n, t, b, activeTexCoord).rgb, opacity);
}
