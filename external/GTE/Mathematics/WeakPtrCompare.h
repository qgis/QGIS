// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <memory>

// Comparison operators for std::weak_ptr objects.  The type T must implement
// comparison operators. You must be careful when managing containers whose
// ordering is based on std::weak_ptr comparisons.  The underlying objects
// can change, which invalidates the container ordering.  If objects do not
// change while the container persists, these are safe to use.

namespace gte
{
    // wp0 == wp1
    template <typename T>
    struct WeakPtrEQ
    {
        bool operator()(std::weak_ptr<T> const& wp0, std::weak_ptr<T> const& wp1) const
        {
            auto sp0 = wp0.lock(), sp1 = wp1.lock();
            return (sp0 ? (sp1 ? *sp0 == *sp1 : false) : !sp1);
        }
    };

    // wp0 != wp1
    template <typename T>
    struct WeakPtrNEQ
    {
        bool operator()(std::weak_ptr<T> const& wp0, std::weak_ptr<T> const& wp1) const
        {
            return !WeakPtrEQ<T>()(wp0, wp1);
        }
    };

    // wp0 < wp1
    template <typename T>
    struct WeakPtrLT
    {
        bool operator()(std::weak_ptr<T> const& wp0, std::weak_ptr<T> const& wp1) const
        {
            auto sp0 = wp0.lock(), sp1 = wp1.lock();
            return (sp1 ? (!sp0 || *sp0 < *sp1) : false);
        }
    };

    // wp0 <= wp1
    template <typename T>
    struct WeakPtrLTE
    {
        bool operator()(std::weak_ptr<T> const& wp0, std::weak_ptr<T> const& wp1) const
        {
            return !WeakPtrLT<T>()(wp1, wp0);
        }
    };

    // wp0 > wp1
    template <typename T>
    struct WeakPtrGT
    {
        bool operator()(std::weak_ptr<T> const& wp0, std::weak_ptr<T> const& wp1) const
        {
            return WeakPtrLT<T>()(wp1, wp0);
        }
    };

    // wp0 >= wp1
    template <typename T>
    struct WeakPtrGTE
    {
        bool operator()(std::weak_ptr<T> const& wp0, std::weak_ptr<T> const& wp1) const
        {
            return !WeakPtrLT<T>()(wp0, wp1);
        }
    };
}
