// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.11

#pragma once

#include <Mathematics/PdeFilter2.h>

namespace gte
{
    template <typename Real>
    class GaussianBlur2 : public PdeFilter2<Real>
    {
    public:
        GaussianBlur2(int xBound, int yBound, Real xSpacing, Real ySpacing,
            Real const* data, bool const* mask, Real borderValue,
            typename PdeFilter<Real>::ScaleType scaleType)
            :
            PdeFilter2<Real>(xBound, yBound, xSpacing, ySpacing, data, mask,
                borderValue, scaleType)
        {
            mMaximumTimeStep = (Real)0.5 / (this->mInvDxDx + this->mInvDyDy);
        }

        virtual ~GaussianBlur2()
        {

        }

        inline Real GetMaximumTimeStep() const
        {
            return mMaximumTimeStep;
        }

    protected:
        virtual void OnUpdateSingle(int x, int y) override
        {
            this->LookUp5(x, y);

            Real uxx = this->mInvDxDx * (this->mUpz - (Real)2 * this->mUzz + this->mUmz);
            Real uyy = this->mInvDyDy * (this->mUzp - (Real)2 * this->mUzz + this->mUzm);

            this->mBuffer[this->mDst][y][x] = this->mUzz + this->mTimeStep * (uxx + uyy);
        }

        Real mMaximumTimeStep;
    };
}
