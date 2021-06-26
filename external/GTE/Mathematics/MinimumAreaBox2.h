// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.01

#pragma once

#include <Mathematics/OrientedBox.h>
#include <Mathematics/ConvexHull2.h>

// Compute a minimum-area oriented box containing the specified points.  The
// algorithm uses the rotating calipers method.
//   http://www-cgrl.cs.mcgill.ca/~godfried/research/calipers.html
//   http://cgm.cs.mcgill.ca/~orm/rotcal.html
// The box is supported by the convex hull of the points, so the algorithm
// is really about computing the minimum-area box containing a convex polygon.
// The rotating calipers approach is O(n) in time for n polygon edges.
//
// A detailed description of the algorithm and implementation is found in
//   https://www.geometrictools.com/Documentation/MinimumAreaRectangle.pdf
//
// NOTE: This algorithm guarantees a correct output only when ComputeType is
// an exact arithmetic type that supports division.  In GTEngine, one such
// type is BSRational<UIntegerAP32> (arbitrary precision).  Another such type
// is BSRational<UIntegerFP32<N>> (fixed precision), where N is chosen large
// enough for your input data sets.  If you choose ComputeType to be 'float'
// or 'double', the output is not guaranteed to be correct.
//
// See GeometricTools/GTEngine/Samples/Geometrics/MinimumAreaBox2 for an
// example of how to use the code.

namespace gte
{
    template <typename InputType, typename ComputeType>
    class MinimumAreaBox2
    {
    public:
        // The class is a functor to support computing the minimum-area box of
        // multiple data sets using the same class object.
        MinimumAreaBox2()
            :
            mNumPoints(0),
            mPoints(nullptr),
            mSupportIndices{ 0, 0, 0, 0 },
            mArea((InputType)0),
            mZero(0),
            mOne(1),
            mNegOne(-1),
            mHalf((InputType)0.5)
        {
        }

        // The points are arbitrary, so we must compute the convex hull from
        // them in order to compute the minimum-area box.  The input
        // parameters are necessary for using ConvexHull2.  NOTE:  ConvexHull2
        // guarantees that the hull does not have three consecutive collinear
        // points.
        OrientedBox2<InputType> operator()(int numPoints,
            Vector2<InputType> const* points,
            bool useRotatingCalipers = !std::is_floating_point<ComputeType>::value)
        {
            mNumPoints = numPoints;
            mPoints = points;
            mHull.clear();

            // Get the convex hull of the points.
            ConvexHull2<InputType> ch2;
            ch2(mNumPoints, mPoints, (InputType)0);
            int dimension = ch2.GetDimension();

            OrientedBox2<InputType> minBox;

            if (dimension == 0)
            {
                // The points are all effectively the same (using fuzzy
                // epsilon).
                minBox.center = mPoints[0];
                minBox.axis[0] = Vector2<InputType>::Unit(0);
                minBox.axis[1] = Vector2<InputType>::Unit(1);
                minBox.extent[0] = (InputType)0;
                minBox.extent[1] = (InputType)0;
                mHull.resize(1);
                mHull[0] = 0;
                return minBox;
            }

            if (dimension == 1)
            {
                // The points effectively lie on a line (using fuzzy epsilon).
                // Determine the extreme t-values for the points represented
                // as P = origin + t*direction.  We know that 'origin' is an
                // input vertex, so we can start both t-extremes at zero.
                Line2<InputType> const& line = ch2.GetLine();
                InputType tmin = (InputType)0, tmax = (InputType)0;
                int imin = 0, imax = 0;
                for (int i = 0; i < mNumPoints; ++i)
                {
                    Vector2<InputType> diff = mPoints[i] - line.origin;
                    InputType t = Dot(diff, line.direction);
                    if (t > tmax)
                    {
                        tmax = t;
                        imax = i;
                    }
                    else if (t < tmin)
                    {
                        tmin = t;
                        imin = i;
                    }
                }

                minBox.center = line.origin + (InputType)0.5 * (tmin + tmax) * line.direction;
                minBox.extent[0] = (InputType)0.5 * (tmax - tmin);
                minBox.extent[1] = (InputType)0;
                minBox.axis[0] = line.direction;
                minBox.axis[1] = -Perp(line.direction);
                mHull.resize(2);
                mHull[0] = imin;
                mHull[1] = imax;
                return minBox;
            }

            mHull = ch2.GetHull();
            Vector2<InputType> const* vertices = ch2.GetPoints();
            std::vector<Vector2<ComputeType>> computePoints(mHull.size());
            for (size_t i = 0; i < mHull.size(); ++i)
            {
                for (int j = 0; j < 2; ++j)
                {
                    computePoints[i][j] = vertices[mHull[i]][j];
                }
            }

            RemoveCollinearPoints(computePoints);

            Box box;
            if (useRotatingCalipers)
            {
                box = ComputeBoxForEdgeOrderN(computePoints);
            }
            else
            {
                box = ComputeBoxForEdgeOrderNSqr(computePoints);
            }

            ConvertTo(box, computePoints, minBox);
            return minBox;
        }

        // The points already form a counterclockwise, nondegenerate convex
        // polygon.  If the points directly are the convex polygon, set
        // numIndices to 0 and indices to nullptr.  If the polygon vertices
        // are a subset of the incoming points, that subset is identified by
        // numIndices >= 3 and indices having numIndices elements.
        OrientedBox2<InputType> operator()(int numPoints,
            Vector2<InputType> const* points, int numIndices, int const* indices,
            bool useRotatingCalipers = !std::is_floating_point<ComputeType>::value)
        {
            mHull.clear();

            OrientedBox2<InputType> minBox;

            if (numPoints < 3 || !points || (indices && numIndices < 3))
            {
                minBox.center = Vector2<InputType>::Zero();
                minBox.axis[0] = Vector2<InputType>::Unit(0);
                minBox.axis[1] = Vector2<InputType>::Unit(1);
                minBox.extent = Vector2<InputType>::Zero();
                return minBox;
            }

            if (indices)
            {
                mHull.resize(numIndices);
                std::copy(indices, indices + numIndices, mHull.begin());
            }
            else
            {
                numIndices = numPoints;
                mHull.resize(numIndices);
                for (int i = 0; i < numIndices; ++i)
                {
                    mHull[i] = i;
                }
            }

            std::vector<Vector2<ComputeType>> computePoints(numIndices);
            for (int i = 0; i < numIndices; ++i)
            {
                int h = mHull[i];
                computePoints[i][0] = (ComputeType)points[h][0];
                computePoints[i][1] = (ComputeType)points[h][1];
            }

            RemoveCollinearPoints(computePoints);

            Box box;
            if (useRotatingCalipers)
            {
                box = ComputeBoxForEdgeOrderN(computePoints);
            }
            else
            {
                box = ComputeBoxForEdgeOrderNSqr(computePoints);
            }

            ConvertTo(box, computePoints, minBox);
            return minBox;
        }

        // Member access.
        inline int GetNumPoints() const
        {
            return mNumPoints;
        }

        inline Vector2<InputType> const* GetPoints() const
        {
            return mPoints;
        }

        inline std::vector<int> const& GetHull() const
        {
            return mHull;
        }

        inline std::array<int, 4> const& GetSupportIndices() const
        {
            return mSupportIndices;
        }

        inline InputType GetArea() const
        {
            return mArea;
        }

    private:
        // The box axes are U[i] and are usually not unit-length in order to
        // allow exact arithmetic.  The box is supported by mPoints[index[i]],
        // where i is one of the enumerations above.  The box axes are not
        // necessarily unit length, but they have the same length.  They need
        // to be normalized for conversion back to InputType.
        struct Box
        {
            Vector2<ComputeType> U[2];
            std::array<int, 4> index;  // order: bottom, right, top, left
            ComputeType sqrLenU0, area;
        };

        // The rotating calipers algorithm has a loop invariant that requires
        // the convex polygon not to have collinear points.  Any such points
        // must be removed first.  The code is also executed for the O(n^2)
        // algorithm to reduce the number of process edges.
        void RemoveCollinearPoints(std::vector<Vector2<ComputeType>>& vertices)
        {
            std::vector<Vector2<ComputeType>> tmpVertices = vertices;

            int const numVertices = static_cast<int>(vertices.size());
            int numNoncollinear = 0;
            Vector2<ComputeType> ePrev = tmpVertices[0] - tmpVertices.back();
            for (int i0 = 0, i1 = 1; i0 < numVertices; ++i0)
            {
                Vector2<ComputeType> eNext = tmpVertices[i1] - tmpVertices[i0];

                ComputeType dp = DotPerp(ePrev, eNext);
                if (dp != mZero)
                {
                    vertices[numNoncollinear++] = tmpVertices[i0];
                }

                ePrev = eNext;
                if (++i1 == numVertices)
                {
                    i1 = 0;
                }
            }

            vertices.resize(numNoncollinear);
        }

        // This is the slow O(n^2) search.
        Box ComputeBoxForEdgeOrderNSqr(std::vector<Vector2<ComputeType>> const& vertices)
        {
            Box minBox;
            minBox.area = mNegOne;
            int const numIndices = static_cast<int>(vertices.size());
            for (int i0 = numIndices - 1, i1 = 0; i1 < numIndices; i0 = i1++)
            {
                Box box = SmallestBox(i0, i1, vertices);
                if (minBox.area == mNegOne || box.area < minBox.area)
                {
                    minBox = box;
                }
            }
            return minBox;
        }

        // The fast O(n) search.
        Box ComputeBoxForEdgeOrderN(std::vector<Vector2<ComputeType>> const& vertices)
        {
            // The inputs are assumed to be the vertices of a convex polygon
            // that is counterclockwise ordered.  The input points must not
            // contain three consecutive collinear points.

            // When the bounding box corresponding to a polygon edge is
            // computed, we mark the edge as visited.  If the edge is
            // encountered later, the algorithm terminates.
            std::vector<bool> visited(vertices.size());
            std::fill(visited.begin(), visited.end(), false);

            // Start the minimum-area rectangle search with the edge from the
            // last polygon vertex to the first.  When updating the extremes,
            // we want the bottom-most point on the left edge, the top-most
            // point on the right edge, the left-most point on the top edge,
            // and the right-most point on the bottom edge.  The polygon edges
            // starting at these points are then guaranteed not to coincide
            // with a box edge except when an extreme point is shared by two
            // box edges (at a corner).
            Box minBox = SmallestBox((int)vertices.size() - 1, 0, vertices);
            visited[minBox.index[0]] = true;

            // Execute the rotating calipers algorithm.
            Box box = minBox;
            for (size_t i = 0; i < vertices.size(); ++i)
            {
                std::array<std::pair<ComputeType, int>, 4> A;
                int numA;
                if (!ComputeAngles(vertices, box, A, numA))
                {
                    // The polygon is a rectangle, so the search is over.
                    break;
                }

                // Indirectly sort the A-array.
                std::array<int, 4> sort = SortAngles(A, numA);

                // Update the supporting indices (box.index[]) and the box
                // axis directions (box.U[]).
                if (!UpdateSupport(A, numA, sort, vertices, visited, box))
                {
                    // We have already processed the box polygon edge, so the
                    // search is over.
                    break;
                }

                if (box.area < minBox.area)
                {
                    minBox = box;
                }
            }

            return minBox;
        }

        // Compute the smallest box for the polygon edge <V[i0],V[i1]>.
        Box SmallestBox(int i0, int i1, std::vector<Vector2<ComputeType>> const& vertices)
        {
            Box box;
            box.U[0] = vertices[i1] - vertices[i0];
            box.U[1] = -Perp(box.U[0]);
            box.index = { i1, i1, i1, i1 };
            box.sqrLenU0 = Dot(box.U[0], box.U[0]);

            Vector2<ComputeType> const& origin = vertices[i1];
            Vector2<ComputeType> support[4];
            for (int j = 0; j < 4; ++j)
            {
                support[j] = { mZero, mZero };
            }

            int i = 0;
            for (auto const& vertex : vertices)
            {
                Vector2<ComputeType> diff = vertex - origin;
                Vector2<ComputeType> v = { Dot(box.U[0], diff), Dot(box.U[1], diff) };

                // The right-most vertex of the bottom edge is vertices[i1].
                // The assumption of no triple of collinear vertices
                // guarantees that box.index[0] is i1, which is the initial
                // value assigned at the beginning of this function.
                // Therefore, there is no need to test for other vertices
                // farther to the right than vertices[i1].

                if (v[0] > support[1][0] ||
                    (v[0] == support[1][0] && v[1] > support[1][1]))
                {
                    // New right maximum OR same right maximum but closer
                    // to top.
                    box.index[1] = i;
                    support[1] = v;
                }

                if (v[1] > support[2][1] ||
                    (v[1] == support[2][1] && v[0] < support[2][0]))
                {
                    // New top maximum OR same top maximum but closer
                    // to left.
                    box.index[2] = i;
                    support[2] = v;
                }

                if (v[0] < support[3][0] ||
                    (v[0] == support[3][0] && v[1] < support[3][1]))
                {
                    // New left minimum OR same left minimum but closer
                    // to bottom.
                    box.index[3] = i;
                    support[3] = v;
                }

                ++i;
            }

            // The comment in the loop has the implication that
            // support[0] = { 0, 0 }, so the scaled height
            // (support[2][1] - support[0][1]) is simply support[2][1].
            ComputeType scaledWidth = support[1][0] - support[3][0];
            ComputeType scaledHeight = support[2][1];
            box.area = scaledWidth * scaledHeight / box.sqrLenU0;
            return box;
        }

        // Compute (sin(angle))^2 for the polygon edges emanating from the
        // support vertices of the box.  The return value is 'true' if at
        // least one angle is in [0,pi/2); otherwise, the return value is
        // 'false' and the original polygon must be a rectangle.
        bool ComputeAngles(std::vector<Vector2<ComputeType>> const& vertices,
            Box const& box, std::array<std::pair<ComputeType, int>, 4>& A, int& numA) const
        {
            int const numVertices = static_cast<int>(vertices.size());
            numA = 0;
            for (int k0 = 3, k1 = 0; k1 < 4; k0 = k1++)
            {
                if (box.index[k0] != box.index[k1])
                {
                    // The box edges are ordered in k1 as U[0], U[1],
                    // -U[0], -U[1].
                    Vector2<ComputeType> D = ((k0 & 2) ? -box.U[k0 & 1] : box.U[k0 & 1]);
                    int j0 = box.index[k0], j1 = j0 + 1;
                    if (j1 == numVertices)
                    {
                        j1 = 0;
                    }
                    Vector2<ComputeType> E = vertices[j1] - vertices[j0];
                    ComputeType dp = DotPerp(D, E);
                    ComputeType esqrlen = Dot(E, E);
                    ComputeType sinThetaSqr = (dp * dp) / esqrlen;
                    A[numA++] = std::make_pair(sinThetaSqr, k0);
                }
            }
            return numA > 0;
        }

        // Sort the angles indirectly.  The sorted indices are returned.
        // This avoids swapping elements of A[], which can be expensive when
        // ComputeType is an exact rational type.
        std::array<int, 4> SortAngles(std::array<std::pair<ComputeType, int>, 4> const& A, int numA) const
        {
            std::array<int, 4> sort = { 0, 1, 2, 3 };
            if (numA > 1)
            {
                if (numA == 2)
                {
                    if (A[sort[0]].first > A[sort[1]].first)
                    {
                        std::swap(sort[0], sort[1]);
                    }
                }
                else if (numA == 3)
                {
                    if (A[sort[0]].first > A[sort[1]].first)
                    {
                        std::swap(sort[0], sort[1]);
                    }
                    if (A[sort[0]].first > A[sort[2]].first)
                    {
                        std::swap(sort[0], sort[2]);
                    }
                    if (A[sort[1]].first > A[sort[2]].first)
                    {
                        std::swap(sort[1], sort[2]);
                    }
                }
                else  // numA == 4
                {
                    if (A[sort[0]].first > A[sort[1]].first)
                    {
                        std::swap(sort[0], sort[1]);
                    }
                    if (A[sort[2]].first > A[sort[3]].first)
                    {
                        std::swap(sort[2], sort[3]);
                    }
                    if (A[sort[0]].first > A[sort[2]].first)
                    {
                        std::swap(sort[0], sort[2]);
                    }
                    if (A[sort[1]].first > A[sort[3]].first)
                    {
                        std::swap(sort[1], sort[3]);
                    }
                    if (A[sort[1]].first > A[sort[2]].first)
                    {
                        std::swap(sort[1], sort[2]);
                    }
                }
            }
            return sort;
        }

        bool UpdateSupport(std::array<std::pair<ComputeType, int>, 4> const& A,
            int numA, std::array<int, 4> const& sort,
            std::vector<Vector2<ComputeType>> const& vertices,
            std::vector<bool>& visited, Box& box)
        {
            // Replace the support vertices of those edges attaining minimum
            // angle with the other endpoints of the edges.
            int const numVertices = static_cast<int>(vertices.size());
            auto const& amin = A[sort[0]];
            for (int k = 0; k < numA; ++k)
            {
                auto const& a = A[sort[k]];
                if (a.first == amin.first)
                {
                    if (++box.index[a.second] == numVertices)
                    {
                        box.index[a.second] = 0;
                    }
                }
                else
                {
                    break;
                }
            }

            int bottom = box.index[amin.second];
            if (visited[bottom])
            {
                // We have already processed this polygon edge.
                return false;
            }
            visited[bottom] = true;

            // Cycle the vertices so that the bottom support occurs first.
            std::array<int, 4> nextIndex;
            for (int k = 0; k < 4; ++k)
            {
                nextIndex[k] = box.index[(amin.second + k) % 4];
            }
            box.index = nextIndex;

            // Compute the box axis directions.
            int j1 = box.index[0], j0 = j1 - 1;
            if (j0 < 0)
            {
                j0 = numVertices - 1;
            }
            box.U[0] = vertices[j1] - vertices[j0];
            box.U[1] = -Perp(box.U[0]);
            box.sqrLenU0 = Dot(box.U[0], box.U[0]);

            // Compute the box area.
            Vector2<ComputeType> diff[2] =
            {
                vertices[box.index[1]] - vertices[box.index[3]],
                vertices[box.index[2]] - vertices[box.index[0]]
            };
            box.area = Dot(box.U[0], diff[0]) * Dot(box.U[1], diff[1]) / box.sqrLenU0;
            return true;
        }

        // Convert the ComputeType box to the InputType box.  When the
        // ComputeType is an exact rational type, the conversions are
        // performed to avoid precision loss until necessary at the last step.
        void ConvertTo(Box const& minBox,
            std::vector<Vector2<ComputeType>> const& computePoints,
            OrientedBox2<InputType>& itMinBox)
        {
            // The sum, difference, and center are all computed exactly.
            Vector2<ComputeType> sum[2] =
            {
                computePoints[minBox.index[1]] + computePoints[minBox.index[3]],
                computePoints[minBox.index[2]] + computePoints[minBox.index[0]]
            };

            Vector2<ComputeType> difference[2] =
            {
                computePoints[minBox.index[1]] - computePoints[minBox.index[3]],
                computePoints[minBox.index[2]] - computePoints[minBox.index[0]]
            };

            Vector2<ComputeType> center = mHalf * (
                Dot(minBox.U[0], sum[0]) * minBox.U[0] +
                Dot(minBox.U[1], sum[1]) * minBox.U[1]) / minBox.sqrLenU0;

            // Calculate the squared extent using ComputeType to avoid loss of
            // precision before computing a squared root.
            Vector2<ComputeType> sqrExtent;
            for (int i = 0; i < 2; ++i)
            {
                sqrExtent[i] = mHalf * Dot(minBox.U[i], difference[i]);
                sqrExtent[i] *= sqrExtent[i];
                sqrExtent[i] /= minBox.sqrLenU0;
            }

            for (int i = 0; i < 2; ++i)
            {
                itMinBox.center[i] = (InputType)center[i];
                itMinBox.extent[i] = std::sqrt((InputType)sqrExtent[i]);

                // Before converting to floating-point, factor out the maximum
                // component using ComputeType to generate rational numbers in
                // a range that avoids loss of precision during the conversion
                // and normalization.
                Vector2<ComputeType> const& axis = minBox.U[i];
                ComputeType cmax = std::max(std::fabs(axis[0]), std::fabs(axis[1]));
                ComputeType invCMax = mOne / cmax;
                for (int j = 0; j < 2; ++j)
                {
                    itMinBox.axis[i][j] = (InputType)(axis[j] * invCMax);
                }
                Normalize(itMinBox.axis[i]);
            }

            mSupportIndices = minBox.index;
            mArea = (InputType)minBox.area;
        }

        // The input points to be bound.
        int mNumPoints;
        Vector2<InputType> const* mPoints;

        // The indices into mPoints/mComputePoints for the convex hull
        // vertices.
        std::vector<int> mHull;

        // The support indices for the minimum-area box.
        std::array<int, 4> mSupportIndices;

        // The area of the minimum-area box.  The ComputeType value is
        // exact,  so the only rounding errors occur in the conversion from
        // ComputeType to InputType (default rounding mode is
        // round-to-nearest-ties-to-even).
        InputType mArea;

        // Convenient values that occur regularly in the code.  When using
        // rational ComputeType, we construct these numbers only once.
        ComputeType mZero, mOne, mNegOne, mHalf;
    };
}
