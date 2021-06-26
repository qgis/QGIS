// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/TIQuery.h>
#include <Mathematics/OrientedBox.h>
#include <Mathematics/Vector3.h>

// The queries consider the box to be a solid.
//
// The test-intersection query uses the method of separating axes.
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
// The set of potential separating directions includes the 3 face normals of
// box0, the 3 face normals of box1, and 9 directions, each of which is the
// cross product of an edge of box0 and and an edge of box1.
//
// The separating axes involving cross products of edges has numerical
// robustness problems when the two edges are nearly parallel.  The cross
// product of the edges is nearly the zero vector, so normalization of the
// cross product may produce unit-length directions that are not close to the
// true direction.  Such a pair of edges occurs when a box0 face normal N0 and
// a box1 face normal N1 are nearly parallel.  In this case, you may skip the
// edge-edge directions, which is equivalent to projecting the boxes onto the
// plane with normal N0 and applying a 2D separating axis test.  The ability
// to do so involves choosing a small nonnegative epsilon.  It is used to
// determine whether two face normals, one from each box, are nearly parallel:
// |Dot(N0,N1)| >= 1 - epsilon.  If the input is negative, it is clamped to
// zero.
//
// The pair of integers 'separating', say, (i0,i1), identify the axis that
// reported separation; there may be more than one but only one is
// reported.  If the separating axis is a face normal N[i0] of the aligned
// box0 in dimension i0, then (i0,-1) is returned.  If the axis is a face
// normal box1.Axis[i1], then (-1,i1) is returned.  If the axis is a cross
// product of edges, Cross(N[i0],box1.Axis[i1]), then (i0,i1) is returned.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, OrientedBox3<Real>, OrientedBox3<Real>>
    {
    public:
        struct Result
        {
            // The 'epsilon' value must be nonnegative.
            Result(Real inEpsilon = (Real)0)
                :
                epsilon(inEpsilon >= (Real)0 ? inEpsilon : (Real)0)
            {
            }

            bool intersect;
            Real epsilon;
            int separating[2];
        };

        Result operator()(OrientedBox3<Real> const& box0, OrientedBox3<Real> const& box1)
        {
            Result result;

            // Convenience variables.
            Vector3<Real> const& C0 = box0.center;
            Vector3<Real> const* A0 = &box0.axis[0];
            Vector3<Real> const& E0 = box0.extent;
            Vector3<Real> const& C1 = box1.center;
            Vector3<Real> const* A1 = &box1.axis[0];
            Vector3<Real> const& E1 = box1.extent;

            Real const cutoff = (Real)1 - result.epsilon;
            bool existsParallelPair = false;

            // Compute difference of box centers.
            Vector3<Real> D = C1 - C0;

            // dot01[i][j] = Dot(A0[i],A1[j]) = A1[j][i]
            Real dot01[3][3];

            // |dot01[i][j]|
            Real absDot01[3][3];

            // Dot(D, A0[i])
            Real dotDA0[3];

            // interval radii and distance between centers
            Real r0, r1, r;

            // r0 + r1
            Real r01;

            // Test for separation on the axis C0 + t*A0[0].
            for (int i = 0; i < 3; ++i)
            {
                dot01[0][i] = Dot(A0[0], A1[i]);
                absDot01[0][i] = std::fabs(dot01[0][i]);
                if (absDot01[0][i] > cutoff)
                {
                    existsParallelPair = true;
                }
            }
            dotDA0[0] = Dot(D, A0[0]);
            r = std::fabs(dotDA0[0]);
            r1 = E1[0] * absDot01[0][0] + E1[1] * absDot01[0][1] + E1[2] * absDot01[0][2];
            r01 = E0[0] + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 0;
                result.separating[1] = -1;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[1].
            for (int i = 0; i < 3; ++i)
            {
                dot01[1][i] = Dot(A0[1], A1[i]);
                absDot01[1][i] = std::fabs(dot01[1][i]);
                if (absDot01[1][i] > cutoff)
                {
                    existsParallelPair = true;
                }
            }
            dotDA0[1] = Dot(D, A0[1]);
            r = std::fabs(dotDA0[1]);
            r1 = E1[0] * absDot01[1][0] + E1[1] * absDot01[1][1] + E1[2] * absDot01[1][2];
            r01 = E0[1] + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 1;
                result.separating[1] = -1;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[2].
            for (int i = 0; i < 3; ++i)
            {
                dot01[2][i] = Dot(A0[2], A1[i]);
                absDot01[2][i] = std::fabs(dot01[2][i]);
                if (absDot01[2][i] > cutoff)
                {
                    existsParallelPair = true;
                }
            }
            dotDA0[2] = Dot(D, A0[2]);
            r = std::fabs(dotDA0[2]);
            r1 = E1[0] * absDot01[2][0] + E1[1] * absDot01[2][1] + E1[2] * absDot01[2][2];
            r01 = E0[2] + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 2;
                result.separating[1] = -1;
                return result;
            }

            // Test for separation on the axis C0 + t*A1[0].
            r = std::fabs(Dot(D, A1[0]));
            r0 = E0[0] * absDot01[0][0] + E0[1] * absDot01[1][0] + E0[2] * absDot01[2][0];
            r01 = r0 + E1[0];
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = -1;
                result.separating[1] = 0;
                return result;
            }

            // Test for separation on the axis C0 + t*A1[1].
            r = std::fabs(Dot(D, A1[1]));
            r0 = E0[0] * absDot01[0][1] + E0[1] * absDot01[1][1] + E0[2] * absDot01[2][1];
            r01 = r0 + E1[1];
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = -1;
                result.separating[1] = 1;
                return result;
            }

            // Test for separation on the axis C0 + t*A1[2].
            r = std::fabs(Dot(D, A1[2]));
            r0 = E0[0] * absDot01[0][2] + E0[1] * absDot01[1][2] + E0[2] * absDot01[2][2];
            r01 = r0 + E1[2];
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = -1;
                result.separating[1] = 2;
                return result;
            }

            // At least one pair of box axes was parallel, so the separation is
            // effectively in 2D.  The edge-edge axes do not need to be tested.
            if (existsParallelPair)
            {
                return true;
            }

            // Test for separation on the axis C0 + t*A0[0]xA1[0].
            r = std::fabs(dotDA0[2] * dot01[1][0] - dotDA0[1] * dot01[2][0]);
            r0 = E0[1] * absDot01[2][0] + E0[2] * absDot01[1][0];
            r1 = E1[1] * absDot01[0][2] + E1[2] * absDot01[0][1];
            r01 = r0 + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 0;
                result.separating[1] = 0;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[0]xA1[1].
            r = std::fabs(dotDA0[2] * dot01[1][1] - dotDA0[1] * dot01[2][1]);
            r0 = E0[1] * absDot01[2][1] + E0[2] * absDot01[1][1];
            r1 = E1[0] * absDot01[0][2] + E1[2] * absDot01[0][0];
            r01 = r0 + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 0;
                result.separating[1] = 1;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[0]xA1[2].
            r = std::fabs(dotDA0[2] * dot01[1][2] - dotDA0[1] * dot01[2][2]);
            r0 = E0[1] * absDot01[2][2] + E0[2] * absDot01[1][2];
            r1 = E1[0] * absDot01[0][1] + E1[1] * absDot01[0][0];
            r01 = r0 + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 0;
                result.separating[1] = 2;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[1]xA1[0].
            r = std::fabs(dotDA0[0] * dot01[2][0] - dotDA0[2] * dot01[0][0]);
            r0 = E0[0] * absDot01[2][0] + E0[2] * absDot01[0][0];
            r1 = E1[1] * absDot01[1][2] + E1[2] * absDot01[1][1];
            r01 = r0 + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 1;
                result.separating[1] = 0;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[1]xA1[1].
            r = std::fabs(dotDA0[0] * dot01[2][1] - dotDA0[2] * dot01[0][1]);
            r0 = E0[0] * absDot01[2][1] + E0[2] * absDot01[0][1];
            r1 = E1[0] * absDot01[1][2] + E1[2] * absDot01[1][0];
            r01 = r0 + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 1;
                result.separating[1] = 1;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[1]xA1[2].
            r = std::fabs(dotDA0[0] * dot01[2][2] - dotDA0[2] * dot01[0][2]);
            r0 = E0[0] * absDot01[2][2] + E0[2] * absDot01[0][2];
            r1 = E1[0] * absDot01[1][1] + E1[1] * absDot01[1][0];
            r01 = r0 + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 1;
                result.separating[1] = 2;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[2]xA1[0].
            r = std::fabs(dotDA0[1] * dot01[0][0] - dotDA0[0] * dot01[1][0]);
            r0 = E0[0] * absDot01[1][0] + E0[1] * absDot01[0][0];
            r1 = E1[1] * absDot01[2][2] + E1[2] * absDot01[2][1];
            r01 = r0 + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 2;
                result.separating[1] = 0;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[2]xA1[1].
            r = std::fabs(dotDA0[1] * dot01[0][1] - dotDA0[0] * dot01[1][1]);
            r0 = E0[0] * absDot01[1][1] + E0[1] * absDot01[0][1];
            r1 = E1[0] * absDot01[2][2] + E1[2] * absDot01[2][0];
            r01 = r0 + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 2;
                result.separating[1] = 1;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[2]xA1[2].
            r = std::fabs(dotDA0[1] * dot01[0][2] - dotDA0[0] * dot01[1][2]);
            r0 = E0[0] * absDot01[1][2] + E0[1] * absDot01[0][2];
            r1 = E1[0] * absDot01[2][1] + E1[1] * absDot01[2][0];
            r01 = r0 + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 2;
                result.separating[1] = 2;
                return result;
            }

            result.intersect = true;
            return result;
        }
    };
}
