// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/Camera.h>
#include <Mathematics/Logger.h>
using namespace gte;

Camera::Camera(bool isPerspective, bool isDepthRangeZeroOne)
    :
    ViewVolume(isPerspective, isDepthRangeZeroOne),
    mPreViewMatrix(Matrix4x4<float>::Identity()),
    mPostProjectionMatrix(Matrix4x4<float>::Identity()),
    mPreViewIsIdentity(true),
    mPostProjectionIsIdentity(true)
{
}

void Camera::SetParallaxProjectionMatrix(Vector4<float> const& p00, Vector4<float> const& p10,
    Vector4<float> const& p11, Vector4<float> const& p01, float nearExtrude, float farExtrude)
{
    LogAssert(nearExtrude > 0.0f && farExtrude > nearExtrude, "Invalid input.");

    // Compute the near face of the view volume.  The commented out line of
    // code shows what q110 should be, but the actual algorithm does not need
    // to compute it.
    Vector4<float> origin{ 0.0f, 0.0f, 0.0f, 1.0f };
    Vector4<float> q000 = origin + nearExtrude*(p00 - origin);
    Vector4<float> q100 = origin + nearExtrude*(p10 - origin);
    //Vector4<float> q110 = origin + nearExtrude*(p11 - origin);
    Vector4<float> q010 = origin + nearExtrude*(p01 - origin);

    // Compute the far face of the view volume.  The commented out lines of
    // code show what q101 and q011 should be, but the actual algorithm does
    // not need to compute them.
    Vector4<float> q001 = origin + farExtrude*(p00 - origin);
    //Vector4<float> q101 = origin + farExtrude*(p10 - origin);
    Vector4<float> q111 = origin + farExtrude*(p11 - origin);
    //Vector4<float> q011 = origin + farExtrude*(p01 - origin);

    // Compute the representation of q111.
    Vector4<float> u0 = q100 - q000;
    Vector4<float> u1 = q010 - q000;
    Vector4<float> u2 = q001 - q000;
#if defined(GTE_USE_MAT_VEC)
    Matrix4x4<float> M;
    M.SetCol(0, u0);
    M.SetCol(1, u1);
    M.SetCol(2, u2);
    M.SetCol(3, q000);
    Matrix4x4<float> invM = Inverse(M);
    Vector4<float> a = invM * q111;
#else
    Matrix4x4<float> M;
    M.SetRow(0, u0);
    M.SetRow(1, u1);
    M.SetRow(2, u2);
    M.SetRow(3, q000);
    Matrix4x4<float> invM = Inverse(M);
    Vector4<float> a = q111 * invM;
#endif

    // Compute the coefficients in the fractional linear transformation.
    //   y[i] = n[i]*x[i]/(d[0]*x[0] + d[1]*x[1] + d[2]*x[2] + d[3])
    float n0 = 2.0f * a[0];
    float n1 = 2.0f * a[1];
    float n2 = 2.0f * a[2];
    float d0 = +a[0] - a[1] - a[2] + 1.0f;
    float d1 = -a[0] + a[1] - a[2] + 1.0f;
    float d2 = -a[0] - a[1] + a[2] + 1.0f;
    float d3 = +a[0] + a[1] + a[2] - 1.0f;

    // Compute the perspective projection from the canonical cuboid to the
    // canonical cube [-1,1]^2 x [0,1].
    float n2divn0 = n2 / n0;
    float n2divn1 = n2 / n1;
    Matrix4x4<float> project;
#if defined(GTE_USE_MAT_VEC)
    project(0,0) = n2divn0 * (2.0f * d3 + d0);
    project(0,1) = n2divn1 * d1;
    project(0,2) = d2;
    project(0,3) = -n2;
    project(1,0) = n2divn0 * d0;
    project(1,1) = n2divn1 * (2.0f * d3 + d1);
    project(1,2) = d2;
    project(1,3) = -n2;
    project(2,0) = 0.0f;
    project(2,1) = 0.0f;
    project(2,2) = d3;
    project(2,3) = 0.0f;
    project(3,0) = -n2divn0 * d0;
    project(3,1) = -n2divn1 * d1;
    project(3,2) = -d2;
    project(3,3) = n2;

    // The full projection requires mapping the extruded-quadrilateral view
    // volume to the canonical cuboid, which is then followed by the
    // perspective projection to the canonical cube.
    SetProjectionMatrix(project * invM);
#else
    project(0,0) = n2divn0 * (2.0f * d3 + d0);
    project(1,0) = n2divn1 * d1;
    project(2,0) = d2;
    project(3,0) = -n2;
    project(0,1) = n2divn0 * d0;
    project(1,1) = n2divn1 * (2.0f * d3 + d1);
    project(2,1) = d2;
    project(3,1) = -n2;
    project(0,2) = 0.0f;
    project(1,2) = 0.0f;
    project(2,2) = d3;
    project(3,2) = 0.0f;
    project(0,3) = -n2divn0 * d0;
    project(1,3) = -n2divn1 * d1;
    project(2,3) = -d2;
    project(3,3) = n2;

    // The full projection requires mapping the extruded-quadrilateral view
    // volume to the canonical cuboid, which is then followed by the
    // perspective projection to the canonical cube.
    SetProjectionMatrix(invM*project);
#endif
}

void Camera::SetPreViewMatrix(Matrix4x4<float> const& preViewMatrix)
{
    mPreViewMatrix = preViewMatrix;
    mPreViewIsIdentity = (mPreViewMatrix == Matrix4x4<float>::Identity());
    UpdatePVMatrix();
}

void Camera::SetPostProjectionMatrix(Matrix4x4<float> const& postProjMatrix)
{
    mPostProjectionMatrix = postProjMatrix;
    mPostProjectionIsIdentity = (mPostProjectionMatrix == Matrix4x4<float>::Identity());
    UpdatePVMatrix();
}

bool Camera::GetPickLine(int viewX, int viewY, int viewW, int viewH, int x, int y,
    Vector4<float>& origin, Vector4<float>& direction) const
{
    if (viewX <= x && x <= viewX + viewW && viewY <= y && y <= viewY + viewH)
    {
        // Get the [0,1]^2-normalized coordinates of (x,y).
        float r = (static_cast<float>(x - viewX)) / static_cast<float>(viewW);
        float u = (static_cast<float>(y - viewY)) / static_cast<float>(viewH);

        // Get the relative coordinates in [rmin,rmax]x[umin,umax].
        float rBlend = (1.0f - r) * GetRMin() + r * GetRMax();
        float uBlend = (1.0f - u) * GetUMin() + u * GetUMax();

        if (IsPerspective())
        {
            origin = GetPosition();
            direction = GetDMin() * GetDVector() + rBlend * GetRVector() + uBlend * GetUVector();
            Normalize(direction);
        }
        else
        {
            origin = GetPosition() + rBlend * GetRVector() + uBlend * GetUVector();
            direction = GetDVector();
        }
        return true;
    }
    else
    {
        // (x,y) is outside the viewport.
        return false;
    }
}

void Camera::UpdatePVMatrix()
{
    ViewVolume::UpdatePVMatrix();

    Matrix4x4<float>& pvMatrix = mProjectionViewMatrix;

#if defined(GTE_USE_MAT_VEC)
    if (!mPostProjectionIsIdentity)
    {
        pvMatrix = mPostProjectionMatrix * pvMatrix;
    }
    if (!mPreViewIsIdentity)
    {
        pvMatrix = pvMatrix * mPreViewMatrix;
    }
#else
    if (!mPostProjectionIsIdentity)
    {
        pvMatrix = pvMatrix * mPostProjectionMatrix;
    }
    if (!mPreViewIsIdentity)
    {
        pvMatrix = mPreViewMatrix * pvMatrix;
    }
#endif
}
