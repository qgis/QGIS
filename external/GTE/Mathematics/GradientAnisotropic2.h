// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.11

#pragma once

#include <Mathematics/PdeFilter2.h>
#include <Mathematics/Math.h>

namespace gte
{
    template <typename Real>
    class GradientAnisotropic2 : public PdeFilter2<Real>
    {
    public:
        GradientAnisotropic2(int xBound, int yBound, Real xSpacing, Real ySpacing,
            Real const* data, bool const* mask, Real borderValue,
            typename PdeFilter<Real>::ScaleType scaleType, Real K)
            :
            PdeFilter2<Real>(xBound, yBound, xSpacing, ySpacing, data, mask,
                borderValue, scaleType),
            mK(K)
        {
            ComputeParameter();
        }

        virtual ~GradientAnisotropic2()
        {
        }

    protected:
        void ComputeParameter()
        {
            Real gradMagSqr = (Real)0;
            for (int y = 1; y <= this->mYBound; ++y)
            {
                for (int x = 1; x <= this->mXBound; ++x)
                {
                    Real ux = this->GetUx(x, y);
                    Real uy = this->GetUy(x, y);
                    gradMagSqr += ux * ux + uy * uy;
                }
            }
            gradMagSqr /= (Real)this->mQuantity;

            mParameter = (Real)1 / (mK * mK * gradMagSqr);
            mMHalfParameter = (Real)-0.5 * mParameter;
        }

        virtual void OnPreUpdate() override
        {
            ComputeParameter();
        }

        virtual void OnUpdateSingle(int x, int y) override
        {
            this->LookUp9(x, y);

            // one-sided U-derivative estimates
            Real uxFwd = this->mInvDx * (this->mUpz - this->mUzz);
            Real uxBwd = this->mInvDx * (this->mUzz - this->mUmz);
            Real uyFwd = this->mInvDy * (this->mUzp - this->mUzz);
            Real uyBwd = this->mInvDy * (this->mUzz - this->mUzm);

            // centered U-derivative estimates
            Real uxCenM = this->mHalfInvDx * (this->mUpm - this->mUmm);
            Real uxCenZ = this->mHalfInvDx * (this->mUpz - this->mUmz);
            Real uxCenP = this->mHalfInvDx * (this->mUpp - this->mUmp);
            Real uyCenM = this->mHalfInvDy * (this->mUmp - this->mUmm);
            Real uyCenZ = this->mHalfInvDy * (this->mUzp - this->mUzm);
            Real uyCenP = this->mHalfInvDy * (this->mUpp - this->mUpm);

            Real uxCenZSqr = uxCenZ * uxCenZ;
            Real uyCenZSqr = uyCenZ * uyCenZ;
            Real gradMagSqr;

            // estimate for C(x+1,y)
            Real uyEstP = (Real)0.5 * (uyCenZ + uyCenP);
            gradMagSqr = uxCenZSqr + uyEstP * uyEstP;
            Real cxp = std::exp(mMHalfParameter * gradMagSqr);

            // estimate for C(x-1,y)
            Real uyEstM = (Real)0.5 * (uyCenZ + uyCenM);
            gradMagSqr = uxCenZSqr + uyEstM * uyEstM;
            Real cxm = std::exp(mMHalfParameter * gradMagSqr);

            // estimate for C(x,y+1)
            Real uxEstP = (Real)0.5 * (uxCenZ + uxCenP);
            gradMagSqr = uyCenZSqr + uxEstP * uxEstP;
            Real cyp = std::exp(mMHalfParameter * gradMagSqr);

            // estimate for C(x,y-1)
            Real uxEstM = (Real)0.5 * (uxCenZ + uxCenM);
            gradMagSqr = uyCenZSqr + uxEstM * uxEstM;
            Real cym = std::exp(mMHalfParameter * gradMagSqr);

            this->mBuffer[this->mDst][y][x] = this->mUzz + this->mTimeStep * (
                cxp * uxFwd - cxm * uxBwd +
                cyp * uyFwd - cym * uyBwd);
        }

        // These are updated on each iteration, since they depend on the
        // current average of the squared length of the gradients at the
        // pixels.
        Real mK;                // k
        Real mParameter;        // 1/(k^2*average(gradMagSqr))
        Real mMHalfParameter;   // -0.5*mParameter
    };
}
