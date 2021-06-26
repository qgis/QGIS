// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <array>
#include <cstdint>
#include <map>
#include <type_traits>
#include <vector>

namespace gte
{
    // The image type T must be one of the integer types:  int8_t, int16_t,
    // int32_t, uint8_t, uint16_t or uint32_t.  Internal integer computations
    // are performed using int64_t.  The type Real is for extraction to
    // floating-point vertices.
    template <typename T, typename Real>
    class CurveExtractor
    {
    public:
        // Abstract base class.
        virtual ~CurveExtractor() = default;

        // The level curves form a graph of vertices and edges.  The vertices
        // are computed as pairs of nonnegative rational numbers.  Vertex
        // represents the rational pair (xNumer/xDenom, yNumer/yDenom) as
        // (xNumer, xDenom, yNumer, yDenom), where all components are
        // nonnegative.  The edges connect pairs of vertices, forming a graph
        // that represents the level set.
        struct Vertex
        {
            Vertex() = default;

            Vertex(int64_t inXNumer, int64_t inXDenom, int64_t inYNumer, int64_t inYDenom)
            {
                // The vertex generation leads to the numerator and
                // denominator having the same sign.  This constructor changes
                // sign to ensure the numerator and denominator are both
                // positive.
                if (inXDenom > 0)
                {
                    xNumer = inXNumer;
                    xDenom = inXDenom;
                }
                else
                {
                    xNumer = -inXNumer;
                    xDenom = -inXDenom;
                }

                if (inYDenom > 0)
                {
                    yNumer = inYNumer;
                    yDenom = inYDenom;
                }
                else
                {
                    yNumer = -inYNumer;
                    yDenom = -inYDenom;
                }
            }

            // The non-default constructor guarantees that xDenom > 0 and
            // yDenom > 0.  The following comparison operators assume that
            // the denominators are positive.
            bool operator==(Vertex const& other) const
            {
                return
                    // xn0 / xd0 == xn1 / xd1
                    xNumer * other.xDenom == other.xNumer * xDenom
                    &&
                    // yn0/yd0 == yn1/yd1
                    yNumer * other.yDenom == other.yNumer * yDenom;
            }

            bool operator<(Vertex const& other) const
            {
                int64_t xn0txd1 = xNumer * other.xDenom;
                int64_t xn1txd0 = other.xNumer * xDenom;
                if (xn0txd1 < xn1txd0)
                {
                    // xn0/xd0 < xn1/xd1
                    return true;
                }
                if (xn0txd1 > xn1txd0)
                {
                    // xn0/xd0 > xn1/xd1
                    return false;
                }

                int64_t yn0tyd1 = yNumer * other.yDenom;
                int64_t yn1tyd0 = other.yNumer * yDenom;
                // yn0/yd0 < yn1/yd1
                return yn0tyd1 < yn1tyd0;
            }

            int64_t xNumer, xDenom, yNumer, yDenom;
        };

        struct Edge
        {
            Edge() = default;

            Edge(int v0, int v1)
            {
                if (v0 < v1)
                {
                    v[0] = v0;
                    v[1] = v1;
                }
                else
                {
                    v[0] = v1;
                    v[1] = v0;
                }
            }

            bool operator==(Edge const& other) const
            {
                return v[0] == other.v[0] && v[1] == other.v[1];
            }

            bool operator<(Edge const& other) const
            {
                for (int i = 0; i < 2; ++i)
                {
                    if (v[i] < other.v[i])
                    {
                        return true;
                    }
                    if (v[i] > other.v[i])
                    {
                        return false;
                    }
                }
                return false;
            }

            std::array<int, 2> v;
        };

        // Extract level curves and return rational vertices.
        virtual void Extract(T level, std::vector<Vertex>& vertices,
            std::vector<Edge>& edges) = 0;

        void Extract(T level, bool removeDuplicateVertices,
            std::vector<std::array<Real, 2>>& vertices, std::vector<Edge>& edges)
        {
            std::vector<Vertex> rationalVertices;
            Extract(level, rationalVertices, edges);
            if (removeDuplicateVertices)
            {
                MakeUnique(rationalVertices, edges);
            }
            Convert(rationalVertices, vertices);
        }

        // The extraction has duplicate vertices on edges shared by pixels.
        // This function will eliminate the duplicates.
        void MakeUnique(std::vector<Vertex>& vertices, std::vector<Edge>& edges)
        {
            size_t numVertices = vertices.size();
            size_t numEdges = edges.size();
            if (numVertices == 0 || numEdges == 0)
            {
                return;
            }

            // Compute the map of unique vertices and assign to them new and
            // unique indices.
            std::map<Vertex, int> vmap;
            int nextVertex = 0;
            for (size_t v = 0; v < numVertices; ++v)
            {
                // Keep only unique vertices.
                auto result = vmap.insert(std::make_pair(vertices[v], nextVertex));
                if (result.second)
                {
                    ++nextVertex;
                }
            }

            // Compute the map of unique edges and assign to them new and
            // unique indices.
            std::map<Edge, int> emap;
            int nextEdge = 0;
            for (size_t e = 0; e < numEdges; ++e)
            {
                // Replace old vertex indices by new vertex indices.
                Edge& edge = edges[e];
                for (int i = 0; i < 2; ++i)
                {
                    auto iter = vmap.find(vertices[edge.v[i]]);
                    LogAssert(iter != vmap.end(), "Expecting the vertex to be in the vmap.");
                    edge.v[i] = iter->second;
                }

                // Keep only unique edges.
                auto result = emap.insert(std::make_pair(edge, nextEdge));
                if (result.second)
                {
                    ++nextEdge;
                }
            }

            // Pack the vertices into an array.
            vertices.resize(vmap.size());
            for (auto const& element : vmap)
            {
                vertices[element.second] = element.first;
            }

            // Pack the edges into an array.
            edges.resize(emap.size());
            for (auto const& element : emap)
            {
                edges[element.second] = element.first;
            }
        }

        // Convert from Vertex to std::array<Real, 2> rationals.
        void Convert(std::vector<Vertex> const& input, std::vector<std::array<Real, 2>>& output)
        {
            output.resize(input.size());
            for (size_t i = 0; i < input.size(); ++i)
            {
                Real rxNumer = static_cast<Real>(input[i].xNumer);
                Real rxDenom = static_cast<Real>(input[i].xDenom);
                Real ryNumer = static_cast<Real>(input[i].yNumer);
                Real ryDenom = static_cast<Real>(input[i].yDenom);
                output[i][0] = rxNumer / rxDenom;
                output[i][1] = ryNumer / ryDenom;
            }
        }

    protected:
        // The input is a 2D image with lexicographically ordered pixels (x,y)
        // stored in a linear array.  Pixel (x,y) is stored in the array at
        // location index = x + xBound * y.  The inputs xBound and yBound must
        // each be 2 or larger so that there is at least one image square to
        // process.  The inputPixels must be nonnull and point to contiguous
        // storage that contains at least xBound * yBound elements.
        CurveExtractor(int xBound, int yBound, T const* inputPixels)
            :
            mXBound(xBound),
            mYBound(yBound),
            mInputPixels(inputPixels)
        {
            static_assert(std::is_integral<T>::value && sizeof(T) <= 4,
                "Type T must be int{8,16,32}_t or uint{8,16,32}_t.");
            LogAssert(mXBound > 1 && mYBound > 1 && mInputPixels != nullptr, "Invalid input.");
            mPixels.resize(static_cast<size_t>(mXBound * mYBound));
        }

        void AddVertex(std::vector<Vertex>& vertices,
            int64_t xNumer, int64_t xDenom, int64_t yNumer, int64_t yDenom)
        {
            vertices.push_back(Vertex(xNumer, xDenom, yNumer, yDenom));
        }

        void AddEdge(std::vector<Vertex>& vertices, std::vector<Edge>& edges,
            int64_t xNumer0, int64_t xDenom0, int64_t yNumer0, int64_t yDenom0,
            int64_t xNumer1, int64_t xDenom1, int64_t yNumer1, int64_t yDenom1)
        {
            int v0 = static_cast<int>(vertices.size());
            int v1 = v0 + 1;
            edges.push_back(Edge(v0, v1));
            vertices.push_back(Vertex(xNumer0, xDenom0, yNumer0, yDenom0));
            vertices.push_back(Vertex(xNumer1, xDenom1, yNumer1, yDenom1));
        }

        int mXBound, mYBound;
        T const* mInputPixels;
        std::vector<int64_t> mPixels;
    };
}
