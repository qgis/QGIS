// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 5.4.2020.11.16

#pragma once

#include <functional>

// Support for creating hash values for a list of types, each such type T
// having a valid std::hash<T>() function.
//
// The code here comes from the book
//   Nicolai M. Josuttis, "The C++ Standard Library: A Tutorial",
//   and Reference, 2nd edition", Addison-Wesley Professional,
//   March 2012 [Section 7.9.2, pp. 364-465.
// Credit for the hash_combine concept is from
//   https://www.boost.org/doc/libs/1_35_0/doc/html/hash/combine.html
// The magic number and shifts are based on the paper
//   Timothy C. Hoad and Justin Zobel, "Methods for Identifying Versioned
//   and Plagiarised Documents", Journal of the American Society for
//   Information Science and Technology, vol. 54, no. 3, February 2003.
//   https://dl.acm.org/doi/10.1002/asi.10170

namespace gte
{
    template <typename T>
    inline void HashCombine(std::size_t& seed, T const& value)
    {
        seed ^= std::hash<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    // Functions to create a hash value using a seed.
    template <typename T>
    inline void HashValue(std::size_t& seed, T const& value)
    {
        HashCombine(seed, value);
    }

    template <typename T, typename... Tail>
    inline void HashValue(std::size_t& seed, T const& value, Tail const&... arguments)
    {
        HashCombine(seed, value);
        HashValue(seed, arguments...);
    }

    // Functions to create a hash value from a list of arguments.
    template <typename... Tail>
    inline std::size_t HashValue(Tail const&... arguments)
    {
        std::size_t seed = 0;
        HashValue(seed, arguments...);
        return seed;
    }
}
