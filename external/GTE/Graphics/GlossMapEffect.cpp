// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/GlossMapEffect.h>
using namespace gte;

GlossMapEffect::GlossMapEffect(
    std::shared_ptr<ProgramFactory> const& factory, BufferUpdater const& updater,
    std::shared_ptr<Material> const& material, std::shared_ptr<Lighting> const& lighting,
    std::shared_ptr<LightCameraGeometry> const& geometry,
    std::shared_ptr<Texture2> const& texture, SamplerState::Filter filter,
    SamplerState::Mode mode0, SamplerState::Mode mode1)
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

    auto vshader = mProgram->GetVertexShader();
    auto pshader = mProgram->GetPixelShader();
    vshader->Set("Material", mMaterialConstant);
    vshader->Set("Lighting", mLightingConstant);
    vshader->Set("LightCameraGeometry", mGeometryConstant);
    pshader->Set("baseTexture", mTexture, "baseSampler", mSampler);
}

void GlossMapEffect::UpdateMaterialConstant()
{
    auto* internalMaterial = mMaterialConstant->Get<InternalMaterial>();
    internalMaterial->emissive = mMaterial->emissive;
    internalMaterial->ambient = mMaterial->ambient;
    internalMaterial->diffuse = mMaterial->diffuse;
    internalMaterial->specular = mMaterial->specular;
    LightEffect::UpdateMaterialConstant();
}

void GlossMapEffect::UpdateLightingConstant()
{
    auto* internalLighting = mLightingConstant->Get<InternalLighting>();
    internalLighting->ambient = mLighting->ambient;
    internalLighting->diffuse = mLighting->diffuse;
    internalLighting->specular = mLighting->specular;
    internalLighting->attenuation = mLighting->attenuation;
    LightEffect::UpdateLightingConstant();
}

void GlossMapEffect::UpdateGeometryConstant()
{
    auto* internalGeometry = mGeometryConstant->Get<InternalGeometry>();
    internalGeometry->lightModelDirection = mGeometry->lightModelDirection;
    internalGeometry->cameraModelPosition = mGeometry->cameraModelPosition;
    LightEffect::UpdateGeometryConstant();
}

std::string const GlossMapEffect::msGLSLVSSource =
LightEffect::GetGLSLLitFunction() +
R"(
    uniform PVWMatrix
    {
        mat4 pvwMatrix;
    };

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
        vec4 lightModelDirection;
        vec4 cameraModelPosition;
    };

    layout(location = 0) in vec3 modelPosition;
    layout(location = 1) in vec3 modelNormal;
    layout(location = 2) in vec2 modelTCoord;
    layout(location = 0) out vec3 emsAmbDifColor;
    layout(location = 1) out vec3 spcColor;
    layout(location = 2) out vec2 vertexTCoord;

    void main()
    {
        float NDotL = -dot(modelNormal, lightModelDirection.xyz);
        vec3 viewVector = normalize(cameraModelPosition.xyz - modelPosition);
        vec3 halfVector = normalize(viewVector - lightModelDirection.xyz);
        float NDotH = dot(modelNormal, halfVector);
        vec4 lighting = lit(NDotL, NDotH, materialSpecular.a);

        emsAmbDifColor = materialEmissive.rgb +
            materialAmbient.rgb * lightingAmbient.rgb +
            lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb;

        spcColor = lighting.z * materialSpecular.rgb * lightingSpecular.rgb;

        vertexTCoord = modelTCoord;

    #if GTE_USE_MAT_VEC
        gl_Position = pvwMatrix * vec4(modelPosition, 1.0f);
    #else
        gl_Position = vec4(modelPosition, 1.0f) * pvwMatrix;
    #endif
    }
)";

std::string const GlossMapEffect::msGLSLPSSource =
R"(
    layout(location = 0) in vec3 emsAmbDifColor;
    layout(location = 1) in vec3 spcColor;
    layout(location = 2) in vec2 vertexTCoord;
    layout(location = 0) out vec4 pixelColor;

    uniform sampler2D baseSampler;

    void main()
    {
        vec4 baseColor = texture(baseSampler, vertexTCoord);
        pixelColor.rgb =
            baseColor.rgb * emsAmbDifColor + baseColor.a * spcColor;
        pixelColor.a = 1.0f;
    }
)";

std::string const GlossMapEffect::msHLSLVSSource =
R"(
    cbuffer PVWMatrix
    {
        float4x4 pvwMatrix;
    };

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
        float4 lightModelDirection;
        float4 cameraModelPosition;
    };

    struct VS_INPUT
    {
        float3 modelPosition : POSITION;
        float3 modelNormal : NORMAL;
        float2 modelTCoord : TEXCOORD0;
    };

    struct VS_OUTPUT
    {
        float3 emsAmbDifColor : COLOR;
        float3 spcColor : TEXCOORD0;
        float2 vertexTCoord : TEXCOORD1;
        float4 clipPosition : SV_POSITION;
    };

    VS_OUTPUT VSMain(VS_INPUT input)
    {
        VS_OUTPUT output;

        float NDotL = -dot(input.modelNormal, lightModelDirection.xyz);
        float3 viewVector = normalize(cameraModelPosition.xyz - input.modelPosition);
        float3 halfVector = normalize(viewVector - lightModelDirection.xyz);
        float NDotH = dot(input.modelNormal, halfVector);
        float4 lighting = lit(NDotL, NDotH, materialSpecular.a);

        output.emsAmbDifColor = materialEmissive.rgb +
            materialAmbient.rgb * lightingAmbient.rgb +
            lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb;

        output.spcColor = lighting.z * materialSpecular.rgb * lightingSpecular.rgb;

        output.vertexTCoord = input.modelTCoord;

    #if GTE_USE_MAT_VEC
        output.clipPosition = mul(pvwMatrix, float4(input.modelPosition, 1.0f));
    #else
        output.clipPosition = mul(float4(input.modelPosition, 1.0f), pvwMatrix);
    #endif
        return output;
    }
)";

std::string const GlossMapEffect::msHLSLPSSource =
R"(
    struct PS_INPUT
    {
        float3 emsAmbDifColor : COLOR;
        float3 spcColor : TEXCOORD0;
        float2 vertexTCoord : TEXCOORD1;
    };

    struct PS_OUTPUT
    {
        float4 pixelColor : SV_TARGET0;
    };

    Texture2D<float4> baseTexture;
    SamplerState baseSampler;

    PS_OUTPUT PSMain(PS_INPUT input)
    {
        PS_OUTPUT output;

        float4 baseColor = baseTexture.Sample(baseSampler, input.vertexTCoord);
        output.pixelColor.rgb =
            baseColor.rgb * input.emsAmbDifColor + baseColor.a * input.spcColor;
        output.pixelColor.a = 1.0f;

        return output;
    }
)";

ProgramSources const GlossMapEffect::msVSSource =
{
    &msGLSLVSSource,
    &msHLSLVSSource
};

ProgramSources const GlossMapEffect::msPSSource =
{
    &msGLSLPSSource,
    &msHLSLPSSource
};
