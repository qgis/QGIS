// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/DepthStencilState.h>
#include <Graphics/GL45/GL45DrawingState.h>

namespace gte
{
    class GL45DepthStencilState : public GL45DrawingState
    {
    public:
        // Construction.
        virtual ~GL45DepthStencilState() = default;
        GL45DepthStencilState(DepthStencilState const* depthStencilState);
        static std::shared_ptr<GEObject> Create(void* unused, GraphicsObject const* object);

        // Member access.
        inline DepthStencilState* GetDepthStencilState()
        {
            return static_cast<DepthStencilState*>(mGTObject);
        }

        // Enable the depth-stencil state.
        void Enable();

    private:
        struct Face
        {
            GLenum onFail;
            GLenum onZFail;
            GLenum onZPass;
            GLenum comparison;
        };

        GLboolean mDepthEnable;
        GLboolean mWriteMask;
        GLenum mComparison;
        GLboolean mStencilEnable;
        GLuint mStencilReadMask;
        GLuint mStencilWriteMask;
        Face mFrontFace;
        Face mBackFace;
        GLuint mReference;

        // Conversions from GTEngine values to GL4 values.
        static GLboolean const msWriteMask[];
        static GLenum const msComparison[];
        static GLenum const msOperation[];
    };
}
