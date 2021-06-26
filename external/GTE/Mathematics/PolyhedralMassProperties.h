// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Matrix3x3.h>

namespace gte
{
    // The input triangle mesh must represent a polyhedron.  The triangles are
    // represented as triples of indices <V0,V1,V2> into the vertex array.
    // The index array has numTriangles such triples.  The Boolean value
    // 'bodyCoords is' 'true' if you want the inertia tensor to be relative to
    // body coordinates but 'false' if you want it to be relative to world
    // coordinates.
    //
    // The code assumes the rigid body has a constant density of 1.  If your
    // application assigns a constant density of 'd', then you must multiply
    // the output 'mass' by 'd' and the output 'inertia' by 'd'.

    template <typename Real>
    void ComputeMassProperties(Vector3<Real> const* vertices, int numTriangles,
        int const* indices, bool bodyCoords, Real& mass, Vector3<Real>& center,
        Matrix3x3<Real>& inertia)
    {
        Real const oneDiv6 = (Real)1 / (Real)6;
        Real const oneDiv24 = (Real)1 / (Real)24;
        Real const oneDiv60 = (Real)1 / (Real)60;
        Real const oneDiv120 = (Real)1 / (Real)120;

        // order:  1, x, y, z, x^2, y^2, z^2, xy, yz, zx
        std::array<Real, 10> integral;
        integral.fill((Real)0);

        int const* index = indices;
        for (int i = 0; i < numTriangles; ++i)
        {
            // Get vertices of triangle i.
            Vector3<Real> v0 = vertices[*index++];
            Vector3<Real> v1 = vertices[*index++];
            Vector3<Real> v2 = vertices[*index++];

            // Get cross product of edges and normal vector.
            Vector3<Real> V1mV0 = v1 - v0;
            Vector3<Real> V2mV0 = v2 - v0;
            Vector3<Real> N = Cross(V1mV0, V2mV0);

            // Compute integral terms.
            Real tmp0, tmp1, tmp2;
            Real f1x, f2x, f3x, g0x, g1x, g2x;
            tmp0 = v0[0] + v1[0];
            f1x = tmp0 + v2[0];
            tmp1 = v0[0] * v0[0];
            tmp2 = tmp1 + v1[0] * tmp0;
            f2x = tmp2 + v2[0] * f1x;
            f3x = v0[0] * tmp1 + v1[0] * tmp2 + v2[0] * f2x;
            g0x = f2x + v0[0] * (f1x + v0[0]);
            g1x = f2x + v1[0] * (f1x + v1[0]);
            g2x = f2x + v2[0] * (f1x + v2[0]);

            Real f1y, f2y, f3y, g0y, g1y, g2y;
            tmp0 = v0[1] + v1[1];
            f1y = tmp0 + v2[1];
            tmp1 = v0[1] * v0[1];
            tmp2 = tmp1 + v1[1] * tmp0;
            f2y = tmp2 + v2[1] * f1y;
            f3y = v0[1] * tmp1 + v1[1] * tmp2 + v2[1] * f2y;
            g0y = f2y + v0[1] * (f1y + v0[1]);
            g1y = f2y + v1[1] * (f1y + v1[1]);
            g2y = f2y + v2[1] * (f1y + v2[1]);

            Real f1z, f2z, f3z, g0z, g1z, g2z;
            tmp0 = v0[2] + v1[2];
            f1z = tmp0 + v2[2];
            tmp1 = v0[2] * v0[2];
            tmp2 = tmp1 + v1[2] * tmp0;
            f2z = tmp2 + v2[2] * f1z;
            f3z = v0[2] * tmp1 + v1[2] * tmp2 + v2[2] * f2z;
            g0z = f2z + v0[2] * (f1z + v0[2]);
            g1z = f2z + v1[2] * (f1z + v1[2]);
            g2z = f2z + v2[2] * (f1z + v2[2]);

            // Update integrals.
            integral[0] += N[0] * f1x;
            integral[1] += N[0] * f2x;
            integral[2] += N[1] * f2y;
            integral[3] += N[2] * f2z;
            integral[4] += N[0] * f3x;
            integral[5] += N[1] * f3y;
            integral[6] += N[2] * f3z;
            integral[7] += N[0] * (v0[1] * g0x + v1[1] * g1x + v2[1] * g2x);
            integral[8] += N[1] * (v0[2] * g0y + v1[2] * g1y + v2[2] * g2y);
            integral[9] += N[2] * (v0[0] * g0z + v1[0] * g1z + v2[0] * g2z);
        }

        integral[0] *= oneDiv6;
        integral[1] *= oneDiv24;
        integral[2] *= oneDiv24;
        integral[3] *= oneDiv24;
        integral[4] *= oneDiv60;
        integral[5] *= oneDiv60;
        integral[6] *= oneDiv60;
        integral[7] *= oneDiv120;
        integral[8] *= oneDiv120;
        integral[9] *= oneDiv120;

        // mass
        mass = integral[0];

        // center of mass
        center = Vector3<Real>{ integral[1], integral[2], integral[3] } / mass;

        // inertia relative to world origin
        inertia(0, 0) = integral[5] + integral[6];
        inertia(0, 1) = -integral[7];
        inertia(0, 2) = -integral[9];
        inertia(1, 0) = inertia(0, 1);
        inertia(1, 1) = integral[4] + integral[6];
        inertia(1, 2) = -integral[8];
        inertia(2, 0) = inertia(0, 2);
        inertia(2, 1) = inertia(1, 2);
        inertia(2, 2) = integral[4] + integral[5];

        // inertia relative to center of mass
        if (bodyCoords)
        {
            inertia(0, 0) -= mass * (center[1] * center[1] + center[2] * center[2]);
            inertia(0, 1) += mass * center[0] * center[1];
            inertia(0, 2) += mass * center[2] * center[0];
            inertia(1, 0) = inertia(0, 1);
            inertia(1, 1) -= mass * (center[2] * center[2] + center[0] * center[0]);
            inertia(1, 2) += mass * center[1] * center[2];
            inertia(2, 0) = inertia(0, 2);
            inertia(2, 1) = inertia(1, 2);
            inertia(2, 2) -= mass * (center[0] * center[0] + center[1] * center[1]);
        }
    }
}
