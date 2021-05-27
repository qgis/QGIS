// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/TIQuery.h>
#include <Mathematics/FIQuery.h>
#include <Mathematics/Hyperplane.h>
#include <Mathematics/Vector.h>
#include <list>
#include <vector>

// The intersection queries are based on the document
// https://www.geometrictools.com/Documentation/ClipConvexPolygonByHyperplane.pdf

namespace gte
{
    template <int N, typename Real>
    class TIQuery<Real, std::vector<Vector<N, Real>>, Hyperplane<N, Real>>
    {
    public:
        enum class Configuration
        {
            SPLIT,
            POSITIVE_SIDE_VERTEX,
            POSITIVE_SIDE_EDGE,
            POSITIVE_SIDE_STRICT,
            NEGATIVE_SIDE_VERTEX,
            NEGATIVE_SIDE_EDGE,
            NEGATIVE_SIDE_STRICT,
            CONTAINED,
            INVALID_POLYGON
        };

        struct Result
        {
            bool intersect;
            Configuration configuration;
        };

        Result operator()(std::vector<Vector<N, Real>> const& polygon, Hyperplane<N, Real> const& hyperplane)
        {
            Result result;

            size_t const numVertices = polygon.size();
            if (numVertices < 3)
            {
                // The convex polygon must have at least 3 vertices.
                result.intersect = false;
                result.configuration = Configuration::INVALID_POLYGON;
                return result;
            }

            // Determine on which side of the hyperplane each vertex lies.
            size_t numPositive = 0, numNegative = 0, numZero = 0;
            for (size_t i = 0; i < numVertices; ++i)
            {
                Real h = Dot(hyperplane.normal, polygon[i]) - hyperplane.constant;
                if (h > (Real)0)
                {
                    ++numPositive;
                }
                else if (h < (Real)0)
                {
                    ++numNegative;
                }
                else
                {
                    ++numZero;
                }
            }

            if (numPositive > 0)
            {
                if (numNegative > 0)
                {
                    result.intersect = true;
                    result.configuration = Configuration::SPLIT;
                }
                else if (numZero == 0)
                {
                    result.intersect = false;
                    result.configuration = Configuration::POSITIVE_SIDE_STRICT;
                }
                else if (numZero == 1)
                {
                    result.intersect = true;
                    result.configuration = Configuration::POSITIVE_SIDE_VERTEX;
                }
                else // numZero > 1
                {
                    result.intersect = true;
                    result.configuration = Configuration::POSITIVE_SIDE_EDGE;
                }
            }
            else if (numNegative > 0)
            {
                if (numZero == 0)
                {
                    result.intersect = false;
                    result.configuration = Configuration::NEGATIVE_SIDE_STRICT;
                }
                else if (numZero == 1)
                {
                    // The polygon touches the plane in a vertex or an edge.
                    result.intersect = true;
                    result.configuration = Configuration::NEGATIVE_SIDE_VERTEX;
                }
                else // numZero > 1
                {
                    result.intersect = true;
                    result.configuration = Configuration::NEGATIVE_SIDE_EDGE;
                }
            }
            else  // numZero == numVertices
            {
                result.intersect = true;
                result.configuration = Configuration::CONTAINED;
            }

            return result;
        }
    };

    template <int N, typename Real>
    class FIQuery<Real, std::vector<Vector<N, Real>>, Hyperplane<N, Real>>
    {
    public:
        enum class Configuration
        {
            SPLIT,
            POSITIVE_SIDE_VERTEX,
            POSITIVE_SIDE_EDGE,
            POSITIVE_SIDE_STRICT,
            NEGATIVE_SIDE_VERTEX,
            NEGATIVE_SIDE_EDGE,
            NEGATIVE_SIDE_STRICT,
            CONTAINED,
            INVALID_POLYGON
        };

        struct Result
        {
            // The intersection is either empty, a single vertex, a single
            // edge or the polygon is contained by the hyperplane.
            Configuration configuration;
            std::vector<Vector<N, Real>> intersection;

            // If 'configuration' is POSITIVE_* or SPLIT, this polygon is the
            // portion of the query input 'polygon' on the positive side of
            // the hyperplane with possibly a vertex or edge on the hyperplane.
            std::vector<Vector<N, Real>> positivePolygon;

            // If 'configuration' is NEGATIVE_* or SPLIT, this polygon is the
            // portion of the query input 'polygon' on the negative side of
            // the hyperplane with possibly a vertex or edge on the hyperplane.
            std::vector<Vector<N, Real>> negativePolygon;
        };

        Result operator()(std::vector<Vector<N, Real>> const& polygon, Hyperplane<N, Real> const& hyperplane)
        {
            Result result;

            size_t const numVertices = polygon.size();
            if (numVertices < 3)
            {
                // The convex polygon must have at least 3 vertices.
                result.configuration = Configuration::INVALID_POLYGON;
                return result;
            }

            // Determine on which side of the hyperplane the vertices live.
            // The index maxPosIndex stores the index of the vertex on the
            // positive side of the hyperplane that is farthest from the
            // hyperplane.  The index maxNegIndex stores the index of the
            // vertex on the negative side of the hyperplane that is farthest
            // from the hyperplane.  If one or the other such vertex does not
            // exist, the corresponding index will remain its initial value of
            // max(size_t).
            std::vector<Real> height(numVertices);
            std::vector<size_t> zeroHeightIndices;
            zeroHeightIndices.reserve(numVertices);
            size_t numPositive = 0, numNegative = 0;
            Real maxPosHeight = -std::numeric_limits<Real>::max();
            Real maxNegHeight = std::numeric_limits<Real>::max();
            size_t maxPosIndex = std::numeric_limits<size_t>::max();
            size_t maxNegIndex = std::numeric_limits<size_t>::max();
            for (size_t i = 0; i < numVertices; ++i)
            {
                height[i] = Dot(hyperplane.normal, polygon[i]) - hyperplane.constant;
                if (height[i] > (Real)0)
                {
                    ++numPositive;
                    if (height[i] > maxPosHeight)
                    {
                        maxPosHeight = height[i];
                        maxPosIndex = i;
                    }
                }
                else if (height[i] < (Real)0)
                {
                    ++numNegative;
                    if (height[i] < maxNegHeight)
                    {
                        maxNegHeight = height[i];
                        maxNegIndex = i;
                    }
                }
                else
                {
                    zeroHeightIndices.push_back(i);
                }
            }

            if (numPositive > 0)
            {
                if (numNegative > 0)
                {
                    result.configuration = Configuration::SPLIT;

                    bool doSwap = (maxPosHeight < -maxNegHeight);
                    if (doSwap)
                    {
                        for (auto& h : height)
                        {
                            h = -h;
                        }
                        std::swap(maxPosIndex, maxNegIndex);
                    }

                    SplitPolygon(polygon, height, maxPosIndex, result);

                    if (doSwap)
                    {
                        std::swap(result.positivePolygon, result.negativePolygon);
                    }
                }
                else
                {
                    size_t numZero = zeroHeightIndices.size();
                    if (numZero == 0)
                    {
                        result.configuration = Configuration::POSITIVE_SIDE_STRICT;
                    }
                    else if (numZero == 1)
                    {
                        result.configuration = Configuration::POSITIVE_SIDE_VERTEX;
                        result.intersection =
                        {
                            polygon[zeroHeightIndices[0]]
                        };
                    }
                    else // numZero > 1
                    {
                        result.configuration = Configuration::POSITIVE_SIDE_EDGE;
                        result.intersection =
                        {
                            polygon[zeroHeightIndices[0]],
                            polygon[zeroHeightIndices[1]]
                        };
                    }
                    result.positivePolygon = polygon;
                }
            }
            else if (numNegative > 0)
            {
                size_t numZero = zeroHeightIndices.size();
                if (numZero == 0)
                {
                    result.configuration = Configuration::NEGATIVE_SIDE_STRICT;
                }
                else if (numZero == 1)
                {
                    result.configuration = Configuration::NEGATIVE_SIDE_VERTEX;
                    result.intersection =
                    {
                        polygon[zeroHeightIndices[0]]
                    };
                }
                else  // numZero > 1
                {
                    result.configuration = Configuration::NEGATIVE_SIDE_EDGE;
                    result.intersection =
                    {
                        polygon[zeroHeightIndices[0]],
                        polygon[zeroHeightIndices[1]]
                    };
                }
                result.negativePolygon = polygon;
            }
            else  // numZero == numVertices
            {
                result.configuration = Configuration::CONTAINED;
                result.intersection = polygon;
            }

            return result;
        }

    protected:
        void SplitPolygon(std::vector<Vector<N, Real>> const& polygon,
            std::vector<Real> const& height, size_t maxPosIndex, Result& result)
        {
            // Find the largest contiguous subset of indices for which
            // height[i] >= 0.
            size_t const numVertices = polygon.size();
            std::list<Vector<N, Real>> positiveList;
            positiveList.push_back(polygon[maxPosIndex]);
            size_t end0 = maxPosIndex;
            size_t end0prev = std::numeric_limits<size_t>::max();
            for (size_t i = 0; i < numVertices; ++i)
            {
                end0prev = (end0 + numVertices - 1) % numVertices;
                if (height[end0prev] >= (Real)0)
                {
                    positiveList.push_front(polygon[end0prev]);
                    end0 = end0prev;
                }
                else
                {
                    break;
                }
            }

            size_t end1 = maxPosIndex;
            size_t end1next = std::numeric_limits<size_t>::max();
            for (size_t i = 0; i < numVertices; ++i)
            {
                end1next = (end1 + 1) % numVertices;
                if (height[end1next] >= (Real)0)
                {
                    positiveList.push_back(polygon[end1next]);
                    end1 = end1next;
                }
                else
                {
                    break;
                }
            }

            size_t index = end1next;
            std::list<Vector<N, Real>> negativeList;
            for (size_t i = 0; i < numVertices; ++i)
            {
                negativeList.push_back(polygon[index]);
                index = (index + 1) % numVertices;
                if (index == end0)
                {
                    break;
                }
            }

            // Clip the polygon.
            if (height[end0] > (Real)0)
            {
                Real t = -height[end0prev] / (height[end0] - height[end0prev]);
                Real omt = (Real)1 - t;
                Vector<N, Real> V = omt * polygon[end0prev] + t * polygon[end0];
                positiveList.push_front(V);
                negativeList.push_back(V);
                result.intersection.push_back(V);
            }
            else
            {
                negativeList.push_back(polygon[end0]);
                result.intersection.push_back(polygon[end0]);
            }

            if (height[end1] > (Real)0)
            {
                Real t = -height[end1next] / (height[end1] - height[end1next]);
                Real omt = (Real)1 - t;
                Vector<N, Real> V = omt * polygon[end1next] + t * polygon[end1];
                positiveList.push_back(V);
                negativeList.push_front(V);
                result.intersection.push_back(V);
            }
            else
            {
                negativeList.push_front(polygon[end1]);
                result.intersection.push_back(polygon[end1]);
            }

            result.positivePolygon.reserve(positiveList.size());
            for (auto const& p : positiveList)
            {
                result.positivePolygon.push_back(p);
            }

            result.negativePolygon.reserve(negativeList.size());
            for (auto const& p : negativeList)
            {
                result.negativePolygon.push_back(p);
            }
        }
    };
}
