// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/Camera.h>
#include <Graphics/Spatial.h>
#include <Mathematics/Logger.h>
using namespace gte;

Culler::~Culler()
{
}

Culler::Culler()
    :
    mPlaneQuantity(6)
{
    // The data members mFrustum, mPlane, and mPlaneState are
    // uninitialized.  They are initialized in the GetVisibleSet call.
    mVisibleSet.reserve(INITIALLY_VISIBLE);
}

bool Culler::PushPlane(CullingPlane<float> const& plane)
{
    if (mPlaneQuantity < MAX_PLANE_QUANTITY)
    {
        // The number of user-defined planes is limited.
        mPlane[mPlaneQuantity] = plane;
        ++mPlaneQuantity;
        return true;
    }
    return false;
}

bool Culler::PopPlane()
{
    if (mPlaneQuantity > Camera::VF_QUANTITY)
    {
        // Only non-view-frustum planes may be removed from the stack.
        --mPlaneQuantity;
        return true;
    }
    return false;
}

void Culler::ComputeVisibleSet(std::shared_ptr<Camera> const& camera,
    std::shared_ptr<Spatial> const& scene)
{
    LogAssert(scene != nullptr, "A scene is required for culling.");
    PushViewFrustumPlanes(camera);
    mVisibleSet.clear();
    scene->OnGetVisibleSet(*this, camera, false);
}

bool Culler::IsVisible(BoundingSphere<float> const& sphere)
{
    if (sphere.GetRadius() == 0.0f)
    {
        // The node is a dummy node and cannot be visible.
        return false;
    }

    // Start with the last pushed plane, which is potentially the most
    // restrictive plane.
    int index = mPlaneQuantity - 1;
    unsigned int mask = (1u << index);

    for (int i = 0; i < mPlaneQuantity; ++i, --index, mask >>= 1)
    {
        if (mPlaneState & mask)
        {
            int side = sphere.WhichSide(mPlane[index]);

            if (side < 0)
            {
                // The object is on the negative side of the plane, so
                // cull it.
                return false;
            }

            if (side > 0)
            {
                // The object is on the positive side of plane.  There is
                // no need to compare subobjects against this plane, so
                // mark it as inactive.
                mPlaneState &= ~mask;
            }
        }
    }

    return true;
}

void Culler::Insert(Visual* visible)
{
    mVisibleSet.push_back(visible);
}

void Culler::PushViewFrustumPlanes(std::shared_ptr<Camera> const& camera)
{
    // Get the frustum values.
    float dMax = camera->GetDMax();
    float dMin = camera->GetDMin(), dMin2 = dMin * dMin;
    float rMax = camera->GetRMax(), rMax2 = rMax * rMax;
    float rMin = camera->GetRMin(), rMin2 = rMin * rMin;
    float uMax = camera->GetUMax(), uMax2 = uMax * uMax;
    float uMin = camera->GetUMin(), uMin2 = uMin * uMin;

    // Get the camera coordinate frame.
    Vector4<float> P = camera->GetPosition();
    Vector4<float> D = camera->GetDVector();
    Vector4<float> U = camera->GetUVector();
    Vector4<float> R = camera->GetRVector();
    float dirDotEye = Dot(D, P);

    // Compute the frustum planes in world coordinates.
    Vector4<float> N;
    float invLength, a0, a1, c;

    // Compute the near plane, N = D.
    c = -(dirDotEye + dMin);
    mPlane[Camera::VF_DMIN].Set(D, c);

    // Compute the far plane, N = -D.
    c = dirDotEye + dMax;
    mPlane[Camera::VF_DMAX].Set(-D, c);

    // Compute the bottom plane
    invLength = 1.0f / std::sqrt(dMin2 + uMin2);
    a0 = -uMin*invLength;  // D component
    a1 = +dMin*invLength;  // U component
    N = a0*D + a1*U;
    c = -Dot(N, P);
    mPlane[Camera::VF_UMIN].Set(N, c);

    // Compute the top plane.
    invLength = 1.0f / std::sqrt(dMin2 + uMax2);
    a0 = +uMax*invLength;  // D component
    a1 = -dMin*invLength;  // U component
    N = a0*D + a1*U;
    c = -Dot(N, P);
    mPlane[Camera::VF_UMAX].Set(N, c);

    // Compute the left plane.
    invLength = 1.0f / std::sqrt(dMin2 + rMin2);
    a0 = -rMin*invLength;  // D component
    a1 = +dMin*invLength;  // R component
    N = a0*D + a1*R;
    c = -Dot(N, P);
    mPlane[Camera::VF_RMIN].Set(N, c);

    // Compute the right plane.
    invLength = 1.0f / std::sqrt(dMin2 + rMax2);
    a0 = +rMax*invLength;  // D component
    a1 = -dMin*invLength;  // R component
    N = a0*D + a1*R;
    c = -Dot(N, P);
    mPlane[Camera::VF_RMAX].Set(N, c);

    // All planes are active initially.
    mPlaneState = 0xFFFFFFFFu;
}
