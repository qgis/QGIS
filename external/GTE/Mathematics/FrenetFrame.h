// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/ParametricCurve.h>
#include <Mathematics/Vector2.h>
#include <Mathematics/Vector3.h>
#include <memory>

namespace gte
{
    template <typename Real>
    class FrenetFrame2
    {
    public:
        // Construction.  The curve must persist as long as the FrenetFrame2
        // object does.
        FrenetFrame2(std::shared_ptr<ParametricCurve<2, Real>> const& curve)
            :
            mCurve(curve)
        {
        }

        // The normal is perpendicular to the tangent, rotated clockwise by
        // pi/2 radians.
        void operator()(Real t, Vector2<Real>& position, Vector2<Real>& tangent,
            Vector2<Real>& normal) const
        {
            std::array<Vector2<Real>, 2> jet;
            mCurve->Evaluate(t, 1, jet.data());
            position = jet[0];
            tangent = jet[1];
            Normalize(tangent);
            normal = Perp(tangent);
        }

        Real GetCurvature(Real t) const
        {
            std::array<Vector2<Real>, 3> jet;
            mCurve->Evaluate(t, 2, jet.data());
            Real speedSqr = Dot(jet[1], jet[1]);
            if (speedSqr > (Real)0)
            {
                Real numer = DotPerp(jet[1], jet[2]);
                Real denom = std::pow(speedSqr, (Real)1.5);
                return numer / denom;
            }
            else
            {
                // Curvature is indeterminate, just return 0.
                return (Real)0;
            }
        }

    private:
        std::shared_ptr<ParametricCurve<2, Real>> mCurve;
    };


    template <typename Real>
    class FrenetFrame3
    {
    public:
        // Construction.  The curve must persist as long as the FrenetFrame3
        // object does.
        FrenetFrame3(std::shared_ptr<ParametricCurve<3, Real>> const& curve)
            :
            mCurve(curve)
        {
        }

        // The binormal is Cross(tangent, normal).
        void operator()(Real t, Vector3<Real>& position, Vector3<Real>& tangent,
            Vector3<Real>& normal, Vector3<Real>& binormal) const
        {
            std::array<Vector3<Real>, 3> jet;
            mCurve->Evaluate(t, 2, jet.data());
            position = jet[0];
            Real VDotV = Dot(jet[1], jet[1]);
            Real VDotA = Dot(jet[1], jet[2]);
            normal = VDotV * jet[2] - VDotA * jet[1];
            Normalize(normal);
            tangent = jet[1];
            Normalize(tangent);
            binormal = Cross(tangent, normal);
        }

        Real GetCurvature(Real t) const
        {
            std::array<Vector3<Real>, 3> jet;
            mCurve->Evaluate(t, 2, jet.data());
            Real speedSqr = Dot(jet[1], jet[1]);
            if (speedSqr > (Real)0)
            {
                Real numer = Length(Cross(jet[1], jet[2]));
                Real denom = std::pow(speedSqr, (Real)1.5);
                return numer / denom;
            }
            else
            {
                // Curvature is indeterminate, just return 0.
                return (Real)0;
            }
        }

        Real GetTorsion(Real t) const
        {
            std::array<Vector3<Real>, 4> jet;
            mCurve->Evaluate(t, 3, jet.data());
            Vector3<Real> cross = Cross(jet[1], jet[2]);
            Real denom = Dot(cross, cross);
            if (denom > (Real)0)
            {
                Real numer = Dot(cross, jet[3]);
                return numer / denom;
            }
            else
            {
                // Torsion is indeterminate, just return 0.
                return (Real)0;
            }
        }

    private:
        std::shared_ptr<ParametricCurve<3, Real>> mCurve;
    };
}
