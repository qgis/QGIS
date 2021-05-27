// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/Texture3Effect.h>
using namespace gte;

Texture3Effect::Texture3Effect(std::shared_ptr<ProgramFactory> const& factory,
    std::shared_ptr<Texture3> const& texture, SamplerState::Filter filter,
    SamplerState::Mode mode0, SamplerState::Mode mode1,
    SamplerState::Mode mode2)
    :
    mTexture(texture)
{
    int api = factory->GetAPI();
    mProgram = factory->CreateFromSources(*msVSSource[api], *msPSSource[api], "");
    if (mProgram)
    {
        mSampler = std::make_shared<SamplerState>();
        mSampler->filter = filter;
        mSampler->mode[0] = mode0;
        mSampler->mode[1] = mode1;
        mSampler->mode[2] = mode2;

        mProgram->GetVertexShader()->Set("PVWMatrix", mPVWMatrixConstant);
        mProgram->GetPixelShader()->Set("baseTexture", mTexture, "baseSampler", mSampler);
    }
}

void Texture3Effect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    VisualEffect::SetPVWMatrixConstant(buffer);
    mProgram->GetVertexShader()->Set("PVWMatrix", mPVWMatrixConstant);
}


std::string const Texture3Effect::msGLSLVSSource =
R"(
    uniform PVWMatrix
    {
        mat4 pvwMatrix;
    };

    layout(location = 0) in vec3 modelPosition;
    layout(location = 1) in vec3 modelTCoord;
    layout(location = 0) out vec3 vertexTCoord;

    void main()
    {
        vertexTCoord = modelTCoord;
    #if GTE_USE_MAT_VEC
        gl_Position = pvwMatrix * vec4(modelPosition, 1.0f);
    #else
        gl_Position = vec4(modelPosition, 1.0f) * pvwMatrix;
    #endif
    }
)";

std::string const Texture3Effect::msGLSLPSSource =
R"(
    uniform sampler3D baseSampler;

    layout(location = 0) in vec3 vertexTCoord;
    layout(location = 0) out vec4 pixelColor;

    void main()
    {
        pixelColor = texture(baseSampler, vertexTCoord);
    }
)";

std::string const Texture3Effect::msHLSLVSSource =
R"(
    cbuffer PVWMatrix
    {
        float4x4 pvwMatrix;
    };

    struct VS_INPUT
    {
        float3 modelPosition : POSITION;
        float3 modelTCoord : TEXCOORD0;
    };

    struct VS_OUTPUT
    {
        float3 vertexTCoord : TEXCOORD0;
        float4 clipPosition : SV_POSITION;
    };

    VS_OUTPUT VSMain (VS_INPUT input)
    {
        VS_OUTPUT output;
    #if GTE_USE_MAT_VEC
        output.clipPosition = mul(pvwMatrix, float4(input.modelPosition, 1.0f));
    #else
        output.clipPosition = mul(float4(input.modelPosition, 1.0f), pvwMatrix);
    #endif
        output.vertexTCoord = input.modelTCoord;
        return output;
    }
)";

std::string const Texture3Effect::msHLSLPSSource =
R"(
    Texture3D baseTexture;
    SamplerState baseSampler;

    struct PS_INPUT
    {
        float3 vertexTCoord : TEXCOORD0;
    };


    struct PS_OUTPUT
    {
        float4 pixelColor : SV_TARGET0;
    };

    PS_OUTPUT PSMain(PS_INPUT input)
    {
        PS_OUTPUT output;
        output.pixelColor = baseTexture.Sample(baseSampler, input.vertexTCoord);
        return output;
    }
)";

ProgramSources const Texture3Effect::msVSSource =
{
    &msGLSLVSSource,
    &msHLSLVSSource
};

ProgramSources const Texture3Effect::msPSSource =
{
    &msGLSLPSSource,
    &msHLSLPSSource
};
