// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/ConstantColorEffect.h>
using namespace gte;

ConstantColorEffect::ConstantColorEffect(std::shared_ptr<ProgramFactory> const& factory,
    Vector4<float> const& color)
{
    int api = factory->GetAPI();
    mProgram = factory->CreateFromSources(*msVSSource[api], *msPSSource[api], "");
    if (mProgram)
    {
        mColorConstant = std::make_shared<ConstantBuffer>(sizeof(Vector4<float>), true);
        *mColorConstant->Get<Vector4<float>>() = color;

        mProgram->GetVertexShader()->Set("PVWMatrix", mPVWMatrixConstant);
        mProgram->GetVertexShader()->Set("ConstantColor", mColorConstant);
    }
}

void ConstantColorEffect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    VisualEffect::SetPVWMatrixConstant(buffer);
    mProgram->GetVertexShader()->Set("PVWMatrix", mPVWMatrixConstant);
}


std::string const ConstantColorEffect::msGLSLVSSource =
R"(
    uniform PVWMatrix
    {
        mat4 pvwMatrix;
    };

    uniform ConstantColor
    {
        vec4 constantColor;
    };

    layout(location = 0) in vec3 modelPosition;
    layout(location = 0) out vec4 vertexColor;

    void main()
    {
    #if GTE_USE_MAT_VEC
        gl_Position = pvwMatrix * vec4(modelPosition, 1.0f);
    #else
        gl_Position = vec4(modelPosition, 1.0f) * pvwMatrix;
    #endif
        vertexColor = constantColor;
    }
)";

std::string const ConstantColorEffect::msGLSLPSSource =
R"(
    layout(location = 0) in vec4 vertexColor;
    layout(location = 0) out vec4 pixelColor;

    void main()
    {
        pixelColor = vertexColor;
    }
)";

std::string const ConstantColorEffect::msHLSLVSSource =
R"(
    cbuffer PVWMatrix
    {
        float4x4 pvwMatrix;
    };

    cbuffer ConstantColor
    {
        float4 constantColor;
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
    #if GTE_USE_MAT_VEC
        output.clipPosition = mul(pvwMatrix, float4(input.modelPosition, 1.0f));
    #else
        output.clipPosition = mul(float4(input.modelPosition, 1.0f), pvwMatrix);
    #endif
        output.vertexColor = constantColor;
        return output;
    }
)";

std::string const ConstantColorEffect::msHLSLPSSource =
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

ProgramSources const ConstantColorEffect::msVSSource =
{
    &msGLSLVSSource,
    &msHLSLVSSource
};

ProgramSources const ConstantColorEffect::msPSSource =
{
    &msGLSLPSSource,
    &msHLSLPSSource
};
