// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/ApprOrthogonalLine3.h>
#include <Mathematics/Capsule.h>
#include <Mathematics/DistPointLine.h>
#include <Mathematics/DistPointSegment.h>
#include <Mathematics/Hypersphere.h>

namespace gte
{
    // Compute the axis of the capsule segment using least-squares fitting.
    // The radius is the maximum distance from the points to the axis.
    // Hemispherical caps are chosen as close together as possible.
    template <typename Real>
    bool GetContainer(int numPoints, Vector3<Real> const* points, Capsule3<Real>& capsule)
    {
        ApprOrthogonalLine3<Real> fitter;
        fitter.Fit(numPoints, points);
        Line3<Real> line = fitter.GetParameters();

        DCPQuery<Real, Vector3<Real>, Line3<Real>> plQuery;
        Real maxRadiusSqr = (Real)0;
        for (int i = 0; i < numPoints; ++i)
        {
            auto result = plQuery(points[i], line);
            if (result.sqrDistance > maxRadiusSqr)
            {
                maxRadiusSqr = result.sqrDistance;
            }
        }

        Vector3<Real> basis[3];
        basis[0] = line.direction;
        ComputeOrthogonalComplement(1, basis);

        Real minValue = std::numeric_limits<Real>::max();
        Real maxValue = -std::numeric_limits<Real>::max();
        for (int i = 0; i < numPoints; ++i)
        {
            Vector3<Real> diff = points[i] - line.origin;
            Real uDotDiff = Dot(diff, basis[1]);
            Real vDotDiff = Dot(diff, basis[2]);
            Real wDotDiff = Dot(diff, basis[0]);
            Real discr = maxRadiusSqr - (uDotDiff * uDotDiff + vDotDiff * vDotDiff);
            Real radical = std::sqrt(std::max(discr, (Real)0));

            Real test = wDotDiff + radical;
            if (test < minValue)
            {
                minValue = test;
            }

            test = wDotDiff - radical;
            if (test > maxValue)
            {
                maxValue = test;
            }
        }

        Vector3<Real> center = line.origin + ((Real)0.5 * (minValue + maxValue)) * line.direction;

        Real extent;
        if (maxValue > minValue)
        {
            // Container is a capsule.
            extent = (Real)0.5 * (maxValue - minValue);
        }
        else
        {
            // Container is a sphere.
            extent = (Real)0;
        }

        capsule.segment = Segment3<Real>(center, line.direction, extent);
        capsule.radius = std::sqrt(maxRadiusSqr);
        return true;
    }

    // Test for containment of a point by a capsule.
    template <typename Real>
    bool InContainer(Vector3<Real> const& point, Capsule3<Real> const& capsule)
    {
        DCPQuery<Real, Vector3<Real>, Segment3<Real>> psQuery;
        auto result = psQuery(point, capsule.segment);
        return result.distance <= capsule.radius;
    }

    // Test for containment of a sphere by a capsule.
    template <typename Real>
    bool InContainer(Sphere3<Real> const& sphere, Capsule3<Real> const& capsule)
    {
        Real rDiff = capsule.radius - sphere.radius;
        if (rDiff >= (Real)0)
        {
            DCPQuery<Real, Vector3<Real>, Segment3<Real>> psQuery;
            auto result = psQuery(sphere.center, capsule.segment);
            return result.distance <= rDiff;
        }
        return false;
    }

    // Test for containment of a capsule by a capsule.
    template <typename Real>
    bool InContainer(Capsule3<Real> const& testCapsule, Capsule3<Real> const& capsule)
    {
        Sphere3<Real> spherePosEnd(testCapsule.segment.p[1], testCapsule.radius);
        Sphere3<Real> sphereNegEnd(testCapsule.segment.p[0], testCapsule.radius);
        return InContainer<Real>(spherePosEnd, capsule)
            && InContainer<Real>(sphereNegEnd, capsule);
    }

    // Compute a capsule that contains the input capsules.  The returned
    // capsule is not necessarily the one of smallest volume that contains
    // the inputs.
    template <typename Real>
    bool MergeContainers(Capsule3<Real> const& capsule0,
        Capsule3<Real> const& capsule1, Capsule3<Real>& merge)
    {
        if (InContainer<Real>(capsule0, capsule1))
        {
            merge = capsule1;
            return true;
        }

        if (InContainer<Real>(capsule1, capsule0))
        {
            merge = capsule0;
            return true;
        }

        Vector3<Real> P0, P1, D0, D1;
        Real extent0, extent1;
        capsule0.segment.GetCenteredForm(P0, D0, extent0);
        capsule1.segment.GetCenteredForm(P1, D1, extent1);

        // Axis of final capsule.
        Line3<Real> line;

        // Axis center is average of input axis centers.
        line.origin = (Real)0.5 * (P0 + P1);

        // Axis unit direction is average of input axis unit directions.
        if (Dot(D0, D1) >= (Real)0)
        {
            line.direction = D0 + D1;
        }
        else
        {
            line.direction = D0 - D1;
        }
        Normalize(line.direction);

        // Cylinder with axis 'line' must contain the spheres centered at the
        // endpoints of the input capsules.
        DCPQuery<Real, Vector3<Real>, Line3<Real>> plQuery;
        Vector3<Real> posEnd0 = capsule0.segment.p[1];
        Real radius = plQuery(posEnd0, line).distance + capsule0.radius;

        Vector3<Real> negEnd0 = capsule0.segment.p[0];
        Real tmp = plQuery(negEnd0, line).distance + capsule0.radius;

        Vector3<Real> posEnd1 = capsule1.segment.p[1];
        tmp = plQuery(posEnd1, line).distance + capsule1.radius;
        if (tmp > radius)
        {
            radius = tmp;
        }

        Vector3<Real> negEnd1 = capsule1.segment.p[0];
        tmp = plQuery(negEnd1, line).distance + capsule1.radius;
        if (tmp > radius)
        {
            radius = tmp;
        }

        // In the following blocks of code, theoretically k1*k1-k0 >= 0, but
        // numerical rounding errors can make it slightly negative.  Guard
        // against this.

        // Process sphere <posEnd0,r0>.
        Real rDiff = radius - capsule0.radius;
        Real rDiffSqr = rDiff * rDiff;
        Vector3<Real> diff = line.origin - posEnd0;
        Real k0 = Dot(diff, diff) - rDiffSqr;
        Real k1 = Dot(diff, line.direction);
        Real discr = k1 * k1 - k0;
        Real root = std::sqrt(std::max(discr, (Real)0));
        Real tPos = -k1 - root;
        Real tNeg = -k1 + root;

        // Process sphere <negEnd0,r0>.
        diff = line.origin - negEnd0;
        k0 = Dot(diff, diff) - rDiffSqr;
        k1 = Dot(diff, line.direction);
        discr = k1 * k1 - k0;
        root = std::sqrt(std::max(discr, (Real)0));
        tmp = -k1 - root;
        if (tmp > tPos)
        {
            tPos = tmp;
        }
        tmp = -k1 + root;
        if (tmp < tNeg)
        {
            tNeg = tmp;
        }

        // Process sphere <posEnd1,r1>.
        rDiff = radius - capsule1.radius;
        rDiffSqr = rDiff * rDiff;
        diff = line.origin - posEnd1;
        k0 = Dot(diff, diff) - rDiffSqr;
        k1 = Dot(diff, line.direction);
        discr = k1 * k1 - k0;
        root = std::sqrt(std::max(discr, (Real)0));
        tmp = -k1 - root;
        if (tmp > tPos)
        {
            tPos = tmp;
        }
        tmp = -k1 + root;
        if (tmp < tNeg)
        {
            tNeg = tmp;
        }

        // Process sphere <negEnd1,r1>.
        diff = line.origin - negEnd1;
        k0 = Dot(diff, diff) - rDiffSqr;
        k1 = Dot(diff, line.direction);
        discr = k1 * k1 - k0;
        root = std::sqrt(std::max(discr, (Real)0));
        tmp = -k1 - root;
        if (tmp > tPos)
        {
            tPos = tmp;
        }
        tmp = -k1 + root;
        if (tmp < tNeg)
        {
            tNeg = tmp;
        }

        Vector3<Real> center = line.origin + (Real)0.5 * (tPos + tNeg) * line.direction;

        Real extent;
        if (tPos > tNeg)
        {
            // Container is a capsule.
            extent = (Real)0.5 * (tPos - tNeg);
        }
        else
        {
            // Container is a sphere.
            extent = (Real)0;
        }

        merge.segment = Segment3<Real>(center, line.direction, extent);
        merge.radius = radius;
        return true;
    }
}
