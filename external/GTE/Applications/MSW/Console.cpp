// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Applications/GTApplicationsPCH.h>
#include <Applications/MSW/Console.h>
using namespace gte;

Console::Parameters::Parameters()
    :
    deviceCreationFlags(0)
{
}

Console::Parameters::Parameters(std::wstring const& inTitle)
    :
    ConsoleApplication::Parameters(inTitle),
    deviceCreationFlags(0)
{
}

Console::~Console()
{
}

Console::Console(Parameters& parameters)
    :
    ConsoleApplication(parameters),
    mEngine(std::static_pointer_cast<GraphicsEngine>(mBaseEngine))
{
}
