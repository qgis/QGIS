// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/Font.h>
#include <Mathematics/Vector2.h>
#include <cstring>
using namespace gte;

Font::Font(std::shared_ptr<ProgramFactory> const& factory,
    unsigned int width, unsigned int height,
    unsigned char const* texels, float const* characterData,
    unsigned int maxMessageLength)
    :
    mMaxMessageLength(maxMessageLength)
{
    // Create a vertex buffer to hold the maximum specified message.
    struct Vertex
    {
        Vector2<float> position, tcoord;
    };

    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
    unsigned int numVertices = 4 * mMaxMessageLength;
    mVertexBuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    mVertexBuffer->SetUsage(Resource::DYNAMIC_UPDATE);

    // Set the y values for top vertex positions and all texture
    // coordinates, since they do not change.
    // 0 -- 2   4 -- 6  ... <-- pos.y = 0, tex.y = 0
    // | \  |   | \  | 
    // |  \ |   |  \ | 
    // 1 -- 3   5 -- 7  ... <-- tex.y = 1
    auto vertices = mVertexBuffer->Get<Vertex>();
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        vertices[i].position = Vector2<float>::Zero();
        vertices[i].tcoord = Vector2<float>::Zero();
    }
    for (unsigned int i = 0; i < mMaxMessageLength; ++i)
    {
        Vertex& v0 = vertices[4 * i + 0];
        Vertex& v1 = vertices[4 * i + 1];
        Vertex& v2 = vertices[4 * i + 2];
        Vertex& v3 = vertices[4 * i + 3];

        v0.position[1] = 0.0f;
        v0.tcoord[1] = 0.0f;
        v1.tcoord[1] = 1.0f;
        v2.position[1] = 0.0f;
        v2.tcoord[1] = 0.0f;
        v3.tcoord[1] = 1.0f;
    }

    // Set the x coordinates on the first two vertices to zero, 
    // since they do not change.
    vertices[0].position[0] = 0.0f;
    vertices[1].position[0] = 0.0f;

    // Create and set the index buffer data.
    // 0 -- 2   4 -- 6  ...
    // | \  |   | \  | 
    // |  \ |   |  \ | 
    // 1 -- 3   5 -- 7  ...
    unsigned int numTriangles = 2 * mMaxMessageLength;
    mIndexBuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, sizeof(unsigned int));
    auto indices = mIndexBuffer->Get<unsigned int>();
    for (unsigned int i = 0; i < mMaxMessageLength; ++i)
    {
        // Bottom triangle
        indices[6 * i + 0] = 4 * i;
        indices[6 * i + 1] = 4 * i + 3;
        indices[6 * i + 2] = 4 * i + 1;

        // Top triangle
        indices[6 * i + 3] = 4 * i;
        indices[6 * i + 4] = 4 * i + 2;
        indices[6 * i + 5] = 4 * i + 3;
    }

    // Create a texture from the specified monochrome bitmap.
    mTexture = std::make_shared<Texture2>(DF_R8_UNORM, width, height);
    std::memcpy(mTexture->GetData(), texels, mTexture->GetNumBytes());
    std::memcpy(mCharacterData, characterData, 257 * sizeof(float));

    // Create an effect for drawing text.
    mTextEffect = std::make_shared<TextEffect>(factory, mTexture);
}

int Font::GetHeight() const
{
    return (mTexture ? mTexture->GetHeight() : 0);
}

int Font::GetWidth(std::string const& message) const
{
    // Get texture information.
    float const tw = static_cast<float>(mTexture->GetWidth());

    float width = 0.0f;
    unsigned int const length = std::min(
        static_cast<unsigned int>(message.length()), mMaxMessageLength);
    for (unsigned int i = 0; i < length; ++i)
    {
        // Get character data.
        int const c = static_cast<int>(message[i]);
        float const tx0 = mCharacterData[c];
        float const tx1 = mCharacterData[c + 1];
        float const charWidthM1 = (tx1 - tx0) * tw - 1.0f;  // in pixels

        width += charWidthM1;
    }

    return static_cast<int>(std::ceil(width));
}

void Font::Typeset(int viewportWidth, int viewportHeight, int x, int y,
    Vector4<float> const& color, std::string const& message) const
{
    // Get texel translation units, depends on viewport width and height.
    float const vdx = 1.0f / static_cast<float>(viewportWidth);
    float const vdy = 1.0f / static_cast<float>(viewportHeight);

    // Get texture information.
    float tw = static_cast<float>(mTexture->GetWidth());
    float th = static_cast<float>(mTexture->GetHeight());

    // Get vertex buffer information.
    unsigned int vertexSize = mVertexBuffer->GetFormat().GetVertexSize();
    char* data = mVertexBuffer->GetData();

    float x0 = 0.0f;
    unsigned int const length = std::min(
        static_cast<unsigned int>(message.length()), mMaxMessageLength);
    for (unsigned int i = 0; i < length; ++i)
    {
        // Get character data.
        int c = static_cast<int>(message[i]);
        float const tx0 = mCharacterData[c];
        float const tx1 = mCharacterData[c + 1];
        float charWidthM1 = (tx1 - tx0)*tw - 1.0f;  // in pixels

        // 0 -- 2   4 -- 6  ...
        // | \  |   | \  | 
        // |  \ |   |  \ | 
        // 1 -- 3   5 -- 7  ...
        float* v0 = reinterpret_cast<float*>(data + (4 * i + 0)*vertexSize);
        float* v1 = reinterpret_cast<float*>(data + (4 * i + 1)*vertexSize);
        float* v2 = reinterpret_cast<float*>(data + (4 * i + 2)*vertexSize);
        float* v3 = reinterpret_cast<float*>(data + (4 * i + 3)*vertexSize);

        // Set bottom left vertex y coordinate.
        v1[1] = vdy * th;

        // Set x-coordinates.
        float x1 = x0 + charWidthM1 *vdx;
        v0[0] = x0;
        v1[0] = x0;
        v2[0] = x1;
        v3[0] = x1;

        // Set bottom right-side y-coordinate.
        v3[1] = vdy * th;

        // Set the four texture x-coordinates.  The y-coordinates were set in
        // the constructor.
        v0[2] = tx0;
        v1[2] = tx0;
        v2[2] = tx1;
        v3[2] = tx1;

        // Update left x coordinate for next quad
        x0 = x1;
    }

    // Update the number of triangles that should be drawn.
    mVertexBuffer->SetNumActiveElements(4 * length);
    mIndexBuffer->SetNumActivePrimitives(2 * length);

    // Set effect parameters.
    float trnX = vdx * static_cast<float>(x);
    float trnY = 1.0f - vdy * static_cast<float>(y);
    mTextEffect->SetTranslate(trnX, trnY);
    mTextEffect->SetColor(color);
}
