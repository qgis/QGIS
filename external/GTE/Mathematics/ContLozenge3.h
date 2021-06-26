// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/ApprGaussian3.h>
#include <Mathematics/DistPoint3Rectangle3.h>
#include <Mathematics/Lozenge3.h>

namespace gte
{
    // Compute the plane of the lozenge rectangle using least-squares fit.
    // Parallel planes are chosen close enough together so that all the data
    // points lie between them.  The radius is half the distance between the
    // two planes.  The half-cylinder and quarter-cylinder side pieces are
    // chosen using a method similar to that used for fitting by capsules.
    template <typename Real>
    bool GetContainer(int numPoints, Vector3<Real> const* points, Lozenge3<Real>& lozenge)
    {
        ApprGaussian3<Real> fitter;
        fitter.Fit(numPoints, points);
        OrientedBox3<Real> box = fitter.GetParameters();

        Vector3<Real> diff = points[0] - box.center;
        Real wMin = Dot(box.axis[0], diff);
        Real wMax = wMin;
        Real w;
        for (int i = 1; i < numPoints; ++i)
        {
            diff = points[i] - box.center;
            w = Dot(box.axis[0], diff);
            if (w < wMin)
            {
                wMin = w;
            }
            else if (w > wMax)
            {
                wMax = w;
            }
        }

        Real radius = (Real)0.5 * (wMax - wMin);
        Real rSqr = radius * radius;
        box.center += ((Real)0.5 * (wMax + wMin)) * box.axis[0];

        Real aMin = std::numeric_limits<Real>::max();
        Real aMax = -aMin;
        Real bMin = std::numeric_limits<Real>::max();
        Real bMax = -bMin;
        Real discr, radical, u, v, test;
        for (int i = 0; i < numPoints; ++i)
        {
            diff = points[i] - box.center;
            u = Dot(box.axis[2], diff);
            v = Dot(box.axis[1], diff);
            w = Dot(box.axis[0], diff);
            discr = rSqr - w * w;
            radical = std::sqrt(std::max(discr, (Real)0));

            test = u + radical;
            if (test < aMin)
            {
                aMin = test;
            }

            test = u - radical;
            if (test > aMax)
            {
                aMax = test;
            }

            test = v + radical;
            if (test < bMin)
            {
                bMin = test;
            }

            test = v - radical;
            if (test > bMax)
            {
                bMax = test;
            }
        }

        // The enclosing region might be a capsule or a sphere.
        if (aMin >= aMax)
        {
            test = (Real)0.5 * (aMin + aMax);
            aMin = test;
            aMax = test;
        }
        if (bMin >= bMax)
        {
            test = (Real)0.5 * (bMin + bMax);
            bMin = test;
            bMax = test;
        }

        // Make correction for points inside mitered corner but outside quarter
        // sphere.
        for (int i = 0; i < numPoints; ++i)
        {
            diff = points[i] - box.center;
            u = Dot(box.axis[2], diff);
            v = Dot(box.axis[1], diff);

            Real* aExtreme = nullptr;
            Real* bExtreme = nullptr;

            if (u > aMax)
            {
                if (v > bMax)
                {
                    aExtreme = &aMax;
                    bExtreme = &bMax;
                }
                else if (v < bMin)
                {
                    aExtreme = &aMax;
                    bExtreme = &bMin;
                }
            }
            else if (u < aMin)
            {
                if (v > bMax)
                {
                    aExtreme = &aMin;
                    bExtreme = &bMax;
                }
                else if (v < bMin)
                {
                    aExtreme = &aMin;
                    bExtreme = &bMin;
                }
            }

            if (aExtreme)
            {
                Real deltaU = u - *aExtreme;
                Real deltaV = v - *bExtreme;
                Real deltaSumSqr = deltaU * deltaU + deltaV * deltaV;
                w = Dot(box.axis[0], diff);
                Real wSqr = w * w;
                test = deltaSumSqr + wSqr;
                if (test > rSqr)
                {
                    discr = (rSqr - wSqr) / deltaSumSqr;
                    Real t = -std::sqrt(std::max(discr, (Real)0));
                    *aExtreme = u + t * deltaU;
                    *bExtreme = v + t * deltaV;
                }
            }
        }

        lozenge.radius = radius;
        lozenge.rectangle.axis[0] = box.axis[2];
        lozenge.rectangle.axis[1] = box.axis[1];

        if (aMin < aMax)
        {
            if (bMin < bMax)
            {
                // Container is a lozenge.
                lozenge.rectangle.center =
                    box.center + aMin * box.axis[2] + bMin * box.axis[1];
                lozenge.rectangle.extent[0] = (Real)0.5 * (aMax - aMin);
                lozenge.rectangle.extent[1] = (Real)0.5 * (bMax - bMin);
            }
            else
            {
                // Container is a capsule.
                lozenge.rectangle.center = box.center + aMin * box.axis[2] +
                    ((Real)0.5 * (bMin + bMax)) * box.axis[1];
                lozenge.rectangle.extent[0] = (Real)0.5 * (aMax - aMin);
                lozenge.rectangle.extent[1] = (Real)0;
            }
        }
        else
        {
            if (bMin < bMax)
            {
                // Container is a capsule.
                lozenge.rectangle.center = box.center + bMin * box.axis[1] +
                    ((Real)0.5 * (aMin + aMax)) * box.axis[2];
                lozenge.rectangle.extent[0] = (Real)0;
                lozenge.rectangle.extent[1] = (Real)0.5 * (bMax - bMin);
            }
            else
            {
                // Container is a sphere.
                lozenge.rectangle.center = box.center +
                    ((Real)0.5 * (aMin + aMax)) * box.axis[2] +
                    ((Real)0.5 * (bMin + bMax)) * box.axis[1];
                lozenge.rectangle.extent[0] = (Real)0;
                lozenge.rectangle.extent[1] = (Real)0;
            }
        }

        return true;
    }

    // Test for containment of a point by a lozenge.
    template <typename Real>
    bool InContainer(Vector3<Real> const& point, Lozenge3<Real> const& lozenge)
    {
        DCPQuery<Real, Vector3<Real>, Rectangle3<Real>> prQuery;
        auto result = prQuery(point, lozenge.rectangle);
        return result.distance <= lozenge.radius;
    }
}
