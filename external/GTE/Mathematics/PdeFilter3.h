// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.11

#pragma once

#include <Mathematics/PdeFilter.h>
#include <Mathematics/Array3.h>
#include <array>
#include <limits>

namespace gte
{
    template <typename Real>
    class PdeFilter3 : public PdeFilter<Real>
    {
    public:
        // Abstract base class.
        PdeFilter3(int xBound, int yBound, int zBound, Real xSpacing, Real ySpacing,
            Real zSpacing, Real const* data, bool const* mask, Real borderValue,
            typename PdeFilter<Real>::ScaleType scaleType)
            :
            PdeFilter<Real>(xBound * yBound * zBound, data, borderValue, scaleType),
            mXBound(xBound),
            mYBound(yBound),
            mZBound(zBound),
            mXSpacing(xSpacing),
            mYSpacing(ySpacing),
            mZSpacing(zSpacing),
            mInvDx((Real)1 / xSpacing),
            mInvDy((Real)1 / ySpacing),
            mInvDz((Real)1 / zSpacing),
            mHalfInvDx((Real)0.5 * mInvDx),
            mHalfInvDy((Real)0.5 * mInvDy),
            mHalfInvDz((Real)0.5 * mInvDz),
            mInvDxDx(mInvDx * mInvDx),
            mFourthInvDxDy(mHalfInvDx * mHalfInvDy),
            mFourthInvDxDz(mHalfInvDx * mHalfInvDz),
            mInvDyDy(mInvDy * mInvDy),
            mFourthInvDyDz(mHalfInvDy * mHalfInvDz),
            mInvDzDz(mInvDz * mInvDz),
            mUmmm(0), mUzmm(0), mUpmm(0),
            mUmzm(0), mUzzm(0), mUpzm(0),
            mUmpm(0), mUzpm(0), mUppm(0),
            mUmmz(0), mUzmz(0), mUpmz(0),
            mUmzz(0), mUzzz(0), mUpzz(0),
            mUmpz(0), mUzpz(0), mUppz(0),
            mUmmp(0), mUzmp(0), mUpmp(0),
            mUmzp(0), mUzzp(0), mUpzp(0),
            mUmpp(0), mUzpp(0), mUppp(0),
            mSrc(0),
            mDst(1),
            mMask(xBound + 2, yBound + 2, zBound + 2),
            mHasMask(mask != nullptr)
        {
            for (int i = 0; i < 2; ++i)
            {
                mBuffer[i] = Array3<Real>(xBound + 2, yBound + 2, zBound + 2);
            }

            // The mBuffer[] are ping-pong buffers for filtering.
            for (int z = 0, zp = 1, i = 0; z < mZBound; ++z, ++zp)
            {
                for (int y = 0, yp = 1; y < mYBound; ++y, ++yp)
                {
                    for (int x = 0, xp = 1; x < mXBound; ++x, ++xp, ++i)
                    {
                        mBuffer[mSrc][zp][yp][xp] = this->mOffset + (data[i] - this->mMin) * this->mScale;
                        mBuffer[mDst][zp][yp][xp] = (Real)0;
                        mMask[zp][yp][xp] = (mHasMask ? mask[i] : 1);
                    }
                }
            }

            // Assign values to the 1-voxel thick border.
            if (this->mBorderValue != std::numeric_limits<Real>::max())
            {
                AssignDirichletImageBorder();
            }
            else
            {
                AssignNeumannImageBorder();
            }

            // To handle masks that do not cover the entire image, assign
            // values to those voxels that are 26-neighbors of the mask
            // voxels.
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

        virtual ~PdeFilter3()
        {
        }

        // Member access.  The internal 2D images for "data" and "mask" are
        // copies of the inputs to the constructor but padded with a 1-voxel
        // thick border to support filtering on the image boundary.  These
        // images are of size (xbound+2)-by-(ybound+2)-by-(zbound+2).  The
        // correct lookups into the padded arrays are handled internally.
        inline int GetXBound() const
        {
            return mXBound;
        }

        inline int GetYBound() const
        {
            return mYBound;
        }

        inline int GetZBound() const
        {
            return mZBound;
        }

        inline Real GetXSpacing() const
        {
            return mXSpacing;
        }

        inline Real GetYSpacing() const
        {
            return mYSpacing;
        }

        inline Real GetZSpacing() const
        {
            return mZSpacing;
        }

        // Voxel access and derivative estimation.  The lookups into the
        // padded data are handled correctly.  The estimation involves only
        // the 3-by-3-by-3 neighborhood of (x,y,z), where 0 <= x < xbound,
        // 0 <= y < ybound and 0 <= z < zbound.  TODO: If larger neighborhoods
        // are desired at a later date, the padding and associated code must
        // be adjusted accordingly.
        Real GetU(int x, int y, int z) const
        {
            auto const& F = mBuffer[mSrc];
            int xp1 = x + 1, yp1 = y + 1, zp1 = z + 1;
            return F[zp1][yp1][xp1];
        }

        Real GetUx(int x, int y, int z) const
        {
            auto const& F = mBuffer[mSrc];
            int xp2 = x + 2, yp1 = y + 1, zp1 = z + 1;
            return mHalfInvDx * (F[zp1][yp1][xp2] - F[zp1][yp1][x]);
        }

        Real GetUy(int x, int y, int z) const
        {
            auto const& F = mBuffer[mSrc];
            int xp1 = x + 1, yp2 = y + 2, zp1 = z + 1;
            return mHalfInvDy * (F[zp1][yp2][xp1] - F[zp1][y][xp1]);
        }

        Real GetUz(int x, int y, int z) const
        {
            auto const& F = mBuffer[mSrc];
            int xp1 = x + 1, yp1 = y + 1, zp2 = z + 2;
            return mHalfInvDz * (F[zp2][yp1][xp1] - F[z][yp1][xp1]);
        }

        Real GetUxx(int x, int y, int z) const
        {
            auto const& F = mBuffer[mSrc];
            int xp1 = x + 1, xp2 = x + 2, yp1 = y + 1, zp1 = z + 1;
            return mInvDxDx * (F[zp1][yp1][xp2] - (Real)2 * F[zp1][yp1][xp1] + F[zp1][yp1][x]);
        }

        Real GetUxy(int x, int y, int z) const
        {
            auto const& F = mBuffer[mSrc];
            int xp2 = x + 2, yp2 = y + 2, zp1 = z + 1;
            return mFourthInvDxDy * (F[zp1][y][x] - F[zp1][y][xp2] + F[zp1][yp2][xp2] - F[zp1][yp2][x]);
        }

        Real GetUxz(int x, int y, int z) const
        {
            auto const& F = mBuffer[mSrc];
            int xp2 = x + 2, yp1 = y + 1, zp2 = z + 2;
            return mFourthInvDxDz * (F[z][yp1][x] - F[z][yp1][xp2] + F[zp2][yp1][xp2] - F[zp2][yp1][x]);
        }

        Real GetUyy(int x, int y, int z) const
        {
            auto const& F = mBuffer[mSrc];
            int xp1 = x + 1, yp1 = y + 1, yp2 = y + 2, zp1 = z + 1;
            return mInvDyDy * (F[zp1][yp2][xp1] - (Real)2 * F[zp1][yp1][zp1] + F[zp1][y][xp1]);
        }

        Real GetUyz(int x, int y, int z) const
        {
            auto const& F = mBuffer[mSrc];
            int xp1 = x + 1, yp2 = y + 2, zp2 = z + 2;
            return mFourthInvDyDz * (F[z][y][xp1] - F[z][yp2][xp1] + F[zp2][yp2][xp1] - F[zp2][y][xp1]);
        }

        Real GetUzz(int x, int y, int z) const
        {
            auto const& F = mBuffer[mSrc];
            int xp1 = x + 1, yp1 = y + 1, zp1 = z + 1, zp2 = z + 2;
            return mInvDzDz * (F[zp2][yp1][xp1] - (Real)2 * F[zp1][yp1][xp1] + F[z][yp1][xp1]);
        }

        int GetMask(int x, int y, int z) const
        {
            int xp1 = x + 1, yp1 = y + 1, zp1 = z + 1;
            return mMask[zp1][yp1][xp1];
        }

    protected:
        // Assign values to the 1-voxel image border.
        void AssignDirichletImageBorder()
        {
            int xBp1 = mXBound + 1, yBp1 = mYBound + 1, zBp1 = mZBound + 1;
            int x, y, z;

            // vertex (0,0,0)
            mBuffer[mSrc][0][0][0] = this->mBorderValue;
            mBuffer[mDst][0][0][0] = this->mBorderValue;
            if (mHasMask)
            {
                mMask[0][0][0] = 0;
            }

            // vertex (xmax,0,0)
            mBuffer[mSrc][0][0][xBp1] = this->mBorderValue;
            mBuffer[mDst][0][0][xBp1] = this->mBorderValue;
            if (mHasMask)
            {
                mMask[0][0][xBp1] = 0;
            }

            // vertex (0,ymax,0)
            mBuffer[mSrc][0][yBp1][0] = this->mBorderValue;
            mBuffer[mDst][0][yBp1][0] = this->mBorderValue;
            if (mHasMask)
            {
                mMask[0][yBp1][0] = 0;
            }

            // vertex (xmax,ymax,0)
            mBuffer[mSrc][0][yBp1][xBp1] = this->mBorderValue;
            mBuffer[mDst][0][yBp1][xBp1] = this->mBorderValue;
            if (mHasMask)
            {
                mMask[0][yBp1][xBp1] = 0;
            }

            // vertex (0,0,zmax)
            mBuffer[mSrc][zBp1][0][0] = this->mBorderValue;
            mBuffer[mDst][zBp1][0][0] = this->mBorderValue;
            if (mHasMask)
            {
                mMask[zBp1][0][0] = 0;
            }

            // vertex (xmax,0,zmax)
            mBuffer[mSrc][zBp1][0][xBp1] = this->mBorderValue;
            mBuffer[mDst][zBp1][0][xBp1] = this->mBorderValue;
            if (mHasMask)
            {
                mMask[zBp1][0][xBp1] = 0;
            }

            // vertex (0,ymax,zmax)
            mBuffer[mSrc][zBp1][yBp1][0] = this->mBorderValue;
            mBuffer[mDst][zBp1][yBp1][0] = this->mBorderValue;
            if (mHasMask)
            {
                mMask[zBp1][yBp1][0] = 0;
            }

            // vertex (xmax,ymax,zmax)
            mBuffer[mSrc][zBp1][yBp1][xBp1] = this->mBorderValue;
            mBuffer[mDst][zBp1][yBp1][xBp1] = this->mBorderValue;
            if (mHasMask)
            {
                mMask[zBp1][yBp1][xBp1] = 0;
            }

            // edges (x,0,0) and (x,ymax,0)
            for (x = 1; x <= mXBound; ++x)
            {
                mBuffer[mSrc][0][0][x] = this->mBorderValue;
                mBuffer[mDst][0][0][x] = this->mBorderValue;
                if (mHasMask)
                {
                    mMask[0][0][x] = 0;
                }

                mBuffer[mSrc][0][yBp1][x] = this->mBorderValue;
                mBuffer[mDst][0][yBp1][x] = this->mBorderValue;
                if (mHasMask)
                {
                    mMask[0][yBp1][x] = 0;
                }
            }

            // edges (0,y,0) and (xmax,y,0)
            for (y = 1; y <= mYBound; ++y)
            {
                mBuffer[mSrc][0][y][0] = this->mBorderValue;
                mBuffer[mDst][0][y][0] = this->mBorderValue;
                if (mHasMask)
                {
                    mMask[0][y][0] = 0;
                }

                mBuffer[mSrc][0][y][xBp1] = this->mBorderValue;
                mBuffer[mDst][0][y][xBp1] = this->mBorderValue;
                if (mHasMask)
                {
                    mMask[0][y][xBp1] = 0;
                }
            }

            // edges (x,0,zmax) and (x,ymax,zmax)
            for (x = 1; x <= mXBound; ++x)
            {
                mBuffer[mSrc][zBp1][0][x] = this->mBorderValue;
                mBuffer[mDst][zBp1][0][x] = this->mBorderValue;
                if (mHasMask)
                {
                    mMask[zBp1][0][x] = 0;
                }

                mBuffer[mSrc][zBp1][yBp1][x] = this->mBorderValue;
                mBuffer[mDst][zBp1][yBp1][x] = this->mBorderValue;
                if (mHasMask)
                {
                    mMask[zBp1][yBp1][x] = 0;
                }
            }

            // edges (0,y,zmax) and (xmax,y,zmax)
            for (y = 1; y <= mYBound; ++y)
            {
                mBuffer[mSrc][zBp1][y][0] = this->mBorderValue;
                mBuffer[mDst][zBp1][y][0] = this->mBorderValue;
                if (mHasMask)
                {
                    mMask[zBp1][y][0] = 0;
                }

                mBuffer[mSrc][zBp1][y][xBp1] = this->mBorderValue;
                mBuffer[mDst][zBp1][y][xBp1] = this->mBorderValue;
                if (mHasMask)
                {
                    mMask[zBp1][y][xBp1] = 0;
                }
            }

            // edges (0,0,z) and (xmax,0,z)
            for (z = 1; z <= mZBound; ++z)
            {
                mBuffer[mSrc][z][0][0] = this->mBorderValue;
                mBuffer[mDst][z][0][0] = this->mBorderValue;
                if (mHasMask)
                {
                    mMask[z][0][0] = 0;
                }

                mBuffer[mSrc][z][0][xBp1] = this->mBorderValue;
                mBuffer[mDst][z][0][xBp1] = this->mBorderValue;
                if (mHasMask)
                {
                    mMask[z][0][xBp1] = 0;
                }
            }

            // edges (0,ymax,z) and (xmax,ymax,z)
            for (z = 1; z <= mZBound; ++z)
            {
                mBuffer[mSrc][z][yBp1][0] = this->mBorderValue;
                mBuffer[mDst][z][yBp1][0] = this->mBorderValue;
                if (mHasMask)
                {
                    mMask[z][yBp1][0] = 0;
                }

                mBuffer[mSrc][z][yBp1][xBp1] = this->mBorderValue;
                mBuffer[mDst][z][yBp1][xBp1] = this->mBorderValue;
                if (mHasMask)
                {
                    mMask[z][yBp1][xBp1] = 0;
                }
            }

            // faces (x,y,0) and (x,y,zmax)
            for (y = 1; y <= mYBound; ++y)
            {
                for (x = 1; x <= mXBound; ++x)
                {
                    mBuffer[mSrc][0][y][x] = this->mBorderValue;
                    mBuffer[mDst][0][y][x] = this->mBorderValue;
                    if (mHasMask)
                    {
                        mMask[0][y][x] = 0;
                    }

                    mBuffer[mSrc][zBp1][y][x] = this->mBorderValue;
                    mBuffer[mDst][zBp1][y][x] = this->mBorderValue;
                    if (mHasMask)
                    {
                        mMask[zBp1][y][x] = 0;
                    }
                }
            }

            // faces (x,0,z) and (x,ymax,z)
            for (z = 1; z <= mZBound; ++z)
            {
                for (x = 1; x <= mXBound; ++x)
                {
                    mBuffer[mSrc][z][0][x] = this->mBorderValue;
                    mBuffer[mDst][z][0][x] = this->mBorderValue;
                    if (mHasMask)
                    {
                        mMask[z][0][x] = 0;
                    }

                    mBuffer[mSrc][z][yBp1][x] = this->mBorderValue;
                    mBuffer[mDst][z][yBp1][x] = this->mBorderValue;
                    if (mHasMask)
                    {
                        mMask[z][yBp1][x] = 0;
                    }
                }
            }

            // faces (0,y,z) and (xmax,y,z)
            for (z = 1; z <= mZBound; ++z)
            {
                for (y = 1; y <= mYBound; ++y)
                {
                    mBuffer[mSrc][z][y][0] = this->mBorderValue;
                    mBuffer[mDst][z][y][0] = this->mBorderValue;
                    if (mHasMask)
                    {
                        mMask[z][y][0] = 0;
                    }

                    mBuffer[mSrc][z][y][xBp1] = this->mBorderValue;
                    mBuffer[mDst][z][y][xBp1] = this->mBorderValue;
                    if (mHasMask)
                    {
                        mMask[z][y][xBp1] = 0;
                    }
                }
            }
        }

        void AssignNeumannImageBorder()
        {
            int xBp1 = mXBound + 1, yBp1 = mYBound + 1, zBp1 = mZBound + 1;
            int x, y, z;
            Real duplicate;

            // vertex (0,0,0)
            duplicate = mBuffer[mSrc][1][1][1];
            mBuffer[mSrc][0][0][0] = duplicate;
            mBuffer[mDst][0][0][0] = duplicate;
            if (mHasMask)
            {
                mMask[0][0][0] = 0;
            }

            // vertex (xmax,0,0)
            duplicate = mBuffer[mSrc][1][1][mXBound];
            mBuffer[mSrc][0][0][xBp1] = duplicate;
            mBuffer[mDst][0][0][xBp1] = duplicate;
            if (mHasMask)
            {
                mMask[0][0][xBp1] = 0;
            }

            // vertex (0,ymax,0)
            duplicate = mBuffer[mSrc][1][mYBound][1];
            mBuffer[mSrc][0][yBp1][0] = duplicate;
            mBuffer[mDst][0][yBp1][0] = duplicate;
            if (mHasMask)
            {
                mMask[0][yBp1][0] = 0;
            }

            // vertex (xmax,ymax,0)
            duplicate = mBuffer[mSrc][1][mYBound][mXBound];
            mBuffer[mSrc][0][yBp1][xBp1] = duplicate;
            mBuffer[mDst][0][yBp1][xBp1] = duplicate;
            if (mHasMask)
            {
                mMask[0][yBp1][xBp1] = 0;
            }

            // vertex (0,0,zmax)
            duplicate = mBuffer[mSrc][mZBound][1][1];
            mBuffer[mSrc][zBp1][0][0] = duplicate;
            mBuffer[mDst][zBp1][0][0] = duplicate;
            if (mHasMask)
            {
                mMask[zBp1][0][0] = 0;
            }

            // vertex (xmax,0,zmax)
            duplicate = mBuffer[mSrc][mZBound][1][mXBound];
            mBuffer[mSrc][zBp1][0][xBp1] = duplicate;
            mBuffer[mDst][zBp1][0][xBp1] = duplicate;
            if (mHasMask)
            {
                mMask[zBp1][0][xBp1] = 0;
            }

            // vertex (0,ymax,zmax)
            duplicate = mBuffer[mSrc][mZBound][mYBound][1];
            mBuffer[mSrc][zBp1][yBp1][0] = duplicate;
            mBuffer[mDst][zBp1][yBp1][0] = duplicate;
            if (mHasMask)
            {
                mMask[zBp1][yBp1][0] = 0;
            }

            // vertex (xmax,ymax,zmax)
            duplicate = mBuffer[mSrc][mZBound][mYBound][mXBound];
            mBuffer[mSrc][zBp1][yBp1][xBp1] = duplicate;
            mBuffer[mDst][zBp1][yBp1][xBp1] = duplicate;
            if (mHasMask)
            {
                mMask[zBp1][yBp1][xBp1] = 0;
            }

            // edges (x,0,0) and (x,ymax,0)
            for (x = 1; x <= mXBound; ++x)
            {
                duplicate = mBuffer[mSrc][1][1][x];
                mBuffer[mSrc][0][0][x] = duplicate;
                mBuffer[mDst][0][0][x] = duplicate;
                if (mHasMask)
                {
                    mMask[0][0][x] = 0;
                }

                duplicate = mBuffer[mSrc][1][mYBound][x];
                mBuffer[mSrc][0][yBp1][x] = duplicate;
                mBuffer[mDst][0][yBp1][x] = duplicate;
                if (mHasMask)
                {
                    mMask[0][yBp1][x] = 0;
                }
            }

            // edges (0,y,0) and (xmax,y,0)
            for (y = 1; y <= mYBound; ++y)
            {
                duplicate = mBuffer[mSrc][1][y][1];
                mBuffer[mSrc][0][y][0] = duplicate;
                mBuffer[mDst][0][y][0] = duplicate;
                if (mHasMask)
                {
                    mMask[0][y][0] = 0;
                }

                duplicate = mBuffer[mSrc][1][y][mXBound];
                mBuffer[mSrc][0][y][xBp1] = duplicate;
                mBuffer[mDst][0][y][xBp1] = duplicate;
                if (mHasMask)
                {
                    mMask[0][y][xBp1] = 0;
                }
            }

            // edges (x,0,zmax) and (x,ymax,zmax)
            for (x = 1; x <= mXBound; ++x)
            {
                duplicate = mBuffer[mSrc][mZBound][1][x];
                mBuffer[mSrc][zBp1][0][x] = duplicate;
                mBuffer[mDst][zBp1][0][x] = duplicate;
                if (mHasMask)
                {
                    mMask[zBp1][0][x] = 0;
                }

                duplicate = mBuffer[mSrc][mZBound][mYBound][x];
                mBuffer[mSrc][zBp1][yBp1][x] = duplicate;
                mBuffer[mDst][zBp1][yBp1][x] = duplicate;
                if (mHasMask)
                {
                    mMask[zBp1][yBp1][x] = 0;
                }
            }

            // edges (0,y,zmax) and (xmax,y,zmax)
            for (y = 1; y <= mYBound; ++y)
            {
                duplicate = mBuffer[mSrc][mZBound][y][1];
                mBuffer[mSrc][zBp1][y][0] = duplicate;
                mBuffer[mDst][zBp1][y][0] = duplicate;
                if (mHasMask)
                {
                    mMask[zBp1][y][0] = 0;
                }

                duplicate = mBuffer[mSrc][mZBound][y][mXBound];
                mBuffer[mSrc][zBp1][y][xBp1] = duplicate;
                mBuffer[mDst][zBp1][y][xBp1] = duplicate;
                if (mHasMask)
                {
                    mMask[zBp1][y][xBp1] = 0;
                }
            }

            // edges (0,0,z) and (xmax,0,z)
            for (z = 1; z <= mZBound; ++z)
            {
                duplicate = mBuffer[mSrc][z][1][1];
                mBuffer[mSrc][z][0][0] = duplicate;
                mBuffer[mDst][z][0][0] = duplicate;
                if (mHasMask)
                {
                    mMask[z][0][0] = 0;
                }

                duplicate = mBuffer[mSrc][z][1][mXBound];
                mBuffer[mSrc][z][0][xBp1] = duplicate;
                mBuffer[mDst][z][0][xBp1] = duplicate;
                if (mHasMask)
                {
                    mMask[z][0][xBp1] = 0;
                }
            }

            // edges (0,ymax,z) and (xmax,ymax,z)
            for (z = 1; z <= mZBound; ++z)
            {
                duplicate = mBuffer[mSrc][z][mYBound][1];
                mBuffer[mSrc][z][yBp1][0] = duplicate;
                mBuffer[mDst][z][yBp1][0] = duplicate;
                if (mHasMask)
                {
                    mMask[z][yBp1][0] = 0;
                }

                duplicate = mBuffer[mSrc][z][mYBound][mXBound];
                mBuffer[mSrc][z][yBp1][xBp1] = duplicate;
                mBuffer[mDst][z][yBp1][xBp1] = duplicate;
                if (mHasMask)
                {
                    mMask[z][yBp1][xBp1] = 0;
                }
            }

            // faces (x,y,0) and (x,y,zmax)
            for (y = 1; y <= mYBound; ++y)
            {
                for (x = 1; x <= mXBound; ++x)
                {
                    duplicate = mBuffer[mSrc][1][y][x];
                    mBuffer[mSrc][0][y][x] = duplicate;
                    mBuffer[mDst][0][y][x] = duplicate;
                    if (mHasMask)
                    {
                        mMask[0][y][x] = 0;
                    }

                    duplicate = mBuffer[mSrc][mZBound][y][x];
                    mBuffer[mSrc][zBp1][y][x] = duplicate;
                    mBuffer[mDst][zBp1][y][x] = duplicate;
                    if (mHasMask)
                    {
                        mMask[zBp1][y][x] = 0;
                    }
                }
            }

            // faces (x,0,z) and (x,ymax,z)
            for (z = 1; z <= mZBound; ++z)
            {
                for (x = 1; x <= mXBound; ++x)
                {
                    duplicate = mBuffer[mSrc][z][1][x];
                    mBuffer[mSrc][z][0][x] = duplicate;
                    mBuffer[mDst][z][0][x] = duplicate;
                    if (mHasMask)
                    {
                        mMask[z][0][x] = 0;
                    }

                    duplicate = mBuffer[mSrc][z][mYBound][x];
                    mBuffer[mSrc][z][yBp1][x] = duplicate;
                    mBuffer[mDst][z][yBp1][x] = duplicate;
                    if (mHasMask)
                    {
                        mMask[z][yBp1][x] = 0;
                    }
                }
            }

            // faces (0,y,z) and (xmax,y,z)
            for (z = 1; z <= mZBound; ++z)
            {
                for (y = 1; y <= mYBound; ++y)
                {
                    duplicate = mBuffer[mSrc][z][y][1];
                    mBuffer[mSrc][z][y][0] = duplicate;
                    mBuffer[mDst][z][y][0] = duplicate;
                    if (mHasMask)
                    {
                        mMask[z][y][0] = 0;
                    }

                    duplicate = mBuffer[mSrc][z][y][mXBound];
                    mBuffer[mSrc][z][y][xBp1] = duplicate;
                    mBuffer[mDst][z][y][xBp1] = duplicate;
                    if (mHasMask)
                    {
                        mMask[z][y][xBp1] = 0;
                    }
                }
            }
        }

        // Assign values to the 1-voxel mask border.
        void AssignDirichletMaskBorder()
        {
            for (int z = 1; z <= mZBound; ++z)
            {
                for (int y = 1; y <= mYBound; ++y)
                {
                    for (int x = 1; x <= mXBound; ++x)
                    {
                        if (mMask[z][y][x])
                        {
                            continue;
                        }

                        bool found = false;
                        for (int i2 = 0, j2 = z - 1; i2 < 3 && !found; ++i2, ++j2)
                        {
                            for (int i1 = 0, j1 = y - 1; i1 < 3 && !found; ++i1, ++j1)
                            {
                                for (int i0 = 0, j0 = x - 1; i0 < 3; ++i0, ++j0)
                                {
                                    if (mMask[j2][j1][j0])
                                    {
                                        mBuffer[mSrc][z][y][x] = this->mBorderValue;
                                        mBuffer[mDst][z][y][x] = this->mBorderValue;
                                        found = true;
                                        break;
                                    }
                                }
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
            for (int z = 1; z <= mZBound; ++z)
            {
                for (int y = 1; y <= mYBound; ++y)
                {
                    for (int x = 1; x <= mXBound; ++x)
                    {
                        if (mMask[z][y][x])
                        {
                            continue;
                        }

                        int count = 0;
                        Real average = (Real)0;
                        for (int i2 = 0, j2 = z - 1; i2 < 3; ++i2, ++j2)
                        {
                            for (int i1 = 0, j1 = y - 1; i1 < 3; ++i1, ++j1)
                            {
                                for (int i0 = 0, j0 = x - 1; i0 < 3; ++i0, ++j0)
                                {
                                    if (mMask[j2][j1][j0])
                                    {
                                        average += mBuffer[mSrc][j2][j1][j0];
                                        count++;
                                    }
                                }
                            }
                        }

                        if (count > 0)
                        {
                            average /= (Real)count;
                            mBuffer[mSrc][z][y][x] = average;
                            mBuffer[mDst][z][y][x] = average;
                        }
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
                // mask border.
                AssignNeumannMaskBorder();
            }
            // else: No mask has been specified or Dirichlet boundary
            // conditions are in use.  Nothing to do.
        }

        // Iterate over all the pixels and call OnUpdate(x,y,z) for each voxel
        // that is not masked out.
        virtual void OnUpdate() override
        {
            for (int z = 1; z <= mZBound; ++z)
            {
                for (int y = 1; y <= mYBound; ++y)
                {
                    for (int x = 1; x <= mXBound; ++x)
                    {
                        if (!mHasMask || mMask[z][y][x])
                        {
                            OnUpdateSingle(x, y, z);
                        }
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

        // The per-pixel processing depends on the PDE algorithm.  The (x,y,z)
        // must be in padded coordinates: 1 <= x <= xbound, 1 <= y <= ybound
        // and 1 <= z <= zbound.
        virtual void OnUpdateSingle(int x, int y, int z) = 0;

        // Copy source data to temporary storage.
        void LookUp7(int x, int y, int z)
        {
            auto const& F = mBuffer[mSrc];
            int xm = x - 1, xp = x + 1;
            int ym = y - 1, yp = y + 1;
            int zm = z - 1, zp = z + 1;
            mUzzm = F[zm][y][x];
            mUzmz = F[z][ym][x];
            mUmzz = F[z][y][xm];
            mUzzz = F[z][y][x];
            mUpzz = F[z][y][xp];
            mUzpz = F[z][yp][x];
            mUzzp = F[zp][y][x];
        }

        void LookUp27(int x, int y, int z)
        {
            auto const& F = mBuffer[mSrc];
            int xm = x - 1, xp = x + 1;
            int ym = y - 1, yp = y + 1;
            int zm = z - 1, zp = z + 1;
            mUmmm = F[zm][ym][xm];
            mUzmm = F[zm][ym][x];
            mUpmm = F[zm][ym][xp];
            mUmzm = F[zm][y][xm];
            mUzzm = F[zm][y][x];
            mUpzm = F[zm][y][xp];
            mUmpm = F[zm][yp][xm];
            mUzpm = F[zm][yp][x];
            mUppm = F[zm][yp][xp];
            mUmmz = F[z][ym][xm];
            mUzmz = F[z][ym][x];
            mUpmz = F[z][ym][xp];
            mUmzz = F[z][y][xm];
            mUzzz = F[z][y][x];
            mUpzz = F[z][y][xp];
            mUmpz = F[z][yp][xm];
            mUzpz = F[z][yp][x];
            mUppz = F[z][yp][xp];
            mUmmp = F[zp][ym][xm];
            mUzmp = F[zp][ym][x];
            mUpmp = F[zp][ym][xp];
            mUmzp = F[zp][y][xm];
            mUzzp = F[zp][y][x];
            mUpzp = F[zp][y][xp];
            mUmpp = F[zp][yp][xm];
            mUzpp = F[zp][yp][x];
            mUppp = F[zp][yp][xp];
        }

        // Image parameters.
        int mXBound, mYBound, mZBound;
        Real mXSpacing;       // dx
        Real mYSpacing;       // dy
        Real mZSpacing;       // dz
        Real mInvDx;          // 1/dx
        Real mInvDy;          // 1/dy
        Real mInvDz;          // 1/dz
        Real mHalfInvDx;      // 1/(2*dx)
        Real mHalfInvDy;      // 1/(2*dy)
        Real mHalfInvDz;      // 1/(2*dz)
        Real mInvDxDx;        // 1/(dx*dx)
        Real mFourthInvDxDy;  // 1/(4*dx*dy)
        Real mFourthInvDxDz;  // 1/(4*dx*dz)
        Real mInvDyDy;        // 1/(dy*dy)
        Real mFourthInvDyDz;  // 1/(4*dy*dz)
        Real mInvDzDz;        // 1/(dz*dz)

        // Temporary storage for 3x3x3 neighborhood.  In the notation mUxyz,
        // the x, y and z indices are in {m,z,p}, referring to subtract 1 (m),
        // no change (z), or add 1 (p) to the appropriate index.
        Real mUmmm, mUzmm, mUpmm;
        Real mUmzm, mUzzm, mUpzm;
        Real mUmpm, mUzpm, mUppm;
        Real mUmmz, mUzmz, mUpmz;
        Real mUmzz, mUzzz, mUpzz;
        Real mUmpz, mUzpz, mUppz;
        Real mUmmp, mUzmp, mUpmp;
        Real mUmzp, mUzzp, mUpzp;
        Real mUmpp, mUzpp, mUppp;

        // Successive iterations toggle between two buffers.
        std::array<Array3<Real>, 2> mBuffer;
        int mSrc, mDst;
        Array3<int> mMask;
        bool mHasMask;
    };
}
