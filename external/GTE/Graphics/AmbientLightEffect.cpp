// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/AmbientLightEffect.h>
using namespace gte;

AmbientLightEffect::AmbientLightEffect(std::shared_ptr<ProgramFactory> const& factory,
    BufferUpdater const& updater, std::shared_ptr<Material> const& material,
    std::shared_ptr<Lighting> const& lighting)
    :
    LightEffect(factory, updater, msVSSource, msPSSource, material, lighting, nullptr)
{
    mMaterialConstant = std::make_shared<ConstantBuffer>(sizeof(InternalMaterial), true);
    UpdateMaterialConstant();
    mProgram->GetVertexShader()->Set("Material", mMaterialConstant);

    mLightingConstant = std::make_shared<ConstantBuffer>(sizeof(InternalLighting), true);
    UpdateLightingConstant();
    mProgram->GetVertexShader()->Set("Lighting", mLightingConstant);
}

void AmbientLightEffect::UpdateMaterialConstant()
{
    InternalMaterial* internalMaterial = mMaterialConstant->Get<InternalMaterial>();
    internalMaterial->emissive = mMaterial->emissive;
    internalMaterial->ambient = mMaterial->ambient;
    LightEffect::UpdateMaterialConstant();
}

void AmbientLightEffect::UpdateLightingConstant()
{
    InternalLighting* internalLighting = mLightingConstant->Get<InternalLighting>();
    internalLighting->ambient = mLighting->ambient;
    internalLighting->attenuation = mLighting->attenuation;
    LightEffect::UpdateLightingConstant();
}


std::string const AmbientLightEffect::msGLSLVSSource =
R"(
    uniform PVWMatrix
    {
        mat4 pvwMatrix;
    };

    uniform Material
    {
        vec4 materialEmissive;
        vec4 materialAmbient;
    };

    uniform Light
    {
        vec4 lightingAmbient;
        vec4 lightingAttenuation;
    };

    layout(location = 0) in vec3 modelPosition;
    layout(location = 0) out vec4 vertexColor;

    void main()
    {
        vec3 ambient = lightingAttenuation.w * lightingAmbient.rgb;
        vertexColor.rgb = materialEmissive.rgb + materialAmbient.rgb * ambient;
        vertexColor.a = 1.0f;
    #if GTE_USE_MAT_VEC
        gl_Position = pvwMatrix * vec4(modelPosition, 1.0f);
    #else
        gl_Position = vec4(modelPosition, 1.0f) * pvwMatrix;
    #endif
    };
)";

std::string const AmbientLightEffect::msGLSLPSSource =
R"(
    layout(location = 0) in vec4 vertexColor;
    layout(location = 0) out vec4 pixelColor;

    void main()
    {
        pixelColor = vertexColor;
    };
)";

std::string const AmbientLightEffect::msHLSLVSSource =
R"(
    cbuffer PVWMatrix
    {
        float4x4 pvwMatrix;
    };

    cbuffer Material
    {
        float4 materialEmissive;
        float4 materialAmbient;
    };

    cbuffer Light
    {
        float4 lightingAmbient;
        float4 lightingAttenuation;
    };

    struct VS_INPUT
    {
        float3 modelPosition : POSITION;
    };

    struct VS_OUTPUT
    {
        float4 vertexColor : COLOR0;
        float4 clipPosition : SV_POSITION;
    };

    VS_OUTPUT VSMain(VS_INPUT input)
    {
        VS_OUTPUT output;

        float3 ambient = lightingAttenuation.w * lightingAmbient.rgb;
        output.vertexColor.rgb = materialEmissive.rgb + materialAmbient.rgb * ambient;
        output.vertexColor.a = 1.0f;
    #if GTE_USE_MAT_VEC
        output.clipPosition = mul(pvwMatrix, float4(input.modelPosition, 1.0f));
    #else
        output.clipPosition = mul(float4(input.modelPosition, 1.0f), pvwMatrix);
    #endif
        return output;
    }
)";

std::string const AmbientLightEffect::msHLSLPSSource =
R"(
    struct PS_INPUT
    {
        float4 vertexColor : COLOR0;
    };

    struct PS_OUTPUT
    {
        float4 pixelColor : SV_TARGET0;
    };

    PS_OUTPUT PSMain(PS_INPUT input)
    {
        PS_OUTPUT output;
        output.pixelColor = input.vertexColor;
        return output;
    }
)";

ProgramSources const AmbientLightEffect::msVSSource =
{
    &msGLSLVSSource,
    &msHLSLVSSource
};

ProgramSources const AmbientLightEffect::msPSSource =
{
    &msGLSLPSSource,
    &msHLSLPSSource
};
