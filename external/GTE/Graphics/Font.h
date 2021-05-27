// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.03.25

#pragma once

#include <Graphics/TextEffect.h>
#include <Graphics/VertexBuffer.h>
#include <Graphics/IndexBuffer.h>

namespace gte
{
    class Font
    {
        // Abstract base class.
    protected:
        // Construction.
        Font(std::shared_ptr<ProgramFactory> const& factory,
            unsigned int width, unsigned int height,
            unsigned char const* texels, float const* characterData,
            unsigned int maxMessageLength);
    public:
        virtual ~Font() = default;

        // Member access.
        inline std::shared_ptr<TextEffect> const& GetTextEffect() const
        {
            return mTextEffect;
        }

        inline std::shared_ptr<VertexBuffer> const& GetVertexBuffer() const
        {
            return mVertexBuffer;
        }

        inline std::shared_ptr<IndexBuffer> const& GetIndexBuffer() const
        {
            return mIndexBuffer;
        }

        int GetHeight() const;
        int GetWidth(std::string const& message) const;

        // Populate the vertex buffer for the specified string.
        void Typeset(int viewportWidth, int viewportHeight, int x, int y,
            Vector4<float> const& color, std::string const& message) const;

    protected:
        unsigned int mMaxMessageLength;
        std::shared_ptr<VertexBuffer> mVertexBuffer;
        std::shared_ptr<IndexBuffer> mIndexBuffer;
        std::shared_ptr<Texture2> mTexture;
        std::shared_ptr<TextEffect> mTextEffect;
        float mCharacterData[257];
    };
}
