// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/GEDrawTarget.h>
#include <Graphics/GL45/GL45TextureDS.h>
#include <Graphics/GL45/GL45TextureRT.h>

namespace gte
{
    class GL45DrawTarget : public GEDrawTarget
    {
    public:
        // Construction and destruction.
        virtual ~GL45DrawTarget();
        GL45DrawTarget(DrawTarget const* target,
            std::vector<GL45TextureRT*>& rtTextures, GL45TextureDS* dsTexture);
        static std::shared_ptr<GEDrawTarget> Create(DrawTarget const* target,
            std::vector<GEObject*>& rtTextures, GEObject* dsTexture);

        // Member access.
        inline GL45TextureRT* GetRTTexture(unsigned int i) const
        {
            return mRTTextures[i];
        }

        inline GL45TextureDS* GetDSTexture() const
        {
            return mDSTexture;
        }

        // Used in the Renderer::Draw function.
        void Enable();
        void Disable();

    private:
        std::vector<GL45TextureRT*> mRTTextures;
        GL45TextureDS* mDSTexture;

        GLuint mFrameBuffer;

        // Temporary storage during enable/disable of targets.
        GLint mSaveViewportX;
        GLint mSaveViewportY;
        GLsizei mSaveViewportWidth;
        GLsizei mSaveViewportHeight;
        GLdouble mSaveViewportNear;
        GLdouble mSaveViewportFar;
    };
}
