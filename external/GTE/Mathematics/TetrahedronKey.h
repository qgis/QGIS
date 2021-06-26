// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/FeatureKey.h>

// An ordered tetrahedron has V[0] = min(v0, v1, v2, v3).  Let {u1, u2, u3}
// be the set of inputs excluding the one assigned to V[0] and define
// V[1] = min(u1, u2, u3).  Choose (V[1], V[2], V[3]) to be a permutation
// of (u1, u2, u3) so that the final storage is one of
//   (v0, v1, v2, v3), (v0, v2, v3, v1), (v0, v3, v1, v2)
//   (v1, v3, v2, v0), (v1, v2, v0, v3), (v1, v0, v3, v2)
//   (v2, v3, v0, v1), (v2, v0, v1, v3), (v2, v1, v3, v0)
//   (v3, v1, v0, v2), (v3, v0, v2, v1), (v3, v2, v1, v0)
// The idea is that if v0 corresponds to (1, 0, 0, 0), v1 corresponds to
// (0, 1, 0, 0), v2 corresponds to (0, 0, 1, 0), and v3 corresponds to
// (0, 0, 0, 1), the ordering (v0, v1, v2, v3) corresponds to the 4x4 identity
// matrix I; the rows are the specified 4-tuples.  The permutation
// (V[0], V[1], V[2], V[3]) induces a permutation of the rows of the identity
// matrix to form a permutation matrix P with det(P) = 1 = det(I).
//
// An unordered tetrahedron stores a permutation of (v0, v1, v2, v3) so
// that V[0] < V[1] < V[2] < V[3].

namespace gte
{
    template <bool Ordered>
    class TetrahedronKey : public FeatureKey<4, Ordered>
    {
    public:
        TetrahedronKey()
        {
            this->V = { -1, -1, -1, -1 };
        }

        explicit TetrahedronKey(int v0, int v1, int v2, int v3)
        {
            Initialize(v0, v1, v2, v3);
        }

        // Indexing for the vertices of the triangle opposite a vertex.  The
        // triangle opposite vertex j is
        //   <oppositeFace[j][0], oppositeFace[j][1], oppositeFace[j][2]>
        // and is listed in counterclockwise order when viewed from outside
        // the tetrahedron.
        static inline std::array<std::array<int, 3>, 4> const& GetOppositeFace()
        {
            static std::array<std::array<int, 3>, 4> const sOppositeFace =
            {{
                { 1, 2, 3 },
                { 0, 3, 2 },
                { 0, 1, 3 },
                { 0, 2, 1 }
            }};
            return sOppositeFace;
        }

    private:
        template <bool Dummy = Ordered>
        typename std::enable_if<Dummy, void>::type
        Initialize(int v0, int v1, int v2, int v3)
        {
            int imin = 0;
            this->V[0] = v0;
            if (v1 < this->V[0])
            {
                this->V[0] = v1;
                imin = 1;
            }
            if (v2 < this->V[0])
            {
                this->V[0] = v2;
                imin = 2;
            }
            if (v3 < this->V[0])
            {
                this->V[0] = v3;
                imin = 3;
            }

            if (imin == 0)
            {
                Permute(v1, v2, v3);
            }
            else if (imin == 1)
            {
                Permute(v0, v3, v2);
            }
            else if (imin == 2)
            {
                Permute(v0, v1, v3);
            }
            else  // imin == 3
            {
                Permute(v0, v2, v1);
            }
        }

        template <bool Dummy = Ordered>
        typename std::enable_if<!Dummy, void>::type
        Initialize(int v0, int v1, int v2, int v3)
        {
            this->V[0] = v0;
            this->V[1] = v1;
            this->V[2] = v2;
            this->V[3] = v3;
            std::sort(this->V.begin(), this->V.end());
        }

        template <bool Dummy = Ordered>
        typename std::enable_if<Dummy, void>::type
        Permute(int u0, int u1, int u2)
        {
            // Once V[0] is determined, create a permutation
            // (V[1], V[2], V[3]) so that (V[0], V[1], V[2], V[3]) is a
            // permutation of (v0, v1, v2, v3) that corresponds to the
            // identity matrix as mentioned in the comments at the beginning
            // of this file.
            if (u0 < u1)
            {
                if (u0 < u2)
                {
                    // u0 is minimum
                    this->V[1] = u0;
                    this->V[2] = u1;
                    this->V[3] = u2;
                }
                else
                {
                    // u2 is minimum
                    this->V[1] = u2;
                    this->V[2] = u0;
                    this->V[3] = u1;
                }
            }
            else
            {
                if (u1 < u2)
                {
                    // u1 is minimum
                    this->V[1] = u1;
                    this->V[2] = u2;
                    this->V[3] = u0;
                }
                else
                {
                    // u2 is minimum
                    this->V[1] = u2;
                    this->V[2] = u0;
                    this->V[3] = u1;
                }
            }
        }
    };
}
