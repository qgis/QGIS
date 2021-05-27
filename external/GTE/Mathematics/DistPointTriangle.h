// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Triangle.h>
#include <Mathematics/Vector.h>

namespace gte
{
    template <int N, typename Real>
    class DCPQuery<Real, Vector<N, Real>, Triangle<N, Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Real parameter[3];  // barycentric coordinates for triangle.v[3]
            Vector<N, Real> closest;
        };

        Result operator()(Vector<N, Real> const& point, Triangle<N, Real> const& triangle)
        {
            Vector<N, Real> diff = point - triangle.v[0];
            Vector<N, Real> edge0 = triangle.v[1] - triangle.v[0];
            Vector<N, Real> edge1 = triangle.v[2] - triangle.v[0];
            Real a00 = Dot(edge0, edge0);
            Real a01 = Dot(edge0, edge1);
            Real a11 = Dot(edge1, edge1);
            Real b0 = -Dot(diff, edge0);
            Real b1 = -Dot(diff, edge1);

            Real f00 = b0;
            Real f10 = b0 + a00;
            Real f01 = b0 + a01;

            Vector<2, Real> p0, p1, p;
            Real dt1, h0, h1;

            // Compute the endpoints p0 and p1 of the segment.  The segment is
            // parameterized by L(z) = (1-z)*p0 + z*p1 for z in [0,1] and the
            // directional derivative of half the quadratic on the segment is
            // H(z) = Dot(p1-p0,gradient[Q](L(z))/2), where gradient[Q]/2 =
            // (F,G).  By design, F(L(z)) = 0 for cases (2), (4), (5), and
            // (6).  Cases (1) and (3) can correspond to no-intersection or
            // intersection of F = 0 with the triangle.
            if (f00 >= (Real)0)
            {
                if (f01 >= (Real)0)
                {
                    // (1) p0 = (0,0), p1 = (0,1), H(z) = G(L(z))
                    GetMinEdge02(a11, b1, p);
                }
                else
                {
                    // (2) p0 = (0,t10), p1 = (t01,1-t01),
                    // H(z) = (t11 - t10)*G(L(z))
                    p0[0] = (Real)0;
                    p0[1] = f00 / (f00 - f01);
                    p1[0] = f01 / (f01 - f10);
                    p1[1] = (Real)1 - p1[0];
                    dt1 = p1[1] - p0[1];
                    h0 = dt1 * (a11 * p0[1] + b1);
                    if (h0 >= (Real)0)
                    {
                        GetMinEdge02(a11, b1, p);
                    }
                    else
                    {
                        h1 = dt1 * (a01 * p1[0] + a11 * p1[1] + b1);
                        if (h1 <= (Real)0)
                        {
                            GetMinEdge12(a01, a11, b1, f10, f01, p);
                        }
                        else
                        {
                            GetMinInterior(p0, h0, p1, h1, p);
                        }
                    }
                }
            }
            else if (f01 <= (Real)0)
            {
                if (f10 <= (Real)0)
                {
                    // (3) p0 = (1,0), p1 = (0,1), H(z) = G(L(z)) - F(L(z))
                    GetMinEdge12(a01, a11, b1, f10, f01, p);
                }
                else
                {
                    // (4) p0 = (t00,0), p1 = (t01,1-t01), H(z) = t11*G(L(z))
                    p0[0] = f00 / (f00 - f10);
                    p0[1] = (Real)0;
                    p1[0] = f01 / (f01 - f10);
                    p1[1] = (Real)1 - p1[0];
                    h0 = p1[1] * (a01 * p0[0] + b1);
                    if (h0 >= (Real)0)
                    {
                        p = p0;  // GetMinEdge01
                    }
                    else
                    {
                        h1 = p1[1] * (a01 * p1[0] + a11 * p1[1] + b1);
                        if (h1 <= (Real)0)
                        {
                            GetMinEdge12(a01, a11, b1, f10, f01, p);
                        }
                        else
                        {
                            GetMinInterior(p0, h0, p1, h1, p);
                        }
                    }
                }
            }
            else if (f10 <= (Real)0)
            {
                // (5) p0 = (0,t10), p1 = (t01,1-t01),
                // H(z) = (t11 - t10)*G(L(z))
                p0[0] = (Real)0;
                p0[1] = f00 / (f00 - f01);
                p1[0] = f01 / (f01 - f10);
                p1[1] = (Real)1 - p1[0];
                dt1 = p1[1] - p0[1];
                h0 = dt1 * (a11 * p0[1] + b1);
                if (h0 >= (Real)0)
                {
                    GetMinEdge02(a11, b1, p);
                }
                else
                {
                    h1 = dt1 * (a01 * p1[0] + a11 * p1[1] + b1);
                    if (h1 <= (Real)0)
                    {
                        GetMinEdge12(a01, a11, b1, f10, f01, p);
                    }
                    else
                    {
                        GetMinInterior(p0, h0, p1, h1, p);
                    }
                }
            }
            else
            {
                // (6) p0 = (t00,0), p1 = (0,t11), H(z) = t11*G(L(z))
                p0[0] = f00 / (f00 - f10);
                p0[1] = (Real)0;
                p1[0] = (Real)0;
                p1[1] = f00 / (f00 - f01);
                h0 = p1[1] * (a01 * p0[0] + b1);
                if (h0 >= (Real)0)
                {
                    p = p0;  // GetMinEdge01
                }
                else
                {
                    h1 = p1[1] * (a11 * p1[1] + b1);
                    if (h1 <= (Real)0)
                    {
                        GetMinEdge02(a11, b1, p);
                    }
                    else
                    {
                        GetMinInterior(p0, h0, p1, h1, p);
                    }
                }
            }

            Result result;
            result.parameter[0] = (Real)1 - p[0] - p[1];
            result.parameter[1] = p[0];
            result.parameter[2] = p[1];
            result.closest = triangle.v[0] + p[0] * edge0 + p[1] * edge1;
            diff = point - result.closest;
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }

        // TODO: This is the previous implementation based on quadratic
        // minimization with constraints. It was replaced by the current
        // operator() that uses the conjugate gradient algorithm. I will
        // keep both in the upcoming GTL code, so the old code is restored
        // here for now.
        Result DistanceByQM(Vector<N, Real> const& point, Triangle<N, Real> const& triangle)
        {
            // The member result.sqrDistance is set each block of the nested
            // if-then-else statements. The remaining members are all set at
            // the end of the function.
            Result result;

            Vector<N, Real> diff = triangle.v[0] - point;
            Vector<N, Real> edge0 = triangle.v[1] - triangle.v[0];
            Vector<N, Real> edge1 = triangle.v[2] - triangle.v[0];
            Real a00 = Dot(edge0, edge0);
            Real a01 = Dot(edge0, edge1);
            Real a11 = Dot(edge1, edge1);
            Real b0 = Dot(diff, edge0);
            Real b1 = Dot(diff, edge1);
            Real c = Dot(diff, diff);
            Real det = std::max(a00 * a11 - a01 * a01, (Real)0);
            Real s = a01 * b1 - a11 * b0;
            Real t = a01 * b0 - a00 * b1;

            if (s + t <= det)
            {
                if (s < (Real)0)
                {
                    if (t < (Real)0)  // region 4
                    {
                        if (b0 < (Real)0)
                        {
                            t = (Real)0;
                            if (-b0 >= a00)
                            {
                                s = (Real)1;
                                result.sqrDistance = a00 + (Real)2 * b0 + c;
                            }
                            else
                            {
                                s = -b0 / a00;
                                result.sqrDistance = b0 * s + c;
                            }
                        }
                        else
                        {
                            s = (Real)0;
                            if (b1 >= (Real)0)
                            {
                                t = (Real)0;
                                result.sqrDistance = c;
                            }
                            else if (-b1 >= a11)
                            {
                                t = (Real)1;
                                result.sqrDistance = a11 + (Real)2 * b1 + c;
                            }
                            else
                            {
                                t = -b1 / a11;
                                result.sqrDistance = b1 * t + c;
                            }
                        }
                    }
                    else  // region 3
                    {
                        s = (Real)0;
                        if (b1 >= (Real)0)
                        {
                            t = (Real)0;
                            result.sqrDistance = c;
                        }
                        else if (-b1 >= a11)
                        {
                            t = (Real)1;
                            result.sqrDistance = a11 + (Real)2 * b1 + c;
                        }
                        else
                        {
                            t = -b1 / a11;
                            result.sqrDistance = b1 * t + c;
                        }
                    }
                }
                else if (t < (Real)0)  // region 5
                {
                    t = (Real)0;
                    if (b0 >= (Real)0)
                    {
                        s = (Real)0;
                        result.sqrDistance = c;
                    }
                    else if (-b0 >= a00)
                    {
                        s = (Real)1;
                        result.sqrDistance = a00 + (Real)2 * b0 + c;
                    }
                    else
                    {
                        s = -b0 / a00;
                        result.sqrDistance = b0 * s + c;
                    }
                }
                else  // region 0
                {
                    // minimum at interior point
                    Real invDet = ((Real)1) / det;
                    s *= invDet;
                    t *= invDet;
                    result.sqrDistance = s * (a00 * s + a01 * t + (Real)2 * b0) +
                        t * (a01 * s + a11 * t + (Real)2 * b1) + c;
                }
            }
            else
            {
                Real tmp0, tmp1, numer, denom;

                if (s < (Real)0)  // region 2
                {
                    tmp0 = a01 + b0;
                    tmp1 = a11 + b1;
                    if (tmp1 > tmp0)
                    {
                        numer = tmp1 - tmp0;
                        denom = a00 - (Real)2 * a01 + a11;
                        if (numer >= denom)
                        {
                            s = (Real)1;
                            t = (Real)0;
                            result.sqrDistance = a00 + (Real)2 * b0 + c;
                        }
                        else
                        {
                            s = numer / denom;
                            t = (Real)1 - s;
                            result.sqrDistance = s * (a00 * s + a01 * t + (Real)2 * b0) +
                                t * (a01 * s + a11 * t + (Real)2 * b1) + c;
                        }
                    }
                    else
                    {
                        s = (Real)0;
                        if (tmp1 <= (Real)0)
                        {
                            t = (Real)1;
                            result.sqrDistance = a11 + (Real)2 * b1 + c;
                        }
                        else if (b1 >= (Real)0)
                        {
                            t = (Real)0;
                            result.sqrDistance = c;
                        }
                        else
                        {
                            t = -b1 / a11;
                            result.sqrDistance = b1 * t + c;
                        }
                    }
                }
                else if (t < (Real)0)  // region 6
                {
                    tmp0 = a01 + b1;
                    tmp1 = a00 + b0;
                    if (tmp1 > tmp0)
                    {
                        numer = tmp1 - tmp0;
                        denom = a00 - (Real)2 * a01 + a11;
                        if (numer >= denom)
                        {
                            t = (Real)1;
                            s = (Real)0;
                            result.sqrDistance = a11 + (Real)2 * b1 + c;
                        }
                        else
                        {
                            t = numer / denom;
                            s = (Real)1 - t;
                            result.sqrDistance = s * (a00 * s + a01 * t + (Real)2 * b0) +
                                t * (a01 * s + a11 * t + (Real)2 * b1) + c;
                        }
                    }
                    else
                    {
                        t = (Real)0;
                        if (tmp1 <= (Real)0)
                        {
                            s = (Real)1;
                            result.sqrDistance = a00 + (Real)2 * b0 + c;
                        }
                        else if (b0 >= (Real)0)
                        {
                            s = (Real)0;
                            result.sqrDistance = c;
                        }
                        else
                        {
                            s = -b0 / a00;
                            result.sqrDistance = b0 * s + c;
                        }
                    }
                }
                else  // region 1
                {
                    numer = a11 + b1 - a01 - b0;
                    if (numer <= (Real)0)
                    {
                        s = (Real)0;
                        t = (Real)1;
                        result.sqrDistance = a11 + (Real)2 * b1 + c;
                    }
                    else
                    {
                        denom = a00 - ((Real)2) * a01 + a11;
                        if (numer >= denom)
                        {
                            s = (Real)1;
                            t = (Real)0;
                            result.sqrDistance = a00 + (Real)2 * b0 + c;
                        }
                        else
                        {
                            s = numer / denom;
                            t = (Real)1 - s;
                            result.sqrDistance = s * (a00 * s + a01 * t + (Real)2 * b0) +
                                t * (a01 * s + a11 * t + (Real)2 * b1) + c;
                        }
                    }
                }
            }

            // Account for numerical round-off error.
            if (result.sqrDistance < (Real)0)
            {
                result.sqrDistance = (Real)0;
            }

            result.distance = sqrt(result.sqrDistance);
            result.closest = triangle.v[0] + s * edge0 + t * edge1;
            result.parameter[1] = s;
            result.parameter[2] = t;
            result.parameter[0] = (Real)1 - s - t;
            return result;
        }

    private:
        void GetMinEdge02(Real const& a11, Real const& b1, Vector<2, Real>& p)
        {
            p[0] = (Real)0;
            if (b1 >= (Real)0)
            {
                p[1] = (Real)0;
            }
            else if (a11 + b1 <= (Real)0)
            {
                p[1] = (Real)1;
            }
            else
            {
                p[1] = -b1 / a11;
            }
        }

        inline void GetMinEdge12(Real const& a01, Real const& a11, Real const& b1,
            Real const& f10, Real const& f01, Vector<2, Real>& p)
        {
            Real h0 = a01 + b1 - f10;
            if (h0 >= (Real)0)
            {
                p[1] = (Real)0;
            }
            else
            {
                Real h1 = a11 + b1 - f01;
                if (h1 <= (Real)0)
                {
                    p[1] = (Real)1;
                }
                else
                {
                    p[1] = h0 / (h0 - h1);
                }
            }
            p[0] = (Real)1 - p[1];
        }

        inline void GetMinInterior(Vector<2, Real> const& p0, Real const& h0,
            Vector<2, Real> const& p1, Real const& h1, Vector<2, Real>& p)
        {
            Real z = h0 / (h0 - h1);
            p = ((Real)1 - z) * p0 + z * p1;
        }
    };

    // Template aliases for convenience.
    template <int N, typename Real>
    using DCPPointTriangle = DCPQuery<Real, Vector<N, Real>, Triangle<N, Real>>;

    template <typename Real>
    using DCPPoint2Triangle2 = DCPPointTriangle<2, Real>;

    template <typename Real>
    using DCPPoint3Triangle3 = DCPPointTriangle<3, Real>;
}
