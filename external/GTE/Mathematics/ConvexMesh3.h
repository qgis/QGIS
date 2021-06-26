// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.6.2020.04.08

#pragma once

#include <Mathematics/Vector3.h>
#include <vector>

namespace gte
{
    template <typename Real>
    class ConvexMesh3
    {
    public:
        // A client of ConvexMesh3 is responsible for populating the vertices
        // and indices so that the resulting mesh represents a convex
        // polyhedron.
        //   1. All elements of 'vertices' must be used by the polyhedron.
        //   2. The triangle faces must have the same chirality when viewed
        //      from outside the polyhedron. They are all counterclockwise
        //      oriented or all clockwise oriented when viewed from outside
        //      the polyhedron.
        //   3. The Real type must be an arbitrary-precision type that
        //      supports division.
        //   4. The polyhedron can be degenerate. All the possibilities are
        //      listed next.
        //        point:
        //          vertices.size() == 1, triangles.size() = 0
        //
        //        line segment:
        //          vertices.size() == 2, triangles.size() == 0
        //
        //        convex polygon:
        //          vertices.size() >= 3, triangles.size() > 0 and the
        //          vertices are coplanar
        //
        //        convex polyhedron:
        //          vertices.size() >= 3, triangles.size() > 0 and the
        //          vertices are not coplanar

        static int constexpr CFG_EMPTY = 0x00000000;
        static int constexpr CFG_POINT = 0x00000001;
        static int constexpr CFG_SEGMENT = 0x00000002;
        static int constexpr CFG_POLYGON = 0x00000004;
        static int constexpr CFG_POLYHEDRON = 0x00000008;

        using Vertex = Vector3<Real>;
        using Triangle = std::array<int, 3>;

        int configuration = CFG_EMPTY;
        std::vector<Vertex> vertices;
        std::vector<Triangle> triangles;
    };
}
