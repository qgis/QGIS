// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Triangle.h>

namespace gte
{
    template <int N, typename Rational>
    class DistancePointTriangleExact
    {
    public:
        struct Result
        {
            Rational sqrDistance;
            // barycentric coordinates for triangle.v[3]
            Rational parameter[3];
            Vector<N, Rational> closest;
        };

        Result operator()(Vector<N, Rational> const& point, Triangle<N, Rational> const& triangle)
        {
            Vector<N, Rational> diff = point - triangle.v[0];
            Vector<N, Rational> edge0 = triangle.v[1] - triangle.v[0];
            Vector<N, Rational> edge1 = triangle.v[2] - triangle.v[0];
            Rational a00 = Dot(edge0, edge0);
            Rational a01 = Dot(edge0, edge1);
            Rational a11 = Dot(edge1, edge1);
            Rational b0 = -Dot(diff, edge0);
            Rational b1 = -Dot(diff, edge1);
            Rational const zero = (Rational)0;
            Rational const one = (Rational)1;
            Rational det = a00 * a11 - a01 * a01;
            Rational t0 = a01 * b1 - a11 * b0;
            Rational t1 = a01 * b0 - a00 * b1;

            if (t0 + t1 <= det)
            {
                if (t0 < zero)
                {
                    if (t1 < zero)  // region 4
                    {
                        if (b0 < zero)
                        {
                            t1 = zero;
                            if (-b0 >= a00)  // V1
                            {
                                t0 = one;
                            }
                            else  // E01
                            {
                                t0 = -b0 / a00;
                            }
                        }
                        else
                        {
                            t0 = zero;
                            if (b1 >= zero)  // V0
                            {
                                t1 = zero;
                            }
                            else if (-b1 >= a11)  // V2
                            {
                                t1 = one;
                            }
                            else  // E20
                            {
                                t1 = -b1 / a11;
                            }
                        }
                    }
                    else  // region 3
                    {
                        t0 = zero;
                        if (b1 >= zero)  // V0
                        {
                            t1 = zero;
                        }
                        else if (-b1 >= a11)  // V2
                        {
                            t1 = one;
                        }
                        else  // E20
                        {
                            t1 = -b1 / a11;
                        }
                    }
                }
                else if (t1 < zero)  // region 5
                {
                    t1 = zero;
                    if (b0 >= zero)  // V0
                    {
                        t0 = zero;
                    }
                    else if (-b0 >= a00)  // V1
                    {
                        t0 = one;
                    }
                    else  // E01
                    {
                        t0 = -b0 / a00;
                    }
                }
                else  // region 0, interior
                {
                    Rational invDet = one / det;
                    t0 *= invDet;
                    t1 *= invDet;
                }
            }
            else
            {
                Rational tmp0, tmp1, numer, denom;

                if (t0 < zero)  // region 2
                {
                    tmp0 = a01 + b0;
                    tmp1 = a11 + b1;
                    if (tmp1 > tmp0)
                    {
                        numer = tmp1 - tmp0;
                        denom = a00 - (Rational)2 * a01 + a11;
                        if (numer >= denom)  // V1
                        {
                            t0 = one;
                            t1 = zero;
                        }
                        else  // E12
                        {
                            t0 = numer / denom;
                            t1 = one - t0;
                        }
                    }
                    else
                    {
                        t0 = zero;
                        if (tmp1 <= zero)  // V2
                        {
                            t1 = one;
                        }
                        else if (b1 >= zero)  // V0
                        {
                            t1 = zero;
                        }
                        else  // E20
                        {
                            t1 = -b1 / a11;
                        }
                    }
                }
                else if (t1 < zero)  // region 6
                {
                    tmp0 = a01 + b1;
                    tmp1 = a00 + b0;
                    if (tmp1 > tmp0)
                    {
                        numer = tmp1 - tmp0;
                        denom = a00 - (Rational)2 * a01 + a11;
                        if (numer >= denom)  // V2
                        {
                            t1 = one;
                            t0 = zero;
                        }
                        else  // E12
                        {
                            t1 = numer / denom;
                            t0 = one - t1;
                        }
                    }
                    else
                    {
                        t1 = zero;
                        if (tmp1 <= zero)  // V1
                        {
                            t0 = one;
                        }
                        else if (b0 >= zero)  // V0
                        {
                            t0 = zero;
                        }
                        else  // E01
                        {
                            t0 = -b0 / a00;
                        }
                    }
                }
                else  // region 1
                {
                    numer = a11 + b1 - a01 - b0;
                    if (numer <= zero)  // V2
                    {
                        t0 = zero;
                        t1 = one;
                    }
                    else
                    {
                        denom = a00 - (Rational)2 * a01 + a11;
                        if (numer >= denom)  // V1
                        {
                            t0 = one;
                            t1 = zero;
                        }
                        else  // 12
                        {
                            t0 = numer / denom;
                            t1 = one - t0;
                        }
                    }
                }
            }

            Result result;
            result.parameter[0] = one - t0 - t1;
            result.parameter[1] = t0;
            result.parameter[2] = t1;
            result.closest = triangle.v[0] + t0 * edge0 + t1 * edge1;
            diff = point - result.closest;
            result.sqrDistance = Dot(diff, diff);
            return result;
        }
    };
}
