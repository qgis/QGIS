// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.11

#pragma once

#include <Mathematics/PdeFilter3.h>

namespace gte
{
    template <typename Real>
    class CurvatureFlow3 : public PdeFilter3<Real>
    {
    public:
        CurvatureFlow3(int xBound, int yBound, int zBound, Real xSpacing,
            Real ySpacing, Real zSpacing, Real const* data, bool const* mask,
            Real borderValue, typename PdeFilter<Real>::ScaleType scaleType)
            :
            PdeFilter3<Real>(xBound, yBound, zBound, xSpacing, ySpacing,
                zSpacing, data, mask, borderValue, scaleType)
        {
        }

        virtual ~CurvatureFlow3()
        {
        }

    protected:
        virtual void OnUpdateSingle(int x, int y, int z) override
        {
            this->LookUp27(x, y, z);

            Real ux = this->mHalfInvDx * (this->mUpzz - this->mUmzz);
            Real uy = this->mHalfInvDy * (this->mUzpz - this->mUzmz);
            Real uz = this->mHalfInvDz * (this->mUzzp - this->mUzzm);
            Real uxx = this->mInvDxDx * (this->mUpzz - (Real)2 * this->mUzzz + this->mUmzz);
            Real uxy = this->mFourthInvDxDy * (this->mUmmz + this->mUppz - this->mUpmz - this->mUmpz);
            Real uxz = this->mFourthInvDxDz * (this->mUmzm + this->mUpzp - this->mUpzm - this->mUmzp);
            Real uyy = this->mInvDyDy * (this->mUzpz - (Real)2 * this->mUzzz + this->mUzmz);
            Real uyz = this->mFourthInvDyDz * (this->mUzmm + this->mUzpp - this->mUzpm - this->mUzmp);
            Real uzz = this->mInvDzDz * (this->mUzzp - (Real)2 * this->mUzzz + this->mUzzm);

            Real denom = ux * ux + uy * uy + uz * uz;
            if (denom > (Real)0)
            {
                Real numer0 = uy * (uxx*uy - uxy * ux) + ux * (uyy*ux - uxy * uy);
                Real numer1 = uz * (uxx*uz - uxz * ux) + ux * (uzz*ux - uxz * uz);
                Real numer2 = uz * (uyy*uz - uyz * uy) + uy * (uzz*uy - uyz * uz);
                Real numer = numer0 + numer1 + numer2;
                this->mBuffer[this->mDst][z][y][x] = this->mUzzz + this->mTimeStep * numer / denom;
            }
            else
            {
                this->mBuffer[this->mDst][z][y][x] = this->mUzzz;
            }
        }
    };
}
