// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Polyhedron3.h>

namespace gte
{
    template <typename Real>
    class ExtremalQuery3
    {
    public:
        // Abstract base class.
        virtual ~ExtremalQuery3() = default;

        // Disallow copying and assignment.
        ExtremalQuery3(ExtremalQuery3 const&) = delete;
        ExtremalQuery3& operator=(ExtremalQuery3 const&) = delete;

        // Member access.
        inline Polyhedron3<Real> const& GetPolytope() const
        {
            return mPolytope;
        }

        inline std::vector<Vector3<Real>> const& GetFaceNormals() const
        {
            return mFaceNormals;
        }

        // Compute the extreme vertices in the specified direction and return
        // the indices of the vertices in the polyhedron vertex array.
        virtual void GetExtremeVertices(Vector3<Real> const& direction,
            int& positiveDirection, int& negativeDirection) = 0;

    protected:
        // The caller must ensure that the input polyhedron is convex.
        ExtremalQuery3(Polyhedron3<Real> const& polytope)
            :
            mPolytope(polytope)
        {
            // Create the face normals.
            auto vertexPool = mPolytope.GetVertices();
            auto const& indices = mPolytope.GetIndices();
            int const numTriangles = static_cast<int>(indices.size()) / 3;
            mFaceNormals.resize(numTriangles);
            for (int t = 0; t < numTriangles; ++t)
            {
                Vector3<Real> v0 = vertexPool[indices[3 * t + 0]];
                Vector3<Real> v1 = vertexPool[indices[3 * t + 1]];
                Vector3<Real> v2 = vertexPool[indices[3 * t + 2]];
                Vector3<Real> edge1 = v1 - v0;
                Vector3<Real> edge2 = v2 - v0;
                mFaceNormals[t] = UnitCross(edge1, edge2);
            }
        }

        Polyhedron3<Real> const& mPolytope;
        std::vector<Vector3<Real>> mFaceNormals;
    };
}
