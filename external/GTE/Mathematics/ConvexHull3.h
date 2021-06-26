// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.04.22

#pragma once

// Compute the convex hull of 3D points using incremental insertion. The only
// way to ensure a correct result for the input vertices is to use an exact
// predicate for computing signs of various expressions. The implementation
// uses interval arithmetic and rational arithmetic for the predicate.
//
// TODO: A couple of potential optimizations need to be explored. The
// divide-and-conquer algorithm computes two convex hulls for a set of points.
// The two hulls are then merged into a single convex hull. The merge step
// is not the theoretical one that attempts to determine mutual visibility
// of the two hulls; rather, it combines the hulls into a single set of points
// and computes the convex hull of that set. This can be improved by using the
// left subhull in its current form and inserting points from the right subhull
// one at a time. It might be possible to insert points and stop the process
// when the partially merged polyhedron is the convex hull.
//
// The other optimization is based on profiling. The VETManifoldMesh memory
// management during insert/remove of vertices, edges and triangles suggests
// that a specialized VET data structure can be designed to avoid this cost.
//
// The main cost of the algorithm is testing which side of a plane a point is
// located. This test uses interval arithmetic to determine an exact sign,
// if possible. If that test fails, rational arithmetic is used. For typical
// datasets, the indeterminate sign from interval arithmetic happens rarely.

#include <Mathematics/ConvexHull2.h>
#include <Mathematics/SWInterval.h>
#include <Mathematics/Vector3.h>
#include <Mathematics/VETManifoldMesh.h>
#include <algorithm>
#include <numeric>
#include <queue>
#include <set>
#include <thread>

namespace gte
{
    template <typename Real>
    class ConvexHull3
    {
    public:
        // Supporting constants and types for rational arithmetic used in
        // the exact predicate for sign computations.
        static int constexpr NumWords = std::is_same<Real, float>::value ? 27 : 197;
        using Rational = BSNumber<UIntegerFP32<NumWords>>;

        // The class is a functor to support computing the convex hull of
        // multiple data sets using the same class object.
        ConvexHull3()
            :
            mPoints(nullptr),
            mRPoints{},
            mConverted{},
            mDimension(0),
            mVertices{},
            mHull{},
            mHullMesh{}
        {
        }

        // Compute the exact convex hull using a blend of interval arithmetic
        // and rational arithmetic. The code runs single-threaded when
        // lgNumThreads = 0. It runs multithreaded when lgNumThreads > 0,
        // where the number of threads is 2^{lgNumThreads} > 1.
        void operator()(size_t numPoints, Vector3<Real> const* points,
            size_t lgNumThreads)
        {
            LogAssert(numPoints > 0 && points != nullptr, "Invalid argument.");

            // Allocate storage for any rational points that must be computed
            // in the exact sign predicates. The rational points are memoized.
            mPoints = points;
            mRPoints.resize(numPoints);
            mConverted.resize(numPoints);
            std::fill(mConverted.begin(), mConverted.end(), 0);

            // Sort all the points indirectly.
            auto lessThanPoints = [this](size_t s0, size_t s1)
            {
                return mPoints[s0] < mPoints[s1];
            };

            auto equalPoints = [this](size_t s0, size_t s1)
            {
                return mPoints[s0] == mPoints[s1];
            };

            std::vector<size_t> sorted(numPoints);
            std::iota(sorted.begin(), sorted.end(), 0);
            std::sort(sorted.begin(), sorted.end(), lessThanPoints);
            auto newEnd = std::unique(sorted.begin(), sorted.end(), equalPoints);
            sorted.erase(newEnd, sorted.end());

            if (lgNumThreads > 0)
            {
                size_t numThreads = (static_cast<size_t>(1) << lgNumThreads);
                size_t load = sorted.size() / numThreads;
                std::vector<size_t> inNumSorted(numThreads);
                std::vector<size_t*> inSorted(numThreads);
                std::vector<std::vector<size_t>> outVertices(numThreads);
                std::vector<std::thread> process(numThreads);
                inNumSorted.back() = sorted.size();
                inSorted.front() = sorted.data();
                for (size_t i0 = 0, i1 = 1; i1 < numThreads; i0 = i1++)
                {
                    inNumSorted[i0] = load;
                    inNumSorted.back() -= load;
                    inSorted[i1] = inSorted[i0] + load;
                }

                while (numThreads > 1)
                {
                    for (size_t i = 0; i < numThreads; ++i)
                    {
                        process[i] = std::thread(
                            [this, i, &inNumSorted, &inSorted, &outVertices]()
                            {
                                size_t dimension = 0;
                                std::vector<size_t> hull;
                                VETManifoldMesh hullMesh;
                                ComputeHull(inNumSorted[i], inSorted[i], dimension,
                                    outVertices[i], hull, hullMesh);
                            });
                    }

                    numThreads /= 2;

                    auto target = sorted.begin();
                    inSorted[0] = sorted.data();
                    for (size_t i = 0, k = 0; i < numThreads; ++i)
                    {
                        process[2 * i].join();
                        process[2 * i + 1].join();

                        inNumSorted[i] = 0;
                        auto begin = target;
                        for (size_t j = 0; j < 2; ++j, ++k)
                        {
                            size_t numVertices = outVertices[k].size();
                            inNumSorted[i] += numVertices;
                            std::copy(outVertices[k].begin(), outVertices[k].end(), target);
                            target += numVertices;
                        }
                        inSorted[i + 1] = inSorted[i] + inNumSorted[i];
                        std::sort(begin, target, lessThanPoints);
                    }
                }

                ComputeHull(inNumSorted[0], inSorted[0], mDimension, mVertices,
                    mHull, mHullMesh);
            }
            else
            {
                ComputeHull(sorted.size(), sorted.data(), mDimension, mVertices,
                    mHull, mHullMesh);
            }
        }

        void operator()(std::vector<Vector3<Real>> const& points, size_t lgNumThreads)
        {
            operator()(points.size(), points.data(), lgNumThreads);
        }

        // The dimension is 0 (hull is a single point), 1 (hull is a line
        // segment), 2 (hull is a convex polygon in 3D) or 3 (hull is a convex
        // polyhedron).
        inline size_t GetDimension() const
        {
            return mDimension;
        }

        // Get the indices into the input 'points[]' that correspond to hull
        // vertices.
        inline std::vector<size_t> const& GetVertices() const
        {
            return mVertices;
        }

        // Get the indices into the input 'points[]' that correspond to hull
        // vertices. The returned array is organized according to the hull
        // dimension.
        //   0: The hull is a single point. The returned array has size 1 with
        //      index corresponding to that point.
        //   1: The hull is a line segment. The returned array has size 2 with
        //      indices corresponding to the segment endpoints.
        //   2: The hull is a convex polygon in 3D. The returned array has
        //      size N with indices corresponding to the polygon vertices.
        //      The vertices are ordered.
        //   3: The hull is a convex polyhedron. The returned array has T
        //      triples of indices, each triple corresponding to a triangle
        //      face of the hull. The face vertices are counterclockwise when
        //      viewed by an observer outside the polyhedron. It is possible
        //      that some triangle faces are coplanar.
        // The number of vertices and triangles can vary depending on the
        // number of threads used for computation. This is not an error. For
        // example, when running with N threads it is possible to have a
        // convex quadrilateral face formed by 2 coplanar triangles {v0,v1,v2}
        // and {v0,v2,v3}. When running with M threads, it is possible that
        // the same convex quadrilateral face is formed by 4 coplanar triangles
        // {v0,v1,v2}, {v1,v2,v4}, {v2,v3,v4} and {v3,v0,v4}, where the
        // vertices v0, v2 and v4 are colinear. In both cases, if V is the
        // number of vertices and T is the number of triangles, then the
        // number of edges is E = T/2 and Euler's formula is satisfied:
        // V - E + T = 2.
        inline std::vector<size_t> const& GetHull() const
        {
            return mHull;
        }

        // Get the hull mesh, which is valid only when the dimension is 3.
        // This allows access to the graph of vertices, edges and triangles
        // of the convex (polyhedron) hull.
        inline VETManifoldMesh const& GetHullMesh() const
        {
            return mHullMesh;
        }

    private:
        void ComputeHull(size_t numSorted, size_t* sorted, size_t& dimension,
            std::vector<size_t>& vertices, std::vector<size_t>& hull,
            VETManifoldMesh& hullMesh)
        {
            dimension = 0;
            vertices.clear();
            hull.reserve(numSorted);
            hull.clear();
            hullMesh.Clear();

            size_t current = 0;
            if (Hull0(hull, numSorted, sorted, dimension, current))
            {
                vertices.resize(1);
                vertices[0] = hull[0];
                return;
            }

            if (Hull1(hull, numSorted, sorted, dimension, current))
            {
                vertices.resize(2);
                vertices[0] = hull[0];
                vertices[1] = hull[1];
                return;
            }

            if (Hull2(hull, numSorted, sorted, dimension, current))
            {
                vertices.resize(hull.size());
                std::copy(hull.begin(), hull.end(), vertices.begin());
                return;
            }

            Hull3(hull, numSorted, sorted, hullMesh, current);

            auto const& vMap = hullMesh.GetVertices();
            vertices.resize(vMap.size());
            size_t index = 0;
            for (auto const& element : vMap)
            {
                vertices[index++] = element.first;
            }

            auto const& tMap = hullMesh.GetTriangles();
            hull.resize(3 * tMap.size());
            index = 0;
            for (auto const& element : tMap)
            {
                hull[index++] = element.first.V[0];
                hull[index++] = element.first.V[1];
                hull[index++] = element.first.V[2];
            }
        }

        // Support for computing a 0-dimensional convex hull.
        bool Hull0(std::vector<size_t>& hull, size_t numSorted, size_t* sorted,
            size_t& dimension, size_t& current)
        {
            hull.push_back(sorted[current]);  // hull[0]
            for (++current; current < numSorted; ++current)
            {
                if (!Colocated(hull[0], sorted[current]))
                {
                    dimension = 1;
                    break;
                }
            }
            return dimension == 0;
        }

        // Support for computing a 1-dimensional convex hull.
        bool Hull1(std::vector<size_t>& hull, size_t numSorted, size_t* sorted,
            size_t& dimension, size_t& current)
        {
            hull.push_back(sorted[current]);  // hull[1]
            for (++current; current < numSorted; ++current)
            {
                if (!Colinear(hull[0], hull[1], sorted[current]))
                {
                    dimension = 2;
                    break;
                }
                hull.push_back(sorted[current]);
            }

            if (hull.size() > 2)
            {
                // Sort the points and choose the extreme points as the
                // endpoints of the line segment that is the convex hull.
                std::sort(hull.begin(), hull.end(),
                    [this](size_t v0, size_t v1)
                    {
                        return mPoints[v0] < mPoints[v1];
                    });

                size_t hmin = hull.front();
                size_t hmax = hull.back();
                hull.clear();
                hull.push_back(hmin);
                hull.push_back(hmax);
            }

            return dimension == 1;
        }

        // Support for computing a 2-dimensional convex hull.
        bool Hull2(std::vector<size_t>& hull, size_t numSorted, size_t* sorted,
            size_t& dimension, size_t& current)
        {
            hull.push_back(sorted[current]);  // hull[2]
            for (++current; current < numSorted; ++current)
            {
                if (ToPlane(hull[0], hull[1], hull[2], sorted[current]) != 0)
                {
                    dimension = 3;
                    break;
                }
                hull.push_back(sorted[current]);
            }

            if (hull.size() > 3)
            {
                // Compute the planar convex hull of the points. The coplanar
                // points are projected onto a 2D plane determined by the
                // maximum absolute component of the normal of the first
                // triangle. The extreme points of the projected hull generate
                // the extreme points of the planar hull in 3D.
                auto const& rV0 = GetRationalPoint(hull[0]);
                auto const& rV1 = GetRationalPoint(hull[1]);
                auto const& rV2 = GetRationalPoint(hull[2]);
                auto const rDiff1 = rV1 - rV0;
                auto const rDiff2 = rV2 - rV0;
                auto rNormal = Cross(rDiff1, rDiff2);

                // The signs are used to select 2 of the 3 point components so
                // that when the planar hull is viewed from the side of the
                // plane to which rNormal is directed, the triangles are
                // counterclockwise ordered.
                std::array<int32_t, 3> sign{};
                for (int32_t i = 0; i < 3; ++i)
                {
                    sign[i] = rNormal[i].GetSign();
                    rNormal[i].SetSign(std::abs(sign[i]));
                };

                std::pair<int32_t, int32_t> c;
                if (rNormal[0] > rNormal[1])
                {
                    if (rNormal[0] > rNormal[2])
                    {
                        c = (sign[0] > 0 ? std::make_pair(1, 2) : std::make_pair(2, 1));
                    }
                    else
                    {
                        c = (sign[2] > 0 ? std::make_pair(0, 1) : std::make_pair(1, 0));
                    }
                }
                else
                {
                    if (rNormal[1] > rNormal[2])
                    {
                        c = (sign[1] > 0 ? std::make_pair(2, 0) : std::make_pair(0, 2));
                    }
                    else
                    {
                        c = (sign[2] > 0 ? std::make_pair(0, 1) : std::make_pair(1, 0));
                    }
                }

                std::vector<Vector2<Real>> projections(hull.size());
                for (size_t i = 0; i < projections.size(); ++i)
                {
                    size_t h = hull[i];
                    projections[i][0] = mPoints[h][c.first];
                    projections[i][1] = mPoints[h][c.second];
                }

                ConvexHull2<Real> ch2;
                ch2((int)projections.size(), projections.data(), static_cast<Real>(0));
                auto const& hull2 = ch2.GetHull();

                std::vector<size_t> tempHull(hull2.size());
                for (size_t i = 0; i < hull2.size(); ++i)
                {
                    tempHull[i] = hull[static_cast<size_t>(hull2[i])];
                }
                hull.clear();
                for (size_t i = 0; i < hull2.size(); ++i)
                {
                    hull.push_back(tempHull[i]);
                }
            }

            return dimension == 2;
        }

        // Support for computing a 3-dimensional convex hull.
        void Hull3(std::vector<size_t>& hull, size_t numSorted, size_t* sorted,
            VETManifoldMesh& hullMesh, size_t& current)
        {
            using TrianglePtr = VETManifoldMesh::Triangle*;
            // The hull points previous to the current one are coplanar and
            // are the vertices of a convex polygon. To initialize the 3D
            // hull, use triangles from a triangle fan of the convex polygon
            // and use triangles connecting the current point to the edges
            // of the convex polygon. The vertex ordering of these triangles
            // depends on whether sorted[current] is on the positive or
            // negative side of the plane determined by hull[0], hull[1] and
            // hull[2].
            int32_t sign = ToPlane(hull[0], hull[1], hull[2], sorted[current]);
            int32_t h0, h1, h2;
            if (sign > 0)
            {
                h0 = static_cast<int32_t>(hull[0]);
                for (size_t i1 = 1, i2 = 2; i2 < hull.size(); i1 = i2++)
                {
                    h1 = static_cast<int32_t>(hull[i1]);
                    h2 = static_cast<int32_t>(hull[i2]);
                    auto inserted = hullMesh.Insert(h0, h2, h1);
                    LogAssert(
                        inserted != nullptr,
                        "Unexpected insertion failure.");
                }

                h0 = static_cast<int32_t>(sorted[current]);
                for (size_t i1 = hull.size() - 1, i2 = 0; i2 < hull.size(); i1 = i2++)
                {
                    h1 = static_cast<int32_t>(hull[i1]);
                    h2 = static_cast<int32_t>(hull[i2]);
                    auto inserted = hullMesh.Insert(h0, h1, h2);
                    LogAssert(
                        inserted != nullptr,
                        "Unexpected insertion failure.");
                }
            }
            else
            {
                h0 = static_cast<int32_t>(hull[0]);
                for (size_t i1 = 1, i2 = 2; i2 < hull.size(); i1 = i2++)
                {
                    h1 = static_cast<int32_t>(hull[i1]);
                    h2 = static_cast<int32_t>(hull[i2]);
                    auto inserted = hullMesh.Insert(h0, h1, h2);
                    LogAssert(
                        inserted != nullptr,
                        "Unexpected insertion failure.");
                }

                h0 = static_cast<int32_t>(sorted[current]);
                for (size_t i1 = hull.size() - 1, i2 = 0; i2 < hull.size(); i1 = i2++)
                {
                    h1 = static_cast<int32_t>(hull[i1]);
                    h2 = static_cast<int32_t>(hull[i2]);
                    auto inserted = hullMesh.Insert(h0, h2, h1);
                    LogAssert(
                        inserted != nullptr,
                        "Unexpected insertion failure.");
                }
            }

            // The hull is now maintained in hullMesh, so there is no need
            // to add members to hull. At the time the full hull is known,
            // hull will be assigned the triangle indices.
            std::vector<std::array<int32_t, 2>> terminator;
            for (++current; current < numSorted; ++current)
            {
                // The index h0 refers to the previously inserted hull point.
                // The index h1 refers to the current point to be inserted
                // into the hull.
                auto const& vMap = hullMesh.GetVertices();
                auto vIter = vMap.find(h0);
                LogAssert(vIter != vMap.end(), "Unexpected condition.");
                h1 = static_cast<int32_t>(sorted[current]);

                // The sorting guarantees that the point at h0 is visible to
                // the point at h1. Find the triangles that share h0 and are
                // visible to h1
                std::queue<TrianglePtr> visible;
                std::set<TrianglePtr> visited;
                for (auto const& tri : vIter->second->TAdjacent)
                {
                    sign = ToPlane(tri->V[0], tri->V[1], tri->V[2], h1);
                    if (sign > 0)
                    {
                        visible.push(tri);
                        visited.insert(tri);
                        break;
                    }
                }
                LogAssert(visible.size() > 0, "Unexpected condition.");

                // Remove the connected component of visible triangles. Save
                // the terminator edges for insertion of the new visible set
                // of triangles.
                terminator.clear();
                while (visible.size() > 0)
                {
                    TrianglePtr tri = visible.front();
                    visible.pop();
                    for (size_t i = 0; i < 3; ++i)
                    {
                        auto adj = tri->T[i];
                        if (adj)
                        {
                            if (ToPlane(adj->V[0], adj->V[1], adj->V[2], h1) <= 0)
                            {
                                // The shared edge of tri and adj is a
                                // terminator.
                                terminator.push_back({ tri->V[i], tri->V[(i + 1) % 3] });
                            }
                            else
                            {
                                if (visited.find(adj) == visited.end())
                                {
                                    visible.push(adj);
                                    visited.insert(adj);
                                }
                            }
                        }
                    }
                    visited.erase(tri);
                    bool removed = hullMesh.Remove(tri->V[0], tri->V[1], tri->V[2]);
                    LogAssert(
                        removed,
                        "Unexpected removal failure.");
                }

                // Insert the new hull triangles.
                for (auto const& edge : terminator)
                {
                    auto inserted = hullMesh.Insert(edge[0], edge[1], h1);
                    LogAssert(
                        inserted != nullptr,
                        "Unexpected insertion failure.");
                }

                // The current index h1 becomes the previous index h0 for the
                // next pass of the 'current' loop.
                h0 = h1;
            }
        }

        // Memoized access to the rational representation of the points.
        Vector3<Rational> const& GetRationalPoint(size_t index)
        {
            if (mConverted[index] == 0)
            {
                mConverted[index] = 1;
                for (int i = 0; i < 3; ++i)
                {
                    mRPoints[index][i] = mPoints[index][i];
                }
            }
            return mRPoints[index];
        }

        bool Colocated(size_t v0, size_t v1)
        {
            auto const& r0 = GetRationalPoint(v0);
            auto const& r1 = GetRationalPoint(v1);
            return r0 == r1;
        }

        bool Colinear(size_t v0, size_t v1, size_t v2)
        {
            auto const& rvec0 = GetRationalPoint(v0);
            auto const& rvec1 = GetRationalPoint(v1);
            auto const& rvec2 = GetRationalPoint(v2);
            auto const rdiff1 = rvec1 - rvec0;
            auto const rdiff2 = rvec2 - rvec0;
            auto const rcross = Cross(rdiff1, rdiff2);
            return rcross[0].GetSign() == 0
                && rcross[1].GetSign() == 0
                && rcross[2].GetSign() == 0;
        }

        // For a plane with origin V0 and normal N = Cross(V1-V0,V2-V0),
        // ToPlane returns
        //   +1, V3 on positive side of plane (side to which N points)
        //   -1, V3 on negative side of plane (side to which -N points)
        //    0, V3 on the plane
        int ToPlane(size_t v0, size_t v1, size_t v2, size_t v3)
        {
            using SInterval = SWInterval<Real>;
            using SVector3 = Vector3<SInterval>;

            // Attempt to classify the sign using interval arithmetic.
            SVector3 const s0{ mPoints[v0][0], mPoints[v0][1], mPoints[v0][2] };
            SVector3 const s1{ mPoints[v1][0], mPoints[v1][1], mPoints[v1][2] };
            SVector3 const s2{ mPoints[v2][0], mPoints[v2][1], mPoints[v2][2] };
            SVector3 const s3{ mPoints[v3][0], mPoints[v3][1], mPoints[v3][2] };
            auto const sDiff1 = s1 - s0;
            auto const sDiff2 = s2 - s0;
            auto const sDiff3 = s3 - s0;
            auto const sDet = DotCross(sDiff1, sDiff2, sDiff3);
            if (sDet[0] > 0)
            {
                return +1;
            }
            if (sDet[1] < 0)
            {
                return -1;
            }

            // The sign is indeterminate using interval arithmetic.
            auto const& r0 = GetRationalPoint(v0);
            auto const& r1 = GetRationalPoint(v1);
            auto const& r2 = GetRationalPoint(v2);
            auto const& r3 = GetRationalPoint(v3);
            auto const rDiff1 = r1 - r0;
            auto const rDiff2 = r2 - r0;
            auto const rDiff3 = r3 - r0;
            auto const rDet = DotCross(rDiff1, rDiff2, rDiff3);
            return rDet.GetSign();
        }

    private:
        // A blend of interval arithmetic and exact arithmetic is used to
        // ensure correctness.
        Vector3<Real> const* mPoints;
        std::vector<Vector3<Rational>> mRPoints;
        std::vector<uint32_t> mConverted;

        // The output data.
        size_t mDimension;
        std::vector<size_t> mVertices;
        std::vector<size_t> mHull;
        VETManifoldMesh mHullMesh;
    };
}
