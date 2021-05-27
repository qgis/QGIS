// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.10

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/SamplerState.h>
using namespace gte;

SamplerState::SamplerState()
    :
    filter(MIN_P_MAG_P_MIP_P),
    mipLODBias(0.0f),
    maxAnisotropy(1),
    comparison(NEVER),
    borderColor{ 1.0f, 1.0f, 1.0f, 1.0f },
    minLOD(-std::numeric_limits<float>::max()),
    maxLOD(std::numeric_limits<float>::max())
{
    mType = GT_SAMPLER_STATE;

    mode[0] = CLAMP;
    mode[1] = CLAMP;
    mode[2] = CLAMP;
}
