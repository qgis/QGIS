// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/VertexBuffer.h>
#include <Graphics/GL45/GL45.h>

namespace gte
{
    class GL45InputLayout
    {
    public:
        // Construction and destruction.
        ~GL45InputLayout();
        GL45InputLayout(GLuint programHandle, GLuint vbufferHandle, VertexBuffer const* vbuffer);

        // Support for drawing geometric primitives.
        void Enable();
        void Disable();

    private:
        GLuint mProgramHandle;
        GLuint mVBufferHandle;
        GLuint mVArrayHandle;

        struct Attribute
        {
            VASemantic semantic;
            GLint numChannels;
            GLint channelType;
            GLboolean normalize;
            GLint location;
            GLintptr offset;
            GLsizei stride;
        };

        int mNumAttributes;
        Attribute mAttributes[VA_MAX_ATTRIBUTES];

        // Conversions from GTEngine values to GL45 values.
        static GLenum const msChannelType[];
    };
}
