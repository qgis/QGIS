// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Matrix3x3.h>
#include <Mathematics/SymmetricEigensolver3x3.h>

namespace gte
{
    // Least-squares fit of a great circle to unit-length vectors (x,y,z) by
    // using distance measurements orthogonal (and measured along great
    // circles) to the proposed great circle.  The inputs akPoint[] are unit
    // length.  The returned value is unit length, call it N.  The fitted
    // great circle is defined by Dot(N,X) = 0, where X is a unit-length
    // vector on the great circle.
    template <typename Real>
    class ApprGreatCircle3
    {
    public:
        void operator()(int numPoints, Vector3<Real> const* points, Vector3<Real>& normal) const
        {
            // Compute the covariance matrix of the vectors.
            Real covar00 = (Real)0, covar01 = (Real)0, covar02 = (Real)0;
            Real covar11 = (Real)0, covar12 = (Real)0, covar22 = (Real)0;
            for (int i = 0; i < numPoints; i++)
            {
                Vector3<Real> diff = points[i];
                covar00 += diff[0] * diff[0];
                covar01 += diff[0] * diff[1];
                covar02 += diff[0] * diff[2];
                covar11 += diff[1] * diff[1];
                covar12 += diff[1] * diff[2];
                covar22 += diff[2] * diff[2];
            }

            Real invNumPoints = (Real)1 / static_cast<Real>(numPoints);
            covar00 *= invNumPoints;
            covar01 *= invNumPoints;
            covar02 *= invNumPoints;
            covar11 *= invNumPoints;
            covar12 *= invNumPoints;
            covar22 *= invNumPoints;

            // Solve the eigensystem.
            SymmetricEigensolver3x3<Real> es;
            std::array<Real, 3> eval;
            std::array<std::array<Real, 3>, 3> evec;
            es(covar00, covar01, covar02, covar11, covar12, covar22, false, +1,
                eval, evec);
            normal = evec[0];
        }
    };


    // In addition to the least-squares fit of a great circle, the input
    // vectors are projected onto that circle.  The sector of smallest angle
    // (possibly obtuse) that contains the points is computed.  The endpoints
    // of the arc of the sector are returned.  The returned endpoints A0 and
    // A1 are perpendicular to the returned normal N.  Moreover, when you view
    // the arc by looking at the plane of the great circle with a view
    // direction of -N, the arc is traversed counterclockwise starting at A0
    // and ending at A1.
    template <typename Real>
    class ApprGreatArc3
    {
    public:
        void operator()(int numPoints, Vector3<Real> const* points,
            Vector3<Real>& normal, Vector3<Real>& arcEnd0,
            Vector3<Real>& arcEnd1) const
        {
            // Get the least-squares great circle for the vectors.  The circle
            // is on the plane Dot(N,X) = 0.  Generate a basis from N.
            Vector3<Real> basis[3];  // { N, U, V }
            ApprGreatCircle3<Real>()(numPoints, points, basis[0]);
            ComputeOrthogonalComplement(1, basis);

            // The vectors are X[i] = u[i]*U + v[i]*V + w[i]*N.  The
            // projections are
            //   P[i] = (u[i]*U + v[i]*V)/sqrt(u[i]*u[i] + v[i]*v[i])
            // The great circle is parameterized by
            //   C(t) = cos(t)*U + sin(t)*V
            // Compute the angles t in [-pi,pi] for the projections onto the
            // great circle.  It is not necesarily to normalize (u[i],v[i]),
            // instead computing t = atan2(v[i],u[i]).  The items[] represents
            // (u, v, angle).
            std::vector<std::array<Real, 3>> items(numPoints);
            for (int i = 0; i < numPoints; ++i)
            {
                items[i][0] = Dot(basis[1], points[i]);
                items[i][1] = Dot(basis[2], points[i]);
                items[i][2] = std::atan2(items[i][1], items[i][0]);
            }
            std::sort(items.begin(), items.end(),
                [](std::array<Real, 3> const& item0, std::array<Real, 3> const& item1)
                {
                    return item0[2] < item1[2];
                }
            );

            // Locate the pair of consecutive angles whose difference is a
            // maximum.  Effectively, we are constructing a cone of minimum
            // angle that contains the unit-length vectors.
            int numPointsM1 = numPoints - 1;
            Real maxDiff = (Real)GTE_C_TWO_PI + items[0][2] - items[numPointsM1][2];
            int end0 = 0, end1 = numPointsM1;
            for (int i0 = 0, i1 = 1; i0 < numPointsM1; i0 = i1++)
            {
                Real diff = items[i1][2] - items[i0][2];
                if (diff > maxDiff)
                {
                    maxDiff = diff;
                    end0 = i1;
                    end1 = i0;
                }
            }

            normal = basis[0];
            arcEnd0 = items[end0][0] * basis[1] + items[end0][1] * basis[2];
            arcEnd1 = items[end1][0] * basis[1] + items[end1][1] * basis[2];
            Normalize(arcEnd0);
            Normalize(arcEnd1);
        }
    };
}
