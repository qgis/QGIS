// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Applications/GTApplicationsPCH.h>
#include <Applications/MSW/Console.h>
using namespace gte;

// The singleton used to create and destroy consoles for applications.
namespace gte
{
    ConsoleSystem TheConsoleSystem;
}

#if defined(GTE_USE_DIRECTX)
void ConsoleSystem::CreateEngineAndProgramFactory(Console::Parameters& parameters)
{
    auto engine = std::make_shared<DX11Engine>(nullptr, D3D_DRIVER_TYPE_HARDWARE,
        nullptr, parameters.deviceCreationFlags);

    if (engine->GetDevice())
    {
        parameters.engine = engine;
        parameters.factory = std::make_shared<HLSLProgramFactory>();
        parameters.created = true;
    }
    else
    {
        LogError("Cannot create compute engine.");
    }
}
#endif

#if defined(GTE_USE_OPENGL)
void ConsoleSystem::CreateEngineAndProgramFactory(Console::Parameters& parameters)
{
    bool saveDriverInfo = ((parameters.deviceCreationFlags & 0x00000001) != 0);
    auto engine = std::make_shared<WGLEngine>(false, saveDriverInfo);
    if (!engine->MeetsRequirements())
    {
        LogError("OpenGL 4.5 or later is required.");
    }

    if (engine->GetDevice())
    {
        parameters.engine = engine;
        parameters.factory = std::make_shared<GLSLProgramFactory>();
        parameters.created = true;
    }
    else
    {
        LogError("Cannot create compute engine.");
    }
}
#endif
