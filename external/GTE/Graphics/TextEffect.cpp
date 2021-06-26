// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.10.11

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/TextEffect.h>
#include <Mathematics/Vector2.h>
using namespace gte;

TextEffect::TextEffect(std::shared_ptr<ProgramFactory> const& factory,
    std::shared_ptr<Texture2> const& texture)
{
    int api = factory->GetAPI();
    mProgram = factory->CreateFromSources(*msVSSource[api], *msPSSource[api], "");
    if (mProgram)
    {
        mTranslate = std::make_shared<ConstantBuffer>(sizeof(Vector3<float>), true);
        mColor = std::make_shared<ConstantBuffer>(sizeof(Vector4<float>), true);
        mSamplerState = std::make_shared<SamplerState>();

        SetTranslate(0.0f, 0.0f);
        SetNormalizedZ(msDefaultNormalizedZ[api]);
        mProgram->GetVertexShader()->Set("Translate", mTranslate);

        SetColor({ 0.0f, 0.0f, 0.0f, 0.0f });
        auto pshader = mProgram->GetPixelShader();
        pshader->Set("TextColor", mColor);
        pshader->Set("baseTexture", texture, "baseSampler", mSamplerState);
    }
}

void TextEffect::SetTranslate(float x, float  y)
{
    auto* data = mTranslate->Get<float>();
    data[0] = x;
    data[1] = y;
}

void TextEffect::SetNormalizedZ(float z)
{
    auto* data = mTranslate->Get<float>();
    data[2] = z;
}

void TextEffect::SetColor(Vector4<float> const& color)
{
    auto* data = mColor->Get<Vector4<float>>();
    *data = color;
}


std::string const TextEffect::msGLSLVSSource =
R"(
    uniform Translate
    {
        vec3 translate;
    };

    layout(location = 0) in vec2 modelPosition;
    layout(location = 1) in vec2 modelTCoord;
    layout(location = 0) out vec2 vertexTCoord;

    void main()
    {
        vertexTCoord = modelTCoord;
        gl_Position.x = 2.0f * modelPosition.x - 1.0f + 2.0f * translate.x;
        gl_Position.y = 2.0f * modelPosition.y - 1.0f + 2.0f * translate.y;
        gl_Position.z = translate.z;
        gl_Position.w = 1.0f;
    }
)";

std::string const TextEffect::msGLSLPSSource =
R"(
    uniform TextColor
    {
        vec4 textColor;
    };

    layout(location = 0) in vec2 vertexTCoord;
    layout(location = 0) out vec4 pixelColor;

    uniform sampler2D baseSampler;

    void main()
    {
        float bitmapAlpha = texture(baseSampler, vertexTCoord).r;
        if (bitmapAlpha > 0.5f)
        {
            discard;
        }
        pixelColor = textColor;
    }
)";

std::string const TextEffect::msHLSLVSSource =
R"(
    cbuffer Translate
    {
        float3 translate;
    };
    struct VS_INPUT
    {
        float2 modelPosition : POSITION;
        float2 modelTCoord : TEXCOORD0;
    };

    struct VS_OUTPUT
    {
        float2 vertexTCoord : TEXCOORD0;
        float4 clipPosition : SV_POSITION;
    };

    VS_OUTPUT VSMain (VS_INPUT input)
    {
        VS_OUTPUT output;
        output.vertexTCoord = input.modelTCoord;
        output.clipPosition.x = 2.0f * input.modelPosition.x - 1.0f + 2.0f * translate.x;
        output.clipPosition.y = 2.0f * input.modelPosition.y - 1.0f + 2.0f * translate.y;
        output.clipPosition.z = translate.z;
        output.clipPosition.w = 1.0f;
        return output;
    }
)";

std::string const TextEffect::msHLSLPSSource =
R"(
    cbuffer TextColor
    {
        float4 textColor;
    };

    Texture2D baseTexture;
    SamplerState baseSampler;

    struct PS_INPUT
    {
        float2 vertexTCoord : TEXCOORD0;
    };

    struct PS_OUTPUT
    {
        float4 pixelColor : SV_TARGET0;
    };

    PS_OUTPUT PSMain(PS_INPUT input)
    {
        PS_OUTPUT output;
        float bitmapAlpha = baseTexture.Sample(baseSampler, input.vertexTCoord).r;
        if (bitmapAlpha > 0.5f)
        {
            discard;
        }
        output.pixelColor = textColor;
        return output;
    }
)";

std::array<float, ProgramFactory::PF_NUM_API> const TextEffect::msDefaultNormalizedZ =
{
    -1.0f,
    0.0f
};

ProgramSources const TextEffect::msVSSource =
{
    &msGLSLVSSource,
    &msHLSLVSSource
};

ProgramSources const TextEffect::msPSSource =
{
    &msGLSLPSSource,
    &msHLSLPSSource
};
