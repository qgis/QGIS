// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Mathematics/Logger.h>
#include <Graphics/GL45/GL45InputLayout.h>
#include <cstring>
using namespace gte;

GL45InputLayout::~GL45InputLayout()
{
    glDeleteVertexArrays(1, &mVArrayHandle);
}

GL45InputLayout::GL45InputLayout(GLuint programHandle, GLuint vbufferHandle,
    VertexBuffer const* vbuffer)
    :
    mProgramHandle(programHandle),
    mVBufferHandle(vbufferHandle),
    mNumAttributes(0)
{
    glGenVertexArrays(1, &mVArrayHandle);
    glBindVertexArray(mVArrayHandle);

    std::memset(&mAttributes[0], 0, VA_MAX_ATTRIBUTES*sizeof(mAttributes[0]));
    if (vbuffer)
    {
        VertexFormat const& format = vbuffer->GetFormat();
        mNumAttributes = format.GetNumAttributes();
        for (int i = 0; i < mNumAttributes; ++i)
        {
            Attribute& attribute = mAttributes[i];

            DFType type;
            unsigned int unit, offset;
            format.GetAttribute(i, attribute.semantic, type, unit, offset);

            attribute.numChannels = static_cast<GLint>(
                DataFormat::GetNumChannels(type));
            attribute.channelType =
                msChannelType[DataFormat::GetChannelType(type)];
            attribute.normalize = static_cast<GLboolean>(
                DataFormat::ConvertChannel(type) ? 1 : 0);
            attribute.location = i;  // layouts must be zero-based sequential
            attribute.offset = static_cast<GLintptr>(offset);
            attribute.stride = static_cast<GLsizei>(format.GetVertexSize());

            glEnableVertexAttribArray(attribute.location);
            glBindVertexBuffer(i, mVBufferHandle, attribute.offset,
                attribute.stride);
            glVertexAttribFormat(attribute.location, attribute.numChannels,
                attribute.channelType, attribute.normalize, 0);
            glVertexAttribBinding(attribute.location, i);
        }
        glBindVertexArray(0);
    }
    else
    {
        LogError("Invalid inputs to GL45InputLayout constructor.");
    }
}

void GL45InputLayout::Enable()
{
    glBindVertexArray(mVArrayHandle);
}

void GL45InputLayout::Disable()
{
    glBindVertexArray(0);
}


GLenum const GL45InputLayout::msChannelType[] =
{
    GL_ZERO,                        // DF_UNSUPPORTED
    GL_BYTE,                        // DF_BYTE
    GL_UNSIGNED_BYTE,               // DF_UBYTE
    GL_SHORT,                       // DF_SHORT
    GL_UNSIGNED_SHORT,              // DF_USHORT
    GL_INT,                         // DF_INT
    GL_UNSIGNED_INT,                // DF_UINT
    GL_HALF_FLOAT,                  // DF_HALF_FLOAT
    GL_FLOAT,                       // DF_FLOAT
    GL_DOUBLE,                      // DF_DOUBLE
    GL_INT_2_10_10_10_REV,          // DF_INT_10_10_2
    GL_UNSIGNED_INT_2_10_10_10_REV, // DF_UINT_10_10_2
    GL_UNSIGNED_INT_10F_11F_11F_REV // DF_FLOAT_11_11_10
};
