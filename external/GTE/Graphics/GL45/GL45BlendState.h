// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/BlendState.h>
#include <Graphics/GL45/GL45DrawingState.h>

namespace gte
{
    class GL45BlendState : public GL45DrawingState
    {
    public:
        // Construction.
        virtual ~GL45BlendState() = default;
        GL45BlendState(BlendState const* blendState);
        static std::shared_ptr<GEObject> Create(void* unused, GraphicsObject const* object);

        // Member access.
        inline BlendState* GetBlendState()
        {
            return static_cast<BlendState*>(mGTObject);
        }

        // Enable the blend state.
        void Enable();

    private:
        struct Target
        {
            GLboolean enable;
            GLenum srcColor;
            GLenum dstColor;
            GLenum opColor;
            GLenum srcAlpha;
            GLenum dstAlpha;
            GLenum opAlpha;
            GLboolean rMask;
            GLboolean gMask;
            GLboolean bMask;
            GLboolean aMask;
        };

        bool mEnableAlphaToCoverage;
        bool mEnableIndependentBlend;
        Target mTarget[BlendState::NUM_TARGETS];
        Vector4<float> mBlendColor;
        unsigned int mSampleMask;

        // Conversions from GTEngine values to GL4 values.
        static GLenum const msMode[];
        static GLenum const msOperation[];
    };
}
