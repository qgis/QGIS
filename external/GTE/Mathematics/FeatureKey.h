// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.11.16

#pragma once

#include <Mathematics/HashCombine.h>
#include <algorithm>
#include <array>

namespace gte
{
    template <int N, bool Ordered>
    class FeatureKey
    {
    protected:
        // Abstract base class with V[] uninitialized.  The derived classes
        // must set the V[] values accordingly.
        //
        // An ordered feature key has V[0] = min(V[]) with
        // (V[0],V[1],...,V[N-1]) a permutation of N inputs with an even
        // number of transpositions.
        //
        // An unordered feature key has V[0] < V[1] < ... < V[N-1].
        //
        // Note that the word 'order' is about the geometry of the feature, not
        // the comparison order for any sorting.
        FeatureKey() = default;

    public:
        // The std::array comparisons were removed to improve the speed of the
        // comparisons when using the C++ Standard Library that ships with
        // Microsoft Visual Studio 2019 16.7.*.
        bool operator==(FeatureKey const& key) const
        {
            for (size_t i = 0; i < N; ++i)
            {
                if (V[i] != key.V[i])
                {
                    return false;
                }
            }
            return true;
        }

        bool operator!=(FeatureKey const& key) const
        {
            return !operator==(key);
        }

        bool operator<(FeatureKey const& key) const
        {
            for (size_t i = 0; i < N; ++i)
            {
                if (V[i] < key.V[i])
                {
                    return true;
                }
                if (V[i] > key.V[i])
                {
                    return false;
                }
            }
            return false;
        }

        bool operator<=(FeatureKey const& key) const
        {
            return !key.operator<(*this);
        }

        bool operator>(FeatureKey const& key) const
        {
            return key.operator<(*this);
        }

        bool operator>=(FeatureKey const& key) const
        {
            return !operator<(key);
        }

        // Support for hashing in std::unordered*<T> container classes. The
        // first operator() is the hash function. The second operator() is
        // the equality comparison used for elements in the same bucket.
        std::size_t operator()(FeatureKey const& key) const
        {
            std::size_t seed = 0;
            for (auto value : key.V)
            {
                HashCombine(seed, value);
            }
            return seed;
        }

        bool operator()(FeatureKey const& v0, FeatureKey const& v1) const
        {
            return v0 == v1;
        }

        std::array<int, N> V;
    };
}
