// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/GL45/GLSLVisualProgram.h>
using namespace gte;

GLSLVisualProgram::~GLSLVisualProgram()
{
    if (glIsProgram(mProgramHandle))
    {
        if (glIsShader(mVertexShaderHandle))
        {
            glDetachShader(mProgramHandle, mVertexShaderHandle);
            glDeleteShader(mVertexShaderHandle);
        }

        if (glIsShader(mPixelShaderHandle))
        {
            glDetachShader(mProgramHandle, mPixelShaderHandle);
            glDeleteShader(mPixelShaderHandle);
        }

        if (glIsShader(mGeometryShaderHandle))
        {
            glDetachShader(mProgramHandle, mGeometryShaderHandle);
            glDeleteShader(mGeometryShaderHandle);
        }

        glDeleteProgram(mProgramHandle);
    }
}

GLSLVisualProgram::GLSLVisualProgram(GLuint programHandle, GLuint vertexShaderHandle,
    GLuint pixedlShaderHandle, GLuint geometryShaderHandle)
    :
    mProgramHandle(programHandle),
    mVertexShaderHandle(vertexShaderHandle),
    mPixelShaderHandle(pixedlShaderHandle),
    mGeometryShaderHandle(geometryShaderHandle),
    mReflector(programHandle)
{
}
