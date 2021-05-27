// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/HLSLComputeProgram.h>
#include <Graphics/DX11/HLSLProgramFactory.h>
#include <Graphics/DX11/HLSLShaderFactory.h>
#include <Graphics/DX11/HLSLVisualProgram.h>
using namespace gte;

HLSLProgramFactory::HLSLProgramFactory()
{
    version = defaultVersion;
    vsEntry = defaultVSEntry;
    psEntry = defaultPSEntry;
    gsEntry = defaultGSEntry;
    csEntry = defaultCSEntry;
    flags = defaultFlags;
}

int HLSLProgramFactory::GetAPI() const
{
    return PF_HLSL;
}

std::shared_ptr<VisualProgram> HLSLProgramFactory::CreateFromBytecode(
    std::vector<unsigned char> const& vsBytecode,
    std::vector<unsigned char> const& psBytecode,
    std::vector<unsigned char> const& gsBytecode)
{
    LogAssert(vsBytecode.size() > 0 && psBytecode.size() > 0,
        "A program must have a vertex shader and a pixel shader.");

    std::shared_ptr<HLSLShader> vshader;
    std::shared_ptr<HLSLShader> pshader;
    std::shared_ptr<HLSLShader> gshader;

    HLSLReflection hlslVShader = HLSLShaderFactory::CreateFromBytecode("vs",
        vsEntry, std::string("vs_") + version, vsBytecode.size(),
        vsBytecode.data());
    if (hlslVShader.IsValid())
    {
        vshader = std::make_shared<HLSLShader>(hlslVShader, GT_VERTEX_SHADER);
    }
    else
    {
        return nullptr;
    }

    HLSLReflection hlslPShader = HLSLShaderFactory::CreateFromBytecode("ps",
        psEntry, std::string("ps_") + version, psBytecode.size(),
        psBytecode.data());
    if (hlslPShader.IsValid())
    {
        pshader = std::make_shared<HLSLShader>(hlslPShader, GT_PIXEL_SHADER);
    }
    else
    {
        return nullptr;
    }

    HLSLReflection hlslGShader;
    if (gsBytecode.size() > 0)
    {
        hlslGShader = HLSLShaderFactory::CreateFromBytecode("gs",
            gsEntry, std::string("gs_") + version, gsBytecode.size(),
            gsBytecode.data());
        if (hlslGShader.IsValid())
        {
            gshader = std::make_shared<HLSLShader>(hlslGShader, GT_GEOMETRY_SHADER);
        }
        else
        {
            return nullptr;
        }
    }

    auto program = std::make_shared<HLSLVisualProgram>();
    program->SetVertexShader(vshader);
    program->SetPixelShader(pshader);
    program->SetGeometryShader(gshader);
    return program;
}

std::shared_ptr<ComputeProgram> HLSLProgramFactory::CreateFromNamedSource(
    std::string const& csName, std::string const& csSource)
{
    LogAssert(csSource != "", "A program must have a compute shader.");

    HLSLReflection hlslCShader = HLSLShaderFactory::CreateFromString(csName,
        csSource, csEntry, std::string("cs_") + version, defines, flags);
    if (hlslCShader.IsValid())
    {
        auto cshader = std::make_shared<HLSLShader>(hlslCShader, GT_COMPUTE_SHADER);
        auto program = std::make_shared<HLSLComputeProgram>();
        program->SetComputeShader(cshader);
        return program;
    }
    else
    {
        return nullptr;
    }
}

std::shared_ptr<VisualProgram> HLSLProgramFactory::CreateFromNamedSources(
    std::string const& vsName, std::string const& vsSource,
    std::string const& psName, std::string const& psSource,
    std::string const& gsName, std::string const& gsSource)
{
    LogAssert(vsSource != "" && psSource != "",
        "A program must have a vertex shader and a pixel shader.");

    std::shared_ptr<HLSLShader> vshader;
    std::shared_ptr<HLSLShader> pshader;
    std::shared_ptr<HLSLShader> gshader;

    HLSLReflection hlslVShader = HLSLShaderFactory::CreateFromString(vsName,
        vsSource, vsEntry, std::string("vs_") + version, defines, flags);
    if (hlslVShader.IsValid())
    {
        vshader = std::make_shared<HLSLShader>(hlslVShader, GT_VERTEX_SHADER);
    }
    else
    {
        return nullptr;
    }

    HLSLReflection hlslPShader = HLSLShaderFactory::CreateFromString(psName,
        psSource, psEntry, std::string("ps_") + version, defines, flags);
    if (hlslPShader.IsValid())
    {
        pshader = std::make_shared<HLSLShader>(hlslPShader, GT_PIXEL_SHADER);
    }
    else
    {
        return nullptr;
    }

    HLSLReflection hlslGShader;
    if (gsSource != "")
    {
        hlslGShader = HLSLShaderFactory::CreateFromString(gsName,
            gsSource, gsEntry, std::string("gs_") + version, defines, flags);
        if (hlslGShader.IsValid())
        {
            gshader = std::make_shared<HLSLShader>(hlslGShader, GT_GEOMETRY_SHADER);
        }
        else
        {
            return nullptr;
        }
    }

    auto program = std::make_shared<HLSLVisualProgram>();
    program->SetVertexShader(vshader);
    program->SetPixelShader(pshader);
    program->SetGeometryShader(gshader);
    return program;
}

std::shared_ptr<ComputeProgram> HLSLProgramFactory::CreateFromBytecode(
    std::vector<unsigned char> const& csBytecode)
{
    LogAssert(csBytecode.size() > 0, "A program must have a compute shader.");

    HLSLReflection hlslCShader = HLSLShaderFactory::CreateFromBytecode("cs",
        csEntry, std::string("cs_") + version, csBytecode.size(),
        csBytecode.data());
    if (hlslCShader.IsValid())
    {
        auto cshader = std::make_shared<HLSLShader>(hlslCShader, GT_COMPUTE_SHADER);
        auto program = std::make_shared<HLSLComputeProgram>();
        program->SetComputeShader(cshader);
        return program;
    }
    else
    {
        return nullptr;
    }
}


std::string HLSLProgramFactory::defaultVersion = "5_0";
std::string HLSLProgramFactory::defaultVSEntry = "VSMain";
std::string HLSLProgramFactory::defaultPSEntry = "PSMain";
std::string HLSLProgramFactory::defaultGSEntry = "GSMain";
std::string HLSLProgramFactory::defaultCSEntry = "CSMain";
unsigned int HLSLProgramFactory::defaultFlags = (
    D3DCOMPILE_ENABLE_STRICTNESS |
    D3DCOMPILE_IEEE_STRICTNESS |
    D3DCOMPILE_OPTIMIZATION_LEVEL3);
