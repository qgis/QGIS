// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/VisualEffect.h>
#include <Graphics/IndexBuffer.h>
#include <Graphics/SamplerState.h>
#include <Graphics/Texture2.h>
#include <Graphics/VertexBuffer.h>
#include <Mathematics/Vector2.h>

namespace gte
{
    class OverlayEffect
    {
    public:
        // Allow derived classes for OverlayEffect.
        virtual ~OverlayEffect() = default;
    protected:
        OverlayEffect(int windowWidth, int windowHeight);

    public:
        // Create an overlay that allows you to use a simple color shader or a
        // simple gray-scale shader.
        OverlayEffect(std::shared_ptr<ProgramFactory> const& factory,
            int windowWidth, int windowHeight, int textureWidth, int textureHeight,
            SamplerState::Filter filter, SamplerState::Mode mode0, SamplerState::Mode mode1,
            bool useColorPShader);

        // Create an overlay that uses a pixel shader other than the color or
        // gray default shaders.  The caller is responsible for creating a
        // sampler for the pixel shader.
        OverlayEffect(std::shared_ptr<ProgramFactory> const& factory,
            int windowWidth, int windowHeight, int textureWidth, int textureHeight,
            std::string const& psSource);

        // Access to the overlay rectangle (px,py,pw,ph), which is in client
        // window coordinates.  The upper-left corner of the overlay rectangle
        // is (px,py).  The width and height of the rectangle are pw and ph,
        // respectively.  The width and height must be positive.  The corner
        // (px,py) may be chosen outside the client window, allowing you to
        // clip overlays against the window boundaries.
        void SetOverlayRectangle(std::array<int, 4> const& rectangle);

        inline std::array<int, 4> GetOverlayRectangle() const
        {
            return mOverlayRectangle;
        }

        // Access to the texture rectangle (tx,ty,tw,th), which is in texture
        // image coordinates.  The upper-left corner of the texture rectangle
        // is (tx,ty).  The width and height of the rectangle are tw and th,
        // respectively.  The width and height must be positive.  The corner
        // (tx,ty) may be chosen outside the texture image, but be aware that
        // the colors drawn outside will depend on the sampler state mode.
        void SetTextureRectangle(std::array<int, 4> const& rectangle);

        inline std::array<int, 4> GetTextureRectangle() const
        {
            return mTextureRectangle;
        }

        // Joint setting of rectangles (rare case), allowing for a single
        // vertex buffer update.
        void SetRectangles(std::array<int, 4> const& overlayRectangle,
            std::array<int, 4> const& textureRectangle);

        // Test whether the input (x,y) is contained by the overlay rectangle.
        // This is useful for hit testing in user interfaces.
        bool Contains(int x, int y) const;

        // For DX11Engine::Draw(VertexBuffer*,IndexBuffer*,VisualEffect*). The
        // vertex buffer is also needed for CPU-to-GPU copies when the overlay
        // and/or texture rectangles are modified.
        inline std::shared_ptr<VisualProgram> const& GetProgram() const
        {
            return mProgram;
        }

        inline std::shared_ptr<VertexBuffer> const& GetVertexBuffer() const
        {
            return mVBuffer;
        }

        inline std::shared_ptr<IndexBuffer> const& GetIndexBuffer() const
        {
            return mIBuffer;
        }

        inline std::shared_ptr<VisualEffect> const& GetEffect() const
        {
            return mEffect;
        }

        // Dynamically update the overlay textures.  The first function uses
        // the default name "imageTexture".  The second function allows you
        // to set textures when you create an overlay with your own pixel
        // shader.
        void SetTexture(std::shared_ptr<Texture2> const& texture);

        void SetTexture(std::string const& textureName, std::shared_ptr<Texture2> const& texture);

        // Set the value that will be used for the rectangle's normalized
        // device z-coordinate. The depth range for DirectX is [0,1] and the
        // depth range for OpenGL is [-1,1]. In this function, set z in [0,1].
        // It is used directly for DirectX. It is converted to [-1,1] using
        // 2*z-1 for OpenGL.
        void SetNormalizedZ(float z);

    protected:
        virtual void Initialize(int windowWidth, int windowHeight, int textureWidth, int textureHeight);

        struct Vertex
        {
            Vector2<float> position;
            Vector2<float> tcoord;
        };

        virtual void UpdateVertexBuffer();

        float mWindowWidth, mWindowHeight;
        float mInvTextureWidth, mInvTextureHeight;
        int mFactoryAPI;
        std::array<int, 4> mOverlayRectangle;
        std::array<int, 4> mTextureRectangle;
        std::shared_ptr<VertexBuffer> mVBuffer;
        std::shared_ptr<IndexBuffer> mIBuffer;
        std::shared_ptr<VisualProgram> mProgram;
        std::shared_ptr<VisualEffect> mEffect;

        // Shader source code as strings.
        static std::string const msGLSLVSSource;
        static std::string const msGLSLPSColorSource;
        static std::string const msGLSLPSGraySource;
        static std::string const msHLSLVSSource;
        static std::string const msHLSLPSColorSource;
        static std::string const msHLSLPSGraySource;
        static ProgramSources const msVSSource;
        static ProgramSources const msPSColorSource;
        static ProgramSources const msPSGraySource;
    };
}
