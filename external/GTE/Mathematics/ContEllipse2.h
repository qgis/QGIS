// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/ApprGaussian2.h>
#include <Mathematics/Hyperellipsoid.h>
#include <Mathematics/Projection.h>

namespace gte
{
    // The input points are fit with a Gaussian distribution.  The center C of
    // the ellipse is chosen to be the mean of the distribution.  The axes of
    // the ellipse are chosen to be the eigenvectors of the covariance matrix
    // M.  The shape of the ellipse is determined by the absolute values of
    // the eigenvalues.  NOTE: The construction is ill-conditioned if the
    // points are (nearly) collinear.  In this case M has a (nearly) zero
    // eigenvalue, so inverting M can be a problem numerically.
    template <typename Real>
    bool GetContainer(int numPoints, Vector2<Real> const* points, Ellipse2<Real>& ellipse)
    {
        // Fit the points with a Gaussian distribution.  The covariance matrix
        // is M = sum_j D[j]*U[j]*U[j]^T, where D[j] are the eigenvalues and
        // U[j] are corresponding unit-length eigenvectors.
        ApprGaussian2<Real> fitter;
        if (fitter.Fit(numPoints, points))
        {
            OrientedBox2<Real> box = fitter.GetParameters();

            // If either eigenvalue is nonpositive, adjust the D[] values so
            // that we actually build an ellipse.
            for (int j = 0; j < 2; ++j)
            {
                if (box.extent[j] < (Real)0)
                {
                    box.extent[j] = -box.extent[j];
                }
            }

            // Grow the ellipse, while retaining its shape determined by the
            // covariance matrix, to enclose all the input points.  The
            // quadratic form that is used for the ellipse construction is
            //   Q(X) = (X-C)^T*M*(X-C)
            //        = (X-C)^T*(sum_j D[j]*U[j]*U[j]^T)*(X-C)
            //        = sum_j D[j]*Dot(U[j],X-C)^2
            // If the maximum value of Q(X[i]) for all input points is V^2,
            // then a bounding ellipse is Q(X) = V^2, because Q(X[i]) <= V^2
            // for all i.

            Real maxValue = (Real)0;
            for (int i = 0; i < numPoints; ++i)
            {
                Vector2<Real> diff = points[i] - box.center;
                Real dot[2] =
                {
                    Dot(box.axis[0], diff),
                    Dot(box.axis[1], diff)
                };

                Real value =
                    box.extent[0] * dot[0] * dot[0] +
                    box.extent[1] * dot[1] * dot[1];

                if (value > maxValue)
                {
                    maxValue = value;
                }
            }

            // Arrange for the quadratic to satisfy Q(X) <= 1.
            ellipse.center = box.center;
            for (int j = 0; j < 2; ++j)
            {
                ellipse.axis[j] = box.axis[j];
                ellipse.extent[j] = std::sqrt(maxValue / box.extent[j]);
            }
            return true;

        }

        return false;
    }

    // Test for containment of a point inside an ellipse.
    template <typename Real>
    bool InContainer(Vector2<Real> const& point, Ellipse2<Real> const& ellipse)
    {
        Vector2<Real> diff = point - ellipse.center;
        Vector2<Real> standardized{
            Dot(diff, ellipse.axis[0]) / ellipse.extent[0],
            Dot(diff, ellipse.axis[1]) / ellipse.extent[1] };
        return Length(standardized) <= (Real)1;
    }

    // Construct a bounding ellipse for the two input ellipses.  The result is
    // not necessarily the minimum-area ellipse containing the two ellipses.
    template <typename Real>
    bool MergeContainers(Ellipse2<Real> const& ellipse0,
        Ellipse2<Real> const& ellipse1, Ellipse2<Real>& merge)
    {
        // Compute the average of the input centers.
        merge.center = (Real)0.5 * (ellipse0.center + ellipse1.center);

        // The bounding ellipse orientation is the average of the input
        // orientations.
        if (Dot(ellipse0.axis[0], ellipse1.axis[0]) >= (Real)0)
        {
            merge.axis[0] = (Real)0.5 * (ellipse0.axis[0] + ellipse1.axis[0]);
        }
        else
        {
            merge.axis[0] = (Real)0.5 * (ellipse0.axis[0] - ellipse1.axis[0]);
        }
        Normalize(merge.axis[0]);
        merge.axis[1] = -Perp(merge.axis[0]);

        // Project the input ellipses onto the axes obtained by the average
        // of the orientations and that go through the center obtained by the
        // average of the centers.
        for (int j = 0; j < 2; ++j)
        {
            // Projection axis.
            Line2<Real> line(merge.center, merge.axis[j]);

            // Project ellipsoids onto the axis.
            Real min0, max0, min1, max1;
            Project(ellipse0, line, min0, max0);
            Project(ellipse1, line, min1, max1);

            // Determine the smallest interval containing the projected
            // intervals.
            Real maxIntr = (max0 >= max1 ? max0 : max1);
            Real minIntr = (min0 <= min1 ? min0 : min1);

            // Update the average center to be the center of the bounding box
            // defined by the projected intervals.
            merge.center += line.direction * ((Real)0.5 * (minIntr + maxIntr));

            // Compute the extents of the box based on the new center.
            merge.extent[j] = (Real)0.5 * (maxIntr - minIntr);
        }

        return true;
    }
}
