// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/RiemannianGeodesic.h>

namespace gte
{
    template <typename Real>
    class EllipsoidGeodesic : public RiemannianGeodesic<Real>
    {
    public:
        // The ellipsoid is (x/a)^2 + (y/b)^2 + (z/c)^2 = 1, where xExtent is
        // 'a', yExtent is 'b', and zExtent is 'c'. The surface is represented
        // parametrically by angles u and v, say
        //   P(u,v) = (x(u,v),y(u,v),z(u,v)),
        //   P(u,v) =(a*cos(u)*sin(v), b*sin(u)*sin(v), c*cos(v))
        // with 0 <= u < 2*pi and 0 <= v <= pi.  The first-order derivatives
        // are
        //   dP/du = (-a*sin(u)*sin(v), b*cos(u)*sin(v), 0)
        //   dP/dv = (a*cos(u)*cos(v), b*sin(u)*cos(v), -c*sin(v))
        // The metric tensor elements are
        //   g_{00} = Dot(dP/du,dP/du)
        //   g_{01} = Dot(dP/du,dP/dv)
        //   g_{10} = g_{01}
        //   g_{11} = Dot(dP/dv,dP/dv)

        EllipsoidGeodesic(Real xExtent, Real yExtent, Real zExtent)
            :
            RiemannianGeodesic<Real>(2),
            mXExtent(xExtent),
            mYExtent(yExtent),
            mZExtent(zExtent)
        {
        }

        virtual ~EllipsoidGeodesic()
        {
        }

        Vector3<Real> ComputePosition(GVector<Real> const& point)
        {
            Real cos0 = std::cos(point[0]);
            Real sin0 = std::sin(point[0]);
            Real cos1 = std::cos(point[1]);
            Real sin1 = std::sin(point[1]);

            return Vector3<Real>
            {
                mXExtent * cos0 * sin1,
                mYExtent * sin0 * sin1,
                mZExtent * cos1
            };
        }

        // To compute the geodesic path connecting two parameter points
        // (u0,v0) and (u1,v1):
        //
        // float a, b, c;  // the extents of the ellipsoid
        // EllipsoidGeodesic<float> EG(a,b,c);
        // GVector<float> param0(2), param1(2);
        // param0[0] = u0;
        // param0[1] = v0;
        // param1[0] = u1;
        // param1[1] = v1;
        //
        // int quantity;
        // std:vector<GVector<float>> path;
        // EG.ComputeGeodesic(param0, param1, quantity, path);

    private:
        virtual void ComputeMetric(GVector<Real> const& point) override
        {
            mCos0 = std::cos(point[0]);
            mSin0 = std::sin(point[0]);
            mCos1 = std::cos(point[1]);
            mSin1 = std::sin(point[1]);

            mDer0 = { -mXExtent * mSin0 * mSin1, mYExtent * mCos0 * mSin1, (Real)0 };
            mDer1 = { mXExtent * mCos0 * mCos1, mYExtent * mSin0 * mCos1, -mZExtent * mSin1 };

            this->mMetric(0, 0) = Dot(mDer0, mDer0);
            this->mMetric(0, 1) = Dot(mDer0, mDer1);
            this->mMetric(1, 0) = this->mMetric(0, 1);
            this->mMetric(1, 1) = Dot(mDer1, mDer1);
        }

        virtual void ComputeChristoffel1(GVector<Real> const&) override
        {
            Vector3<Real> der00
            {
                -mXExtent * mCos0 * mSin1,
                -mYExtent * mSin0 * mSin1,
                (Real)0
            };

            Vector3<Real> der01
            {
                -mXExtent * mSin0 * mCos1,
                mYExtent * mCos0 * mCos1,
                (Real)0
            };

            Vector3<Real> der11
            {
                -mXExtent * mCos0 * mSin1,
                -mYExtent * mSin0 * mSin1,
                -mZExtent * mCos1
            };

            this->mChristoffel1[0](0, 0) = Dot(der00, mDer0);
            this->mChristoffel1[0](0, 1) = Dot(der01, mDer0);
            this->mChristoffel1[0](1, 0) = this->mChristoffel1[0](0, 1);
            this->mChristoffel1[0](1, 1) = Dot(der11, mDer0);

            this->mChristoffel1[1](0, 0) = Dot(der00, mDer1);
            this->mChristoffel1[1](0, 1) = Dot(der01, mDer1);
            this->mChristoffel1[1](1, 0) = this->mChristoffel1[1](0, 1);
            this->mChristoffel1[1](1, 1) = Dot(der11, mDer1);
        }

        // The ellipsoid axis half-lengths.
        Real mXExtent, mYExtent, mZExtent;

        // We are guaranteed that RiemannianGeodesic calls ComputeMetric
        // before ComputeChristoffel1.  Thus, we can compute the surface
        // first- and second-order derivatives in ComputeMetric and cache
        // the results for use in ComputeChristoffel1.
        Real mCos0, mSin0, mCos1, mSin1;
        Vector3<Real> mDer0, mDer1;
    };
}
