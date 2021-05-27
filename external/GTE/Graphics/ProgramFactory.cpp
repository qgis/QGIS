// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/ProgramFactory.h>
#include <Mathematics/Logger.h>
using namespace gte;

ProgramFactory::ProgramFactory()
    :
    version(""),
    vsEntry(""),
    psEntry(""),
    gsEntry(""),
    csEntry(""),
    defines(),
    flags(0)
{
}

std::shared_ptr<VisualProgram> ProgramFactory::CreateFromFiles(
    std::string const& vsFile, std::string const& psFile,
    std::string const& gsFile)
{
    LogAssert(vsFile != "" && psFile != "",
        "A program must have a vertex shader and a pixel shader.");

    std::string vsSource = GetStringFromFile(vsFile);
    LogAssert(vsSource != "", "Empty vertex shader source string.");

    std::string psSource = GetStringFromFile(psFile);
    LogAssert(psSource != "", "Empty pixel shader source string.");

    std::string gsSource = "";
    if (gsFile != "")
    {
        gsSource = GetStringFromFile(gsFile);
        LogAssert(gsSource != "", "Empty geometry shader source string.");
    }

    return CreateFromNamedSources(vsFile, vsSource, psFile, psSource, gsFile, gsSource);
}

std::shared_ptr<VisualProgram> ProgramFactory::CreateFromSources(
    std::string const& vsSource, std::string const& psSource,
    std::string const& gsSource)
{
    return CreateFromNamedSources("vs", vsSource, "ps", psSource, "gs", gsSource);
}

std::shared_ptr<ComputeProgram> ProgramFactory::CreateFromFile(
    std::string const& csFile)
{
    LogAssert(csFile != "", "A program must have a compute shader.");

    std::string csSource = GetStringFromFile(csFile);
    LogAssert(csSource != "", "Empty compute shader source string.");

    return CreateFromNamedSource(csFile, csSource);
}

std::shared_ptr<ComputeProgram> ProgramFactory::CreateFromSource(std::string const& csSource)
{
    return CreateFromNamedSource("cs", csSource);
}

std::string ProgramFactory::GetStringFromFile(std::string const& filename)
{
    std::ifstream input(filename);
    LogAssert(input, "Cannot open file " + filename);
    std::string source = "";
    while (!input.eof())
    {
        std::string line;
        getline(input, line);
        source += line + "\n";
    }
    input.close();
    return source;
}

void ProgramFactory::PushDefines()
{
    mDefinesStack.push(defines);
    defines.Clear();
}

void ProgramFactory::PopDefines()
{
    if (mDefinesStack.size() > 0)
    {
        defines = mDefinesStack.top();
        mDefinesStack.pop();
    }
}

void ProgramFactory::PushFlags()
{
    mFlagsStack.push(flags);
    flags = 0;
}

void ProgramFactory::PopFlags()
{
    if (mFlagsStack.size() > 0)
    {
        flags = mFlagsStack.top();
        mFlagsStack.pop();
    }
}
