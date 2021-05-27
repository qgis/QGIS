// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/Spatial.h>
using namespace gte;

Spatial::~Spatial()
{
    // The mParent member is not reference counted by Spatial, so do not
    // release it here.
}

Spatial::Spatial()
    :
    name(""),
    worldTransformIsCurrent(false),
    culling(CULL_DYNAMIC),
    worldBoundIsCurrent(false),
    mParent(nullptr)
{
}

void Spatial::Update(double applicationTime, bool initiator)
{
    UpdateWorldData(applicationTime);
    UpdateWorldBound();
    if (initiator)
    {
        PropagateBoundToRoot();
    }
}

void Spatial::OnGetVisibleSet(Culler& culler, std::shared_ptr<Camera> const& camera,
    bool noCull)
{
    if (culling == CULL_ALWAYS)
    {
        return;
    }

    if (culling == CULL_NEVER)
    {
        noCull = true;
    }

    unsigned int savePlaneState = culler.GetPlaneState();
    if (noCull || culler.IsVisible(worldBound))
    {
        GetVisibleSet(culler, camera, noCull);
    }
    culler.SetPlaneState(savePlaneState);
}

void Spatial::UpdateWorldData(double applicationTime)
{
    // Update any controllers associated with this object.
    UpdateControllers(applicationTime);

    // Update world transforms.
    if (!worldTransformIsCurrent)
    {
        if (mParent)
        {
#if defined(GTE_USE_MAT_VEC)
            worldTransform = mParent->worldTransform*localTransform;
#else
            worldTransform = localTransform*mParent->worldTransform;
#endif
        }
        else
        {
            worldTransform = localTransform;
        }
    }
}

void Spatial::PropagateBoundToRoot ()
{
    if (mParent)
    {
        mParent->UpdateWorldBound();
        mParent->PropagateBoundToRoot();
    }
}
