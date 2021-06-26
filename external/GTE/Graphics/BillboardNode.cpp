// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/BillboardNode.h>
#include <Mathematics/Rotation.h>
using namespace gte;

BillboardNode::BillboardNode(std::shared_ptr<Camera> const& camera)
    :
    mCamera(camera)
{
}

void BillboardNode::UpdateWorldData(double applicationTime)
{
    // Compute the billboard's world transforms based on its parent's world
    // transform and its local transforms.  Notice that you should not call
    // Node::UpdateWorldData since that function updates its children.  The
    // children of a BillboardNode cannot be updated until the billboard is
    // aligned with the camera.
    Spatial::UpdateWorldData(applicationTime);

    if (mCamera)
    {
        // Inverse-transform the camera to the model space of the billboard.
        Matrix4x4<float> const& inverse = worldTransform.GetHInverse();
#if defined(GTE_USE_MAT_VEC)
        Vector4<float> modelPos = inverse * mCamera->GetPosition();
#else
        Vector4<float> modelPos = mCamera->GetPosition() * inverse;
#endif

        // To align the billboard, the projection of the camera to the
        // xz-plane of the billboard's model space determines the angle of
        // rotation about the billboard's model y-axis.  If the projected
        // camera is on the model axis (x = 0 and z = 0), atan2 returns zero
        // (rather than NaN), so there is no need to trap this degenerate
        // case and handle it separately.
        float angle = std::atan2(modelPos[0], modelPos[2]);
        Matrix4x4<float> orient =
            Rotation<4, float>(AxisAngle<4, float>(Vector4<float>::Unit(1), angle));
#if defined(GTE_USE_MAT_VEC)
        worldTransform.SetRotation(worldTransform.GetRotation() * orient);
#else
        worldTransform.SetRotation(orient * worldTransform.GetRotation());
#endif
    }

    // Update the children now that the billboard orientation is known.
    for (auto& child : mChild)
    {
        if (child)
        {
            child->Update(applicationTime, false);
        }
    }
}
