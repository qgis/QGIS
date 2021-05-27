// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.11

#pragma once

#include <Mathematics/PdeFilter.h>
#include <Mathematics/Array2.h>
#include <array>
#include <limits>

namespace gte
{
    template <typename Real>
    class PdeFilter2 : public PdeFilter<Real>
    {
    public:
        // Abstract base class.
        PdeFilter2(int xBound, int yBound, Real xSpacing, Real ySpacing,
            Real const* data, bool const* mask, Real borderValue,
            typename PdeFilter<Real>::ScaleType scaleType)
            :
            PdeFilter<Real>(xBound * yBound, data, borderValue, scaleType),
            mXBound(xBound),
            mYBound(yBound),
            mXSpacing(xSpacing),
            mYSpacing(ySpacing),
            mInvDx((Real)1 / xSpacing),
            mInvDy((Real)1 / ySpacing),
            mHalfInvDx((Real)0.5 * mInvDx),
            mHalfInvDy((Real)0.5 * mInvDy),
            mInvDxDx(mInvDx * mInvDx),
            mFourthInvDxDy(mHalfInvDx * mHalfInvDy),
            mInvDyDy(mInvDy * mInvDy),
            mUmm(0), mUzm(0), mUpm(0),
            mUmz(0), mUzz(0), mUpz(0),
            mUmp(0), mUzp(0), mUpp(0),
            mSrc(0),
            mDst(1),
            mMask(xBound + 2, yBound + 2),
            mHasMask(mask != nullptr)
        {
            for (int i = 0; i < 2; ++i)
            {
                mBuffer[i] = Array2<Real>(xBound + 2, yBound + 2);
            }
                    
            // The mBuffer[] are ping-pong buffers for filtering.
            for (int y = 0, yp = 1, i = 0; y < mYBound; ++y, ++yp)
            {
                for (int x = 0, xp = 1; x < mXBound; ++x, ++xp, ++i)
                {
                    mBuffer[mSrc][yp][xp] = this->mOffset + (data[i] - this->mMin) * this->mScale;
                    mBuffer[mDst][yp][xp] = (Real)0;
                    mMask[yp][xp] = (mHasMask ? mask[i] : 1);
                }
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

        virtual ~PdeFilter2()
        {
        }

        // Member access.  The internal 2D images for "data" and "mask" are
        // copies of the inputs to the constructor but padded with a 1-pixel
        // thick border to support filtering on the image boundary.  These
        // images are of size (xbound+2)-by-(ybound+2).  The correct lookups
        // into the padded arrays are handled internally.
        inline int GetXBound() const
        {
            return mXBound;
        }

        inline int GetYBound() const
        {
            return mYBound;
        }

        inline Real GetXSpacing() const
        {
            return mXSpacing;
        }

        inline Real GetYSpacing() const
        {
            return mYSpacing;
        }

        // Pixel access and derivative estimation.  The lookups into the
        // padded data are handled correctly.  The estimation involves only
        // the 3-by-3 neighborhood of (x,y), where 0 <= x < xbound and
        // 0 <= y < ybound.  TODO: If larger neighborhoods are desired at a
        // later date, the padding and associated code must be adjusted
        // accordingly.
        Real GetU(int x, int y) const
        {
            auto const& F = mBuffer[mSrc];
            int xp1 = x + 1, yp1 = y + 1;
            return F[yp1][xp1];
        }

        Real GetUx(int x, int y) const
        {
            auto const& F = mBuffer[mSrc];
            int xp2 = x + 2, yp1 = y + 1;
            return mHalfInvDx * (F[yp1][xp2] - F[yp1][x]);
        }

        Real GetUy(int x, int y) const
        {
            auto const& F = mBuffer[mSrc];
            int xp1 = x + 1, yp2 = y + 2;
            return mHalfInvDy * (F[yp2][xp1] - F[y][xp1]);
        }

        Real GetUxx(int x, int y) const
        {
            auto const& F = mBuffer[mSrc];
            int xp1 = x + 1, xp2 = x + 2, yp1 = y + 1;
            return mInvDxDx * (F[yp1][xp2] - (Real)2 * F[yp1][xp1] + F[yp1][x]);
        }

        Real GetUxy(int x, int y) const
        {
            auto const& F = mBuffer[mSrc];
            int xp2 = x + 2, yp2 = y + 2;
            return mFourthInvDxDy * (F[y][x] - F[y][xp2] + F[yp2][xp2] - F[yp2][x]);
        }

        Real GetUyy(int x, int y) const
        {
            auto const& F = mBuffer[mSrc];
            int xp1 = x + 1, yp1 = y + 1, yp2 = y + 2;
            return mInvDyDy * (F[yp2][xp1] - (Real)2 * F[yp1][xp1] + F[y][xp1]);
        }

        int GetMask(int x, int y) const
        {
            int xp1 = x + 1, yp1 = y + 1;
            return mMask[yp1][xp1];
        }

    protected:
        // Assign values to the 1-pixel image border.
        void AssignDirichletImageBorder()
        {
            int xBp1 = mXBound + 1, yBp1 = mYBound + 1;
            int x, y;

            // vertex (0,0)
            mBuffer[mSrc][0][0] = this->mBorderValue;
            mBuffer[mDst][0][0] = this->mBorderValue;
            if (mHasMask)
            {
                mMask[0][0] = 0;
            }

            // vertex (xmax,0)
            mBuffer[mSrc][0][xBp1] = this->mBorderValue;
            mBuffer[mDst][0][xBp1] = this->mBorderValue;
            if (mHasMask)
            {
                mMask[0][xBp1] = 0;
            }

            // vertex (0,ymax)
            mBuffer[mSrc][yBp1][0] = this->mBorderValue;
            mBuffer[mDst][yBp1][0] = this->mBorderValue;
            if (mHasMask)
            {
                mMask[yBp1][0] = 0;
            }

            // vertex (xmax,ymax)
            mBuffer[mSrc][yBp1][xBp1] = this->mBorderValue;
            mBuffer[mDst][yBp1][xBp1] = this->mBorderValue;
            if (mHasMask)
            {
                mMask[yBp1][xBp1] = 0;
            }

            // edges (x,0) and (x,ymax)
            for (x = 1; x <= mXBound; ++x)
            {
                mBuffer[mSrc][0][x] = this->mBorderValue;
                mBuffer[mDst][0][x] = this->mBorderValue;
                if (mHasMask)
                {
                    mMask[0][x] = 0;
                }

                mBuffer[mSrc][yBp1][x] = this->mBorderValue;
                mBuffer[mDst][yBp1][x] = this->mBorderValue;
                if (mHasMask)
                {
                    mMask[yBp1][x] = 0;
                }
            }

            // edges (0,y) and (xmax,y)
            for (y = 1; y <= mYBound; ++y)
            {
                mBuffer[mSrc][y][0] = this->mBorderValue;
                mBuffer[mDst][y][0] = this->mBorderValue;
                if (mHasMask)
                {
                    mMask[y][0] = 0;
                }

                mBuffer[mSrc][y][xBp1] = this->mBorderValue;
                mBuffer[mDst][y][xBp1] = this->mBorderValue;
                if (mHasMask)
                {
                    mMask[y][xBp1] = 0;
                }
            }
        }

        void AssignNeumannImageBorder()
        {
            int xBp1 = mXBound + 1, yBp1 = mYBound + 1;
            int x, y;
            Real duplicate;

            // vertex (0,0)
            duplicate = mBuffer[mSrc][1][1];
            mBuffer[mSrc][0][0] = duplicate;
            mBuffer[mDst][0][0] = duplicate;
            if (mHasMask)
            {
                mMask[0][0] = 0;
            }

            // vertex (xmax,0)
            duplicate = mBuffer[mSrc][1][mXBound];
            mBuffer[mSrc][0][xBp1] = duplicate;
            mBuffer[mDst][0][xBp1] = duplicate;
            if (mHasMask)
            {
                mMask[0][xBp1] = 0;
            }

            // vertex (0,ymax)
            duplicate = mBuffer[mSrc][mYBound][1];
            mBuffer[mSrc][yBp1][0] = duplicate;
            mBuffer[mDst][yBp1][0] = duplicate;
            if (mHasMask)
            {
                mMask[yBp1][0] = 0;
            }

            // vertex (xmax,ymax)
            duplicate = mBuffer[mSrc][mYBound][mXBound];
            mBuffer[mSrc][yBp1][xBp1] = duplicate;
            mBuffer[mDst][yBp1][xBp1] = duplicate;
            if (mHasMask)
            {
                mMask[yBp1][xBp1] = 0;
            }

            // edges (x,0) and (x,ymax)
            for (x = 1; x <= mXBound; ++x)
            {
                duplicate = mBuffer[mSrc][1][x];
                mBuffer[mSrc][0][x] = duplicate;
                mBuffer[mDst][0][x] = duplicate;
                if (mHasMask)
                {
                    mMask[0][x] = 0;
                }

                duplicate = mBuffer[mSrc][mYBound][x];
                mBuffer[mSrc][yBp1][x] = duplicate;
                mBuffer[mDst][yBp1][x] = duplicate;
                if (mHasMask)
                {
                    mMask[yBp1][x] = 0;
                }
            }

            // edges (0,y) and (xmax,y)
            for (y = 1; y <= mYBound; ++y)
            {
                duplicate = mBuffer[mSrc][y][1];
                mBuffer[mSrc][y][0] = duplicate;
                mBuffer[mDst][y][0] = duplicate;
                if (mHasMask)
                {
                    mMask[y][0] = 0;
                }

                duplicate = mBuffer[mSrc][y][mXBound];
                mBuffer[mSrc][y][xBp1] = duplicate;
                mBuffer[mDst][y][xBp1] = duplicate;
                if (mHasMask)
                {
                    mMask[y][xBp1] = 0;
                }
            }
        }

        // Assign values to the 1-pixel mask border.
        void AssignDirichletMaskBorder()
        {
            for (int y = 1; y <= mYBound; ++y)
            {
                for (int x = 1; x <= mXBound; ++x)
                {
                    if (mMask[y][x])
                    {
                        continue;
                    }

                    bool found = false;
                    for (int i1 = 0, j1 = y - 1; i1 < 3 && !found; ++i1, ++j1)
                    {
                        for (int i0 = 0, j0 = x - 1; i0 < 3; ++i0, ++j0)
                        {
                            if (mMask[j1][j0])
                            {
                                mBuffer[mSrc][y][x] = this->mBorderValue;
                                mBuffer[mDst][y][x] = this->mBorderValue;
                                found = true;
                                break;
                            }
                        }
                    }
                }
            }
        }

        void AssignNeumannMaskBorder()
        {
            // Recompute the values just outside the masked region.  This
            // guarantees that derivative estimations use the current values
            // around the boundary.
            for (int y = 1; y <= mYBound; ++y)
            {
                for (int x = 1; x <= mXBound; ++x)
                {
                    if (mMask[y][x])
                    {
                        continue;
                    }

                    int count = 0;
                    Real average = (Real)0;
                    for (int i1 = 0, j1 = y - 1; i1 < 3; ++i1, ++j1)
                    {
                        for (int i0 = 0, j0 = x - 1; i0 < 3; ++i0, ++j0)
                        {
                            if (mMask[j1][j0])
                            {
                                average += mBuffer[mSrc][j1][j0];
                                ++count;
                            }
                        }
                    }

                    if (count > 0)
                    {
                        average /= (Real)count;
                        mBuffer[mSrc][y][x] = average;
                        mBuffer[mDst][y][x] = average;
                    }
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

        // Iterate over all the pixels and call OnUpdate(x,y) for each pixel
        // that is not masked out.
        virtual void OnUpdate() override
        {
            for (int y = 1; y <= mYBound; ++y)
            {
                for (int x = 1; x <= mXBound; ++x)
                {
                    if (!mHasMask || mMask[y][x])
                    {
                        OnUpdateSingle(x, y);
                    }
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

        // The per-pixel processing depends on the PDE algorithm.  The (x,y)
        // must be in padded coordinates: 1 <= x <= xbound and
        // 1 <= y <= ybound.
        virtual void OnUpdateSingle(int x, int y) = 0;

        // Copy source data to temporary storage.
        void LookUp5(int x, int y)
        {
            auto const& F = mBuffer[mSrc];
            int xm = x - 1, xp = x + 1;
            int ym = y - 1, yp = y + 1;
            mUzm = F[ym][x];
            mUmz = F[y][xm];
            mUzz = F[y][x];
            mUpz = F[y][xp];
            mUzp = F[yp][x];
        }

        void LookUp9(int x, int y)
        {
            auto const& F = mBuffer[mSrc];
            int xm = x - 1, xp = x + 1;
            int ym = y - 1, yp = y + 1;
            mUmm = F[ym][xm];
            mUzm = F[ym][x];
            mUpm = F[ym][xp];
            mUmz = F[y][xm];
            mUzz = F[y][x];
            mUpz = F[y][xp];
            mUmp = F[yp][xm];
            mUzp = F[yp][x];
            mUpp = F[yp][xp];
        }

        // Image parameters.
        int mXBound, mYBound;
        Real mXSpacing;       // dx
        Real mYSpacing;       // dy
        Real mInvDx;          // 1/dx
        Real mInvDy;          // 1/dy
        Real mHalfInvDx;      // 1/(2*dx)
        Real mHalfInvDy;      // 1/(2*dy)
        Real mInvDxDx;        // 1/(dx*dx)
        Real mFourthInvDxDy;  // 1/(4*dx*dy)
        Real mInvDyDy;        // 1/(dy*dy)

        // Temporary storage for 3x3 neighborhood.  In the notation mUxy, the
        // x and y indices are in {m,z,p}, referring to subtract 1 (m), no
        // change (z), or add 1 (p) to the appropriate index.
        Real mUmm, mUzm, mUpm;
        Real mUmz, mUzz, mUpz;
        Real mUmp, mUzp, mUpp;

        // Successive iterations toggle between two buffers.
        std::array<Array2<Real>, 2> mBuffer;
        int mSrc, mDst;
        Array2<int> mMask;
        bool mHasMask;
    };
}
