// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Applications/GTApplicationsPCH.h>
#include <Applications/CameraRig.h>
#include <Mathematics/Rotation.h>
using namespace gte;

CameraRig::CameraRig()
{
    Set(nullptr, 0.0f, 0.0f);
}

CameraRig::CameraRig(std::shared_ptr<Camera> const& camera,
    float translationSpeed, float rotationSpeed)
{
    Set(camera, translationSpeed, rotationSpeed);
}

void CameraRig::Set(std::shared_ptr<Camera> const& camera,
    float translationSpeed, float rotationSpeed)
{
    mCamera = camera;
    mTranslationSpeed = translationSpeed;
    mRotationSpeed = rotationSpeed;
    ComputeWorldAxes();
    ClearMotions();
}

void CameraRig::ComputeWorldAxes()
{
    if (mCamera)
    {
        mWorldAxis[0] = mCamera->GetDVector();
        mWorldAxis[1] = mCamera->GetUVector();
        mWorldAxis[2] = mCamera->GetRVector();
    }
    else
    {
        mWorldAxis[0].MakeZero();
        mWorldAxis[1].MakeZero();
        mWorldAxis[2].MakeZero();
    }
}

bool CameraRig::PushMotion(int trigger)
{
    auto element = mIndirectMap.find(trigger);
    return (element != mIndirectMap.end() ? SetActive(element->second) : false);
}

bool CameraRig::PopMotion(int trigger)
{
    auto element = mIndirectMap.find(trigger);
    return (element != mIndirectMap.end() ? SetInactive(element->second) : false);
}

bool CameraRig::Move()
{
    // The current semantics allow for processing all active motions,
    // which was the semantics in Wild Magic 5.  For example, if you
    // move the camera with the up-arrow (forward motion) and with
    // the right-arrow (turn-right motion), both will occur during the
    // idle loop.
    if (mNumActiveMotions > 0)
    {
        for (int i = 0; i < mNumActiveMotions; ++i)
        {
            (this->*mActiveMotions[i])();
        }
        return true;
    }
    return false;

    // If you prefer the previous semantics where only one key press is
    // processed at a time, use this previous version of the code instead
    // of the block of code above.
    //
    // if (mMotion)
    // {
    //     (this->*mMotion)();
    //     return true;
    // }
    // return false;
}

void CameraRig::ClearMotions()
{
    mMotion = nullptr;
    mIndirectMap.clear();
    mNumActiveMotions = 0;
    std::fill(mActiveMotions.begin(), mActiveMotions.end(), nullptr);
}

void CameraRig::MoveForward()
{
    if (mCamera)
    {
        mCamera->SetPosition(mCamera->GetPosition() + mTranslationSpeed * mWorldAxis[0]);
    }
}

void CameraRig::MoveBackward()
{
    if (mCamera)
    {
        mCamera->SetPosition(mCamera->GetPosition() - mTranslationSpeed * mWorldAxis[0]);
    }
}

void CameraRig::MoveUp()
{
    if (mCamera)
    {
        mCamera->SetPosition(mCamera->GetPosition() + mTranslationSpeed*mWorldAxis[1]);
    }
}

void CameraRig::MoveDown()
{
    if (mCamera)
    {
        mCamera->SetPosition(mCamera->GetPosition() - mTranslationSpeed*mWorldAxis[1]);
    }
}

void CameraRig::MoveRight()
{
    if (mCamera)
    {
        mCamera->SetPosition(mCamera->GetPosition() + mTranslationSpeed*mWorldAxis[2]);
    }
}

void CameraRig::MoveLeft()
{
    if (mCamera)
    {
        mCamera->SetPosition(mCamera->GetPosition() - mTranslationSpeed*mWorldAxis[2]);
    }
}

void CameraRig::TurnRight()
{
    if (mCamera)
    {
        Matrix4x4<float> incremental = Rotation<4, float>(
            AxisAngle<4, float>(mWorldAxis[1], -mRotationSpeed));

#if defined(GTE_USE_MAT_VEC)
        mWorldAxis[0] = incremental * mWorldAxis[0];
        mWorldAxis[2] = incremental * mWorldAxis[2];
        mCamera->SetAxes(
            incremental * mCamera->GetDVector(),
            incremental * mCamera->GetUVector(),
            incremental * mCamera->GetRVector());
#else
        mWorldAxis[0] = mWorldAxis[0] * incremental;
        mWorldAxis[2] = mWorldAxis[2] * incremental;
        mCamera->SetAxes(
            mCamera->GetDVector() * incremental,
            mCamera->GetUVector() * incremental,
            mCamera->GetRVector() * incremental);
#endif
    }
}

void CameraRig::TurnLeft()
{
    if (mCamera)
    {
        Matrix4x4<float> incremental = Rotation<4, float>(
            AxisAngle<4, float>(mWorldAxis[1], +mRotationSpeed));

#if defined(GTE_USE_MAT_VEC)
        mWorldAxis[0] = incremental * mWorldAxis[0];
        mWorldAxis[2] = incremental * mWorldAxis[2];
        mCamera->SetAxes(
            incremental*mCamera->GetDVector(),
            incremental*mCamera->GetUVector(),
            incremental*mCamera->GetRVector());
#else
        mWorldAxis[0] = mWorldAxis[0] * incremental;
        mWorldAxis[2] = mWorldAxis[2] * incremental;
        mCamera->SetAxes(
            mCamera->GetDVector() * incremental,
            mCamera->GetUVector() * incremental,
            mCamera->GetRVector() * incremental);
#endif
    }
}

void CameraRig::LookUp()
{
    if (mCamera)
    {
        Matrix4x4<float> incremental = Rotation<4, float>(
            AxisAngle<4, float>(mWorldAxis[2], +mRotationSpeed));

#if defined(GTE_USE_MAT_VEC)
        mCamera->SetAxes(
            incremental * mCamera->GetDVector(),
            incremental * mCamera->GetUVector(),
            incremental * mCamera->GetRVector());
#else
        mCamera->SetAxes(
            mCamera->GetDVector() * incremental,
            mCamera->GetUVector() * incremental,
            mCamera->GetRVector() * incremental);
#endif
    }
}

void CameraRig::LookDown()
{
    if (mCamera)
    {
        Matrix4x4<float> incremental = Rotation<4, float>(
            AxisAngle<4, float>(mWorldAxis[2], -mRotationSpeed));

#if defined(GTE_USE_MAT_VEC)
        mCamera->SetAxes(
            incremental * mCamera->GetDVector(),
            incremental * mCamera->GetUVector(),
            incremental * mCamera->GetRVector());
#else
        mCamera->SetAxes(
            mCamera->GetDVector() * incremental,
            mCamera->GetUVector() * incremental,
            mCamera->GetRVector() * incremental);
#endif
    }
}

void CameraRig::RollClockwise()
{
    if (mCamera)
    {
        Matrix4x4<float> incremental = Rotation<4, float>(
            AxisAngle<4, float>(mWorldAxis[0], +mRotationSpeed));

#if defined(GTE_USE_MAT_VEC)
        mCamera->SetAxes(
            incremental * mCamera->GetDVector(),
            incremental * mCamera->GetUVector(),
            incremental * mCamera->GetRVector());
#else
        mCamera->SetAxes(
            mCamera->GetDVector() * incremental,
            mCamera->GetUVector() * incremental,
            mCamera->GetRVector() * incremental);
#endif
    }
}

void CameraRig::RollCounterclockwise()
{
    if (mCamera)
    {
        Matrix4x4<float> incremental = Rotation<4, float>(
            AxisAngle<4, float>(mWorldAxis[0], -mRotationSpeed));

#if defined(GTE_USE_MAT_VEC)
        mCamera->SetAxes(
            incremental * mCamera->GetDVector(),
            incremental * mCamera->GetUVector(),
            incremental * mCamera->GetRVector());
#else
        mCamera->SetAxes(
            mCamera->GetDVector() * incremental,
            mCamera->GetUVector() * incremental,
            mCamera->GetRVector() * incremental);
#endif
    }
}

void CameraRig::Register(int trigger, MoveFunction function)
{
    if (trigger >= 0)
    {
        auto element = mIndirectMap.find(trigger);
        if (element == mIndirectMap.end())
        {
            mIndirectMap.insert(std::make_pair(trigger, function));
        }
    }
    else
    {
        for (auto element : mIndirectMap)
        {
            if (element.second == function)
            {
                mIndirectMap.erase(trigger);
                return;
            }
        }
    }
}

bool CameraRig::SetActive(MoveFunction function)
{
    for (int i = 0; i < mNumActiveMotions; ++i)
    {
        if (mActiveMotions[i] == function)
        {
            return false;
        }
    }

    if (mNumActiveMotions < MAX_ACTIVE_MOTIONS)
    {
        mMotion = function;
        mActiveMotions[mNumActiveMotions] = function;
        ++mNumActiveMotions;
        return true;
    }

    return false;
}

bool CameraRig::SetInactive(MoveFunction function)
{
    for (int i = 0; i < mNumActiveMotions; ++i)
    {
        if (mActiveMotions[i] == function)
        {
            for (int j0 = i, j1 = j0 + 1; j1 < mNumActiveMotions; j0 = j1++)
            {
                mActiveMotions[j0] = mActiveMotions[j1];
            }
            --mNumActiveMotions;
            mActiveMotions[mNumActiveMotions] = nullptr;
            if (mNumActiveMotions > 0)
            {
                mMotion = mActiveMotions[mNumActiveMotions - 1];
            }
            else
            {
                mMotion = nullptr;
            }
            return true;
        }
    }
    return false;
}
