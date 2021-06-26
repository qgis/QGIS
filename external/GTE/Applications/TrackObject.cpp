// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Applications/GTApplicationsPCH.h>
#include <Applications/TrackObject.h>
using namespace gte;

TrackObject::TrackObject()
    :
    mXSize(0),
    mYSize(0),
    mX0(0.0f),
    mY0(0.0f),
    mX1(0.0f),
    mY1(0.0f),
    mMultiplier(0.0f),
    mActive(false),
    mValid(false)
{
    mRoot = std::make_shared<Node>();
}

TrackObject::TrackObject(int xSize, int ySize, std::shared_ptr<Camera> const& camera)
    :
    mXSize(0),
    mYSize(0),
    mX0(0.0f),
    mY0(0.0f),
    mX1(0.0f),
    mY1(0.0f),
    mMultiplier(0.0f),
    mActive(false),
    mValid(false)
{
    Set(xSize, ySize, camera);
    mRoot = std::make_shared<Node>();
}

void TrackObject::Set(int xSize, int ySize, std::shared_ptr<Camera> const& camera)
{
    if (xSize > 0 && ySize > 0 && camera)
    {
        mXSize = xSize;
        mYSize = ySize;
        mCamera = camera;
        mMultiplier = 1.0f / (mXSize >= mYSize ? mYSize : mXSize);
        mX0 = 0.5f * mXSize;
        mY0 = 0.5f * mYSize;
        mX1 = mX0;
        mY1 = mY0;
        mValid = true;
    }
    else
    {
        mXSize = 0;
        mYSize = 0;
        mCamera = nullptr;
        mX0 = 0.0f;
        mY0 = 0.0f;
        mMultiplier = 0.0f;
        mValid = false;
    }
}

void TrackObject::Attach(std::shared_ptr<Spatial> const& object)
{
    if (mValid && object)
    {
        auto node = std::dynamic_pointer_cast<Node>(mRoot);
        if (node)
        {
            node->AttachChild(object);
        }
    }
}

void TrackObject::Detach(std::shared_ptr<Spatial> const& object)
{
    if (mValid && object)
    {
        auto node = std::dynamic_pointer_cast<Node>(mRoot);
        if (node)
        {
            node->DetachChild(object);
        }
    }
}

void TrackObject::DetachAll()
{
    if (mValid)
    {
        auto node = std::dynamic_pointer_cast<Node>(mRoot);
        if (node)
        {
            node->DetachAllChildren();
        }
    }
}

void TrackObject::SetInitialPoint(int x, int y)
{
    if (mValid && mRoot)
    {
        mX0 = (2.0f * x - mXSize) * mMultiplier;
        mY0 = (2.0f * y - mYSize) * mMultiplier;
        OnSetInitialPoint();
    }
}

void TrackObject::SetFinalPoint(int x, int y)
{
    if (mValid && mRoot)
    {
        mX1 = (2.0f * x - mXSize) * mMultiplier;
        mY1 = (2.0f * y - mYSize) * mMultiplier;
        if (mX1 != mX0 || mY1 != mY0)
        {
            OnSetFinalPoint();
        }
    }
}

void TrackObject::NormalizeAndUpdateRoot(Matrix4x4<float>& rotate)
{
    // Renormalize to avoid accumulated rounding errors that can cause the
    // rotation matrix to degenerate.
    Vector4<float> v[3] =
    {
        rotate.GetCol(0),
        rotate.GetCol(1),
        rotate.GetCol(2)
    };

    Orthonormalize<4, float>(3, v);

    rotate.SetCol(0, v[0]);
    rotate.SetCol(1, v[1]);
    rotate.SetCol(2, v[2]);

    mRoot->localTransform.SetRotation(rotate);
    mRoot->Update();
}
