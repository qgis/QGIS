// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/GL45/GLSLComputeProgram.h>
#include <Graphics/GL45/GLSLProgramFactory.h>
#include <Graphics/GL45/GLSLVisualProgram.h>
#include <Graphics/GL45/GLSLShader.h>
using namespace gte;

std::string GLSLProgramFactory::defaultVersion = "#version 430";
std::string GLSLProgramFactory::defaultVSEntry = "main";
std::string GLSLProgramFactory::defaultPSEntry = "main";
std::string GLSLProgramFactory::defaultGSEntry = "main";
std::string GLSLProgramFactory::defaultCSEntry = "main";
unsigned int GLSLProgramFactory::defaultFlags = 0;  // unused in GLSL for now

GLSLProgramFactory::GLSLProgramFactory()
{
    version = defaultVersion;
    vsEntry = defaultVSEntry;
    psEntry = defaultPSEntry;
    gsEntry = defaultGSEntry;
    csEntry = defaultCSEntry;
    flags = defaultFlags;
}

std::shared_ptr<VisualProgram> GLSLProgramFactory::CreateFromNamedSources(
    std::string const&, std::string const& vsSource,
    std::string const&, std::string const& psSource,
    std::string const&, std::string const& gsSource)
{
    if (vsSource == "" || psSource == "")
    {
        LogError("A program must have a vertex shader and a pixel shader.");
    }

    GLuint vsHandle = Compile(GL_VERTEX_SHADER, vsSource);
    if (vsHandle == 0)
    {
        return nullptr;
    }

    GLuint psHandle = Compile(GL_FRAGMENT_SHADER, psSource);
    if (psHandle == 0)
    {
        return nullptr;
    }

    GLuint gsHandle = 0;
    if (gsSource != "")
    {
        gsHandle = Compile(GL_GEOMETRY_SHADER, gsSource);
        if (gsHandle == 0)
        {
            return nullptr;
        }
    }

    GLuint programHandle = glCreateProgram();
    if (programHandle == 0)
    {
        LogError("Program creation failed.");
    }

    glAttachShader(programHandle, vsHandle);
    glAttachShader(programHandle, psHandle);
    if (gsHandle > 0)
    {
        glAttachShader(programHandle, gsHandle);
    }

    if (!Link(programHandle))
    {
        glDetachShader(programHandle, vsHandle);
        glDeleteShader(vsHandle);
        glDetachShader(programHandle, psHandle);
        glDeleteShader(psHandle);
        if (gsHandle)
        {
            glDetachShader(programHandle, gsHandle);
            glDeleteShader(gsHandle);
        }
        glDeleteProgram(programHandle);
        return nullptr;
    }

    std::shared_ptr<GLSLVisualProgram> program =
        std::make_shared<GLSLVisualProgram>(programHandle, vsHandle,
        psHandle, gsHandle);

    GLSLReflection const& reflector = program->GetReflector();
    auto vshader = std::make_shared<GLSLShader>(reflector, GT_VERTEX_SHADER, GLSLReflection::ST_VERTEX);
    auto pshader = std::make_shared<GLSLShader>(reflector, GT_PIXEL_SHADER, GLSLReflection::ST_PIXEL);
    program->SetVertexShader(vshader);
    program->SetPixelShader(pshader);
    if (gsHandle > 0)
    {
        auto gshader = std::make_shared<GLSLShader>(reflector, GT_GEOMETRY_SHADER, GLSLReflection::ST_GEOMETRY);
        program->SetGeometryShader(gshader);
    }
    return program;
}

std::shared_ptr<ComputeProgram> GLSLProgramFactory::CreateFromNamedSource(
    std::string const&, std::string const& csSource)
{
    if (csSource == "")
    {
        LogError("A program must have a compute shader.");
    }

    GLuint csHandle = Compile(GL_COMPUTE_SHADER, csSource);
    if (csHandle == 0)
    {
        return nullptr;
    }

    GLuint programHandle = glCreateProgram();
    if (programHandle == 0)
    {
        LogError("Program creation failed.");
    }

    glAttachShader(programHandle, csHandle);

    if (!Link(programHandle))
    {
        glDetachShader(programHandle, csHandle);
        glDeleteShader(csHandle);
        glDeleteProgram(programHandle);
        return nullptr;
    }

    auto program = std::make_shared<GLSLComputeProgram>(programHandle, csHandle);
    GLSLReflection const& reflector = program->GetReflector();
    auto cshader = std::make_shared<GLSLShader>(reflector, GT_COMPUTE_SHADER, GLSLReflection::ST_COMPUTE);
    program->SetComputeShader(cshader);
    return program;
}

GLuint GLSLProgramFactory::Compile(GLenum shaderType, std::string const& source)
{
    GLuint handle = glCreateShader(shaderType);
    if (handle > 0)
    {
        // Prepend to the definitions
        // 1. The version of the GLSL program; for example, "#version 400".
        // 2. A define for the matrix-vector multiplication convention if
        //    it is selected as GTE_USE_MAT_VEC: "define GTE_USE_MAT_VEC 1"
        //    else "define GTE_USE_MAT_VEC 0".
        // 3. "layout(std140, *_major) uniform;" for either row_major or
        //    column_major to select default for all uniform matrices and
        //    select std140 layout.
        // 4. "layout(std430, *_major) buffer;" for either row_major or
        //    column_major to select default for all buffer matrices and
        //    select std430 layout.
        // Append to the definitions the source-code string.
        auto const& definitions = defines.Get();
        std::vector<std::string> glslDefines;
        glslDefines.reserve(definitions.size() + 5);
        glslDefines.push_back(version + "\n");
#if defined(GTE_USE_MAT_VEC)
        glslDefines.push_back("#define GTE_USE_MAT_VEC 1\n");
#else
        glslDefines.push_back("#define GTE_USE_MAT_VEC 0\n");
#endif
#if defined(GTE_USE_ROW_MAJOR)
        glslDefines.push_back("layout(std140, row_major) uniform;\n");
        glslDefines.push_back("layout(std430, row_major) buffer;\n");
#else
        glslDefines.push_back("layout(std140, column_major) uniform;\n");
        glslDefines.push_back("layout(std430, column_major) buffer;\n");
#endif
        for (auto d : definitions)
        {
            glslDefines.push_back("#define " + d.first + " " + d.second + "\n");
        }
        glslDefines.push_back(source);

        // Repackage the definitions for glShaderSource.
        std::vector<GLchar const*> code;
        code.reserve(glslDefines.size());
        for (auto const& d : glslDefines)
        {
            code.push_back(d.c_str());
        }

        glShaderSource(handle, static_cast<GLsizei>(code.size()), &code[0], nullptr);

        glCompileShader(handle);
        GLint status;
        glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
        if (status == GL_TRUE)
        {
            return handle;
        }

        GLint logLength;
        glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0)
        {
            std::vector<GLchar> log(logLength);
            GLsizei numWritten;
            glGetShaderInfoLog(handle, static_cast<GLsizei>(logLength), &numWritten, log.data());
            std::string message(log.data());
            LogError("Compile failed:\n" + message);
        }
        else
        {
            LogError("Invalid info log length.");
        }
    }
    else
    {
        LogError("Cannot create shader.");
    }
}

bool GLSLProgramFactory::Link(GLuint programHandle)
{
    glLinkProgram(programHandle);
    int status;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &status);
    if (status == GL_TRUE)
    {
        return true;
    }

    int logLength;
    glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        std::vector<GLchar> log(logLength);
        int numWritten;
        glGetProgramInfoLog(programHandle, logLength, &numWritten, log.data());
        std::string message(log.data());
        LogError("Link failed:\n" + message);
    }
    else
    {
        LogError("Invalid info log length.");
    }
}
