// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Frustum3.h>

namespace gte
{
    template <typename Real>
    class DCPQuery<Real, Vector3<Real>, Frustum3<Real>>
    {
    public:
        struct Result
        {
            Real distance, sqrDistance;
            Vector3<Real> frustumClosestPoint;
        };

        Result operator()(Vector3<Real> const& point, Frustum3<Real> const& frustum)
        {
            Result result;

            // Compute coordinates of point with respect to frustum coordinate
            // system.
            Vector3<Real> diff = point - frustum.origin;
            Vector3<Real> test = {
                Dot(diff, frustum.rVector),
                Dot(diff, frustum.uVector),
                Dot(diff, frustum.dVector) };

            // Perform calculations in octant with nonnegative R and U
            // coordinates.
            bool rSignChange;
            if (test[0] < (Real)0)
            {
                rSignChange = true;
                test[0] = -test[0];
            }
            else
            {
                rSignChange = false;
            }

            bool uSignChange;
            if (test[1] < (Real)0)
            {
                uSignChange = true;
                test[1] = -test[1];
            }
            else
            {
                uSignChange = false;
            }

            // Frustum derived parameters.
            Real rmin = frustum.rBound;
            Real rmax = frustum.GetDRatio() * rmin;
            Real umin = frustum.uBound;
            Real umax = frustum.GetDRatio() * umin;
            Real dmin = frustum.dMin;
            Real dmax = frustum.dMax;
            Real rminSqr = rmin * rmin;
            Real uminSqr = umin * umin;
            Real dminSqr = dmin * dmin;
            Real minRDDot = rminSqr + dminSqr;
            Real minUDDot = uminSqr + dminSqr;
            Real minRUDDot = rminSqr + minUDDot;
            Real maxRDDot = frustum.GetDRatio() * minRDDot;
            Real maxUDDot = frustum.GetDRatio() * minUDDot;
            Real maxRUDDot = frustum.GetDRatio() * minRUDDot;

            // Algorithm computes closest point in all cases by determining
            // in which Voronoi region of the vertices, edges, and faces of
            // the frustum that the test point lives.
            Vector3<Real> closest;
            Real rDot, uDot, rdDot, udDot, rudDot, rEdgeDot, uEdgeDot, t;
            if (test[2] >= dmax)
            {
                if (test[0] <= rmax)
                {
                    if (test[1] <= umax)
                    {
                        // F-face
                        closest[0] = test[0];
                        closest[1] = test[1];
                        closest[2] = dmax;
                    }
                    else
                    {
                        // UF-edge
                        closest[0] = test[0];
                        closest[1] = umax;
                        closest[2] = dmax;
                    }
                }
                else
                {
                    if (test[1] <= umax)
                    {
                        // LF-edge
                        closest[0] = rmax;
                        closest[1] = test[1];
                        closest[2] = dmax;
                    }
                    else
                    {
                        // LUF-vertex
                        closest[0] = rmax;
                        closest[1] = umax;
                        closest[2] = dmax;
                    }
                }
            }
            else if (test[2] <= dmin)
            {
                if (test[0] <= rmin)
                {
                    if (test[1] <= umin)
                    {
                        // N-face
                        closest[0] = test[0];
                        closest[1] = test[1];
                        closest[2] = dmin;
                    }
                    else
                    {
                        udDot = umin * test[1] + dmin * test[2];
                        if (udDot >= maxUDDot)
                        {
                            // UF-edge
                            closest[0] = test[0];
                            closest[1] = umax;
                            closest[2] = dmax;
                        }
                        else if (udDot >= minUDDot)
                        {
                            // U-face
                            uDot = dmin * test[1] - umin * test[2];
                            t = uDot / minUDDot;
                            closest[0] = test[0];
                            closest[1] = test[1] - t * dmin;
                            closest[2] = test[2] + t * umin;
                        }
                        else
                        {
                            // UN-edge
                            closest[0] = test[0];
                            closest[1] = umin;
                            closest[2] = dmin;
                        }
                    }
                }
                else
                {
                    if (test[1] <= umin)
                    {
                        rdDot = rmin * test[0] + dmin * test[2];
                        if (rdDot >= maxRDDot)
                        {
                            // LF-edge
                            closest[0] = rmax;
                            closest[1] = test[1];
                            closest[2] = dmax;
                        }
                        else if (rdDot >= minRDDot)
                        {
                            // L-face
                            rDot = dmin * test[0] - rmin * test[2];
                            t = rDot / minRDDot;
                            closest[0] = test[0] - t * dmin;
                            closest[1] = test[1];
                            closest[2] = test[2] + t * rmin;
                        }
                        else
                        {
                            // LN-edge
                            closest[0] = rmin;
                            closest[1] = test[1];
                            closest[2] = dmin;
                        }
                    }
                    else
                    {
                        rudDot = rmin * test[0] + umin * test[1] + dmin * test[2];
                        rEdgeDot = umin * rudDot - minRUDDot * test[1];
                        if (rEdgeDot >= (Real)0)
                        {
                            rdDot = rmin * test[0] + dmin * test[2];
                            if (rdDot >= maxRDDot)
                            {
                                // LF-edge
                                closest[0] = rmax;
                                closest[1] = test[1];
                                closest[2] = dmax;
                            }
                            else if (rdDot >= minRDDot)
                            {
                                // L-face
                                rDot = dmin * test[0] - rmin * test[2];
                                t = rDot / minRDDot;
                                closest[0] = test[0] - t * dmin;
                                closest[1] = test[1];
                                closest[2] = test[2] + t * rmin;
                            }
                            else
                            {
                                // LN-edge
                                closest[0] = rmin;
                                closest[1] = test[1];
                                closest[2] = dmin;
                            }
                        }
                        else
                        {
                            uEdgeDot = rmin * rudDot - minRUDDot * test[0];
                            if (uEdgeDot >= (Real)0)
                            {
                                udDot = umin * test[1] + dmin * test[2];
                                if (udDot >= maxUDDot)
                                {
                                    // UF-edge
                                    closest[0] = test[0];
                                    closest[1] = umax;
                                    closest[2] = dmax;
                                }
                                else if (udDot >= minUDDot)
                                {
                                    // U-face
                                    uDot = dmin * test[1] - umin * test[2];
                                    t = uDot / minUDDot;
                                    closest[0] = test[0];
                                    closest[1] = test[1] - t * dmin;
                                    closest[2] = test[2] + t * umin;
                                }
                                else
                                {
                                    // UN-edge
                                    closest[0] = test[0];
                                    closest[1] = umin;
                                    closest[2] = dmin;
                                }
                            }
                            else
                            {
                                if (rudDot >= maxRUDDot)
                                {
                                    // LUF-vertex
                                    closest[0] = rmax;
                                    closest[1] = umax;
                                    closest[2] = dmax;
                                }
                                else if (rudDot >= minRUDDot)
                                {
                                    // LU-edge
                                    t = rudDot / minRUDDot;
                                    closest[0] = t * rmin;
                                    closest[1] = t * umin;
                                    closest[2] = t * dmin;
                                }
                                else
                                {
                                    // LUN-vertex
                                    closest[0] = rmin;
                                    closest[1] = umin;
                                    closest[2] = dmin;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                rDot = dmin * test[0] - rmin * test[2];
                uDot = dmin * test[1] - umin * test[2];
                if (rDot <= (Real)0)
                {
                    if (uDot <= (Real)0)
                    {
                        // point inside frustum
                        closest = test;
                    }
                    else
                    {
                        udDot = umin * test[1] + dmin * test[2];
                        if (udDot >= maxUDDot)
                        {
                            // UF-edge
                            closest[0] = test[0];
                            closest[1] = umax;
                            closest[2] = dmax;
                        }
                        else
                        {
                            // U-face
                            t = uDot / minUDDot;
                            closest[0] = test[0];
                            closest[1] = test[1] - t * dmin;
                            closest[2] = test[2] + t * umin;
                        }
                    }
                }
                else
                {
                    if (uDot <= (Real)0)
                    {
                        rdDot = rmin * test[0] + dmin * test[2];
                        if (rdDot >= maxRDDot)
                        {
                            // LF-edge
                            closest[0] = rmax;
                            closest[1] = test[1];
                            closest[2] = dmax;
                        }
                        else
                        {
                            // L-face
                            t = rDot / minRDDot;
                            closest[0] = test[0] - t * dmin;
                            closest[1] = test[1];
                            closest[2] = test[2] + t * rmin;
                        }
                    }
                    else
                    {
                        rudDot = rmin * test[0] + umin * test[1] + dmin * test[2];
                        rEdgeDot = umin * rudDot - minRUDDot * test[1];
                        if (rEdgeDot >= (Real)0)
                        {
                            rdDot = rmin * test[0] + dmin * test[2];
                            if (rdDot >= maxRDDot)
                            {
                                // LF-edge
                                closest[0] = rmax;
                                closest[1] = test[1];
                                closest[2] = dmax;
                            }
                            else // assert( rdDot >= minRDDot )
                            {
                                // L-face
                                t = rDot / minRDDot;
                                closest[0] = test[0] - t * dmin;
                                closest[1] = test[1];
                                closest[2] = test[2] + t * rmin;
                            }
                        }
                        else
                        {
                            uEdgeDot = rmin * rudDot - minRUDDot * test[0];
                            if (uEdgeDot >= (Real)0)
                            {
                                udDot = umin * test[1] + dmin * test[2];
                                if (udDot >= maxUDDot)
                                {
                                    // UF-edge
                                    closest[0] = test[0];
                                    closest[1] = umax;
                                    closest[2] = dmax;
                                }
                                else // assert( udDot >= minUDDot )
                                {
                                    // U-face
                                    t = uDot / minUDDot;
                                    closest[0] = test[0];
                                    closest[1] = test[1] - t * dmin;
                                    closest[2] = test[2] + t * umin;
                                }
                            }
                            else
                            {
                                if (rudDot >= maxRUDDot)
                                {
                                    // LUF-vertex
                                    closest[0] = rmax;
                                    closest[1] = umax;
                                    closest[2] = dmax;
                                }
                                else // assert( rudDot >= minRUDDot )
                                {
                                    // LU-edge
                                    t = rudDot / minRUDDot;
                                    closest[0] = t * rmin;
                                    closest[1] = t * umin;
                                    closest[2] = t * dmin;
                                }
                            }
                        }
                    }
                }
            }

            diff = test - closest;

            // Convert back to original quadrant.
            if (rSignChange)
            {
                closest[0] = -closest[0];
            }

            if (uSignChange)
            {
                closest[1] = -closest[1];
            }

            // Convert back to original coordinates.
            result.frustumClosestPoint = frustum.origin +
                closest[0] * frustum.rVector +
                closest[1] * frustum.uVector +
                closest[2] * frustum.dVector;

            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }
    };
}
