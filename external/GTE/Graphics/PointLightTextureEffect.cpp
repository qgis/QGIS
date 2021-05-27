// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/PointLightTextureEffect.h>
using namespace gte;

PointLightTextureEffect::PointLightTextureEffect(std::shared_ptr<ProgramFactory> const& factory,
    BufferUpdater const& updater, std::shared_ptr<Material> const& material,
    std::shared_ptr<Lighting> const& lighting, std::shared_ptr<LightCameraGeometry> const& geometry,
    std::shared_ptr<Texture2> const& texture, SamplerState::Filter filter, SamplerState::Mode mode0,
    SamplerState::Mode mode1)
    :
    LightEffect(factory, updater, msVSSource, msPSSource, material, lighting, geometry),
    mTexture(texture)
{
    mSampler = std::make_shared<SamplerState>();
    mSampler->filter = filter;
    mSampler->mode[0] = mode0;
    mSampler->mode[1] = mode1;

    mMaterialConstant = std::make_shared<ConstantBuffer>(sizeof(InternalMaterial), true);
    UpdateMaterialConstant();

    mLightingConstant = std::make_shared<ConstantBuffer>(sizeof(InternalLighting), true);
    UpdateLightingConstant();

    mGeometryConstant = std::make_shared<ConstantBuffer>(sizeof(InternalGeometry), true);
    UpdateGeometryConstant();

    auto pshader = mProgram->GetPixelShader();
    pshader->Set("Material", mMaterialConstant);
    pshader->Set("Lighting", mLightingConstant);
    pshader->Set("LightCameraGeometry", mGeometryConstant);
    pshader->Set("baseTexture", mTexture, "baseSampler", mSampler);
}

void PointLightTextureEffect::UpdateMaterialConstant()
{
    InternalMaterial* internalMaterial = mMaterialConstant->Get<InternalMaterial>();
    internalMaterial->emissive = mMaterial->emissive;
    internalMaterial->ambient = mMaterial->ambient;
    internalMaterial->diffuse = mMaterial->diffuse;
    internalMaterial->specular = mMaterial->specular;
    LightEffect::UpdateMaterialConstant();
}

void PointLightTextureEffect::UpdateLightingConstant()
{
    InternalLighting* internalLighting = mLightingConstant->Get<InternalLighting>();
    internalLighting->ambient = mLighting->ambient;
    internalLighting->diffuse = mLighting->diffuse;
    internalLighting->specular = mLighting->specular;
    internalLighting->attenuation = mLighting->attenuation;
    LightEffect::UpdateLightingConstant();
}

void PointLightTextureEffect::UpdateGeometryConstant()
{
    InternalGeometry* internalGeometry = mGeometryConstant->Get<InternalGeometry>();
    internalGeometry->lightModelPosition = mGeometry->lightModelPosition;
    internalGeometry->cameraModelPosition = mGeometry->cameraModelPosition;
    LightEffect::UpdateGeometryConstant();
}


std::string const PointLightTextureEffect::msGLSLVSSource =
R"(
    uniform PVWMatrix
    {
        mat4 pvwMatrix;
    };

    layout(location = 0) in vec3 modelPosition;
    layout(location = 1) in vec3 modelNormal;
    layout(location = 2) in vec2 modelTCoord;
    layout(location = 0) out vec3 vertexPosition;
    layout(location = 1) out vec3 vertexNormal;
    layout(location = 2) out vec2 vertexTCoord;

    void main()
    {
        vertexPosition = modelPosition;
        vertexNormal = modelNormal;
        vertexTCoord = modelTCoord;
    #if GTE_USE_MAT_VEC
        gl_Position = pvwMatrix * vec4(modelPosition, 1.0f);
    #else
        gl_Position = vec4(modelPosition, 1.0f) * pvwMatrix;
    #endif
    }
)";

std::string const PointLightTextureEffect::msGLSLPSSource =
LightEffect::GetGLSLLitFunction() +
R"(
    uniform Material
    {
        vec4 materialEmissive;
        vec4 materialAmbient;
        vec4 materialDiffuse;
        vec4 materialSpecular;
    };

    uniform Lighting
    {
        vec4 lightingAmbient;
        vec4 lightingDiffuse;
        vec4 lightingSpecular;
        vec4 lightingAttenuation;
    };

    uniform LightCameraGeometry
    {
        vec4 lightModelPosition;
        vec4 cameraModelPosition;
    };

    uniform sampler2D baseSampler;

    layout(location = 0) in vec3 vertexPosition;
    layout(location = 1) in vec3 vertexNormal;
    layout(location = 2) in vec2 vertexTCoord;
    layout(location = 0) out vec4 pixelColor;

    void main()
    {
        vec3 modelLightDiff = vertexPosition - lightModelPosition.xyz;
        vec3 vertexDirection = normalize(modelLightDiff);
        float NDotL = -dot(vertexNormal, vertexDirection);
        vec3 viewVector = normalize(cameraModelPosition.xyz - vertexPosition.xyz);
        vec3 halfVector = normalize(viewVector - vertexDirection);
        float NDotH = dot(vertexNormal, halfVector);
        vec4 lighting = lit(NDotL, NDotH, materialSpecular.a);
        vec3 lightingColor = materialAmbient.rgb * lightingAmbient.rgb +
            lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +
            lighting.z * materialSpecular.rgb * lightingSpecular.rgb;

        float distance = length(modelLightDiff);
        float attenuation = lightingAttenuation.w / (lightingAttenuation.x + distance *
            (lightingAttenuation.y + distance * lightingAttenuation.z));

        vec4 textureColor = texture(baseSampler, vertexTCoord);

        vec3 color = lightingColor * textureColor.rgb;
        pixelColor.rgb = materialEmissive.rgb + attenuation * color;
        pixelColor.a = materialDiffuse.a * textureColor.a;
    }
)";

std::string const PointLightTextureEffect::msHLSLVSSource =
R"(
    cbuffer PVWMatrix
    {
        float4x4 pvwMatrix;
    };

    struct VS_INPUT
    {
        float3 modelPosition : POSITION;
        float3 modelNormal : NORMAL;
        float2 modelTCoord : TEXCOORD0;
    };

    struct VS_OUTPUT
    {
        float3 vertexPosition : TEXCOORD0;
        float3 vertexNormal : TEXCOORD1;
        float2 vertexTCoord : TEXCOORD2;
        float4 clipPosition : SV_POSITION;
    };

    VS_OUTPUT VSMain(VS_INPUT input)
    {
        VS_OUTPUT output;

        output.vertexPosition = input.modelPosition;
        output.vertexNormal = input.modelNormal;
        output.vertexTCoord = input.modelTCoord;
    #if GTE_USE_MAT_VEC
        output.clipPosition = mul(pvwMatrix, float4(input.modelPosition, 1.0f));
    #else
        output.clipPosition = mul(float4(input.modelPosition, 1.0f), pvwMatrix);
    #endif
        return output;
    }
)";

std::string const PointLightTextureEffect::msHLSLPSSource =
R"(
    cbuffer Material
    {
        float4 materialEmissive;
        float4 materialAmbient;
        float4 materialDiffuse;
        float4 materialSpecular;
    };

    cbuffer Lighting
    {
        float4 lightingAmbient;
        float4 lightingDiffuse;
        float4 lightingSpecular;
        float4 lightingAttenuation;
    };

    cbuffer LightCameraGeometry
    {
        float4 lightModelPosition;
        float4 cameraModelPosition;
    };

    Texture2D<float4> baseTexture;
    SamplerState baseSampler;

    struct PS_INPUT
    {
        float3 vertexPosition : TEXCOORD0;
        float3 vertexNormal : TEXCOORD1;
        float2 vertexTCoord : TEXCOORD2;
    };

    struct PS_OUTPUT
    {
        float4 pixelColor : SV_TARGET0;
    };

    PS_OUTPUT PSMain(PS_INPUT input)
    {
        PS_OUTPUT output;

        float3 modelLightDiff = input.vertexPosition - lightModelPosition.xyz;
        float3 vertexDirection = normalize(modelLightDiff);
        float NDotL = -dot(input.vertexNormal, vertexDirection);
        float3 viewVector = normalize(cameraModelPosition.xyz - input.vertexPosition.xyz);
        float3 halfVector = normalize(viewVector - vertexDirection);
        float NDotH = dot(input.vertexNormal, halfVector);
        float4 lighting = lit(NDotL, NDotH, materialSpecular.a);
        float3 lightingColor = materialAmbient.rgb * lightingAmbient.rgb +
            lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +
            lighting.z * materialSpecular.rgb * lightingSpecular.rgb;

        float distance = length(modelLightDiff);
        float attenuation = lightingAttenuation.w / (lightingAttenuation.x + distance *
            (lightingAttenuation.y + distance * lightingAttenuation.z));

        float4 textureColor = baseTexture.Sample(baseSampler, input.vertexTCoord);

        float3 color = lightingColor * textureColor.rgb;
        output.pixelColor.rgb = materialEmissive.rgb + attenuation * color;
        output.pixelColor.a = materialDiffuse.a * textureColor.a;
        return output;
    }
)";

ProgramSources const PointLightTextureEffect::msVSSource =
{
    &msGLSLVSSource,
    &msHLSLVSSource
};

ProgramSources const PointLightTextureEffect::msPSSource =
{
    &msGLSLPSSource,
    &msHLSLPSSource
};
