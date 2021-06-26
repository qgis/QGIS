// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DistPointAlignedBox.h>
#include <Mathematics/Line.h>
#include <Mathematics/Vector3.h>

namespace gte
{
    template <typename Real>
    class DCPQuery<Real, Line3<Real>, AlignedBox3<Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Real lineParameter;
            Vector3<Real> closestPoint[2];
        };

        Result operator()(Line3<Real> const& line, AlignedBox3<Real> const& box)
        {
            // Translate the line and box so that the box has center at the
            // origin.
            Vector3<Real> boxCenter, boxExtent;
            box.GetCenteredForm(boxCenter, boxExtent);
            Vector3<Real> point = line.origin - boxCenter;
            Vector3<Real> direction = line.direction;

            Result result;
            DoQuery(point, direction, boxExtent, result);

            // Compute the closest point on the line.
            result.closestPoint[0] = line.origin + result.lineParameter * line.direction;

            // Compute the closest point on the box.
            result.closestPoint[1] = boxCenter + point;
            return result;
        }

    protected:
        // Compute the distance and closest point between a line and an
        // axis-aligned box whose center is the origin.  On input, 'point' is
        // the line origin and 'direction' is the line direction.  On output,
        // 'point' is the point on the box closest to the line.  The
        // 'direction' is non-const to allow transforming the problem into
        // the first octant.
        void DoQuery(Vector3<Real>& point, Vector3<Real>& direction,
            Vector3<Real> const& boxExtent, Result& result)
        {
            result.sqrDistance = (Real)0;
            result.lineParameter = (Real)0;

            // Apply reflections so that direction vector has nonnegative
            // components.
            bool reflect[3];
            for (int i = 0; i < 3; ++i)
            {
                if (direction[i] < (Real)0)
                {
                    point[i] = -point[i];
                    direction[i] = -direction[i];
                    reflect[i] = true;
                }
                else
                {
                    reflect[i] = false;
                }
            }

            if (direction[0] > (Real)0)
            {
                if (direction[1] > (Real)0)
                {
                    if (direction[2] > (Real)0)  // (+,+,+)
                    {
                        CaseNoZeros(point, direction, boxExtent, result);
                    }
                    else  // (+,+,0)
                    {
                        Case0(0, 1, 2, point, direction, boxExtent, result);
                    }
                }
                else
                {
                    if (direction[2] > (Real)0)  // (+,0,+)
                    {
                        Case0(0, 2, 1, point, direction, boxExtent, result);
                    }
                    else  // (+,0,0)
                    {
                        Case00(0, 1, 2, point, direction, boxExtent, result);
                    }
                }
            }
            else
            {
                if (direction[1] > (Real)0)
                {
                    if (direction[2] > (Real)0)  // (0,+,+)
                    {
                        Case0(1, 2, 0, point, direction, boxExtent, result);
                    }
                    else  // (0,+,0)
                    {
                        Case00(1, 0, 2, point, direction, boxExtent, result);
                    }
                }
                else
                {
                    if (direction[2] > (Real)0)  // (0,0,+)
                    {
                        Case00(2, 0, 1, point, direction, boxExtent, result);
                    }
                    else  // (0,0,0)
                    {
                        Case000(point, boxExtent, result);
                    }
                }
            }

            // Undo the reflections applied previously.
            for (int i = 0; i < 3; ++i)
            {
                if (reflect[i])
                {
                    point[i] = -point[i];
                }
            }

            result.distance = std::sqrt(result.sqrDistance);
        }

    private:
        void Face(int i0, int i1, int i2, Vector3<Real>& pnt,
            Vector3<Real> const& dir, Vector3<Real> const& PmE,
            Vector3<Real> const& boxExtent, Result& result)
        {
            Vector3<Real> PpE;
            Real lenSqr, inv, tmp, param, t, delta;

            PpE[i1] = pnt[i1] + boxExtent[i1];
            PpE[i2] = pnt[i2] + boxExtent[i2];
            if (dir[i0] * PpE[i1] >= dir[i1] * PmE[i0])
            {
                if (dir[i0] * PpE[i2] >= dir[i2] * PmE[i0])
                {
                    // v[i1] >= -e[i1], v[i2] >= -e[i2] (distance = 0)
                    pnt[i0] = boxExtent[i0];
                    inv = ((Real)1) / dir[i0];
                    pnt[i1] -= dir[i1] * PmE[i0] * inv;
                    pnt[i2] -= dir[i2] * PmE[i0] * inv;
                    result.lineParameter = -PmE[i0] * inv;
                }
                else
                {
                    // v[i1] >= -e[i1], v[i2] < -e[i2]
                    lenSqr = dir[i0] * dir[i0] + dir[i2] * dir[i2];
                    tmp = lenSqr * PpE[i1] - dir[i1] * (dir[i0] * PmE[i0] +
                        dir[i2] * PpE[i2]);
                    if (tmp <= ((Real)2) * lenSqr * boxExtent[i1])
                    {
                        t = tmp / lenSqr;
                        lenSqr += dir[i1] * dir[i1];
                        tmp = PpE[i1] - t;
                        delta = dir[i0] * PmE[i0] + dir[i1] * tmp + dir[i2] * PpE[i2];
                        param = -delta / lenSqr;
                        result.sqrDistance += PmE[i0] * PmE[i0] + tmp * tmp +
                            PpE[i2] * PpE[i2] + delta * param;

                        result.lineParameter = param;
                        pnt[i0] = boxExtent[i0];
                        pnt[i1] = t - boxExtent[i1];
                        pnt[i2] = -boxExtent[i2];
                    }
                    else
                    {
                        lenSqr += dir[i1] * dir[i1];
                        delta = dir[i0] * PmE[i0] + dir[i1] * PmE[i1] + dir[i2] * PpE[i2];
                        param = -delta / lenSqr;
                        result.sqrDistance += PmE[i0] * PmE[i0] + PmE[i1] * PmE[i1]
                            + PpE[i2] * PpE[i2] + delta * param;

                        result.lineParameter = param;
                        pnt[i0] = boxExtent[i0];
                        pnt[i1] = boxExtent[i1];
                        pnt[i2] = -boxExtent[i2];
                    }
                }
            }
            else
            {
                if (dir[i0] * PpE[i2] >= dir[i2] * PmE[i0])
                {
                    // v[i1] < -e[i1], v[i2] >= -e[i2]
                    lenSqr = dir[i0] * dir[i0] + dir[i1] * dir[i1];
                    tmp = lenSqr * PpE[i2] - dir[i2] * (dir[i0] * PmE[i0] +
                        dir[i1] * PpE[i1]);
                    if (tmp <= ((Real)2) * lenSqr * boxExtent[i2])
                    {
                        t = tmp / lenSqr;
                        lenSqr += dir[i2] * dir[i2];
                        tmp = PpE[i2] - t;
                        delta = dir[i0] * PmE[i0] + dir[i1] * PpE[i1] + dir[i2] * tmp;
                        param = -delta / lenSqr;
                        result.sqrDistance += PmE[i0] * PmE[i0] + PpE[i1] * PpE[i1] +
                            tmp * tmp + delta * param;

                        result.lineParameter = param;
                        pnt[i0] = boxExtent[i0];
                        pnt[i1] = -boxExtent[i1];
                        pnt[i2] = t - boxExtent[i2];
                    }
                    else
                    {
                        lenSqr += dir[i2] * dir[i2];
                        delta = dir[i0] * PmE[i0] + dir[i1] * PpE[i1] + dir[i2] * PmE[i2];
                        param = -delta / lenSqr;
                        result.sqrDistance += PmE[i0] * PmE[i0] + PpE[i1] * PpE[i1] +
                            PmE[i2] * PmE[i2] + delta * param;

                        result.lineParameter = param;
                        pnt[i0] = boxExtent[i0];
                        pnt[i1] = -boxExtent[i1];
                        pnt[i2] = boxExtent[i2];
                    }
                }
                else
                {
                    // v[i1] < -e[i1], v[i2] < -e[i2]
                    lenSqr = dir[i0] * dir[i0] + dir[i2] * dir[i2];
                    tmp = lenSqr * PpE[i1] - dir[i1] * (dir[i0] * PmE[i0] +
                        dir[i2] * PpE[i2]);
                    if (tmp >= (Real)0)
                    {
                        // v[i1]-edge is closest
                        if (tmp <= ((Real)2) * lenSqr * boxExtent[i1])
                        {
                            t = tmp / lenSqr;
                            lenSqr += dir[i1] * dir[i1];
                            tmp = PpE[i1] - t;
                            delta = dir[i0] * PmE[i0] + dir[i1] * tmp + dir[i2] * PpE[i2];
                            param = -delta / lenSqr;
                            result.sqrDistance += PmE[i0] * PmE[i0] + tmp * tmp +
                                PpE[i2] * PpE[i2] + delta * param;

                            result.lineParameter = param;
                            pnt[i0] = boxExtent[i0];
                            pnt[i1] = t - boxExtent[i1];
                            pnt[i2] = -boxExtent[i2];
                        }
                        else
                        {
                            lenSqr += dir[i1] * dir[i1];
                            delta = dir[i0] * PmE[i0] + dir[i1] * PmE[i1]
                                + dir[i2] * PpE[i2];
                            param = -delta / lenSqr;
                            result.sqrDistance += PmE[i0] * PmE[i0] + PmE[i1] * PmE[i1]
                                + PpE[i2] * PpE[i2] + delta * param;

                            result.lineParameter = param;
                            pnt[i0] = boxExtent[i0];
                            pnt[i1] = boxExtent[i1];
                            pnt[i2] = -boxExtent[i2];
                        }
                        return;
                    }

                    lenSqr = dir[i0] * dir[i0] + dir[i1] * dir[i1];
                    tmp = lenSqr * PpE[i2] - dir[i2] * (dir[i0] * PmE[i0] +
                        dir[i1] * PpE[i1]);
                    if (tmp >= (Real)0)
                    {
                        // v[i2]-edge is closest
                        if (tmp <= ((Real)2) * lenSqr * boxExtent[i2])
                        {
                            t = tmp / lenSqr;
                            lenSqr += dir[i2] * dir[i2];
                            tmp = PpE[i2] - t;
                            delta = dir[i0] * PmE[i0] + dir[i1] * PpE[i1] + dir[i2] * tmp;
                            param = -delta / lenSqr;
                            result.sqrDistance += PmE[i0] * PmE[i0] + PpE[i1] * PpE[i1] +
                                tmp * tmp + delta * param;

                            result.lineParameter = param;
                            pnt[i0] = boxExtent[i0];
                            pnt[i1] = -boxExtent[i1];
                            pnt[i2] = t - boxExtent[i2];
                        }
                        else
                        {
                            lenSqr += dir[i2] * dir[i2];
                            delta = dir[i0] * PmE[i0] + dir[i1] * PpE[i1]
                                + dir[i2] * PmE[i2];
                            param = -delta / lenSqr;
                            result.sqrDistance += PmE[i0] * PmE[i0] + PpE[i1] * PpE[i1]
                                + PmE[i2] * PmE[i2] + delta * param;

                            result.lineParameter = param;
                            pnt[i0] = boxExtent[i0];
                            pnt[i1] = -boxExtent[i1];
                            pnt[i2] = boxExtent[i2];
                        }
                        return;
                    }

                    // (v[i1],v[i2])-corner is closest
                    lenSqr += dir[i2] * dir[i2];
                    delta = dir[i0] * PmE[i0] + dir[i1] * PpE[i1] + dir[i2] * PpE[i2];
                    param = -delta / lenSqr;
                    result.sqrDistance += PmE[i0] * PmE[i0] + PpE[i1] * PpE[i1]
                        + PpE[i2] * PpE[i2] + delta * param;

                    result.lineParameter = param;
                    pnt[i0] = boxExtent[i0];
                    pnt[i1] = -boxExtent[i1];
                    pnt[i2] = -boxExtent[i2];
                }
            }
        }

        void CaseNoZeros(Vector3<Real>& pnt, Vector3<Real> const& dir,
            Vector3<Real> const& boxExtent, Result& result)
        {
            Vector3<Real> PmE = pnt - boxExtent;
            Real prodDxPy = dir[0] * PmE[1];
            Real prodDyPx = dir[1] * PmE[0];
            Real prodDzPx, prodDxPz, prodDzPy, prodDyPz;

            if (prodDyPx >= prodDxPy)
            {
                prodDzPx = dir[2] * PmE[0];
                prodDxPz = dir[0] * PmE[2];
                if (prodDzPx >= prodDxPz)
                {
                    // line intersects x = e0
                    Face(0, 1, 2, pnt, dir, PmE, boxExtent, result);
                }
                else
                {
                    // line intersects z = e2
                    Face(2, 0, 1, pnt, dir, PmE, boxExtent, result);
                }
            }
            else
            {
                prodDzPy = dir[2] * PmE[1];
                prodDyPz = dir[1] * PmE[2];
                if (prodDzPy >= prodDyPz)
                {
                    // line intersects y = e1
                    Face(1, 2, 0, pnt, dir, PmE, boxExtent, result);
                }
                else
                {
                    // line intersects z = e2
                    Face(2, 0, 1, pnt, dir, PmE, boxExtent, result);
                }
            }
        }

        void Case0(int i0, int i1, int i2, Vector3<Real>& pnt,
            Vector3<Real> const& dir, Vector3<Real> const& boxExtent, Result& result)
        {
            Real PmE0 = pnt[i0] - boxExtent[i0];
            Real PmE1 = pnt[i1] - boxExtent[i1];
            Real prod0 = dir[i1] * PmE0;
            Real prod1 = dir[i0] * PmE1;
            Real delta, invLSqr, inv;

            if (prod0 >= prod1)
            {
                // line intersects P[i0] = e[i0]
                pnt[i0] = boxExtent[i0];

                Real PpE1 = pnt[i1] + boxExtent[i1];
                delta = prod0 - dir[i0] * PpE1;
                if (delta >= (Real)0)
                {
                    invLSqr = ((Real)1) / (dir[i0] * dir[i0] + dir[i1] * dir[i1]);
                    result.sqrDistance += delta * delta * invLSqr;
                    pnt[i1] = -boxExtent[i1];
                    result.lineParameter = -(dir[i0] * PmE0 + dir[i1] * PpE1) * invLSqr;
                }
                else
                {
                    inv = ((Real)1) / dir[i0];
                    pnt[i1] -= prod0 * inv;
                    result.lineParameter = -PmE0 * inv;
                }
            }
            else
            {
                // line intersects P[i1] = e[i1]
                pnt[i1] = boxExtent[i1];

                Real PpE0 = pnt[i0] + boxExtent[i0];
                delta = prod1 - dir[i1] * PpE0;
                if (delta >= (Real)0)
                {
                    invLSqr = ((Real)1) / (dir[i0] * dir[i0] + dir[i1] * dir[i1]);
                    result.sqrDistance += delta * delta * invLSqr;
                    pnt[i0] = -boxExtent[i0];
                    result.lineParameter = -(dir[i0] * PpE0 + dir[i1] * PmE1) * invLSqr;
                }
                else
                {
                    inv = ((Real)1) / dir[i1];
                    pnt[i0] -= prod1 * inv;
                    result.lineParameter = -PmE1 * inv;
                }
            }

            if (pnt[i2] < -boxExtent[i2])
            {
                delta = pnt[i2] + boxExtent[i2];
                result.sqrDistance += delta * delta;
                pnt[i2] = -boxExtent[i2];
            }
            else if (pnt[i2] > boxExtent[i2])
            {
                delta = pnt[i2] - boxExtent[i2];
                result.sqrDistance += delta * delta;
                pnt[i2] = boxExtent[i2];
            }
        }

        void Case00(int i0, int i1, int i2, Vector3<Real>& pnt,
            Vector3<Real> const& dir, Vector3<Real> const& boxExtent, Result& result)
        {
            Real delta;

            result.lineParameter = (boxExtent[i0] - pnt[i0]) / dir[i0];

            pnt[i0] = boxExtent[i0];

            if (pnt[i1] < -boxExtent[i1])
            {
                delta = pnt[i1] + boxExtent[i1];
                result.sqrDistance += delta * delta;
                pnt[i1] = -boxExtent[i1];
            }
            else if (pnt[i1] > boxExtent[i1])
            {
                delta = pnt[i1] - boxExtent[i1];
                result.sqrDistance += delta * delta;
                pnt[i1] = boxExtent[i1];
            }

            if (pnt[i2] < -boxExtent[i2])
            {
                delta = pnt[i2] + boxExtent[i2];
                result.sqrDistance += delta * delta;
                pnt[i2] = -boxExtent[i2];
            }
            else if (pnt[i2] > boxExtent[i2])
            {
                delta = pnt[i2] - boxExtent[i2];
                result.sqrDistance += delta * delta;
                pnt[i2] = boxExtent[i2];
            }
        }

        void Case000(Vector3<Real>& pnt, Vector3<Real> const& boxExtent, Result& result)
        {
            Real delta;

            if (pnt[0] < -boxExtent[0])
            {
                delta = pnt[0] + boxExtent[0];
                result.sqrDistance += delta * delta;
                pnt[0] = -boxExtent[0];
            }
            else if (pnt[0] > boxExtent[0])
            {
                delta = pnt[0] - boxExtent[0];
                result.sqrDistance += delta * delta;
                pnt[0] = boxExtent[0];
            }

            if (pnt[1] < -boxExtent[1])
            {
                delta = pnt[1] + boxExtent[1];
                result.sqrDistance += delta * delta;
                pnt[1] = -boxExtent[1];
            }
            else if (pnt[1] > boxExtent[1])
            {
                delta = pnt[1] - boxExtent[1];
                result.sqrDistance += delta * delta;
                pnt[1] = boxExtent[1];
            }

            if (pnt[2] < -boxExtent[2])
            {
                delta = pnt[2] + boxExtent[2];
                result.sqrDistance += delta * delta;
                pnt[2] = -boxExtent[2];
            }
            else if (pnt[2] > boxExtent[2])
            {
                delta = pnt[2] - boxExtent[2];
                result.sqrDistance += delta * delta;
                pnt[2] = boxExtent[2];
            }
        }
    };
}
