// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/VisualProgram.h>
#include <Graphics/GL45/GLSLReflection.h>

// TODO: Move GLSLReflection out of the class.  The reflection work should
// be done in GLSLProgramFactory, GTEngine-required data packaged in the
// factory, and this graphics-API-independent data passed to the Shader
// constructors.  HLSL factory creation should do the same so that Shader
// does not know about graphics API.  We also don't want VisualProgram-derived
// classes storing so much information that is not used.

namespace gte
{
    class GLSLVisualProgram : public VisualProgram
    {
    public:
        // Construction and destruction.
        virtual ~GLSLVisualProgram();
        GLSLVisualProgram(GLuint programHandle, GLuint vertexShaderHandle,
            GLuint pixedlShaderHandle, GLuint geometryShaderHandle);

        // Member access.  GLEngine needs the program handle for enabling and
        // disabling the program.  TODO: Do we need the Get*ShaderHandle
        // functions?
        inline GLuint GetProgramHandle() const
        {
            return mProgramHandle;
        }

        inline GLuint GetVertexShaderHandle() const
        {
            return mVertexShaderHandle;
        }

        inline GLuint GetPixelShaderHandle() const
        {
            return mPixelShaderHandle;
        }

        inline GLuint GetGShaderHandle() const
        {
            return mGeometryShaderHandle;
        }

        inline GLSLReflection const& GetReflector() const
        {
            return mReflector;
        }

    private:
        GLuint mProgramHandle;
        GLuint mVertexShaderHandle;
        GLuint mPixelShaderHandle;
        GLuint mGeometryShaderHandle;
        GLSLReflection mReflector;
    };
}
