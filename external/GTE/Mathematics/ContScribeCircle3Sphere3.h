// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Circle3.h>
#include <Mathematics/Hypersphere.h>
#include <Mathematics/LinearSystem.h>

namespace gte
{
    // All functions return 'true' if circle/sphere has been constructed,
    // 'false' otherwise (input points are linearly dependent).

    // Circle circumscribing a triangle in 3D.
    template <typename Real>
    bool Circumscribe(Vector3<Real> const& v0, Vector3<Real> const& v1,
        Vector3<Real> const& v2, Circle3<Real>& circle)
    {
        Vector3<Real> E02 = v0 - v2;
        Vector3<Real> E12 = v1 - v2;
        Real e02e02 = Dot(E02, E02);
        Real e02e12 = Dot(E02, E12);
        Real e12e12 = Dot(E12, E12);
        Real det = e02e02 * e12e12 - e02e12 * e02e12;
        if (det != (Real)0)
        {
            Real halfInvDet = (Real)0.5 / det;
            Real u0 = halfInvDet * e12e12 * (e02e02 - e02e12);
            Real u1 = halfInvDet * e02e02 * (e12e12 - e02e12);
            Vector3<Real> tmp = u0 * E02 + u1 * E12;
            circle.center = v2 + tmp;
            circle.normal = UnitCross(E02, E12);
            circle.radius = Length(tmp);
            return true;
        }
        return false;
    }

    // Sphere circumscribing a tetrahedron.
    template <typename Real>
    bool Circumscribe(Vector3<Real> const& v0, Vector3<Real> const& v1,
        Vector3<Real> const& v2, Vector3<Real> const& v3, Sphere3<Real>& sphere)
    {
        Vector3<Real> E10 = v1 - v0;
        Vector3<Real> E20 = v2 - v0;
        Vector3<Real> E30 = v3 - v0;

        Matrix3x3<Real> A;
        A.SetRow(0, E10);
        A.SetRow(1, E20);
        A.SetRow(2, E30);

        Vector3<Real> B{
            (Real)0.5 * Dot(E10, E10),
            (Real)0.5 * Dot(E20, E20),
            (Real)0.5 * Dot(E30, E30) };

        Vector3<Real> solution;
        if (LinearSystem<Real>::Solve(A, B, solution))
        {
            sphere.center = v0 + solution;
            sphere.radius = Length(solution);
            return true;
        }
        return false;
    }

    // Circle inscribing a triangle in 3D.
    template <typename Real>
    bool Inscribe(Vector3<Real> const& v0, Vector3<Real> const& v1,
        Vector3<Real> const& v2, Circle3<Real>& circle)
    {
        // Edges.
        Vector3<Real> E0 = v1 - v0;
        Vector3<Real> E1 = v2 - v1;
        Vector3<Real> E2 = v0 - v2;

        // Plane normal.
        circle.normal = Cross(E1, E0);

        // Edge normals within the plane.
        Vector3<Real> N0 = UnitCross(circle.normal, E0);
        Vector3<Real> N1 = UnitCross(circle.normal, E1);
        Vector3<Real> N2 = UnitCross(circle.normal, E2);

        Real a0 = Dot(N1, E0);
        if (a0 == (Real)0)
        {
            return false;
        }

        Real a1 = Dot(N2, E1);
        if (a1 == (Real)0)
        {
            return false;
        }

        Real a2 = Dot(N0, E2);
        if (a2 == (Real)0)
        {
            return false;
        }

        Real invA0 = (Real)1 / a0;
        Real invA1 = (Real)1 / a1;
        Real invA2 = (Real)1 / a2;

        circle.radius = (Real)1 / (invA0 + invA1 + invA2);
        circle.center = circle.radius * (invA0 * v0 + invA1 * v1 + invA2 * v2);
        Normalize(circle.normal);
        return true;
    }

    // Sphere inscribing tetrahedron.
    template <typename Real>
    bool Inscribe(Vector3<Real> const& v0, Vector3<Real> const& v1,
        Vector3<Real> const& v2, Vector3<Real> const& v3, Sphere3<Real>& sphere)
    {
        // Edges.
        Vector3<Real> E10 = v1 - v0;
        Vector3<Real> E20 = v2 - v0;
        Vector3<Real> E30 = v3 - v0;
        Vector3<Real> E21 = v2 - v1;
        Vector3<Real> E31 = v3 - v1;

        // Normals.
        Vector3<Real> N0 = Cross(E31, E21);
        Vector3<Real> N1 = Cross(E20, E30);
        Vector3<Real> N2 = Cross(E30, E10);
        Vector3<Real> N3 = Cross(E10, E20);

        // Normalize the normals.
        if (Normalize(N0) == (Real)0)
        {
            return false;
        }
        if (Normalize(N1) == (Real)0)
        {
            return false;
        }
        if (Normalize(N2) == (Real)0)
        {
            return false;
        }
        if (Normalize(N3) == (Real)0)
        {
            return false;
        }

        Matrix3x3<Real> A;
        A.SetRow(0, N1 - N0);
        A.SetRow(1, N2 - N0);
        A.SetRow(2, N3 - N0);
        Vector3<Real> B{ (Real)0, (Real)0, -Dot(N3, E30) };
        Vector3<Real> solution;
        if (LinearSystem<Real>::Solve(A, B, solution))
        {
            sphere.center = v3 + solution;
            sphere.radius = std::fabs(Dot(N0, solution));
            return true;
        }
        return false;
    }
}
