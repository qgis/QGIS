// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Vector3.h>

// Orthogonal frustum.  Let E be the origin, D be the direction vector, U be
// the up vector, and R be the right vector.  Let u > 0 and r > 0 be the
// extents in the U and R directions, respectively.  Let n and f be the
// extents in the D direction with 0 < n < f.  The four corners of the frustum
// in the near plane are E + n*D + s0*u*U + s1*r*R where |s0| = |s1| = 1 (four
// choices).  The four corners of the frustum in the far plane are
// E + f*D + (f/n)*(s0*u*U + s1*r*R) where |s0| = |s1| = 1 (four choices).

namespace gte
{
    template <typename Real>
    class Frustum3
    {
    public:
        // Construction and destruction.  The default constructor sets the
        // following values:  origin (E) to (0,0,0), dVector (D) to (0,0,1),
        // uVector (U) to (0,1,0), rVector (R) to (1,0,0), dMin (n) to 1,
        // dMax (f) to 2, uBound (u) to 1, and rBound (r) to 1.
        Frustum3()
            :
            origin(Vector3<Real>::Zero()),
            dVector(Vector3<Real>::Unit(2)),
            uVector(Vector3<Real>::Unit(1)),
            rVector(Vector3<Real>::Unit(0)),
            dMin((Real)1),
            dMax((Real)2),
            uBound((Real)1),
            rBound((Real)1)
        {
            Update();
        }

        Frustum3(Vector3<Real> const& inOrigin, Vector3<Real> const& inDVector,
            Vector3<Real> const& inUVector, Vector3<Real> const& inRVector,
            Real inDMin, Real inDMax, Real inUBound, Real inRBound)
            :
            origin(inOrigin),
            dVector(inDVector),
            uVector(inUVector),
            rVector(inRVector),
            dMin(inDMin),
            dMax(inDMax),
            uBound(inUBound),
            rBound(inRBound)
        {
            Update();
        }

        // The Update() function must be called whenever changes are made to
        // dMin, dMax, uBound or rBound.  The values mDRatio, mMTwoUF and
        // mMTwoRF are dependent on the changes, so call the Get*() accessors
        // only after the Update() call.
        void Update()
        {
            mDRatio = dMax / dMin;
            mMTwoUF = (Real)-2 * uBound * dMax;
            mMTwoRF = (Real)-2 * rBound * dMax;
        }

        inline Real GetDRatio() const
        {
            return mDRatio;
        }

        inline Real GetMTwoUF() const
        {
            return mMTwoUF;
        }

        inline Real GetMTwoRF() const
        {
            return mMTwoRF;
        }

        void ComputeVertices(std::array<Vector3<Real>, 8>& vertex) const
        {
            Vector3<Real> dScaled = dMin * dVector;
            Vector3<Real> uScaled = uBound * uVector;
            Vector3<Real> rScaled = rBound * rVector;

            vertex[0] = dScaled - uScaled - rScaled;
            vertex[1] = dScaled - uScaled + rScaled;
            vertex[2] = dScaled + uScaled + rScaled;
            vertex[3] = dScaled + uScaled - rScaled;

            for (int i = 0, ip = 4; i < 4; ++i, ++ip)
            {
                vertex[ip] = origin + mDRatio * vertex[i];
                vertex[i] += origin;
            }
        }

        Vector3<Real> origin, dVector, uVector, rVector;
        Real dMin, dMax, uBound, rBound;

    public:
        // Comparisons to support sorted containers.
        bool operator==(Frustum3 const& frustum) const
        {
            return origin == frustum.origin
                && dVector == frustum.dVector
                && uVector == frustum.uVector
                && rVector == frustum.rVector
                && dMin == frustum.dMin
                && dMax == frustum.dMax
                && uBound == frustum.uBound
                && rBound == frustum.rBound;
        }

        bool operator!=(Frustum3 const& frustum) const
        {
            return !operator==(frustum);
        }

        bool operator< (Frustum3 const& frustum) const
        {
            if (origin < frustum.origin)
            {
                return true;
            }

            if (origin > frustum.origin)
            {
                return false;
            }

            if (dVector < frustum.dVector)
            {
                return true;
            }

            if (dVector > frustum.dVector)
            {
                return false;
            }

            if (uVector < frustum.uVector)
            {
                return true;
            }

            if (uVector > frustum.uVector)
            {
                return false;
            }

            if (rVector < frustum.rVector)
            {
                return true;
            }

            if (rVector > frustum.rVector)
            {
                return false;
            }

            if (dMin < frustum.dMin)
            {
                return true;
            }

            if (dMin > frustum.dMin)
            {
                return false;
            }

            if (dMax < frustum.dMax)
            {
                return true;
            }

            if (dMax > frustum.dMax)
            {
                return false;
            }

            if (uBound < frustum.uBound)
            {
                return true;
            }

            if (uBound > frustum.uBound)
            {
                return false;
            }

            return rBound < frustum.rBound;
        }

        bool operator<=(Frustum3 const& frustum) const
        {
            return !frustum.operator<(*this);
        }

        bool operator> (Frustum3 const& frustum) const
        {
            return frustum.operator<(*this);
        }

        bool operator>=(Frustum3 const& frustum) const
        {
            return !operator<(frustum);
        }

    protected:
        // Quantities derived from the constructor inputs.
        Real mDRatio, mMTwoUF, mMTwoRF;
    };
}
