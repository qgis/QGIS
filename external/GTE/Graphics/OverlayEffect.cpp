// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/OverlayEffect.h>
#include <Mathematics/Logger.h>
using namespace gte;

OverlayEffect::OverlayEffect(int windowWidth, int windowHeight)
    :
    mWindowWidth(static_cast<float>(windowWidth)),
    mWindowHeight(static_cast<float>(windowHeight))
{
}

OverlayEffect::OverlayEffect(std::shared_ptr<ProgramFactory> const& factory,
    int windowWidth, int windowHeight, int textureWidth, int textureHeight,
    SamplerState::Filter filter, SamplerState::Mode mode0, SamplerState::Mode mode1,
    bool useColorPShader)
    :
    mWindowWidth(static_cast<float>(windowWidth)),
    mWindowHeight(static_cast<float>(windowHeight))
{
    Initialize(windowWidth, windowHeight, textureWidth, textureHeight);

    mFactoryAPI = factory->GetAPI();
    std::string psSource =
        (useColorPShader ? *msPSColorSource[mFactoryAPI] : *msPSGraySource[mFactoryAPI]);

    mProgram = factory->CreateFromSources(*msVSSource[mFactoryAPI], psSource, "");
    if (mProgram)
    {
        SetNormalizedZ(0.0f);
        auto sampler = std::make_shared<SamplerState>();
        sampler->filter = filter;
        sampler->mode[0] = mode0;
        sampler->mode[1] = mode1;
        mProgram->GetPixelShader()->Set("imageSampler", sampler);
        mEffect = std::make_shared<VisualEffect>(mProgram);
    }
}

OverlayEffect::OverlayEffect(std::shared_ptr<ProgramFactory> const& factory,
    int windowWidth, int windowHeight, int textureWidth, int textureHeight,
    std::string const& psSource)
    :
    mWindowWidth(static_cast<float>(windowWidth)),
    mWindowHeight(static_cast<float>(windowHeight))
{
    mFactoryAPI = factory->GetAPI();
    Initialize(windowWidth, windowHeight, textureWidth, textureHeight);

    mProgram = factory->CreateFromSources(*msVSSource[mFactoryAPI], psSource, "");
    if (mProgram)
    {
        SetNormalizedZ(0.0f);
        mEffect = std::make_shared<VisualEffect>(mProgram);
    }
}

void OverlayEffect::SetOverlayRectangle(std::array<int, 4> const& rectangle)
{
    mOverlayRectangle = rectangle;
    UpdateVertexBuffer();
}

void OverlayEffect::SetTextureRectangle(std::array<int, 4> const& rectangle)
{
    mTextureRectangle = rectangle;
    UpdateVertexBuffer();
}

void OverlayEffect::SetRectangles(std::array<int, 4> const& overlayRectangle,
    std::array<int, 4> const& textureRectangle)
{
    mOverlayRectangle = overlayRectangle;
    mTextureRectangle = textureRectangle;
    UpdateVertexBuffer();
}

bool OverlayEffect::Contains(int x, int y) const
{
    return mOverlayRectangle[0] <= x
        && x < mOverlayRectangle[0] + mOverlayRectangle[2]
        && mOverlayRectangle[1] <= y
        && y < mOverlayRectangle[1] + mOverlayRectangle[3];
}

void OverlayEffect::SetTexture(std::shared_ptr<Texture2> const& texture)
{
    if (texture)
    {
        auto pshader = mEffect->GetPixelShader();
        auto sampler = pshader->Get<SamplerState>("imageSampler");
        pshader->Set("imageTexture", texture, "imageSampler", sampler);
    }
}

void OverlayEffect::SetTexture(std::string const& textureName,
    std::shared_ptr<Texture2> const& texture)
{
    if (texture)
    {
        auto pshader = mEffect->GetPixelShader();
        auto sampler = pshader->Get<SamplerState>("imageSampler");
        pshader->Set(textureName, texture, "imageSampler", sampler);
    }
}

void OverlayEffect::SetNormalizedZ(float z)
{
    std::shared_ptr<ConstantBuffer> params(new ConstantBuffer(sizeof(float), true));
    if (mFactoryAPI == ProgramFactory::PF_HLSL)
    {
        *params->Get<float>() = z;
    }
    else
    {
        *params->Get<float>() = 2.0f * z - 1.0f;
    }
    mProgram->GetVertexShader()->Set("ZNDC", params);
}

void OverlayEffect::Initialize(int windowWidth, int windowHeight,
    int textureWidth, int textureHeight)
{
    LogAssert(windowWidth > 0 && windowHeight > 0 && textureWidth > 0
        && textureHeight > 0, "Invalid input rectangle.");

    mInvTextureWidth = 1.0f / static_cast<float>(textureWidth);
    mInvTextureHeight = 1.0f / static_cast<float>(textureHeight);

    mOverlayRectangle[0] = 0;
    mOverlayRectangle[1] = 0;
    mOverlayRectangle[2] = windowWidth;
    mOverlayRectangle[3] = windowHeight;

    mTextureRectangle[0] = 0;
    mTextureRectangle[1] = 0;
    mTextureRectangle[2] = textureWidth;
    mTextureRectangle[3] = textureHeight;

    // Create the vertex buffer.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
    mVBuffer = std::make_shared<VertexBuffer>(vformat, 4);
    mVBuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    UpdateVertexBuffer();

    // Create the index buffer.
    mIBuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 2, sizeof(unsigned int));
    auto indices = mIBuffer->Get<unsigned int>();
    indices[0] = 0;  indices[1] = 2;  indices[2] = 3;
    indices[3] = 0;  indices[4] = 3;  indices[5] = 1;
}

void OverlayEffect::UpdateVertexBuffer()
{
    // Convert to normalized coordinates.
    float invWindowWidth = 1.0f / mWindowWidth;
    float invWindowHeight = 1.0f / mWindowHeight;
    float px = static_cast<float>(mOverlayRectangle[0]) * invWindowWidth;
    float py = static_cast<float>(mOverlayRectangle[1]) * invWindowHeight;
    float pw = static_cast<float>(mOverlayRectangle[2]) * invWindowWidth;
    float ph = static_cast<float>(mOverlayRectangle[3]) * invWindowHeight;

    float tx = static_cast<float>(mTextureRectangle[0]) * mInvTextureWidth;
    float ty = static_cast<float>(mTextureRectangle[1]) * mInvTextureHeight;
    float tw = static_cast<float>(mTextureRectangle[2]) * mInvTextureWidth;
    float th = static_cast<float>(mTextureRectangle[3]) * mInvTextureHeight;

    auto vertex = mVBuffer->Get<Vertex>();
    vertex[0].position = { px, py };
    vertex[0].tcoord = { tx, ty };
    vertex[1].position = { px + pw, py };
    vertex[1].tcoord = { tx + tw, ty };
    vertex[2].position = { px, py + ph };
    vertex[2].tcoord = { tx, ty + th };
    vertex[3].position = { px + pw, py + ph };
    vertex[3].tcoord = { tx + tw, ty + th };
}


std::string const OverlayEffect::msGLSLVSSource =
R"(
    uniform ZNDC
    {
        float zNDC;
    };

    layout(location = 0) in vec3 modelPosition;
    layout(location = 1) in vec2 modelTCoord;
    layout(location = 0) out vec2 vertexTCoord;

    void main()
    {
        vertexTCoord = modelTCoord;
        gl_Position.x = 2.0f*modelPosition.x - 1.0f;
        gl_Position.y = -2.0f*modelPosition.y + 1.0f;
        gl_Position.z = zNDC;
        gl_Position.w = 1.0f;
    }
)";

std::string const OverlayEffect::msGLSLPSColorSource =
R"(
    uniform sampler2D imageSampler;

    layout(location = 0) in vec2 vertexTCoord;
    layout(location = 0) out vec4 pixelColor;

    void main()
    {
        pixelColor = texture(imageSampler, vertexTCoord);
    }
)";

std::string const OverlayEffect::msGLSLPSGraySource =
R"(
    uniform sampler2D imageSampler;

    layout(location = 0) in vec2 vertexTCoord;
    layout(location = 0) out vec4 pixelColor;

    void main()
    {
        float gray = texture(imageSampler, vertexTCoord).r;
        pixelColor = vec4(gray, gray, gray, 1.0f);
    }
)";

std::string const OverlayEffect::msHLSLVSSource =
R"(
    cbuffer ZNDC
    {
        float zNDC;
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
        output.clipPosition.x = 2.0f*input.modelPosition.x - 1.0f;
        output.clipPosition.y = -2.0f*input.modelPosition.y + 1.0f;
        output.clipPosition.z = zNDC;
        output.clipPosition.w = 1.0f;
        output.vertexTCoord = input.modelTCoord;
        return output;
    }
)";

std::string const OverlayEffect::msHLSLPSColorSource =
R"(
    Texture2D<float4> imageTexture;
    SamplerState imageSampler;

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
        output.pixelColor = imageTexture.Sample(imageSampler, input.vertexTCoord);
        return output;
    }
)";

std::string const OverlayEffect::msHLSLPSGraySource =
R"(
    Texture2D<float> imageTexture;
    SamplerState imageSampler;

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
        float gray = imageTexture.Sample(imageSampler, input.vertexTCoord);
        output.pixelColor = float4(gray, gray, gray, 1.0f);
        return output;
    }
)";

ProgramSources const OverlayEffect::msVSSource =
{
    &msGLSLVSSource,
    &msHLSLVSSource
};

ProgramSources const OverlayEffect::msPSColorSource =
{
    &msGLSLPSColorSource,
    &msHLSLPSColorSource
};

ProgramSources const OverlayEffect::msPSGraySource =
{
    &msGLSLPSGraySource,
    &msHLSLPSGraySource
};
