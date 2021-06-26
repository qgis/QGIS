// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.11

#pragma once

#include <Mathematics/PdeFilter3.h>
#include <Mathematics/Math.h>

namespace gte
{
    template <typename Real>
    class GradientAnisotropic3 : public PdeFilter3<Real>
    {
    public:
        GradientAnisotropic3(int xBound, int yBound, int zBound, Real xSpacing,
            Real ySpacing, Real zSpacing, Real const* data, bool const* mask,
            Real borderValue, typename PdeFilter<Real>::ScaleType scaleType, Real K)
            :
            PdeFilter3<Real>(xBound, yBound, zBound, xSpacing, ySpacing, zSpacing,
                data, mask, borderValue, scaleType),
            mK(K)
        {
            ComputeParameter();
        }

        virtual ~GradientAnisotropic3()
        {
        }

    protected:
        void ComputeParameter()
        {
            Real gradMagSqr = (Real)0;
            for (int z = 1; z <= this->mZBound; ++z)
            {
                for (int y = 1; y <= this->mYBound; ++y)
                {
                    for (int x = 1; x <= this->mXBound; ++x)
                    {
                        Real ux = this->GetUx(x, y, z);
                        Real uy = this->GetUy(x, y, z);
                        Real uz = this->GetUz(x, y, z);
                        gradMagSqr += ux * ux + uy * uy + uz * uz;
                    }
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

        virtual void OnUpdateSingle(int x, int y, int z) override
        {
            this->LookUp27(x, y, z);

            // one-sided U-derivative estimates
            Real uxFwd = this->mInvDx * (this->mUpzz - this->mUzzz);
            Real uxBwd = this->mInvDx * (this->mUzzz - this->mUmzz);
            Real uyFwd = this->mInvDy * (this->mUzpz - this->mUzzz);
            Real uyBwd = this->mInvDy * (this->mUzzz - this->mUzmz);
            Real uzFwd = this->mInvDz * (this->mUzzp - this->mUzzz);
            Real uzBwd = this->mInvDz * (this->mUzzz - this->mUzzm);

            // centered U-derivative estimates
            Real duvzz = this->mHalfInvDx * (this->mUpzz - this->mUmzz);
            Real duvpz = this->mHalfInvDx * (this->mUppz - this->mUmpz);
            Real duvmz = this->mHalfInvDx * (this->mUpmz - this->mUmmz);
            Real duvzp = this->mHalfInvDx * (this->mUpzp - this->mUmzp);
            Real duvzm = this->mHalfInvDx * (this->mUpzm - this->mUmzm);

            Real duzvz = this->mHalfInvDy * (this->mUzpz - this->mUzmz);
            Real dupvz = this->mHalfInvDy * (this->mUppz - this->mUpmz);
            Real dumvz = this->mHalfInvDy * (this->mUmpz - this->mUmmz);
            Real duzvp = this->mHalfInvDy * (this->mUzpp - this->mUzmp);
            Real duzvm = this->mHalfInvDy * (this->mUzpm - this->mUzmm);

            Real duzzv = this->mHalfInvDz * (this->mUzzp - this->mUzzm);
            Real dupzv = this->mHalfInvDz * (this->mUpzp - this->mUpzm);
            Real dumzv = this->mHalfInvDz * (this->mUmzp - this->mUmzm);
            Real duzpv = this->mHalfInvDz * (this->mUzpp - this->mUzpm);
            Real duzmv = this->mHalfInvDz * (this->mUzmp - this->mUzmm);

            Real uxCenSqr = duvzz * duvzz;
            Real uyCenSqr = duzvz * duzvz;
            Real uzCenSqr = duzzv * duzzv;

            Real uxEst, uyEst, uzEst, gradMagSqr;

            // estimate for C(x+1,y,z)
            uyEst = (Real)0.5 *(duzvz + dupvz);
            uzEst = (Real)0.5 *(duzzv + dupzv);
            gradMagSqr = uxCenSqr + uyEst * uyEst + uzEst * uzEst;
            Real cxp = std::exp(mMHalfParameter * gradMagSqr);

            // estimate for C(x-1,y,z)
            uyEst = (Real)0.5 *(duzvz + dumvz);
            uzEst = (Real)0.5 *(duzzv + dumzv);
            gradMagSqr = uxCenSqr + uyEst * uyEst + uzEst * uzEst;
            Real cxm = std::exp(mMHalfParameter * gradMagSqr);

            // estimate for C(x,y+1,z)
            uxEst = (Real)0.5 *(duvzz + duvpz);
            uzEst = (Real)0.5 *(duzzv + duzpv);
            gradMagSqr = uxEst * uxEst + uyCenSqr + uzEst * uzEst;
            Real cyp = std::exp(mMHalfParameter * gradMagSqr);

            // estimate for C(x,y-1,z)
            uxEst = (Real)0.5 *(duvzz + duvmz);
            uzEst = (Real)0.5 *(duzzv + duzmv);
            gradMagSqr = uxEst * uxEst + uyCenSqr + uzEst * uzEst;
            Real cym = std::exp(mMHalfParameter * gradMagSqr);

            // estimate for C(x,y,z+1)
            uxEst = (Real)0.5 *(duvzz + duvzp);
            uyEst = (Real)0.5 *(duzvz + duzvp);
            gradMagSqr = uxEst * uxEst + uyEst * uyEst + uzCenSqr;
            Real czp = std::exp(mMHalfParameter * gradMagSqr);

            // estimate for C(x,y,z-1)
            uxEst = (Real)0.5 *(duvzz + duvzm);
            uyEst = (Real)0.5 *(duzvz + duzvm);
            gradMagSqr = uxEst * uxEst + uyEst * uyEst + uzCenSqr;
            Real czm = std::exp(mMHalfParameter * gradMagSqr);

            this->mBuffer[this->mDst][z][y][x] = this->mUzzz + this->mTimeStep * (
                cxp * uxFwd - cxm * uxBwd +
                cyp * uyFwd - cym * uyBwd +
                czp * uzFwd - czm * uzBwd);
        }

        // These are updated on each iteration, since they depend on the
        // current average of the squared length of the gradients at the
        // voxels.
        Real mK;               // k
        Real mParameter;       // 1/(k^2*average(gradMagSqr))
        Real mMHalfParameter;  // -0.5*mParameter
    };
}