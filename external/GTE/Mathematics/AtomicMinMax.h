// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <algorithm>
#include <atomic>

// Implementations of atomic minimum and atomic maximum computations.  These
// are based on std::atomic_compare_exchange_strong.

namespace gte
{
    template <typename T>
    T AtomicMin(std::atomic<T>& v0, T const& v1)
    {
        T vInitial, vMin;
        do
        {
            vInitial = v0;
            vMin = std::min(vInitial, v1);
        }
        while (!std::atomic_compare_exchange_strong(&v0, &vInitial, vMin));
        return vInitial;
    }

    template <typename T>
    T AtomicMax(std::atomic<T>& v0, T const& v1)
    {
        T vInitial, vMax;
        do
        {
            vInitial = v0;
            vMax = std::max(vInitial, v1);
        }
        while (!std::atomic_compare_exchange_strong(&v0, &vInitial, vMax));
        return vInitial;
    }
}
