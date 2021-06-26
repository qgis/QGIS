// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/PdeFilter.h>
#include <array>
#include <limits>
#include <vector>

namespace gte
{
    template <typename Real>
    class PdeFilter1 : public PdeFilter<Real>
    {
    public:
        // Abstract base class.
        PdeFilter1(int xBound, Real xSpacing, Real const* data, bool const* mask,
            Real borderValue, typename PdeFilter<Real>::ScaleType scaleType)
            :
            PdeFilter<Real>(xBound, data, borderValue, scaleType),
            mXBound(xBound),
            mXSpacing(xSpacing),
            mInvDx((Real)1 / xSpacing),
            mHalfInvDx((Real)0.5 * mInvDx),
            mInvDxDx(mInvDx * mInvDx),
            mUm(0), mUz(0), mUp(0),
            mSrc(0),
            mDst(1),
            mMask(xBound + 2),
            mHasMask(mask != nullptr)
        {
            // The mBuffer[] are ping-pong buffers for filtering.
            for (int i = 0; i < 2; ++i)
            {
                mBuffer[i].resize(xBound + 2);
            }

            for (int x = 0, xp = 1, i = 0; x < mXBound; ++x, ++xp, ++i)
            {
                mBuffer[mSrc][xp] = this->mOffset + (data[i] - this->mMin) * this->mScale;
                mBuffer[mDst][xp] = (Real)0;
                mMask[xp] = (mHasMask ? mask[i] : 1);
            }

            // Assign values to the 1-pixel image border.
            if (this->mBorderValue != std::numeric_limits<Real>::max())
            {
                AssignDirichletImageBorder();
            }
            else
            {
                AssignNeumannImageBorder();
            }

            // To handle masks that do not cover the entire image, assign
            // values to those pixels that are 8-neighbors of the mask pixels.
            if (mHasMask)
            {
                if (this->mBorderValue != std::numeric_limits<Real>::max())
                {
                    AssignDirichletMaskBorder();
                }
                else
                {
                    AssignNeumannMaskBorder();
                }
            }
        }

        virtual ~PdeFilter1()
        {
        }

        // Member access.  The internal 1D images for "data" and "mask" are
        // copies of the inputs to the constructor but padded with a 1-pixel
        // thick border to support filtering on the image boundary.  These
        // images are of size (xbound+2).  The correct lookups into the padded
        // arrays are handled internally.
        inline int GetXBound() const
        {
            return mXBound;
        }

        inline Real GetXSpacing() const
        {
            return mXSpacing;
        }

        // Pixel access and derivative estimation.  The lookups into the
        // padded data are handled correctly.  The estimation involves only
        // the 3-tuple neighborhood of (x), where 0 <= x < xbound.  TODO: If
        // larger neighborhoods are desired at a later date, the padding and
        // associated code must be adjusted accordingly.
        Real GetU(int x) const
        {
            auto const& F = mBuffer[mSrc];
            return F[x + 1];
        }

        Real GetUx(int x) const
        {
            auto const& F = mBuffer[mSrc];
            return mHalfInvDx * (F[x + 2] - F[x]);
        }

        Real GetUxx(int x) const
        {
            auto const& F = mBuffer[mSrc];
            return mInvDxDx * (F[x + 2] - (Real)2 * F[x + 1] + F[x]);
        }

        int GetMask(int x) const
        {
            return mMask[x + 1];
        }

    protected:
        // Assign values to the 1-pixel image border.
        void AssignDirichletImageBorder()
        {
            int xBp1 = mXBound + 1;

            // vertex (0,0)
            mBuffer[mSrc][0] = this->mBorderValue;
            mBuffer[mDst][0] = this->mBorderValue;
            if (mHasMask)
            {
                mMask[0] = 0;
            }

            // vertex (xmax,0)
            mBuffer[mSrc][xBp1] = this->mBorderValue;
            mBuffer[mDst][xBp1] = this->mBorderValue;
            if (mHasMask)
            {
                mMask[xBp1] = 0;
            }
        }

        void AssignNeumannImageBorder()
        {
            int xBp1 = mXBound + 1;
            Real duplicate;

            // vertex (0,0)
            duplicate = mBuffer[mSrc][1];
            mBuffer[mSrc][0] = duplicate;
            mBuffer[mDst][0] = duplicate;
            if (mHasMask)
            {
                mMask[0] = 0;
            }

            // vertex (xmax,0)
            duplicate = mBuffer[mSrc][mXBound];
            mBuffer[mSrc][xBp1] = duplicate;
            mBuffer[mDst][xBp1] = duplicate;
            if (mHasMask)
            {
                mMask[xBp1] = 0;
            }
        }

        // Assign values to the 1-pixel mask border.
        void AssignDirichletMaskBorder()
        {
            for (int x = 1; x <= mXBound; ++x)
            {
                if (mMask[x])
                {
                    continue;
                }

                for (int i0 = 0, j0 = x - 1; i0 < 3; ++i0, ++j0)
                {
                    if (mMask[j0])
                    {
                        mBuffer[mSrc][x] = this->mBorderValue;
                        mBuffer[mDst][x] = this->mBorderValue;
                        break;
                    }
                }
            }
        }

        void AssignNeumannMaskBorder()
        {
            // Recompute the values just outside the masked region.  This
            // guarantees that derivative estimations use the current values
            // around the boundary.
            for (int x = 1; x <= mXBound; ++x)
            {
                if (mMask[x])
                {
                    continue;
                }

                int count = 0;
                Real average = (Real)0;
                for (int i0 = 0, j0 = x - 1; i0 < 3; ++i0, ++j0)
                {
                    if (mMask[j0])
                    {
                        average += mBuffer[mSrc][j0];
                        ++count;
                    }
                }

                if (count > 0)
                {
                    average /= (Real)count;
                    mBuffer[mSrc][x] = average;
                    mBuffer[mDst][x] = average;
                }
            }
        }

        // This function recomputes the boundary values when Neumann
        // conditions are used.  If a derived class overrides this, it must
        // call the base-class OnPreUpdate first.
        virtual void OnPreUpdate() override
        {
            if (mHasMask && this->mBorderValue == std::numeric_limits<Real>::max())
            {
                // Neumann boundary conditions are in use, so recompute the
                // mask/ border.
                AssignNeumannMaskBorder();
            }
            // else: No mask has been specified or Dirichlet boundary
            // conditions are in use.  Nothing to do.
        }

        // Iterate over all the pixels and call OnUpdate(x) for each pixel
        // that is not masked out.
        virtual void OnUpdate() override
        {
            for (int x = 1; x <= mXBound; ++x)
            {
                if (!mHasMask || mMask[x])
                {
                    OnUpdate(x);
                }
            }
        }

        // If a derived class overrides this, it must call the base-class
        // OnPostUpdate last.  The base-class function swaps the buffers for
        // the next pass.
        virtual void OnPostUpdate() override
        {
            std::swap(mSrc, mDst);
        }

        // The per-pixel processing depends on the PDE algorithm.  The (x)
        // must be in padded coordinates: 1 <= x <= xbound.
        virtual void OnUpdate(int x) = 0;

        // Copy source data to temporary storage.
        void LookUp3(int x)
        {
            auto const& F = mBuffer[mSrc];
            mUm = F[x - 1];
            mUz = F[x];
            mUp = F[x + 1];
        }

        // Image parameters.
        int mXBound;
        Real mXSpacing;       // dx
        Real mInvDx;          // 1/dx
        Real mHalfInvDx;      // 1/(2*dx)
        Real mInvDxDx;        // 1/(dx*dx)

        // Temporary storage for 3-tuple neighborhood.  In the notation mUx,
        // the x index is in {m,z,p}, referring to subtract 1 (m), no change
        // (z), or add 1 (p) to the appropriate index.
        Real mUm, mUz, mUp;

        // Successive iterations toggle between two buffers.
        std::array<std::vector<Real>, 2> mBuffer;
        int mSrc, mDst;
        std::vector<int> mMask;
        bool mHasMask;
    };
}
