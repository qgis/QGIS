// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/FeatureKey.h>

// An ordered triangle has V[0] = min(v0, v1, v2).  Choose
// (V[0], V[1], V[2]) to be a permutation of (v0, v1, v2) so that the final
// is one of (v0, v1, v2), (v1, v2, v0) or (v2,v0,v1).  The idea is that if
// v0 corresponds to (1,0,0), v1 corresponds to (0,1,0), and v2 corresponds
// to (0,0,1), the ordering (v0, v1, v2) corresponds to the 3x3 identity
// matrix I; the rows are the specified 3-tuples.  The permutation
// (V[0], V[1], V[2]) induces a permutation of the rows of the identity
// matrix to form a permutation matrix P with det(P) = 1 = det(I).
//
// An unordered triangle stores a permutation of (v0, v1, v2) so that
// V[0] < V[1] < V[2].

namespace gte
{
    template <bool Ordered>
    class TriangleKey : public FeatureKey<3, Ordered>
    {
    public:
        TriangleKey()
        {
            this->V = { -1, -1, -1 };
        }

        // This constructor is specialized based on Ordered.
        explicit TriangleKey(int v0, int v1, int v2)
        {
            Initialize(v0, v1, v2);
        }

    private:
        template <bool Dummy = Ordered>
        typename std::enable_if<Dummy, void>::type
        Initialize(int v0, int v1, int v2)
        {
            if (v0 < v1)
            {
                if (v0 < v2)
                {
                    // v0 is minimum
                    this->V[0] = v0;
                    this->V[1] = v1;
                    this->V[2] = v2;
                }
                else
                {
                    // v2 is minimum
                    this->V[0] = v2;
                    this->V[1] = v0;
                    this->V[2] = v1;
                }
            }
            else
            {
                if (v1 < v2)
                {
                    // v1 is minimum
                    this->V[0] = v1;
                    this->V[1] = v2;
                    this->V[2] = v0;
                }
                else
                {
                    // v2 is minimum
                    this->V[0] = v2;
                    this->V[1] = v0;
                    this->V[2] = v1;
                }
            }
        }

        template <bool Dummy = Ordered>
        typename std::enable_if<!Dummy, void>::type
        Initialize(int v0, int v1, int v2)
        {
            if (v0 < v1)
            {
                if (v0 < v2)
                {
                    // v0 is minimum
                    this->V[0] = v0;
                    this->V[1] = std::min(v1, v2);
                    this->V[2] = std::max(v1, v2);
                }
                else
                {
                    // v2 is minimum
                    this->V[0] = v2;
                    this->V[1] = std::min(v0, v1);
                    this->V[2] = std::max(v0, v1);
                }
            }
            else
            {
                if (v1 < v2)
                {
                    // v1 is minimum
                    this->V[0] = v1;
                    this->V[1] = std::min(v2, v0);
                    this->V[2] = std::max(v2, v0);
                }
                else
                {
                    // v2 is minimum
                    this->V[0] = v2;
                    this->V[1] = std::min(v0, v1);
                    this->V[2] = std::max(v0, v1);
                }
            }
        }
    };
}
