// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/ApprGaussian2.h>

namespace gte
{
    // Compute an oriented bounding box of the points.  The box center is the
    // average of the points.  The box axes are the eigenvectors of the
    // covariance matrix.
    template <typename Real>
    bool GetContainer(int numPoints, Vector2<Real> const* points, OrientedBox2<Real>& box)
    {
        // Fit the points with a Gaussian distribution.
        ApprGaussian2<Real> fitter;
        if (fitter.Fit(numPoints, points))
        {
            box = fitter.GetParameters();

            // Let C be the box center and let U0 and U1 be the box axes.
            // Each input point is of the form X = C + y0*U0 + y1*U1.  The
            // following code computes min(y0), max(y0), min(y1), and max(y1).
            // The box center is then adjusted to be
            //   C' = C + 0.5*(min(y0)+max(y0))*U0 + 0.5*(min(y1)+max(y1))*U1

            Vector2<Real> diff = points[0] - box.center;
            Vector2<Real> pmin{ Dot(diff, box.axis[0]), Dot(diff, box.axis[1]) };
            Vector2<Real> pmax = pmin;
            for (int i = 1; i < numPoints; ++i)
            {
                diff = points[i] - box.center;
                for (int j = 0; j < 2; ++j)
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

            for (int j = 0; j < 2; ++j)
            {
                box.center += ((Real)0.5 * (pmin[j] + pmax[j])) * box.axis[j];
                box.extent[j] = (Real)0.5 * (pmax[j] - pmin[j]);
            }
            return true;
        }

        return false;
    }

    template <typename Real>
    bool GetContainer(std::vector<Vector2<Real>> const& points, OrientedBox2<Real>& box)
    {
        return GetContainer(static_cast<int>(points.size()), points.data(), box);
    }

    // Test for containment.  Let X = C + y0*U0 + y1*U1 where C is the box
    // center and U0 and U1 are the orthonormal axes of the box.  X is in the
    // box when |y_i| <= E_i for all i, where E_i are the extents of the box.
    template <typename Real>
    bool InContainer(Vector2<Real> const& point, OrientedBox2<Real> const& box)
    {
        Vector2<Real> diff = point - box.center;
        for (int i = 0; i < 2; ++i)
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
    // result is not guaranteed to be the minimum area box containing the
    // input boxes.
    template <typename Real>
    bool MergeContainers(OrientedBox2<Real> const& box0,
        OrientedBox2<Real> const& box1, OrientedBox2<Real>& merge)
    {
        // The first guess at the box center.  This value will be updated
        // later after the input box vertices are projected onto axes
        // determined by an average of box axes.
        merge.center = (Real)0.5 * (box0.center + box1.center);

        // The merged box axes are the averages of the input box axes.  The
        // axes of the second box are negated, if necessary, so they form
        // acute angles with the axes of the first box.
        if (Dot(box0.axis[0], box1.axis[0]) >= (Real)0)
        {
            merge.axis[0] = (Real)0.5 * (box0.axis[0] + box1.axis[0]);
        }
        else
        {
            merge.axis[0] = (Real)0.5 * (box0.axis[0] - box1.axis[0]);
        }
        Normalize(merge.axis[0]);
        merge.axis[1] = -Perp(merge.axis[0]);

        // Project the input box vertices onto the merged-box axes.  Each
        // axis D[i] containing the current center C has a minimum projected
        // value min[i] and a maximum projected value max[i].  The
        // corresponding endpoints on the axes are C+min[i]*D[i] and
        // C+max[i]*D[i].  The point C is not necessarily the midpoint for
        // any of the intervals.  The actual box center will be adjusted from
        // C to a point C' that is the midpoint of each interval,
        //   C' = C + sum_{i=0}^1 0.5*(min[i]+max[i])*D[i]
        // The box extents are
        //   e[i] = 0.5*(max[i]-min[i])

        std::array<Vector2<Real>, 4> vertex;
        Vector2<Real> pmin{ (Real)0, (Real)0 };
        Vector2<Real> pmax{ (Real)0, (Real)0 };

        box0.GetVertices(vertex);
        for (int i = 0; i < 4; ++i)
        {
            Vector2<Real> diff = vertex[i] - merge.center;
            for (int j = 0; j < 2; ++j)
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
        for (int i = 0; i < 4; ++i)
        {
            Vector2<Real> diff = vertex[i] - merge.center;
            for (int j = 0; j < 2; ++j)
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
        for (int j = 0; j < 2; ++j)
        {
            merge.center += half * (pmax[j] + pmin[j]) * merge.axis[j];
            merge.extent[j] = half * (pmax[j] - pmin[j]);
        }

        return true;
    }
}
