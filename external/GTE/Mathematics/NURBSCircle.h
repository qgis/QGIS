// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/NURBSCurve.h>

// The algorithm for representing a circle as a NURBS curve or a sphere as a
// NURBS surface is described in
//   https://www.geometrictools.com/Documentation/NURBSCircleSphere.pdf
// The implementations are related to the documents as shown next.
//   NURBSQuarterCircleDegree2 implements equation (9)
//   NURBSQuarterCircleDegree4 implements equation (10)
//   NURBSHalfCircleDegree3 implements equation (12)
//   NURBSFullCircleDegree3 implements Section 2.3

namespace gte
{
    template <typename Real>
    class NURBSQuarterCircleDegree2 : public NURBSCurve<2, Real>
    {
    public:
        // Construction.  The quarter circle is x^2 + y^2 = 1 for x >= 0
        // and y >= 0.  The direction of traversal is counterclockwise as
        // u increase from 0 to 1.
        NURBSQuarterCircleDegree2()
            :
            NURBSCurve<2, Real>(BasisFunctionInput<Real>(3, 2), nullptr, nullptr)
        {
            Real const sqrt2 = std::sqrt((Real)2);
            this->mWeights[0] = sqrt2;
            this->mWeights[1] = (Real)1;
            this->mWeights[2] = sqrt2;

            this->mControls[0] = { (Real)1, (Real)0 };
            this->mControls[1] = { (Real)1, (Real)1 };
            this->mControls[2] = { (Real)0, (Real)1 };
        }
    };

    template <typename Real>
    class NURBSQuarterCircleDegree4 : public NURBSCurve<2, Real>
    {
    public:
        // Construction.  The quarter circle is x^2 + y^2 = 1 for x >= 0
        // and y >= 0.  The direction of traversal is counterclockwise as
        // u increases from 0 to 1.
        NURBSQuarterCircleDegree4()
            :
            NURBSCurve<2, Real>(BasisFunctionInput<Real>(5, 4), nullptr, nullptr)
        {
            Real const sqrt2 = std::sqrt((Real)2);
            this->mWeights[0] = (Real)1;
            this->mWeights[1] = (Real)1;
            this->mWeights[2] = (Real)2 * sqrt2 / (Real)3;
            this->mWeights[3] = (Real)1;
            this->mWeights[4] = (Real)1;

            Real const x1 = (Real)1;
            Real const y1 = (Real)0.5 / sqrt2;
            Real const x2 = (Real)1 - sqrt2 / (Real)8;
            this->mControls[0] = { (Real)1, (Real)0 };
            this->mControls[1] = { x1, y1 };
            this->mControls[2] = { x2, x2 };
            this->mControls[3] = { y1, x1 };
            this->mControls[4] = { (Real)0, (Real)1 };
        }
    };

    template <typename Real>
    class NURBSHalfCircleDegree3 : public NURBSCurve<2, Real>
    {
    public:
        // Construction.  The half circle is x^2 + y^2 = 1 for x >= 0.  The
        // direction of traversal is counterclockwise as u increases from
        // 0 to 1.
        NURBSHalfCircleDegree3()
            :
            NURBSCurve<2, Real>(BasisFunctionInput<Real>(4, 3), nullptr, nullptr)
        {
            Real const oneThird = (Real)1 / (Real)3;
            this->mWeights[0] = (Real)1;
            this->mWeights[1] = oneThird;
            this->mWeights[2] = oneThird;
            this->mWeights[3] = (Real)1;

            this->mControls[0] = { (Real)1, (Real)0 };
            this->mControls[1] = { (Real)1, (Real)2 };
            this->mControls[2] = { (Real)-1, (Real)2 };
            this->mControls[3] = { (Real)-1, (Real)0 };
        }
    };

    template <typename Real>
    class NURBSFullCircleDegree3 : public NURBSCurve<2, Real>
    {
    public:
        // Construction.  The full circle is x^2 + y^2 = 1.  The direction of
        // traversal is counterclockwise as u increases from 0 to 1.
        NURBSFullCircleDegree3()
            :
            NURBSCurve<2, Real>(CreateBasisFunctionInput(), nullptr, nullptr)
        {
            Real const oneThird = (Real)1 / (Real)3;
            this->mWeights[0] = (Real)1;
            this->mWeights[1] = oneThird;
            this->mWeights[2] = oneThird;
            this->mWeights[3] = (Real)1;
            this->mWeights[4] = oneThird;
            this->mWeights[5] = oneThird;
            this->mWeights[6] = (Real)1;

            this->mControls[0] = { (Real)1, (Real)0 };
            this->mControls[1] = { (Real)1, (Real)2 };
            this->mControls[2] = { (Real)-1, (Real)2 };
            this->mControls[3] = { (Real)-1, (Real)0 };
            this->mControls[4] = { (Real)-1, (Real)-2 };
            this->mControls[5] = { (Real)1, (Real)-2 };
            this->mControls[6] = { (Real)1, (Real)0 };
        }

    private:
        static BasisFunctionInput<Real> CreateBasisFunctionInput()
        {
            // We need knots (0,0,0,0,1/2,1/2,1/2,1,1,1,1).
            BasisFunctionInput<Real> input;
            input.numControls = 7;
            input.degree = 3;
            input.uniform = true;
            input.periodic = false;
            input.numUniqueKnots = 3;
            input.uniqueKnots.resize(input.numUniqueKnots);
            input.uniqueKnots[0] = { (Real)0, 4 };
            input.uniqueKnots[1] = { (Real)0.5, 3 };
            input.uniqueKnots[2] = { (Real)1, 4 };
            return input;
        }
    };
}
