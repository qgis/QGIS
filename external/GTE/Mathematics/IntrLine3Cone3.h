// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Vector3.h>
#include <Mathematics/Cone.h>
#include <Mathematics/Line.h>
#include <Mathematics/QFNumber.h>
#include <Mathematics/IntrIntervals.h>

// The queries consider the cone to be single sided and solid.  The
// cone height range is [hmin,hmax].  The cone can be infinite where
// hmin = 0 and hmax = +infinity, infinite truncated where hmin > 0
// and hmax = +infinity, finite where hmin = 0 and hmax < +infinity,
// or a cone frustum where hmin > 0 and hmax < +infinity.  The
// algorithm details are found in
// https://www.geometrictools.com/Documentation/IntersectionLineCone.pdf

namespace gte
{
    template <typename Real>
    class FIQuery<Real, Line3<Real>, Cone3<Real>>
    {
    public:
        // The rational quadratic field type with elements x + y * sqrt(d).
        // This type supports error-free computation.
        using QFN1 = QFNumber<Real, 1>;

        // Convenient naming for interval find-intersection queries.
        using IIQuery = FIIntervalInterval<QFN1>;

        struct Result
        {
            // Because the intersection of line and cone with infinite height
            // can be a ray or a line, we use a 'type' value that allows you
            // to decide how to interpret the t[] and P[] values.

            // No interesection.
            static int const isEmpty = 0;

            // t[0] is finite, t[1] is set to t[0], P[0] is the point of
            // intersection, P[1] is set to P[0].
            static int const isPoint = 1;

            // t[0] and t[1] are finite with t[0] < t[1], P[0] and P[1] are
            // the endpoints of the segment of intersection.
            static int const isSegment = 2;

            // Dot(line.direction, cone.ray.direction) > 0:
            // t[0] is finite, t[1] is +infinity (set to +1), P[0] is the ray
            // origin, P[1] is the ray direction (set to line.direction).
            // NOTE: The ray starts at P[0] and you walk away from it in the
            // line direction.
            static int const isRayPositive = 3;

            // Dot(line.direction, cone.ray.direction) < 0:
            // t[0] is -infinity (set to -1), t[1] is finite, P[0] is the ray
            // endpoint, P[1] is the ray direction (set to line.direction).
            // NOTE: The ray ends at P[1] and you walk towards it in the line
            // direction.
            static int const isRayNegative = 4;

            Result()
                :
                intersect(false),
                type(Result::isEmpty)
            {
                // t[], h[] and P[] are initialized to zero via QFN1 constructors
            }

            void ComputePoints(Vector3<Real> const& origin, Vector3<Real> const& direction)
            {
                switch (type)
                {
                case Result::isEmpty:
                    for (int i = 0; i < 3; ++i)
                    {
                        P[0][i] = QFN1();
                        P[1][i] = P[0][i];
                    }
                    break;
                case Result::isPoint:
                    for (int i = 0; i < 3; ++i)
                    {
                        P[0][i] = origin[i] + direction[i] * t[0];
                        P[1][i] = P[0][i];
                    }
                    break;
                case Result::isSegment:
                    for (int i = 0; i < 3; ++i)
                    {
                        P[0][i] = origin[i] + direction[i] * t[0];
                        P[1][i] = origin[i] + direction[i] * t[1];
                    }
                    break;
                case Result::isRayPositive:
                    for (int i = 0; i < 3; ++i)
                    {
                        P[0][i] = origin[i] + direction[i] * t[0];
                        P[1][i] = QFN1(direction[i], 0, t[0].d);
                    }
                    break;
                case Result::isRayNegative:
                    for (int i = 0; i < 3; ++i)
                    {
                        P[0][i] = origin[i] + direction[i] * t[1];
                        P[1][i] = QFN1(direction[i], 0, t[1].d);
                    }
                    break;
                default:
                    LogError("Invalid case.");
                    break;
                }
            }

            template <typename OutputType>
            static void Convert(QFN1 const& input, OutputType& output)
            {
                output = static_cast<Real>(input);
            }

            template <typename OutputType>
            static void Convert(Vector3<QFN1> const& input, Vector3<OutputType>& output)
            {
                for (int i = 0; i < 3; ++i)
                {
                    output[i] = static_cast<Real>(input[i]);
                }
            }

            bool intersect;
            int type;
            std::array<QFN1, 2> t;
            std::array<Vector3<QFN1>, 2> P;
        };

        Result operator()(Line3<Real> const& line, Cone3<Real> const& cone)
        {
            Result result;
            DoQuery(line.origin, line.direction, cone, result);
            result.ComputePoints(line.origin, line.direction);
            result.intersect = (result.type != Result::isEmpty);
            return result;
        }

    protected:
        // The result.type and result.t[] values are computed by DoQuery. The
        // result.P[] and result.intersect values are computed from them in
        // the operator()(...) function.
        void DoQuery(Vector3<Real> const& lineOrigin, Vector3<Real> const& lineDirection,
            Cone3<Real> const& cone, Result& result)
        {
            // The algorithm implemented in DoQuery avoids extra branches if
            // we choose a line whose direction forms an acute angle with the
            // cone direction.
            if (Dot(lineDirection, cone.ray.direction) >= (Real)0)
            {
                DoQuerySpecial(lineOrigin, lineDirection, cone, result);
            }
            else
            {
                DoQuerySpecial(lineOrigin, -lineDirection, cone, result);
                result.t[0] = -result.t[0];
                result.t[1] = -result.t[1];
                std::swap(result.t[0], result.t[1]);
                if (result.type == Result::isRayPositive)
                {
                    result.type = Result::isRayNegative;
                }
            }
        }

        void DoQuerySpecial(Vector3<Real> const& lineOrigin, Vector3<Real> const& lineDirection,
            Cone3<Real> const& cone, Result& result)
        {
            // Compute the number of real-valued roots and represent them
            // using rational quadratic field elements to support when Real
            // is an exact rational arithmetic type. TODO: Adjust by noting
            // that we should use D/|D| because a normalized floating-point
            // D still might not have |D| = 1 (although it is close to 1).
            Vector3<Real> PmV = lineOrigin - cone.ray.origin;
            Real UdU = Dot(lineDirection, lineDirection);
            Real DdU = Dot(cone.ray.direction, lineDirection);  // >= 0
            Real DdPmV = Dot(cone.ray.direction, PmV);
            Real UdPmV = Dot(lineDirection, PmV);
            Real PmVdPmV = Dot(PmV, PmV);
            Real c2 = DdU * DdU - cone.cosAngleSqr * UdU;
            Real c1 = DdU * DdPmV - cone.cosAngleSqr * UdPmV;
            Real c0 = DdPmV * DdPmV - cone.cosAngleSqr * PmVdPmV;

            if (c2 != (Real)0)
            {
                Real discr = c1 * c1 - c0 * c2;
                if (discr < (Real)0)
                {
                    CaseC2NotZeroDiscrNeg(result);
                }
                else if (discr > (Real)0)
                {
                    CaseC2NotZeroDiscrPos(c1, c2, discr, DdU, DdPmV, cone, result);
                }
                else // discr == 0
                {
                    CaseC2NotZeroDiscrZero(c1, c2, UdU, UdPmV, DdU, DdPmV, cone, result);
                }
            }
            else if (c1 != (Real)0)
            {
                CaseC2ZeroC1NotZero(c0, c1, DdU, DdPmV, cone, result);
            }
            else
            {
                CaseC2ZeroC1Zero(c0, UdU, UdPmV, DdU, DdPmV, cone, result);
            }
        }

        void CaseC2NotZeroDiscrNeg(Result& result)
        {
            // Block 0. The quadratic has no real-valued roots. The line does
            // not intersect the double-sided cone.
            SetEmpty(result);
        }

        void CaseC2NotZeroDiscrPos(Real const& c1, Real const& c2, Real const& discr,
            Real const& DdU, Real const& DdPmV, Cone3<Real> const& cone, Result& result)
        {
            // The quadratic has two distinct real-valued roots, t[0] and t[1]
            // with t[0] < t[1].
            Real x = -c1 / c2;
            Real y = (c2 > (Real)0 ? (Real)1 / c2 : (Real)-1 / c2);
            std::array<QFN1, 2> t = { QFN1(x, -y, discr), QFN1(x, y, discr) };

            // Compute the signed heights at the intersection points, h[0] and
            // h[1] with h[0] <= h[1]. The ordering is guaranteed because we
            // have arranged for the input line to satisfy Dot(D,U) >= 0.
            std::array<QFN1, 2> h = { t[0] * DdU + DdPmV, t[1] * DdU + DdPmV };

            QFN1 zero(0, 0, discr);
            if (h[0] >= zero)
            {
                // Block 1. The line intersects the positive cone in two
                // points.
                SetSegmentClamp(t, h, DdU, DdPmV, cone, result);
            }
            else if (h[1] <= zero)
            {
                // Block 2. The line intersects the negative cone in two
                // points.
                SetEmpty(result);
            }
            else  // h[0] < 0 < h[1]
            {
                // Block 3. The line intersects the positive cone in a single
                // point and the negative cone in a single point.
                SetRayClamp(h[1], DdU, DdPmV, cone, result);
            }
        }

        void CaseC2NotZeroDiscrZero(Real const& c1, Real const& c2,
            Real const& UdU, Real const& UdPmV, Real const& DdU, Real const& DdPmV,
            Cone3<Real> const& cone, Result& result)
        {
            Real t = -c1 / c2;
            if (t * UdU + UdPmV == (Real)0)
            {
                // To get here, it must be that V = P + (-c1/c2) * U, where
                // U is not necessarily a unit-length vector. The line
                // intersects the cone vertex.
                if (c2 < (Real)0)
                {
                    // Block 4. The line is outside the double-sided cone and
                    // intersects it only at V.
                    SetPointClamp(QFN1(t, 0, 0), QFN1(0, 0, 0), cone, result);
                }
                else
                {
                    // Block 5. The line is inside the double-sided cone, so
                    // the intersection is a ray with origin V.
                    SetRayClamp(QFN1(0, 0, 0), DdU, DdPmV, cone, result);
                }
            }
            else
            {
                // The line is tangent to the cone at a point different from
                // the vertex.
                Real h = t * DdU + DdPmV;
                if (h >= (Real)0)
                {
                    // Block 6. The line is tangent to the positive cone.
                    SetPointClamp(QFN1(t, 0, 0), QFN1(h, 0, 0), cone, result);
                }
                else
                {
                    // Block 7. The line is tangent to the negative cone.
                    SetEmpty(result);
                }
            }
        }

        void CaseC2ZeroC1NotZero(Real const& c0, Real const& c1, Real const& DdU,
            Real const& DdPmV, Cone3<Real> const& cone, Result& result)
        {
            // U is a direction vector on the cone boundary. Compute the
            // t-value for the intersection point and compute the
            // corresponding height h to determine whether that point is on
            // the positive cone or negative cone.
            Real t = (Real)-0.5 * c0 / c1;
            Real h = t * DdU + DdPmV;
            if (h > (Real)0)
            {
                // Block 8. The line intersects the positive cone and the ray
                // of intersection is interior to the positive cone. The
                // intersection is a ray or segment.
                SetRayClamp(QFN1(h, 0, 0), DdU, DdPmV, cone, result);
            }
            else
            {
                // Block 9. The line intersects the negative cone and the ray
                // of intersection is interior to the negative cone.
                SetEmpty(result);
            }
        }

        void CaseC2ZeroC1Zero(Real const& c0, Real const& UdU, Real const& UdPmV,
            Real const& DdU, Real const& DdPmV, Cone3<Real> const& cone, Result& result)
        {
            if (c0 != (Real)0)
            {
                // Block 10. The line does not intersect the double-sided
                // cone.
                SetEmpty(result);
            }
            else
            {
                // Block 11. The line is on the cone boundary. The
                // intersection with the positive cone is a ray that contains
                // the cone vertex.  The intersection is either a ray or
                // segment.
                Real t = -UdPmV / UdU;
                Real h = t * DdU + DdPmV;
                SetRayClamp(QFN1(h, 0, 0), DdU, DdPmV, cone, result);
            }
        }

        void SetEmpty(Result& result)
        {
            result.type = Result::isEmpty;
            result.t[0] = QFN1();
            result.t[1] = QFN1();
        }

        void SetPoint(QFN1 const& t, Result& result)
        {
            result.type = Result::isPoint;
            result.t[0] = t;
            result.t[1] = result.t[0];
        }

        void SetSegment(QFN1 const& t0, QFN1 const& t1, Result& result)
        {
            result.type = Result::isSegment;
            result.t[0] = t0;
            result.t[1] = t1;
        }

        void SetRayPositive(QFN1 const& t, Result& result)
        {
            result.type = Result::isRayPositive;
            result.t[0] = t;
            result.t[1] = QFN1(+1, 0, t.d);  // +infinity
        }

        void SetRayNegative(QFN1 const& t, Result& result)
        {
            result.type = Result::isRayNegative;
            result.t[0] = QFN1(-1, 0, t.d);  // +infinity
            result.t[1] = t;
        }

        void SetPointClamp(QFN1 const& t, QFN1 const& h,
            Cone3<Real> const& cone, Result& result)
        {
            if (cone.HeightInRange(h.x[0]))
            {
                // P0.
                SetPoint(t, result);
            }
            else
            {
                // P1.
                SetEmpty(result);
            }
        }

        void SetSegmentClamp(std::array<QFN1, 2> const& t, std::array<QFN1, 2> const& h,
            Real const& DdU, Real const& DdPmV, Cone3<Real> const& cone, Result& result)
        {
            std::array<QFN1, 2> hrange =
            {
                QFN1(cone.GetMinHeight(), 0, h[0].d),
                QFN1(cone.GetMaxHeight(), 0, h[0].d)
            };

            if (h[1] > h[0])
            {
                auto iir = (cone.IsFinite() ? IIQuery()(h, hrange) : IIQuery()(h, hrange[0], true));
                if (iir.numIntersections == 2)
                {
                    // S0.
                    SetSegment((iir.overlap[0] - DdPmV) / DdU, (iir.overlap[1] - DdPmV) / DdU, result);
                }
                else if (iir.numIntersections == 1)
                {
                    // S1.
                    SetPoint((iir.overlap[0] - DdPmV) / DdU, result);
                }
                else  // iir.numIntersections == 0
                {
                    // S2.
                    SetEmpty(result);
                }
            }
            else  // h[1] == h[0]
            {
                if (hrange[0] <= h[0] && (cone.IsFinite() ? h[0] <= hrange[1] : true))
                {
                    // S3. DdU > 0 and the line is not perpendicular to the
                    // cone axis.
                    SetSegment(t[0], t[1], result);
                }
                else
                {
                    // S4. DdU == 0 and the line is perpendicular to the
                    // cone axis.
                    SetEmpty(result);
                }
            }
        }

        void SetRayClamp(QFN1 const& h, Real const& DdU, Real const& DdPmV,
            Cone3<Real> const& cone, Result& result)
        {
            std::array<QFN1, 2> hrange =
            {
                QFN1(cone.GetMinHeight(), 0, h.d),
                QFN1(cone.GetMaxHeight(), 0, h.d)
            };

            if (cone.IsFinite())
            {
                auto iir = IIQuery()(hrange, h, true);
                if (iir.numIntersections == 2)
                {
                    // R0.
                    SetSegment((iir.overlap[0] - DdPmV) / DdU, (iir.overlap[1] - DdPmV) / DdU, result);
                }
                else if (iir.numIntersections == 1)
                {
                    // R1.
                    SetPoint((iir.overlap[0] - DdPmV) / DdU, result);
                }
                else  // iir.numIntersections == 0
                {
                    // R2.
                    SetEmpty(result);
                }
            }
            else
            {
                // R3.
                SetRayPositive((std::max(hrange[0], h) - DdPmV) / DdU, result);
            }
        }
    };
}
