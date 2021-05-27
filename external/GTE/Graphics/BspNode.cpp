// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.10

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/BspNode.h>
#include <Graphics/Camera.h>
using namespace gte;

BspNode::BspNode()
    :
    mModelPlane{ 0.0f, 0.0f, 0.0f, 0.0f },
    mWorldPlane{ 0.0f, 0.0f, 0.0f, 0.0f }
{
    mChild.push_back(nullptr);  // left child
    mChild.push_back(nullptr);  // middle child
    mChild.push_back(nullptr);  // right child
}

BspNode::BspNode(Plane3<float> const& modelPlane)
{
    SetModelPlane(modelPlane);
    SetWorldPlane(modelPlane);
    mChild.push_back(nullptr);  // left child
    mChild.push_back(nullptr);  // middle child
    mChild.push_back(nullptr);  // right child
}

void BspNode::SetModelPlane(Plane3<float> const& plane)
{
    mModelPlane[0] = plane.normal[0];
    mModelPlane[1] = plane.normal[1];
    mModelPlane[2] = plane.normal[2];
    mModelPlane[3] = -plane.constant;
    NormalizePlane(mModelPlane);
}

Plane3<float> BspNode::GetModelPlane() const
{
    Plane3<float> plane;
    plane.normal[0] = mModelPlane[0];
    plane.normal[1] = mModelPlane[1];
    plane.normal[2] = mModelPlane[2];
    plane.constant = -mModelPlane[3];
    return plane;
}

Plane3<float> BspNode::GetWorldPlane() const
{
    Plane3<float> plane;
    plane.normal[0] = mWorldPlane[0];
    plane.normal[1] = mWorldPlane[1];
    plane.normal[2] = mWorldPlane[2];
    plane.constant = -mWorldPlane[3];
    return plane;
}

Spatial* BspNode::GetContainingNode(Vector4<float> const& point)
{
    std::shared_ptr<Spatial> posChild = GetPositiveChild();
    std::shared_ptr<Spatial> negChild = GetNegativeChild();

    if (posChild || negChild)
    {
        BspNode* bspChild;

        if (WhichSide(point) < 0)
        {
            bspChild = dynamic_cast<BspNode*>(negChild.get());
            if (bspChild)
            {
                return bspChild->GetContainingNode(point);
            }
            else
            {
                return negChild.get();
            }
        }
        else
        {
            bspChild = dynamic_cast<BspNode*>(posChild.get());
            if (bspChild)
            {
                return bspChild->GetContainingNode(point);
            }
            else
            {
                return posChild.get();
            }
        }
    }

    return this;
}

void BspNode::NormalizePlane(Vector4<float>& plane)
{
    // Normalize to make (n0,n1,n2,-c), where (n0,n1,n2) is unit length.
    Vector3<float> normal{ plane[0], plane[1], plane[2] };
    float length = Normalize(normal);
    plane[0] = normal[0];
    plane[1] = normal[1];
    plane[2] = normal[2];
    plane[3] /= length;
}

void BspNode::SetWorldPlane(Plane3<float> const& plane)
{
    mWorldPlane[0] = plane.normal[0];
    mWorldPlane[1] = plane.normal[1];
    mWorldPlane[2] = plane.normal[2];
    mWorldPlane[3] = -plane.constant;
    NormalizePlane(mWorldPlane);
}

void BspNode::UpdateWorldData(double applicationTime)
{
    Node::UpdateWorldData(applicationTime);

#if defined(GTE_USE_MAT_VEC)
    mWorldPlane = mModelPlane * worldTransform.GetHInverse();
#else
    mWorldPlane = worldTransform.GetHInverse() * mModelPlane;
#endif
    NormalizePlane(mWorldPlane);
}

void BspNode::GetVisibleSet(Culler& culler,
    std::shared_ptr<Camera> const& camera, bool noCull)
{
    // Get visible geometry in back-to-front order.  If a global effect is
    // active, the geometry objects in the subtree will be drawn using it.
    std::shared_ptr<Spatial> posChild = GetPositiveChild();
    std::shared_ptr<Spatial> copChild = GetCoplanarChild();
    std::shared_ptr<Spatial> negChild = GetNegativeChild();

    int positionSide = WhichSide(camera->GetPosition());
    int frustumSide = WhichSide(camera);

    if (positionSide > 0)
    {
        // Camera origin on positive side of plane.

        if (frustumSide <= 0)
        {
            // The frustum is on the negative side of the plane or straddles
            // the plane.  In either case, the negative child is potentially
            // visible.
            if (negChild)
            {
                negChild->OnGetVisibleSet(culler, camera, noCull);
            }
        }

        if (frustumSide == 0)
        {
            // The frustum straddles the plane.  The coplanar child is
            // potentially visible.
            if (copChild)
            {
                copChild->OnGetVisibleSet(culler, camera, noCull);
            }
        }

        if (frustumSide >= 0)
        {
            // The frustum is on the positive side of the plane or straddles
            // the plane.  In either case, the positive child is potentially
            // visible.
            if (posChild)
            {
                posChild->OnGetVisibleSet(culler, camera, noCull);
            }
        }
    }
    else if (positionSide < 0)
    {
        // Camera origin on negative side of plane.

        if (frustumSide >= 0)
        {
            // The frustum is on the positive side of the plane or straddles
            // the plane.  In either case, the positive child is potentially
            // visible.
            if (posChild)
            {
                posChild->OnGetVisibleSet(culler, camera, noCull);
            }
        }

        if (frustumSide == 0)
        {
            // The frustum straddles the plane.  The coplanar child is
            // potentially visible.
            if (copChild)
            {
                copChild->OnGetVisibleSet(culler, camera, noCull);
            }
        }

        if (frustumSide <= 0)
        {
            // The frustum is on the negative side of the plane or straddles
            // the plane.  In either case, the negative child is potentially
            // visible.
            if (negChild)
            {
                negChild->OnGetVisibleSet(culler, camera, noCull);
            }
        }
    }
    else
    {
        // Camera origin on plane itself.  Both sides of the plane are
        // potentially visible as well as the plane itself.  Select the
        // first-to-be-drawn half space to be the one to which the camera
        // direction points.
        float NdD = Dot(mWorldPlane, camera->GetDVector());
        if (NdD >= 0.0f)
        {
            if (posChild)
            {
                posChild->OnGetVisibleSet(culler, camera, noCull);
            }

            if (copChild)
            {
                copChild->OnGetVisibleSet(culler, camera, noCull);
            }

            if (negChild)
            {
                negChild->OnGetVisibleSet(culler, camera, noCull);
            }
        }
        else
        {
            if (negChild)
            {
                negChild->OnGetVisibleSet(culler, camera, noCull);
            }

            if (copChild)
            {
                copChild->OnGetVisibleSet(culler, camera, noCull);
            }

            if (posChild)
            {
                posChild->OnGetVisibleSet(culler, camera, noCull);
            }
        }
    }
}

int BspNode::WhichSide(Vector4<float> const& point) const
{
    float dot = Dot(mWorldPlane, point);
    return (dot > 0.0f ? +1 : (dot < 0.0f ? -1 : 0));
}

int BspNode::WhichSide(std::shared_ptr<Camera> const& camera) const
{
    auto const& frustum = camera->GetFrustum();

    // The plane is N*(X-C) = 0 where the * indicates dot product.  The signed
    // distance from the camera location E to the plane is N*(E-C).
    float NdEmC = Dot(mWorldPlane, camera->GetPosition());

    Vector4<float> normal{ mWorldPlane[0], mWorldPlane[1], mWorldPlane[2], 0.0f };
    float NdD = Dot(normal, camera->GetDVector());
    float NdU = Dot(normal, camera->GetUVector());
    float NdR = Dot(normal, camera->GetRVector());
    float FdN = frustum[Camera::VF_DMAX] / frustum[Camera::VF_DMIN];

    int positive = 0, negative = 0;
    float sgnDist;

    // Check near-plane vertices.
    float PDMin = frustum[Camera::VF_DMIN] * NdD;
    float NUMin = frustum[Camera::VF_UMIN] * NdU;
    float NUMax = frustum[Camera::VF_UMAX] * NdU;
    float NRMin = frustum[Camera::VF_RMIN] * NdR;
    float NRMax = frustum[Camera::VF_RMAX] * NdR;

    // V = E + dmin*D + umin*U + rmin*R
    // N*(V-C) = N*(E-C) + dmin*(N*D) + umin*(N*U) + rmin*(N*R)
    sgnDist = NdEmC + PDMin + NUMin + NRMin;
    if (sgnDist > 0.0f)
    {
        ++positive;
    }
    else if (sgnDist < 0.0f)
    {
        ++negative;
    }

    // V = E + dmin*D + umin*U + rmax*R
    // N*(V-C) = N*(E-C) + dmin*(N*D) + umin*(N*U) + rmax*(N*R)
    sgnDist = NdEmC + PDMin + NUMin + NRMax;
    if (sgnDist > 0.0f)
    {
        ++positive;
    }
    else if (sgnDist < 0.0f)
    {
        ++negative;
    }

    // V = E + dmin*D + umax*U + rmin*R
    // N*(V-C) = N*(E-C) + dmin*(N*D) + umax*(N*U) + rmin*(N*R)
    sgnDist = NdEmC + PDMin + NUMax + NRMin;
    if (sgnDist > 0.0f)
    {
        ++positive;
    }
    else if (sgnDist < 0.0f)
    {
        ++negative;
    }

    // V = E + dmin*D + umax*U + rmax*R
    // N*(V-C) = N*(E-C) + dmin*(N*D) + umax*(N*U) + rmax*(N*R)
    sgnDist = NdEmC + PDMin + NUMax + NRMax;
    if (sgnDist > 0.0f)
    {
        ++positive;
    }
    else if (sgnDist < 0.0f)
    {
        ++negative;
    }

    // check far-plane vertices (s = dmax/dmin)
    float PDMax = frustum[Camera::VF_DMAX] * NdD;
    float FUMin = FdN * NUMin;
    float FUMax = FdN * NUMax;
    float FRMin = FdN * NRMin;
    float FRMax = FdN * NRMax;

    // V = E + dmax*D + umin*U + rmin*R
    // N*(V-C) = N*(E-C) + dmax*(N*D) + s*umin*(N*U) + s*rmin*(N*R)
    sgnDist = NdEmC + PDMax + FUMin + FRMin;
    if (sgnDist > 0.0f)
    {
        ++positive;
    }
    else if (sgnDist < 0.0f)
    {
        ++negative;
    }

    // V = E + dmax*D + umin*U + rmax*R
    // N*(V-C) = N*(E-C) + dmax*(N*D) + s*umin*(N*U) + s*rmax*(N*R)
    sgnDist = NdEmC + PDMax + FUMin + FRMax;
    if (sgnDist > 0.0f)
    {
        ++positive;
    }
    else if (sgnDist < 0.0f)
    {
        ++negative;
    }

    // V = E + dmax*D + umax*U + rmin*R
    // N*(V-C) = N*(E-C) + dmax*(N*D) + s*umax*(N*U) + s*rmin*(N*R)
    sgnDist = NdEmC + PDMax + FUMax + FRMin;
    if (sgnDist > 0.0f)
    {
        ++positive;
    }
    else if (sgnDist < 0.0f)
    {
        ++negative;
    }

    // V = E + dmax*D + umax*U + rmax*R
    // N*(V-C) = N*(E-C) + dmax*(N*D) + s*umax*(N*U) + s*rmax*(N*R)
    sgnDist = NdEmC + PDMax + FUMax + FRMax;
    if (sgnDist > 0.0f)
    {
        ++positive;
    }
    else if (sgnDist < 0.0f)
    {
        ++negative;
    }

    if (positive > 0)
    {
        if (negative > 0)
        {
            // Frustum straddles the plane.
            return 0;
        }

        // Frustum is fully on the positive side.
        return +1;
    }

    // Frustum is fully on the negative side.
    return -1;
}
