// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/ComputeProgram.h>
#include <Graphics/GL45/GLSLReflection.h>

namespace gte
{
    class GLSLComputeProgram : public ComputeProgram
    {
    public:
        // Construction and destruction.
        virtual ~GLSLComputeProgram();
        GLSLComputeProgram(GLuint programHandle, GLuint computeShaderHandle);

        // Member access.  GLEngine needs the program handle for enabling and
        // disabling the program.  TODO: Do we need the GetComputeShaderHandle
        // function?
        inline GLuint GetProgramHandle() const
        {
            return mProgramHandle;
        }

        inline GLuint GetComputeShaderHandle() const
        {
            return mComputeShaderHandle;
        }

        inline GLSLReflection const& GetReflector() const
        {
            return mReflector;
        }

    private:
        GLuint mProgramHandle;
        GLuint mComputeShaderHandle;
        GLSLReflection mReflector;
    };
}
