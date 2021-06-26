// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/ViewVolume.h>
#include <Mathematics/Logger.h>
#include <Mathematics/Math.h>
using namespace gte;

ViewVolume::ViewVolume(bool isPerspective, bool isDepthRangeZeroOne)
    :
    mIsPerspective(isPerspective),
    mIsDepthRangeZeroOne(isDepthRangeZeroOne),
    mValidateCoordinateFrame(true)
{
    // NOTE:  SetFrame calls OnFrameChange and SetFrustum calls
    // OnFrustumChange, and both On*Change functions call UpdatePVMatrix().
    // OnFrameChange, OnFrustumChange, and UpdatePVMatrix are virtual, so the
    // calls in this constructor resolve to those of ViewVolume.  Derived
    // classes that need additional semantics in their constructors must apply
    // those explicitly.
    SetFrame({ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f });

    if (mIsPerspective)
    {
        SetFrustum(90.0f, 1.0f, 1.0f, 10000.0f);
    }
    else
    {
        SetFrustum(0.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    }
}

void ViewVolume::SetPosition(Vector4<float> const& position)
{
    mPosition = position;
    OnFrameChange();
}

void ViewVolume::SetAxes(Vector4<float> const& dVector,
    Vector4<float> const& uVector, Vector4<float> const& rVector)
{
    mDVector = dVector;
    mUVector = uVector;
    mRVector = rVector;

    float const epsilon = 0.01f;
    float det = DotCross(mDVector, mUVector, mRVector);
    if (std::fabs(1.0f - det) > epsilon)
    {
#if defined(GTE_VALIDATE_COORDINATE_FRAME_ONCE)
        if (mValidateCoordinateFrame)
        {
            mValidateCoordinateFrame = false;

            float lenD = Length(mDVector);
            float lenU = Length(mUVector);
            float lenR = Length(mRVector);
            float dotDU = Dot(mDVector, mUVector);
            float dotDR = Dot(mDVector, mRVector);
            float dotUR = Dot(mUVector, mRVector);
            if (std::fabs(1.0f - lenD) > epsilon
                || std::fabs(1.0f - lenU) > epsilon
                || std::fabs(1.0f - lenR) > epsilon
                || std::fabs(dotDU) > epsilon
                || std::fabs(dotDR) > epsilon
                || std::fabs(dotUR) > epsilon)
            {
                LogError("Coordinate frame is not orthonormal.");
            }
        }
#endif
        // The input vectors do not appear to form an orthonormal set.  Time
        // to renormalize.
        Vector4<float> v[3] = { mDVector, mUVector, mRVector };
        Orthonormalize<4, float>(3, v);
        mDVector = v[0];
        mUVector = v[1];
        mRVector = v[2];
    }

    OnFrameChange();
}

void ViewVolume::SetFrame(Vector4<float> const& position, Vector4<float> const& dVector,
    Vector4<float> const& uVector, Vector4<float> const& rVector)
{
    mPosition = position;
    SetAxes(dVector, uVector, rVector);
}

void ViewVolume::GetAxes(Vector4<float>& dVector, Vector4<float>& uVector,
    Vector4<float>& rVector) const
{
    dVector = mDVector;
    uVector = mUVector;
    rVector = mRVector;
}

void ViewVolume::GetFrame(Vector4<float>& position, Vector4<float>& dVector,
    Vector4<float>& uVector, Vector4<float>& rVector) const
{
    position = mPosition;
    dVector = mDVector;
    uVector = mUVector;
    rVector = mRVector;
}

void ViewVolume::SetFrustum(float dMin, float dMax, float uMin, float uMax, float rMin, float rMax)
{
    mFrustum[VF_DMIN] = dMin;
    mFrustum[VF_DMAX] = dMax;
    mFrustum[VF_UMIN] = uMin;
    mFrustum[VF_UMAX] = uMax;
    mFrustum[VF_RMIN] = rMin;
    mFrustum[VF_RMAX] = rMax;
    OnFrustumChange();
}

void ViewVolume::SetFrustum(std::array<float, VF_QUANTITY> const& frustum)
{
    mFrustum = frustum;
    OnFrustumChange();
}

void ViewVolume::SetFrustum(float upFovDegrees, float aspectRatio, float dMin, float dMax)
{
    float halfAngleRadians = 0.5f * upFovDegrees * (float)GTE_C_DEG_TO_RAD;
    mFrustum[VF_UMAX] = dMin * std::tan(halfAngleRadians);
    mFrustum[VF_RMAX] = aspectRatio*mFrustum[VF_UMAX];
    mFrustum[VF_UMIN] = -mFrustum[VF_UMAX];
    mFrustum[VF_RMIN] = -mFrustum[VF_RMAX];
    mFrustum[VF_DMIN] = dMin;
    mFrustum[VF_DMAX] = dMax;
    OnFrustumChange();
}

void ViewVolume::GetFrustum(float& dMin, float& dMax, float& uMin, float& uMax, float& rMin, float& rMax) const
{
    dMin = mFrustum[VF_DMIN];
    dMax = mFrustum[VF_DMAX];
    uMin = mFrustum[VF_UMIN];
    uMax = mFrustum[VF_UMAX];
    rMin = mFrustum[VF_RMIN];
    rMax = mFrustum[VF_RMAX];
}

bool ViewVolume::GetFrustum(float& upFovDegrees, float& aspectRatio, float& dMin, float& dMax) const
{
    if (mFrustum[VF_RMIN] == -mFrustum[VF_RMAX]
        && mFrustum[VF_UMIN] == -mFrustum[VF_UMAX])
    {
        float tmp = mFrustum[VF_UMAX] / mFrustum[VF_DMIN];
        upFovDegrees = 2.0f * std::atan(tmp) * static_cast<float>(GTE_C_RAD_TO_DEG);
        aspectRatio = mFrustum[VF_RMAX] / mFrustum[VF_UMAX];
        dMin = mFrustum[VF_DMIN];
        dMax = mFrustum[VF_DMAX];
        return true;
    }
    return false;
}

void ViewVolume::OnFrameChange()
{
    // This leads to left-handed coordinates for the camera frame.
#if defined(GTE_USE_MAT_VEC)
    mViewMatrix(0, 0) = mRVector[0];
    mViewMatrix(0, 1) = mRVector[1];
    mViewMatrix(0, 2) = mRVector[2];
    mViewMatrix(0, 3) = -Dot(mPosition, mRVector);
    mViewMatrix(1, 0) = mUVector[0];
    mViewMatrix(1, 1) = mUVector[1];
    mViewMatrix(1, 2) = mUVector[2];
    mViewMatrix(1, 3) = -Dot(mPosition, mUVector);
    mViewMatrix(2, 0) = mDVector[0];
    mViewMatrix(2, 1) = mDVector[1];
    mViewMatrix(2, 2) = mDVector[2];
    mViewMatrix(2, 3) = -Dot(mPosition, mDVector);
    mViewMatrix(3, 0) = 0.0f;
    mViewMatrix(3, 1) = 0.0f;
    mViewMatrix(3, 2) = 0.0f;
    mViewMatrix(3, 3) = 1.0f;

    mInverseViewMatrix(0, 0) = mRVector[0];
    mInverseViewMatrix(0, 1) = mUVector[0];
    mInverseViewMatrix(0, 2) = mDVector[0];
    mInverseViewMatrix(0, 3) = mPosition[0];
    mInverseViewMatrix(1, 0) = mRVector[1];
    mInverseViewMatrix(1, 1) = mUVector[1];
    mInverseViewMatrix(1, 2) = mDVector[1];
    mInverseViewMatrix(1, 3) = mPosition[1];
    mInverseViewMatrix(2, 0) = mRVector[2];
    mInverseViewMatrix(2, 1) = mUVector[2];
    mInverseViewMatrix(2, 2) = mDVector[2];
    mInverseViewMatrix(2, 3) = mPosition[2];
    mInverseViewMatrix(3, 0) = 0.0f;
    mInverseViewMatrix(3, 1) = 0.0f;
    mInverseViewMatrix(3, 2) = 0.0f;
    mInverseViewMatrix(3, 3) = 1.0f;
#else
    mViewMatrix(0, 0) = mRVector[0];
    mViewMatrix(1, 0) = mRVector[1];
    mViewMatrix(2, 0) = mRVector[2];
    mViewMatrix(3, 0) = -Dot(mPosition, mRVector);
    mViewMatrix(0, 1) = mUVector[0];
    mViewMatrix(1, 1) = mUVector[1];
    mViewMatrix(2, 1) = mUVector[2];
    mViewMatrix(3, 1) = -Dot(mPosition, mUVector);
    mViewMatrix(0, 2) = mDVector[0];
    mViewMatrix(1, 2) = mDVector[1];
    mViewMatrix(2, 2) = mDVector[2];
    mViewMatrix(3, 2) = -Dot(mPosition, mDVector);
    mViewMatrix(0, 3) = 0.0f;
    mViewMatrix(1, 3) = 0.0f;
    mViewMatrix(2, 3) = 0.0f;
    mViewMatrix(3, 3) = 1.0f;

    mInverseViewMatrix(0, 0) = mRVector[0];
    mInverseViewMatrix(1, 0) = mUVector[0];
    mInverseViewMatrix(2, 0) = mDVector[0];
    mInverseViewMatrix(3, 0) = mPosition[0];
    mInverseViewMatrix(0, 1) = mRVector[1];
    mInverseViewMatrix(1, 1) = mUVector[1];
    mInverseViewMatrix(2, 1) = mDVector[1];
    mInverseViewMatrix(3, 1) = mPosition[1];
    mInverseViewMatrix(0, 2) = mRVector[2];
    mInverseViewMatrix(1, 2) = mUVector[2];
    mInverseViewMatrix(2, 2) = mDVector[2];
    mInverseViewMatrix(3, 2) = mPosition[2];
    mInverseViewMatrix(0, 3) = 0.0f;
    mInverseViewMatrix(1, 3) = 0.0f;
    mInverseViewMatrix(2, 3) = 0.0f;
    mInverseViewMatrix(3, 3) = 1.0f;
#endif

    UpdatePVMatrix();
}

void ViewVolume::OnFrustumChange()
{
    // mIsDepthRangeZeroOne true:  map (x,y,z) into [-1,1]x[-1,1]x[0,1].
    // mIsDepthRangeZeroOne false: map (x,y,z) into [-1,1]x[-1,1]x[-1,1].
    float dMin = mFrustum[VF_DMIN];
    float dMax = mFrustum[VF_DMAX];
    float uMin = mFrustum[VF_UMIN];
    float uMax = mFrustum[VF_UMAX];
    float rMin = mFrustum[VF_RMIN];
    float rMax = mFrustum[VF_RMAX];
    float invDDiff = 1.0f / (dMax - dMin);
    float invUDiff = 1.0f / (uMax - uMin);
    float invRDiff = 1.0f / (rMax - rMin);

    if (mIsPerspective)
    {
#if defined(GTE_USE_MAT_VEC)
        if (mIsDepthRangeZeroOne)
        {
            mProjectionMatrix(0, 0) = 2.0f * dMin * invRDiff;
            mProjectionMatrix(0, 1) = 0.0f;
            mProjectionMatrix(0, 2) = -(rMin + rMax) * invRDiff;
            mProjectionMatrix(0, 3) = 0.0f;
            mProjectionMatrix(1, 0) = 0.0f;
            mProjectionMatrix(1, 1) = 2.0f * dMin * invUDiff;
            mProjectionMatrix(1, 2) = -(uMin + uMax) * invUDiff;
            mProjectionMatrix(1, 3) = 0.0f;
            mProjectionMatrix(2, 0) = 0.0f;
            mProjectionMatrix(2, 1) = 0.0f;
            mProjectionMatrix(2, 2) = dMax * invDDiff;
            mProjectionMatrix(2, 3) = -dMin * dMax * invDDiff;
            mProjectionMatrix(3, 0) = 0.0f;
            mProjectionMatrix(3, 1) = 0.0f;
            mProjectionMatrix(3, 2) = 1.0f;
            mProjectionMatrix(3, 3) = 0.0f;
        }
        else
        {
            mProjectionMatrix(0, 0) = 2.0f * dMin * invRDiff;
            mProjectionMatrix(0, 1) = 0.0f;
            mProjectionMatrix(0, 2) = -(rMin + rMax) * invRDiff;
            mProjectionMatrix(0, 3) = 0.0f;
            mProjectionMatrix(1, 0) = 0.0f;
            mProjectionMatrix(1, 1) = 2.0f * dMin * invUDiff;
            mProjectionMatrix(1, 2) = -(uMin + uMax) * invUDiff;
            mProjectionMatrix(1, 3) = 0.0f;
            mProjectionMatrix(2, 0) = 0.0f;
            mProjectionMatrix(2, 1) = 0.0f;
            mProjectionMatrix(2, 2) = (dMin + dMax) * invDDiff;
            mProjectionMatrix(2, 3) = -2.0f * dMin * dMax * invDDiff;
            mProjectionMatrix(3, 0) = 0.0f;
            mProjectionMatrix(3, 1) = 0.0f;
            mProjectionMatrix(3, 2) = 1.0f;
            mProjectionMatrix(3, 3) = 0.0f;
        }
#else
        if (mIsDepthRangeZeroOne)
        {
            mProjectionMatrix(0, 0) = 2.0f * dMin * invRDiff;
            mProjectionMatrix(1, 0) = 0.0f;
            mProjectionMatrix(2, 0) = -(rMin + rMax) * invRDiff;
            mProjectionMatrix(3, 0) = 0.0f;
            mProjectionMatrix(0, 1) = 0.0f;
            mProjectionMatrix(1, 1) = 2.0f * dMin * invUDiff;
            mProjectionMatrix(2, 1) = -(uMin + uMax) * invUDiff;
            mProjectionMatrix(3, 1) = 0.0f;
            mProjectionMatrix(0, 2) = 0.0f;
            mProjectionMatrix(1, 2) = 0.0f;
            mProjectionMatrix(2, 2) = dMax * invDDiff;
            mProjectionMatrix(3, 2) = -dMin * dMax * invDDiff;
            mProjectionMatrix(0, 3) = 0.0f;
            mProjectionMatrix(1, 3) = 0.0f;
            mProjectionMatrix(2, 3) = 1.0f;
            mProjectionMatrix(3, 3) = 0.0f;
        }
        else
        {
            mProjectionMatrix(0, 0) = 2.0f * dMin * invRDiff;
            mProjectionMatrix(1, 0) = 0.0f;
            mProjectionMatrix(2, 0) = -(rMin + rMax) * invRDiff;
            mProjectionMatrix(3, 0) = 0.0f;
            mProjectionMatrix(0, 1) = 0.0f;
            mProjectionMatrix(1, 1) = 2.0f * dMin * invUDiff;
            mProjectionMatrix(2, 1) = -(uMin + uMax) * invUDiff;
            mProjectionMatrix(3, 1) = 0.0f;
            mProjectionMatrix(0, 2) = 0.0f;
            mProjectionMatrix(1, 2) = 0.0f;
            mProjectionMatrix(2, 2) = (dMin + dMax) * invDDiff;
            mProjectionMatrix(3, 2) = -2.0f * dMin * dMax * invDDiff;
            mProjectionMatrix(0, 3) = 0.0f;
            mProjectionMatrix(1, 3) = 0.0f;
            mProjectionMatrix(2, 3) = 1.0f;
            mProjectionMatrix(3, 3) = 0.0f;
        }
#endif
    }
    else
    {
#if defined (GTE_USE_MAT_VEC)
        if (mIsDepthRangeZeroOne)
        {
            mProjectionMatrix(0, 0) = 2.0f * invRDiff;
            mProjectionMatrix(0, 1) = 0.0f;
            mProjectionMatrix(0, 2) = 0.0f;
            mProjectionMatrix(0, 3) = -(rMin + rMax) * invRDiff;
            mProjectionMatrix(1, 0) = 0.0f;
            mProjectionMatrix(1, 1) = 2.0f * invUDiff;
            mProjectionMatrix(1, 2) = 0.0f;
            mProjectionMatrix(1, 3) = -(uMin + uMax) * invUDiff;
            mProjectionMatrix(2, 0) = 0.0f;
            mProjectionMatrix(2, 1) = 0.0f;
            mProjectionMatrix(2, 2) = invDDiff;
            mProjectionMatrix(2, 3) = -dMin * invDDiff;
            mProjectionMatrix(3, 0) = 0.0f;
            mProjectionMatrix(3, 1) = 0.0f;
            mProjectionMatrix(3, 2) = 0.0f;
            mProjectionMatrix(3, 3) = 1.0f;
        }
        else
        {
            mProjectionMatrix(0, 0) = 2.0f * invRDiff;
            mProjectionMatrix(0, 1) = 0.0f;
            mProjectionMatrix(0, 2) = 0.0f;
            mProjectionMatrix(0, 3) = -(rMin + rMax) * invRDiff;
            mProjectionMatrix(1, 0) = 0.0f;
            mProjectionMatrix(1, 1) = 2.0f * invUDiff;
            mProjectionMatrix(1, 2) = 0.0f;
            mProjectionMatrix(1, 3) = -(uMin + uMax) * invUDiff;
            mProjectionMatrix(2, 0) = 0.0f;
            mProjectionMatrix(2, 1) = 0.0f;
            mProjectionMatrix(2, 2) = 2.0f * invDDiff;
            mProjectionMatrix(2, 3) = -(dMin + dMax) * invDDiff;
            mProjectionMatrix(3, 0) = 0.0f;
            mProjectionMatrix(3, 1) = 0.0f;
            mProjectionMatrix(3, 2) = 0.0f;
            mProjectionMatrix(3, 3) = 1.0f;
        }
#else
        if (mIsDepthRangeZeroOne)
        {
            mProjectionMatrix(0, 0) = 2.0f * invRDiff;
            mProjectionMatrix(1, 0) = 0.0f;
            mProjectionMatrix(2, 0) = 0.0f;
            mProjectionMatrix(3, 0) = -(rMin + rMax) * invRDiff;
            mProjectionMatrix(0, 1) = 0.0f;
            mProjectionMatrix(1, 1) = 2.0f * invUDiff;
            mProjectionMatrix(2, 1) = 0.0f;
            mProjectionMatrix(3, 1) = -(uMin + uMax) * invUDiff;
            mProjectionMatrix(0, 2) = 0.0f;
            mProjectionMatrix(1, 2) = 0.0f;
            mProjectionMatrix(2, 2) = invDDiff;
            mProjectionMatrix(3, 2) = -dMin * invDDiff;
            mProjectionMatrix(0, 3) = 0.0f;
            mProjectionMatrix(1, 3) = 0.0f;
            mProjectionMatrix(2, 3) = 0.0f;
            mProjectionMatrix(3, 3) = 1.0f;
        }
        else
        {
            mProjectionMatrix(0, 0) = 2.0f * invRDiff;
            mProjectionMatrix(1, 0) = 0.0f;
            mProjectionMatrix(2, 0) = 0.0f;
            mProjectionMatrix(3, 0) = -(rMin + rMax) * invRDiff;
            mProjectionMatrix(0, 1) = 0.0f;
            mProjectionMatrix(1, 1) = 2.0f * invUDiff;
            mProjectionMatrix(2, 1) = 0.0f;
            mProjectionMatrix(3, 1) = -(uMin + uMax) * invUDiff;
            mProjectionMatrix(0, 2) = 0.0f;
            mProjectionMatrix(1, 2) = 0.0f;
            mProjectionMatrix(2, 2) = invDDiff;
            mProjectionMatrix(3, 2) = -(dMin + dMax) * invDDiff;
            mProjectionMatrix(0, 3) = 0.0f;
            mProjectionMatrix(1, 3) = 0.0f;
            mProjectionMatrix(2, 3) = 0.0f;
            mProjectionMatrix(3, 3) = 1.0f;
        }
#endif
    }

    UpdatePVMatrix();
}

void ViewVolume::UpdatePVMatrix()
{
    Matrix4x4<float>& pMatrix = mProjectionMatrix;
    Matrix4x4<float>& pvMatrix = mProjectionViewMatrix;

#if defined(GTE_USE_MAT_VEC)
    pvMatrix = pMatrix * mViewMatrix;
#else
    pvMatrix = mViewMatrix * pMatrix;
#endif
}
