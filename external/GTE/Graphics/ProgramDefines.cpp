// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/ProgramDefines.h>
using namespace gte;

void ProgramDefines::Update(std::string const& name, std::string const& value)
{
    // If an item already exists with the specified name, update it.
    for (auto& definition : mDefinitions)
    {
        if (name == definition.first)
        {
            definition.second = value;
            return;
        }
    }

    // The item is new, so append it.
    mDefinitions.push_back(std::make_pair(name, value));
}

void ProgramDefines::Remove(std::string const& name)
{
    for (auto iter = mDefinitions.begin(); iter != mDefinitions.end(); ++iter)
    {
        if (name == iter->first)
        {
            mDefinitions.erase(iter);
            break;
        }
    }
}

void ProgramDefines::Clear()
{
    mDefinitions.clear();
}
