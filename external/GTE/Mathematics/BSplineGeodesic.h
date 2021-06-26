// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/RiemannianGeodesic.h>
#include <Mathematics/BSplineSurface.h>

namespace gte
{
    template <typename Real>
    class BSplineGeodesic : public RiemannianGeodesic<Real>
    {
    public:
        BSplineGeodesic(BSplineSurface<3, Real> const& spline)
            :
            RiemannianGeodesic<Real>(2),
            mSpline(&spline)
        {
        }

        virtual ~BSplineGeodesic()
        {
        }

    private:
        virtual void ComputeMetric(const GVector<Real>& point) override
        {
            mSpline->Evaluate(point[0], point[1], 2, mJet);
            Vector<3, Real> const& der0 = mJet[1];
            Vector<3, Real> const& der1 = mJet[2];

            this->mMetric(0, 0) = Dot(der0, der0);
            this->mMetric(0, 1) = Dot(der0, der1);
            this->mMetric(1, 0) = this->mMetric(0, 1);
            this->mMetric(1, 1) = Dot(der1, der1);
        }

        virtual void ComputeChristoffel1(const GVector<Real>&) override
        {
            Vector<3, Real> const& der0 = mJet[1];
            Vector<3, Real> const& der1 = mJet[2];
            Vector<3, Real> const& der00 = mJet[3];
            Vector<3, Real> const& der01 = mJet[4];
            Vector<3, Real> const& der11 = mJet[5];

            this->mChristoffel1[0](0, 0) = Dot(der00, der0);
            this->mChristoffel1[0](0, 1) = Dot(der01, der0);
            this->mChristoffel1[0](1, 0) = this->mChristoffel1[0](0, 1);
            this->mChristoffel1[0](1, 1) = Dot(der11, der0);

            this->mChristoffel1[1](0, 0) = Dot(der00, der1);
            this->mChristoffel1[1](0, 1) = Dot(der01, der1);
            this->mChristoffel1[1](1, 0) = this->mChristoffel1[1](0, 1);
            this->mChristoffel1[1](1, 1) = Dot(der11, der1);
        }

        BSplineSurface<3, Real> const* mSpline;

        // We are guaranteed that RiemannianGeodesic calls ComputeMetric
        // before ComputeChristoffel1.  Thus, we can compute the B-spline
        // first- and second-order derivatives in ComputeMetric and cache
        // the results for use in ComputeChristoffel1.
        Vector<3, Real> mJet[6];
    };
}
