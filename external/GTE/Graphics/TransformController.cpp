// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/TransformController.h>
#include <Graphics/Spatial.h>
using namespace gte;

TransformController::TransformController(Transform<float> const& localTransform)
    :
    mLocalTransform(localTransform)
{
}

bool TransformController::Update(double applicationTime)
{
    if (!Controller::Update(applicationTime))
    {
        return false;
    }

    Spatial* spatial = reinterpret_cast<Spatial*>(mObject);
    spatial->localTransform = mLocalTransform;
    return true;
}
