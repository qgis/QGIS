// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Delaunay3.h>

namespace gte
{
    template <typename InputType, typename ComputeType, typename RationalType>
    class Delaunay3Mesh
    {
    public:
        // Construction.
        Delaunay3Mesh(Delaunay3<InputType, ComputeType> const& delaunay)
            :
            mDelaunay(&delaunay)
        {
        }

        // Mesh information.
        inline int GetNumVertices() const
        {
            return mDelaunay->GetNumVertices();
        }

        inline int GetNumTetrahedra() const
        {
            return mDelaunay->GetNumTetrahedra();
        }

        inline Vector3<InputType> const* GetVertices() const
        {
            return mDelaunay->GetVertices();
        }

        inline int const* GetIndices() const
        {
            return &mDelaunay->GetIndices()[0];
        }

        inline int const* GetAdjacencies() const
        {
            return &mDelaunay->GetAdjacencies()[0];
        }

        // Containment queries.

        int GetContainingTetrahedron(Vector3<InputType> const& P) const
        {
            typename Delaunay3<InputType, ComputeType>::SearchInfo info;
            return mDelaunay->GetContainingTetrahedron(P, info);
        }

        bool GetVertices(int t, std::array<Vector3<InputType>, 4>& vertices) const
        {
            if (mDelaunay->GetDimension() == 3)
            {
                std::array<int, 4> indices;
                if (mDelaunay->GetIndices(t, indices))
                {
                    PrimalQuery3<ComputeType> const& query = mDelaunay->GetQuery();
                    Vector3<ComputeType> const* ctVertices = query.GetVertices();
                    for (int i = 0; i < 4; ++i)
                    {
                        Vector3<ComputeType> const& V = ctVertices[indices[i]];
                        for (int j = 0; j < 3; ++j)
                        {
                            vertices[i][j] = (InputType)V[j];
                        }
                    }
                    return true;
                }
            }
            return false;
        }

        bool GetIndices(int t, std::array<int, 4>& indices) const
        {
            return mDelaunay->GetIndices(t, indices);
        }

        bool GetAdjacencies(int t, std::array<int, 4>& adjacencies) const
        {
            return mDelaunay->GetAdjacencies(t, adjacencies);
        }

        bool GetBarycentrics(int t, Vector3<InputType> const& P, std::array<InputType, 4>& bary) const
        {
            std::array<int, 4> indices;
            if (mDelaunay->GetIndices(t, indices))
            {
                PrimalQuery3<ComputeType> const& query = mDelaunay->GetQuery();
                Vector3<ComputeType> const* vertices = query.GetVertices();
                Vector3<RationalType> rtP{ P[0], P[1], P[2] };
                std::array<Vector3<RationalType>, 4> rtV;
                for (int i = 0; i < 4; ++i)
                {
                    Vector3<ComputeType> const& V = vertices[indices[i]];
                    for (int j = 0; j < 3; ++j)
                    {
                        rtV[i][j] = (RationalType)V[j];
                    }
                };

                RationalType rtBary[4];
                if (ComputeBarycentrics(rtP, rtV[0], rtV[1], rtV[2], rtV[3], rtBary))
                {
                    for (int i = 0; i < 4; ++i)
                    {
                        bary[i] = (InputType)rtBary[i];
                    }
                    return true;
                }
            }
            return false;
        }

    private:
        Delaunay3<InputType, ComputeType> const* mDelaunay;
    };
}
