// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/AlignedBox.h>
#include <Mathematics/Vector4.h>
#include <cstdint>
#include <vector>

namespace gte
{
    template <typename Real>
    class ConvexPolyhedron3
    {
    public:
        // Construction.  The convex polyhedra represented by this class has
        // triangle faces that are counterclockwise ordered when viewed from
        // outside the polyhedron.  No attempt is made to verify that the
        // polyhedron is convex; the caller is responsible for enforcing this.
        // The constructors move (not copy!) the input arrays.  The
        // constructor succeeds when the number of vertices is at least 4 and
        // the number of indices is at least 12.  If the constructor fails,
        // no move occurs and the member arrays have no elements.
        //
        // To support geometric algorithms that are formulated using convex
        // quadratic programming (such as computing the distance from a point
        // to a convex polyhedron), it is necessary to know the planes of the
        // faces and an axis-aligned bounding box.  If you want either the
        // faces or the box, pass 'true' to the appropriate parameters.  When
        // planes are generated, the normals are not created to be unit length
        // in order to support queries using exact rational arithmetic.  If a
        // normal to a face is N = (n0,n1,n2) and V is a vertex of the face,
        // the plane is Dot(N,X-V) = 0 and is stored as
        // Dot(n0,n1,n2,-Dot(N,V)).  The normals are computed to be outer
        // pointing.
        ConvexPolyhedron3() = default;

        ConvexPolyhedron3(std::vector<Vector3<Real>>&& inVertices, std::vector<int>&& inIndices,
            bool wantPlanes, bool wantAlignedBox)
        {
            if (inVertices.size() >= 4 && inIndices.size() >= 12)
            {
                vertices = std::move(inVertices);
                indices = std::move(inIndices);

                if (wantPlanes)
                {
                    GeneratePlanes();
                }

                if (wantAlignedBox)
                {
                    GenerateAlignedBox();
                }
            }
        }

        // If you modifty the vertices or indices and you want the new face
        // planes or aligned box computed, call these functions.
        void GeneratePlanes()
        {
            if (vertices.size() > 0 && indices.size() > 0)
            {
                uint32_t const numTriangles = static_cast<uint32_t>(indices.size()) / 3;
                planes.resize(numTriangles);
                for (uint32_t t = 0, i = 0; t < numTriangles; ++t)
                {
                    Vector3<Real> V0 = vertices[indices[i++]];
                    Vector3<Real> V1 = vertices[indices[i++]];
                    Vector3<Real> V2 = vertices[indices[i++]];
                    Vector3<Real> E1 = V1 - V0;
                    Vector3<Real> E2 = V2 - V0;
                    Vector3<Real> N = Cross(E1, E2);
                    planes[t] = HLift(N, -Dot(N, V0));
                }
            }
        }

        void GenerateAlignedBox()
        {
            if (vertices.size() > 0 && indices.size() > 0)
            {
                ComputeExtremes(static_cast<int>(vertices.size()), vertices.data(),
                    alignedBox.min, alignedBox.max);
            }
        }

        std::vector<Vector3<Real>> vertices;
        std::vector<int> indices;
        std::vector<Vector4<Real>> planes;
        AlignedBox3<Real> alignedBox;
    };
}
