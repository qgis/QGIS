// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/BlendState.h>
#include <Graphics/Buffer.h>
#include <Graphics/DepthStencilState.h>
#include <Graphics/Font.h>
#include <Graphics/OverlayEffect.h>
#include <Graphics/RasterizerState.h>
#include <Graphics/TextureSingle.h>

// Expose this preprocessor symbol to allow names to be assigned to graphics
// API-specific objects for debugging.
//
//#define GTE_GRAPHICS_USE_NAMED_OBJECTS

namespace gte
{
    class BaseEngine
    {
    public:
        // Abstract base class.
        virtual ~BaseEngine() = default;
    protected:
        BaseEngine();

        // Disallow copy and assignment.
        BaseEngine(BaseEngine const&) = delete;
        BaseEngine& operator=(BaseEngine const&) = delete;

    public:
        // Viewport management.  The measurements are in window coordinates.
        // The origin of the window is (x,y), the window width is w, and the
        // window height is h.  The depth range for the view volume is
        // [zmin,zmax].  The DirectX viewport is left-handed with origin the
        // upper-left corner of the window, the x-axis is directed rightward,
        // the y-axis is directed downward, and the depth range is a subset of
        // [0,1].  The OpenGL viewport is right-handed with origin the
        // lower-left corner of the window, the x-axis is directed rightward,
        // the y-axis is directed upward, and the depth range is a subset of
        // [-1,1].
        virtual void SetViewport(int x, int y, int w, int h) = 0;
        virtual void GetViewport(int& x, int& y, int& w, int& h) const = 0;
        virtual void SetDepthRange(float zmin, float zmax) = 0;
        virtual void GetDepthRange(float& zmin, float& zmax) const = 0;

        // The function returns 'true' when the depth range is [0,1] (DirectX)
        // or 'false' when the depth range is [-1,1] (OpenGL).
        virtual bool HasDepthRange01() const = 0;

        // Append the extension of the shader file to 'name' (.hlsl for DirectX,
        // .glsl for OpenGL).
        virtual std::string GetShaderName(std::string const& name) const = 0;

        // Window resizing.
        virtual bool Resize(unsigned int w, unsigned int h) = 0;

        // Support for clearing the color, depth, and stencil back buffers.
        inline void SetClearColor(std::array<float, 4> const& clearColor)
        {
            mClearColor = clearColor;
        }

        inline void SetClearDepth(float clearDepth)
        {
            mClearDepth = clearDepth;
        }

        inline void SetClearStencil(unsigned int clearStencil)
        {
            mClearStencil = clearStencil;
        }

        inline std::array<float, 4> const& GetClearColor() const
        {
            return mClearColor;
        }

        inline float GetClearDepth() const
        {
            return mClearDepth;
        }

        inline unsigned int GetClearStencil() const
        {
            return mClearStencil;
        }

        virtual void DisplayColorBuffer(unsigned int syncInterval) = 0;

        // Support for bitmapped fonts used in text rendering.  The default
        // font is Arial (height 18, no italics, no bold).
        virtual void SetFont(std::shared_ptr<Font> const& font);

        inline std::shared_ptr<Font> const& GetFont() const
        {
            return mActiveFont;
        }

        inline void SetDefaultFont()
        {
            SetFont(mDefaultFont);
        }

        inline std::shared_ptr<Font> const& GetDefaultFont() const
        {
            return mDefaultFont;
        }

        // Global drawing state.  The default states are listed in the headers
        // BlendState.h, DepthStencil.h and RasterizerState.h.
        virtual void SetBlendState(std::shared_ptr<BlendState> const& state) = 0;

        inline std::shared_ptr<BlendState> const& GetBlendState() const
        {
            return mActiveBlendState;
        }

        inline void SetDefaultBlendState()
        {
            SetBlendState(mDefaultBlendState);
        }

        inline std::shared_ptr<BlendState> const& GetDefaultBlendState() const
        {
            return mDefaultBlendState;
        }

        virtual void SetDepthStencilState(std::shared_ptr<DepthStencilState> const& state) = 0;

        inline std::shared_ptr<DepthStencilState> const& GetDepthStencilState() const
        {
            return mActiveDepthStencilState;
        }

        inline void SetDefaultDepthStencilState()
        {
            SetDepthStencilState(mDefaultDepthStencilState);
        }

        inline std::shared_ptr<DepthStencilState> const& GetDefaultDepthStencilState() const
        {
            return mDefaultDepthStencilState;
        }

        virtual void SetRasterizerState(std::shared_ptr<RasterizerState> const& state) = 0;

        inline std::shared_ptr<RasterizerState> const& GetRasterizerState() const
        {
            return mActiveRasterizerState;
        }

        inline void SetDefaultRasterizerState()
        {
            SetRasterizerState(mDefaultRasterizerState);
        }

        inline std::shared_ptr<RasterizerState> const& GetDefaultRasterizerState() const
        {
            return mDefaultRasterizerState;
        }

        // Support for copying from CPU to GPU via mapped memory.
        virtual bool Update(std::shared_ptr<Buffer> const& buffer) = 0;
        virtual bool Update(std::shared_ptr<TextureSingle> const& texture) = 0;

        // TODO: Include this here for now until DX12 engine has drawing
        // encapsulated.
        virtual uint64_t Draw(std::shared_ptr<OverlayEffect> const& effect) = 0;

    protected:
        // Helpers for construction and destruction.
        void CreateDefaultGlobalState();
        virtual void DestroyDefaultGlobalState();

        // The window size.
        unsigned int mXSize, mYSize;

        // Clear values.
        std::array<float, 4> mClearColor;
        float mClearDepth;
        unsigned int mClearStencil;

        // Fonts for text rendering.
        std::shared_ptr<Font> mDefaultFont;
        std::shared_ptr<Font> mActiveFont;

        // Global state.
        std::shared_ptr<BlendState> mDefaultBlendState;
        std::shared_ptr<BlendState> mActiveBlendState;
        std::shared_ptr<DepthStencilState> mDefaultDepthStencilState;
        std::shared_ptr<DepthStencilState> mActiveDepthStencilState;
        std::shared_ptr<RasterizerState> mDefaultRasterizerState;
        std::shared_ptr<RasterizerState> mActiveRasterizerState;
    };
}
