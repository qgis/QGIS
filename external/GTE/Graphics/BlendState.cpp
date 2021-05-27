// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.10

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/BlendState.h>
using namespace gte;

BlendState::BlendState()
    :
    enableAlphaToCoverage(false),
    enableIndependentBlend(false),
    blendColor{ 0.0f, 0.0f, 0.0f, 0.0f },
    sampleMask(0xFFFFFFFFu)
{
    mType = GT_BLEND_STATE;

    for (int i = 0; i < NUM_TARGETS; ++i)
    {
        Target& trg = target[i];
        trg.enable = false;
        trg.srcColor = BM_ONE;
        trg.dstColor = BM_ZERO;
        trg.opColor = OP_ADD;
        trg.srcAlpha = BM_ONE;
        trg.dstAlpha = BM_ZERO;
        trg.opAlpha = OP_ADD;
        trg.mask = CW_ENABLE_ALL;
    }
}
