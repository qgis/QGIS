// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/ApprGaussian3.h>
#include <Mathematics/Hyperellipsoid.h>
#include <Mathematics/Matrix3x3.h>
#include <Mathematics/Projection.h>
#include <Mathematics/Rotation.h>

namespace gte
{
    // The input points are fit with a Gaussian distribution.  The center C of
    // the ellipsoid is chosen to be the mean of the distribution.  The axes
    // of the ellipsoid are chosen to be the eigenvectors of the covariance
    // matrix M.  The shape of the ellipsoid is determined by the absolute
    // values of the eigenvalues.  NOTE: The construction is ill-conditioned
    // if the points are (nearly) collinear or (nearly) planar.  In this case
    // M has a (nearly) zero eigenvalue, so inverting M is problematic.
    template <typename Real>
    bool GetContainer(int numPoints, Vector3<Real> const* points, Ellipsoid3<Real>& ellipsoid)
    {
        // Fit the points with a Gaussian distribution.  The covariance
        // matrix is M = sum_j D[j]*U[j]*U[j]^T, where D[j] are the
        // eigenvalues and U[j] are corresponding unit-length eigenvectors.
        ApprGaussian3<Real> fitter;
        if (fitter.Fit(numPoints, points))
        {
            OrientedBox3<Real> box = fitter.GetParameters();

            // If either eigenvalue is nonpositive, adjust the D[] values so
            // that we actually build an ellipsoid.
            for (int j = 0; j < 3; ++j)
            {
                if (box.extent[j] < (Real)0)
                {
                    box.extent[j] = -box.extent[j];
                }
            }

            // Grow the ellipsoid, while retaining its shape determined by the
            // covariance matrix, to enclose all the input points.  The
            // quadratic/ form that is used for the ellipsoid construction is
            //   Q(X) = (X-C)^T*M*(X-C)
            //        = (X-C)^T*(sum_j D[j]*U[j]*U[j]^T)*(X-C)
            //        = sum_j D[j]*Dot(U[j],X-C)^2
            // If the maximum value of Q(X[i]) for all input points is V^2,
            // then a bounding ellipsoid is Q(X) = V^2 since Q(X[i]) <= V^2
            // for all i.

            Real maxValue = (Real)0;
            for (int i = 0; i < numPoints; ++i)
            {
                Vector3<Real> diff = points[i] - box.center;
                Real dot[3] =
                {
                    Dot(box.axis[0], diff),
                    Dot(box.axis[1], diff),
                    Dot(box.axis[2], diff)
                };

                Real value =
                    box.extent[0] * dot[0] * dot[0] +
                    box.extent[1] * dot[1] * dot[1] +
                    box.extent[2] * dot[2] * dot[2];

                if (value > maxValue)
                {
                    maxValue = value;
                }
            }

            // Arrange for the quadratic to satisfy Q(X) <= 1.
            ellipsoid.center = box.center;
            for (int j = 0; j < 3; ++j)
            {
                ellipsoid.axis[j] = box.axis[j];
                ellipsoid.extent[j] = std::sqrt(maxValue / box.extent[j]);
            }
            return true;
        }

        return false;
    }

    // Test for containment of a point inside an ellipsoid.
    template <typename Real>
    bool InContainer(Vector3<Real> const& point, Ellipsoid3<Real> const& ellipsoid)
    {
        Vector3<Real> diff = point - ellipsoid.center;
        Vector3<Real> standardized{
            Dot(diff, ellipsoid.axis[0]) / ellipsoid.extent[0],
            Dot(diff, ellipsoid.axis[1]) / ellipsoid.extent[1],
            Dot(diff, ellipsoid.axis[2]) / ellipsoid.extent[2] };
        return Length(standardized) <= (Real)1;
    }

    // Construct a bounding ellipsoid for the two input ellipsoids.  The result is
    // not necessarily the minimum-volume ellipsoid containing the two ellipsoids.
    template <typename Real>
    bool MergeContainers(Ellipsoid3<Real> const& ellipsoid0,
        Ellipsoid3<Real> const& ellipsoid1, Ellipsoid3<Real>& merge)
    {
        // Compute the average of the input centers
        merge.center = (Real)0.5 * (ellipsoid0.center + ellipsoid1.center);

        // The bounding ellipsoid orientation is the average of the input
        // orientations.
        Matrix3x3<Real> rot0, rot1;
        rot0.SetCol(0, ellipsoid0.axis[0]);
        rot0.SetCol(1, ellipsoid0.axis[1]);
        rot0.SetCol(2, ellipsoid0.axis[2]);
        rot1.SetCol(0, ellipsoid1.axis[0]);
        rot1.SetCol(1, ellipsoid1.axis[1]);
        rot1.SetCol(2, ellipsoid1.axis[2]);
        Quaternion<Real> q0 = Rotation<3, Real>(rot0);
        Quaternion<Real> q1 = Rotation<3, Real>(rot1);
        if (Dot(q0, q1) < (Real)0)
        {
            q1 = -q1;
        }

        Quaternion<Real> q = q0 + q1;
        Normalize(q);
        Matrix3x3<Real> rot = Rotation<3, Real>(q);
        for (int j = 0; j < 3; ++j)
        {
            merge.axis[j] = rot.GetCol(j);
        }

        // Project the input ellipsoids onto the axes obtained by the average
        // of the orientations and that go through the center obtained by the
        // average of the centers.
        for (int i = 0; i < 3; ++i)
        {
            // Projection axis.
            Line3<Real> line(merge.center, merge.axis[i]);

            // Project ellipsoids onto the axis.
            Real min0, max0, min1, max1;
            Project(ellipsoid0, line, min0, max0);
            Project(ellipsoid1, line, min1, max1);

            // Determine the smallest interval containing the projected
            // intervals.
            Real maxIntr = (max0 >= max1 ? max0 : max1);
            Real minIntr = (min0 <= min1 ? min0 : min1);

            // Update the average center to be the center of the bounding box
            // defined by the projected intervals.
            merge.center += line.direction * ((Real)0.5 * (minIntr + maxIntr));

            // Compute the extents of the box based on the new center.
            merge.extent[i] = (Real)0.5 * (maxIntr - minIntr);
        }

        return true;
    }
}
