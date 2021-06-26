// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/ApprGaussian3.h>
#include <Mathematics/Matrix3x3.h>
#include <Mathematics/Rotation.h>

namespace gte
{
    // Compute an oriented bounding box of the points.  The box center is the
    // average of the points.  The box axes are the eigenvectors of the
    // covariance matrix.
    template <typename Real>
    bool GetContainer(int numPoints, Vector3<Real> const* points, OrientedBox3<Real>& box)
    {
        // Fit the points with a Gaussian distribution.
        ApprGaussian3<Real> fitter;
        if (fitter.Fit(numPoints, points))
        {
            box = fitter.GetParameters();

            // Let C be the box center and let U0, U1, and U2 be the box axes.
            // Each input point is of the form X = C + y0*U0 + y1*U1 + y2*U2.
            // The following code computes min(y0), max(y0), min(y1), max(y1),
            // min(y2), and max(y2).  The box center is then adjusted to be
            //   C' = C + 0.5*(min(y0)+max(y0))*U0 + 0.5*(min(y1)+max(y1))*U1
            //        + 0.5*(min(y2)+max(y2))*U2

            Vector3<Real> diff = points[0] - box.center;
            Vector3<Real> pmin{ Dot(diff, box.axis[0]), Dot(diff, box.axis[1]),
                Dot(diff, box.axis[2]) };
            Vector3<Real> pmax = pmin;
            for (int i = 1; i < numPoints; ++i)
            {
                diff = points[i] - box.center;
                for (int j = 0; j < 3; ++j)
                {
                    Real dot = Dot(diff, box.axis[j]);
                    if (dot < pmin[j])
                    {
                        pmin[j] = dot;
                    }
                    else if (dot > pmax[j])
                    {
                        pmax[j] = dot;
                    }
                }
            }

            for (int j = 0; j < 3; ++j)
            {
                box.center += ((Real)0.5 * (pmin[j] + pmax[j])) * box.axis[j];
                box.extent[j] = (Real)0.5 * (pmax[j] - pmin[j]);
            }
            return true;
        }

        return false;
    }

    template <typename Real>
    bool GetContainer(std::vector<Vector3<Real>> const& points, OrientedBox3<Real>& box)
    {
        return GetContainer(static_cast<int>(points.size()), points.data(), box);
    }

    // Test for containment.  Let X = C + y0*U0 + y1*U1 + y2*U2 where C is the
    // box center and U0, U1, U2 are the orthonormal axes of the box.  X is in
    // the box if |y_i| <= E_i for all i where E_i are the extents of the box.
    template <typename Real>
    bool InContainer(Vector3<Real> const& point, OrientedBox3<Real> const& box)
    {
        Vector3<Real> diff = point - box.center;
        for (int i = 0; i < 3; ++i)
        {
            Real coeff = Dot(diff, box.axis[i]);
            if (std::fabs(coeff) > box.extent[i])
            {
                return false;
            }
        }
        return true;
    }

    // Construct an oriented box that contains two other oriented boxes.  The
    // result is not guaranteed to be the minimum volume box containing the
    // input boxes.
    template <typename Real>
    bool MergeContainers(OrientedBox3<Real> const& box0,
        OrientedBox3<Real> const& box1, OrientedBox3<Real>& merge)
    {
        // The first guess at the box center.  This value will be updated
        // later after the input box vertices are projected onto axes
        // determined by an average of box axes.
        merge.center = (Real)0.5 * (box0.center + box1.center);

        // A box's axes, when viewed as the columns of a matrix, form a
        // rotation matrix.  The input box axes are converted to quaternions.
        // The average quaternion is computed, then normalized to unit length.
        // The result is the slerp of the two input quaternions with t-value
        // of 1/2.  The result is converted back to a rotation matrix and its
        // columns are selected as the merged box axes.
        //
        // TODO: When the GTL Lie Algebra code is posted, use the geodesic
        // path between the affine matrices formed by the box centers and
        // orientations.  Choose t = 1/2 along that geodesic.
        Matrix3x3<Real> rot0, rot1;
        rot0.SetCol(0, box0.axis[0]);
        rot0.SetCol(1, box0.axis[1]);
        rot0.SetCol(2, box0.axis[2]);
        rot1.SetCol(0, box1.axis[0]);
        rot1.SetCol(1, box1.axis[1]);
        rot1.SetCol(2, box1.axis[2]);
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

        // Project the input box vertices onto the merged-box axes.  Each axis
        // D[i] containing the current center C has a minimum projected value
        // min[i] and a maximum projected value max[i].  The corresponding end
        // points on the axes are C+min[i]*D[i] and C+max[i]*D[i].  The point
        // C is not necessarily the midpoint for any of the intervals.  The
        // actual box center will be adjusted from C to a point C' that is the
        // midpoint of each interval,
        //   C' = C + sum_{i=0}^2 0.5*(min[i]+max[i])*D[i]
        // The box extents are
        //   e[i] = 0.5*(max[i]-min[i])

        std::array<Vector3<Real>, 8> vertex;
        Vector3<Real> pmin{ (Real)0, (Real)0, (Real)0 };
        Vector3<Real> pmax{ (Real)0, (Real)0, (Real)0 };

        box0.GetVertices(vertex);
        for (int i = 0; i < 8; ++i)
        {
            Vector3<Real> diff = vertex[i] - merge.center;
            for (int j = 0; j < 3; ++j)
            {
                Real dot = Dot(diff, merge.axis[j]);
                if (dot > pmax[j])
                {
                    pmax[j] = dot;
                }
                else if (dot < pmin[j])
                {
                    pmin[j] = dot;
                }
            }
        }

        box1.GetVertices(vertex);
        for (int i = 0; i < 8; ++i)
        {
            Vector3<Real> diff = vertex[i] - merge.center;
            for (int j = 0; j < 3; ++j)
            {
                Real dot = Dot(diff, merge.axis[j]);
                if (dot > pmax[j])
                {
                    pmax[j] = dot;
                }
                else if (dot < pmin[j])
                {
                    pmin[j] = dot;
                }
            }
        }

        // [min,max] is the axis-aligned box in the coordinate system of the
        // merged box axes.  Update the current box center to be the center of
        // the new box.  Compute the extents based on the new center.
        Real const half = (Real)0.5;
        for (int j = 0; j < 3; ++j)
        {
            merge.center += half * (pmax[j] + pmin[j]) * merge.axis[j];
            merge.extent[j] = half * (pmax[j] - pmin[j]);
        }

        return true;
    }
}
