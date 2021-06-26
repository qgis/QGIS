// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/DepthStencilState.h>
using namespace gte;

DepthStencilState::DepthStencilState()
    :
    depthEnable(true),
    writeMask(MASK_ALL),
    comparison(LESS_EQUAL),
    stencilEnable(false),
    stencilReadMask(0xFF),
    stencilWriteMask(0xFF),
    reference(0)
{
    mType = GT_DEPTH_STENCIL_STATE;

    frontFace.fail = OP_KEEP;
    frontFace.depthFail = OP_KEEP;
    frontFace.pass = OP_KEEP;
    frontFace.comparison = ALWAYS;
    backFace.fail = OP_KEEP;
    backFace.depthFail = OP_KEEP;
    backFace.pass = OP_KEEP;
    backFace.comparison = ALWAYS;
}
