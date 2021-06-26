// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/TrackObject.h>

namespace gte
{
    class TrackBall : public TrackObject
    {
    public:
        // Construction and destruction.  The trackball is the largest circle
        // centered in the rectangle of dimensions xSize-by-ySize.  The
        // rectangle is assumed to be defined in right-handed coordinates, so
        // y-values in SetInitialPoint and SetFinalPoint are reflected to
        // (ySize - 1 - y).
        virtual ~TrackBall() = default;
        TrackBall();
        TrackBall(int xSize, int ySize, std::shared_ptr<Camera> const& camera);

        // Reset the trackball rotation to the identity.
        void Reset();

    protected:
        virtual void OnSetInitialPoint() override;
        virtual void OnSetFinalPoint() override;

        Matrix4x4<float> mInitialOrientation;
    };
}
