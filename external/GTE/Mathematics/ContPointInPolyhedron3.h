// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/ContPointInPolygon2.h>
#include <Mathematics/IntrRay3Plane3.h>
#include <Mathematics/IntrRay3Triangle3.h>
#include <vector>

// This class contains various implementations for point-in-polyhedron
// queries.  The planes stored with the faces are used in all cases to
// reject ray-face intersection tests, a quick culling operation.
//
// The algorithm is to cast a ray from the input point P and test for
// intersection against each face of the polyhedron.  If the ray only
// intersects faces at interior points (not vertices, not edge points),
// then the point is inside when the number of intersections is odd and
// the point is outside when the number of intersections is even.  If the
// ray intersects an edge or a vertex, then the counting must be handled
// differently.  The details are tedious.  As an alternative, the approach
// here is to allow you to specify 2*N+1 rays, where N >= 0.  You should
// choose these rays randomly.  Each ray reports "inside" or "outside".
// Whichever result occurs N+1 or more times is the "winner".  The input
// rayQuantity is 2*N+1.  The input array Direction must have rayQuantity
// elements.  If you are feeling lucky, choose rayQuantity to be 1.

namespace gte
{
    template <typename Real>
    class PointInPolyhedron3
    {
    public:
        // For simple polyhedra with triangle faces.
        class TriangleFace
        {
        public:
            // When you view the face from outside, the vertices are
            // counterclockwise ordered.  The indices array stores the indices
            // into the vertex array.
            std::array<int, 3> indices;

            // The normal vector is unit length and points to the outside of
            // the polyhedron.
            Plane3<Real> plane;
        };

        // The Contains query will use ray-triangle intersection queries.
        PointInPolyhedron3(int numPoints, Vector3<Real> const* points,
            int numFaces, TriangleFace const* faces, int numRays,
            Vector3<Real> const* directions)
            :
            mNumPoints(numPoints),
            mPoints(points),
            mNumFaces(numFaces),
            mTFaces(faces),
            mCFaces(nullptr),
            mSFaces(nullptr),
            mMethod(0),
            mNumRays(numRays),
            mDirections(directions)
        {
        }

        // For simple polyhedra with convex polygon faces.
        class ConvexFace
        {
        public:
            // When you view the face from outside, the vertices are
            // counterclockwise ordered.  The indices array stores the indices
            // into the vertex array.
            std::vector<int> indices;

            // The normal vector is unit length and points to the outside of
            // the polyhedron.
            Plane3<Real> plane;
        };

        // The Contains() query will use ray-convexpolygon intersection
        // queries.  A ray-convexpolygon intersection query can be implemented
        // in many ways.  In this context, uiMethod is one of three value:
        //   0 : Use a triangle fan and perform a ray-triangle intersection
        //       query for each triangle.
        //   1 : Find the point of intersection of ray and plane of polygon.
        //       Test whether that point is inside the convex polygon using an
        //       O(N) test.
        //   2 : Find the point of intersection of ray and plane of polygon.
        //       Test whether that point is inside the convex polygon using an
        //       O(log N) test.
        PointInPolyhedron3(int numPoints, Vector3<Real> const* points,
            int numFaces, ConvexFace const* faces, int numRays,
            Vector3<Real> const* directions, unsigned int method)
            :
            mNumPoints(numPoints),
            mPoints(points),
            mNumFaces(numFaces),
            mTFaces(nullptr),
            mCFaces(faces),
            mSFaces(nullptr),
            mMethod(method),
            mNumRays(numRays),
            mDirections(directions)
        {
        }

        // For simple polyhedra with simple polygon faces that are generally
        // not all convex.
        class SimpleFace
        {
        public:
            // When you view the face from outside, the vertices are
            // counterclockwise ordered.  The Indices array stores the indices
            // into the vertex array.
            std::vector<int> indices;

            // The normal vector is unit length and points to the outside of
            // the polyhedron.
            Plane3<Real> plane;

            // Each simple face may be triangulated.  The indices are relative
            // to the vertex array.  Each triple of indices represents a
            // triangle in the triangulation.
            std::vector<int> triangles;
        };

        // The Contains query will use ray-simplepolygon intersection queries.
        // A ray-simplepolygon intersection query can be implemented in a
        // couple of ways.  In this context, uiMethod is one of two value:
        //   0 : Iterate over the triangles of each face and perform a
        //       ray-triangle intersection query for each triangle.  This
        //       requires that the SimpleFace::Triangles array be initialized
        //       for each face.
        //   1 : Find the point of intersection of ray and plane of polygon.
        //       Test whether that point is inside the polygon using an O(N)
        //       test.  The SimpleFace::Triangles array is not used for this
        //       method, so it does not have to be initialized for each face.
        PointInPolyhedron3(int numPoints, Vector3<Real> const* points,
            int numFaces, SimpleFace const* faces, int numRays,
            Vector3<Real> const* directions, unsigned int method)
            :
            mNumPoints(numPoints),
            mPoints(points),
            mNumFaces(numFaces),
            mTFaces(nullptr),
            mCFaces(nullptr),
            mSFaces(faces),
            mMethod(method),
            mNumRays(numRays),
            mDirections(directions)
        {
        }

        // This function will select the actual algorithm based on which
        // constructor you used for this class.
        bool Contains(Vector3<Real> const& p) const
        {
            if (mTFaces)
            {
                return ContainsT0(p);
            }

            if (mCFaces)
            {
                if (mMethod == 0)
                {
                    return ContainsC0(p);
                }

                return ContainsC1C2(p, mMethod);
            }

            if (mSFaces)
            {
                if (mMethod == 0)
                {
                    return ContainsS0(p);
                }

                if (mMethod == 1)
                {
                    return ContainsS1(p);
                }
            }

            return false;
        }

    private:
        // For all types of faces.  The ray origin is the test point.  The ray
        // direction is one of those passed to the constructors.  The plane
        // origin is a point on the plane of the face.  The plane normal is a
        // unit-length normal to the face and that points outside the
        // polyhedron.
        static bool FastNoIntersect(Ray3<Real> const& ray, Plane3<Real> const& plane)
        {
            Real planeDistance = Dot(plane.normal, ray.origin) - plane.constant;
            Real planeAngle = Dot(plane.normal, ray.direction);

            if (planeDistance < (Real)0)
            {
                // The ray origin is on the negative side of the plane.
                if (planeAngle <= (Real)0)
                {
                    // The ray points away from the plane.
                    return true;
                }
            }

            if (planeDistance > (Real)0)
            {
                // The ray origin is on the positive side of the plane.
                if (planeAngle >= (Real)0)
                {
                    // The ray points away from the plane.
                    return true;
                }
            }

            return false;
        }

        // For triangle faces.
        bool ContainsT0(Vector3<Real> const& p) const
        {
            int insideCount = 0;

            TIQuery<Real, Ray3<Real>, Triangle3<Real>> rtQuery;
            Triangle3<Real> triangle;
            Ray3<Real> ray;
            ray.origin = p;

            for (int j = 0; j < mNumRays; ++j)
            {
                ray.direction = mDirections[j];

                // Zero intersections to start with.
                bool odd = false;

                TriangleFace const* face = mTFaces;
                for (int i = 0; i < mNumFaces; ++i, ++face)
                {
                    // Attempt to quickly cull the triangle.
                    if (FastNoIntersect(ray, face->plane))
                    {
                        continue;
                    }

                    // Get the triangle vertices.
                    for (int k = 0; k < 3; ++k)
                    {
                        triangle.v[k] = mPoints[face->indices[k]];
                    }

                    // Test for intersection.
                    if (rtQuery(ray, triangle).intersect)
                    {
                        // The ray intersects the triangle.
                        odd = !odd;
                    }
                }

                if (odd)
                {
                    insideCount++;
                }
            }

            return insideCount > mNumRays / 2;
        }

        // For convex faces.
        bool ContainsC0(Vector3<Real> const& p) const
        {
            int insideCount = 0;

            TIQuery<Real, Ray3<Real>, Triangle3<Real>> rtQuery;
            Triangle3<Real> triangle;
            Ray3<Real> ray;
            ray.origin = p;

            for (int j = 0; j < mNumRays; ++j)
            {
                ray.direction = mDirections[j];

                // Zero intersections to start with.
                bool odd = false;

                ConvexFace const* face = mCFaces;
                for (int i = 0; i < mNumFaces; ++i, ++face)
                {
                    // Attempt to quickly cull the triangle.
                    if (FastNoIntersect(ray, face->plane))
                    {
                        continue;
                    }

                    // Process the triangles in a trifan of the face.
                    size_t numVerticesM1 = face->indices.size() - 1;
                    triangle.v[0] = mPoints[face->indices[0]];
                    for (size_t k = 1; k < numVerticesM1; ++k)
                    {
                        triangle.v[1] = mPoints[face->indices[k]];
                        triangle.v[2] = mPoints[face->indices[k + 1]];

                        if (rtQuery(ray, triangle).intersect)
                        {
                            // The ray intersects the triangle.
                            odd = !odd;
                        }
                    }
                }

                if (odd)
                {
                    insideCount++;
                }
            }

            return insideCount > mNumRays / 2;
        }

        bool ContainsC1C2(Vector3<Real> const& p, unsigned int method) const
        {
            int insideCount = 0;

            FIQuery<Real, Ray3<Real>, Plane3<Real>> rpQuery;
            Ray3<Real> ray;
            ray.origin = p;

            for (int j = 0; j < mNumRays; ++j)
            {
                ray.direction = mDirections[j];

                // Zero intersections to start with.
                bool odd = false;

                ConvexFace const* face = mCFaces;
                for (int i = 0; i < mNumFaces; ++i, ++face)
                {
                    // Attempt to quickly cull the triangle.
                    if (FastNoIntersect(ray, face->plane))
                    {
                        continue;
                    }

                    // Compute the ray-plane intersection.
                    auto result = rpQuery(ray, face->plane);

                    // If you trigger this assertion, numerical round-off
                    // errors have led to a discrepancy between
                    // FastNoIntersect and the Find() result.
                    LogAssert(result.intersect, "Unexpected condition.");

                    // Get a coordinate system for the plane.  Use vertex 0
                    // as the origin.
                    Vector3<Real> const& V0 = mPoints[face->indices[0]];
                    Vector3<Real> basis[3];
                    basis[0] = face->plane.normal;
                    ComputeOrthogonalComplement(1, basis);

                    // Project the intersection onto the plane.
                    Vector3<Real> diff = result.point - V0;
                    Vector2<Real> projIntersect{ Dot(basis[1], diff), Dot(basis[2], diff) };

                    // Project the face vertices onto the plane of the face.
                    if (face->indices.size() > mProjVertices.size())
                    {
                        mProjVertices.resize(face->indices.size());
                    }

                    // Project the remaining vertices.  Vertex 0 is always the
                    // origin.
                    size_t numIndices = face->indices.size();
                    mProjVertices[0] = Vector2<Real>::Zero();
                    for (size_t k = 1; k < numIndices; ++k)
                    {
                        diff = mPoints[face->indices[k]] - V0;
                        mProjVertices[k][0] = Dot(basis[1], diff);
                        mProjVertices[k][1] = Dot(basis[2], diff);
                    }

                    // Test whether the intersection point is in the convex
                    // polygon.
                    PointInPolygon2<Real> PIP(static_cast<int>(mProjVertices.size()),
                        &mProjVertices[0]);

                    if (method == 1)
                    {
                        if (PIP.ContainsConvexOrderN(projIntersect))
                        {
                            // The ray intersects the triangle.
                            odd = !odd;
                        }
                    }
                    else
                    {
                        if (PIP.ContainsConvexOrderLogN(projIntersect))
                        {
                            // The ray intersects the triangle.
                            odd = !odd;
                        }
                    }
                }

                if (odd)
                {
                    insideCount++;
                }
            }

            return insideCount > mNumRays / 2;
        }

        // For simple faces.
        bool ContainsS0(Vector3<Real> const& p) const
        {
            int insideCount = 0;

            TIQuery<Real, Ray3<Real>, Triangle3<Real>> rtQuery;
            Triangle3<Real> triangle;
            Ray3<Real> ray;
            ray.origin = p;

            for (int j = 0; j < mNumRays; ++j)
            {
                ray.direction = mDirections[j];

                // Zero intersections to start with.
                bool odd = false;

                SimpleFace const* face = mSFaces;
                for (int i = 0; i < mNumFaces; ++i, ++face)
                {
                    // Attempt to quickly cull the triangle.
                    if (FastNoIntersect(ray, face->plane))
                    {
                        continue;
                    }

                    // The triangulation must exist to use it.
                    size_t numTriangles = face->triangles.size() / 3;
                    LogAssert(numTriangles > 0, "Triangulation must exist.");

                    // Process the triangles in a triangulation of the face.
                    int const* currIndex = &face->triangles[0];
                    for (size_t t = 0; t < numTriangles; ++t)
                    {
                        // Get the triangle vertices.
                        for (int k = 0; k < 3; ++k)
                        {
                            triangle.v[k] = mPoints[*currIndex++];
                        }

                        // Test for intersection.
                        if (rtQuery(ray, triangle).intersect)
                        {
                            // The ray intersects the triangle.
                            odd = !odd;
                        }
                    }
                }

                if (odd)
                {
                    insideCount++;
                }
            }

            return insideCount > mNumRays / 2;
        }

        bool ContainsS1(Vector3<Real> const& p) const
        {
            int insideCount = 0;

            FIQuery<Real, Ray3<Real>, Plane3<Real>> rpQuery;
            Ray3<Real> ray;
            ray.origin = p;

            for (int j = 0; j < mNumRays; ++j)
            {
                ray.direction = mDirections[j];

                // Zero intersections to start with.
                bool odd = false;

                SimpleFace const* face = mSFaces;
                for (int i = 0; i < mNumFaces; ++i, ++face)
                {
                    // Attempt to quickly cull the triangle.
                    if (FastNoIntersect(ray, face->plane))
                    {
                        continue;
                    }

                    // Compute the ray-plane intersection.
                    auto result = rpQuery(ray, face->plane);

                    // If you trigger this assertion, numerical round-off
                    // errors have led to a discrepancy between
                    // FastNoIntersect and the Find() result.
                    LogAssert(result.intersect, "Unexpected condition.");

                    // Get a coordinate system for the plane.  Use vertex 0
                    // as the origin.
                    Vector3<Real> const& V0 = mPoints[face->indices[0]];
                    Vector3<Real> basis[3];
                    basis[0] = face->plane.normal;
                    ComputeOrthogonalComplement(1, basis);

                    // Project the intersection onto the plane.
                    Vector3<Real> diff = result.point - V0;
                    Vector2<Real> projIntersect{ Dot(basis[1], diff), Dot(basis[2], diff) };

                    // Project the face vertices onto the plane of the face.
                    if (face->indices.size() > mProjVertices.size())
                    {
                        mProjVertices.resize(face->indices.size());
                    }

                    // Project the remaining vertices.  Vertex 0 is always the
                    // origin.
                    size_t numIndices = face->indices.size();
                    mProjVertices[0] = Vector2<Real>::Zero();
                    for (size_t k = 1; k < numIndices; ++k)
                    {
                        diff = mPoints[face->indices[k]] - V0;
                        mProjVertices[k][0] = Dot(basis[1], diff);
                        mProjVertices[k][1] = Dot(basis[2], diff);
                    }

                    // Test whether the intersection point is in the convex
                    // polygon.
                    PointInPolygon2<Real> PIP(static_cast<int>(mProjVertices.size()),
                        &mProjVertices[0]);

                    if (PIP.Contains(projIntersect))
                    {
                        // The ray intersects the triangle.
                        odd = !odd;
                    }
                }

                if (odd)
                {
                    insideCount++;
                }
            }

            return insideCount > mNumRays / 2;
        }

        int mNumPoints;
        Vector3<Real> const* mPoints;

        int mNumFaces;
        TriangleFace const* mTFaces;
        ConvexFace const* mCFaces;
        SimpleFace const* mSFaces;

        unsigned int mMethod;
        int mNumRays;
        Vector3<Real> const* mDirections;

        // Temporary storage for those methods that reduce the problem to 2D
        // point-in-polygon queries.  The array stores the projections of
        // face vertices onto the plane of the face.  It is resized as needed.
        mutable std::vector<Vector2<Real>> mProjVertices;
    };
}
