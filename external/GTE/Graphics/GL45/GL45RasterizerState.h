// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/RasterizerState.h>
#include <Graphics/GL45/GL45DrawingState.h>

namespace gte
{
    class GL45RasterizerState : public GL45DrawingState
    {
    public:
        // Construction.
        virtual ~GL45RasterizerState() = default;
        GL45RasterizerState(RasterizerState const* rasterizerState);
        static std::shared_ptr<GEObject> Create(void* unused, GraphicsObject const* object);

        // Member access.
        inline RasterizerState* GetRasterizerState()
        {
            return static_cast<RasterizerState*>(mGTObject);
        }

        // Enable the rasterizer state.
        void Enable();

    private:
        GLenum mFillMode;
        GLenum mCullFace;
        GLenum mFrontFace;
        float mDepthScale;
        float mDepthBias;
        GLboolean mEnableScissor;

        // TODO: D3D11_RASTERIZER_DESC has the following.  We need to determine
        // how to handle these in OpenGL.
        //   DepthBiasClamp
        //   DepthClipEnable
        //   MultisampleEnable
        //   AntialiasedLineEnable

        // Conversions from GTEngine values to GL45 values.
        static GLenum const msFillMode[];
        static GLenum const msCullFace[];
    };
}
