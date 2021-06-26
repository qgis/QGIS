// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.08

#pragma once

#include <Mathematics/Vector4.h>

// The plane is defined by Dot((n0,n1,n2,c),(x0,x1,x2,1)) = 0, where a
// plane normal is N = (n0,n1,n2,0), c is a constant, and X = (x0,x1,x2,1)
// is a point on the plane.  If P = (p0,p1,p2,1) is a point on the plane, then
// Dot(N,X-P) = 0, in which case c = -Dot(N,P).  If P0, P1, and P2 are points
// on the plane and are not collinear, then N = Cross(P1-P0,P2-P0) and
// c = -Dot(N,P0).

namespace gte
{
    template <typename Real>
    class CullingPlane
    {
    public:
        // Construction and destruction.  The destructor hides the base-class
        // destructor, but the latter has no side effects.
        ~CullingPlane()
        {
        }

        CullingPlane()
            :
            mTuple{ (Real)0, (Real)0, (Real)0, (Real)0 }
        {
        }

        CullingPlane(CullingPlane const& plane)
            :
            mTuple(plane.mTuple)
        {
        }

        CullingPlane(Vector4<Real> const& N, Real c)
            :
            mTuple{ N[0], N[1], N[2], c }
        {
        }

        CullingPlane(Real n0, Real n1, Real n2, Real c)
            :
            mTuple{ n0, n1, n2, c }
        {
        }

        CullingPlane(Vector4<Real> const& N, Vector4<Real> const& P)
            :
            mTuple{ N[0], N[1], N[2], -Dot(N, P) }
        {
        }

        CullingPlane(Vector4<Real> const& P0, Vector4<Real> const& P1, Vector4<Real> const& P2)
        {
            Vector4<Real> edge1 = P1 - P0;
            Vector4<Real> edge2 = P2 - P0;
            Vector4<Real> N = Cross(edge1, edge2);
            mTuple[0] = N[0];
            mTuple[1] = N[1];
            mTuple[2] = N[2];
            mTuple[3] = -Dot(N, P0);
        }

        // Assignment.
        CullingPlane& operator= (CullingPlane const& plane)
        {
            mTuple = plane.mTuple;
            return *this;
        }

        // Member access.  Because N and c are interdependent, there are no
        // accessors to set N or c individually.
        void Set(Vector4<Real> const& N, Real c)
        {
            mTuple[0] = N[0];
            mTuple[1] = N[1];
            mTuple[2] = N[2];
            mTuple[3] = c;
        }

        void Get(Vector4<Real>& N, Real& c) const
        {
            N[0] = mTuple[0];
            N[1] = mTuple[1];
            N[2] = mTuple[2];
            c = mTuple[3];
        }

        Vector4<Real> GetNormal() const
        {
            return Vector4<Real>{ mTuple[0], mTuple[1], mTuple[2], (Real)0 };
        }

        Real GetConstant() const
        {
            return mTuple[3];
        }

        // Compute L = Length(n0,n1,n2) and set the plane to (n0,n1,n2,c)/L.
        // This is useful when transforming planes by homogeneous matrices.
        // The function returns L.
        Real Normalize()
        {
            Real length = std::sqrt(mTuple[0] * mTuple[0] + mTuple[1] * mTuple[1] + mTuple[2] * mTuple[2]);
            mTuple /= length;
            return length;
        }

        // The "positive side" of the plane is the half space to which the
        // plane normal is directed.  The "negative side" is the other half
        // space.  The function returns +1 when P is on the positive side,
        // -1 when P is on the negative side, or 0 when P is on the plane.
        int WhichSide(Vector4<Real> const& P) const
        {
            Real distance = Dot(mTuple, P);
            return (distance > (Real)0 ? +1 : (distance < (Real)0 ? -1 : 0));
        }

        // Compute d = Dot(N,P)+c where N is the plane normal and c is the
        // plane constant.  This is a signed pseudodistance.  The sign of the
        // return value is consistent with that in the comments for
        // WhichSide(...).
        Real DistanceTo(Vector4<Real> const& P) const
        {
            return Dot(mTuple, P);
        }

    private:
        Vector4<Real> mTuple;
    };
}
