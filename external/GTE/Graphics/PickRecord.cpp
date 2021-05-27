// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.10

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/PickRecord.h>
using namespace gte;

PickRecord::PickRecord()
    :
    primitiveType(IP_NONE),
    primitiveIndex(0),
    t(0.0f),
    linePoint{ 0.0f, 0.0f, 0.0f, 1.0f },
    primitivePoint{ 0.0f, 0.0f, 0.0f, 1.0f },
    distanceToLinePoint(0.0f),
    distanceToPrimitivePoint(0.0f),
    distanceBetweenLinePrimitive(0.0f)
{
    for (int i = 0; i < 3; ++i)
    {
        vertexIndex[i] = 0;
        bary[i] = 0.0f;
    }
}
