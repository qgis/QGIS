// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Texture.h>
#include <Graphics/GL45/GL45Resource.h>

namespace gte
{
    class GL45Texture : public GL45Resource
    {
    public:
        // Abstract base class.
        virtual ~GL45Texture() = default;
    protected:
        GL45Texture(Texture const* texture, GLenum target, GLenum targetBinding);

    public:
        // Member access.
        inline Texture* GetTexture() const
        {
            return static_cast<Texture*>(mGTObject);
        }

        inline GLenum GetTarget() const
        {
            return mTarget;
        }

        inline GLenum GetTargetBinding() const
        {
            return mTargetBinding;
        }

        // Get the GL4 internal format for the specified Texture data format.
        inline static GLuint GetInternalFormat(DFType dataFormat)
        {
            return msGLTextureInternalFormat[dataFormat];
        }

    protected:
        GLenum mTarget;
        GLenum mTargetBinding;

        // Properties of overall texture.
        GLint mNumLevels;
        GLuint mInternalFormat;
        GLuint mExternalFormat;
        GLuint mExternalType;

        // Mapping from DFType to GL4 specific types
        static GLuint const msGLTextureInternalFormat[DF_NUM_FORMATS];
        static GLuint const msGLTextureExternalFormat[DF_NUM_FORMATS];
        static GLuint const msGLTextureExternalType[DF_NUM_CHANNEL_TYPES];
    };
}
