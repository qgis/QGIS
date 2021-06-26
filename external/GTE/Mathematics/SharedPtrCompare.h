// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <memory>

// Comparison operators for std::shared_ptr objects. The type T must implement
// comparison operators. You must be careful when managing containers whose
// ordering is based on std::shared_ptr comparisons.  The underlying objects
// can change, which invalidates the container ordering.  If objects do not
// change while the container persists, these are safe to use.
//
// NOTE: std::shared_ptr<T> already has comparison operators, but these
// compare pointer values instead of comparing the objects referenced by the
// pointers.  If a container sorted using std::shared_ptr<T> is created for
// two different executions of a program, the object ordering implied by the
// pointer ordering can differ.  This might be undesirable for reproducibility
// of results between executions.

namespace gte
{
    // sp0 == sp1
    template <typename T>
    struct SharedPtrEQ
    {
        bool operator()(std::shared_ptr<T> const& sp0, std::shared_ptr<T> const& sp1) const
        {
            return (sp0 ? (sp1 ? *sp0 == *sp1 : false) : !sp1);
        }
    };

    // sp0 != sp1
    template <typename T>
    struct SharedPtrNEQ
    {
        bool operator()(std::shared_ptr<T> const& sp0, std::shared_ptr<T> const& sp1) const
        {
            return !SharedPtrEQ<T>()(sp0, sp1);
        }
    };

    // sp0 < sp1
    template <typename T>
    struct SharedPtrLT
    {
        bool operator()(std::shared_ptr<T> const& sp0, std::shared_ptr<T> const& sp1) const
        {
            return (sp1 ? (!sp0 || *sp0 < *sp1) : false);
        }
    };

    // sp0 <= sp1
    template <typename T>
    struct SharedPtrLTE
    {
        bool operator()(std::shared_ptr<T> const& sp0, std::shared_ptr<T> const& sp1) const
        {
            return !SharedPtrLT<T>()(sp1, sp0);
        }
    };

    // sp0 > sp1
    template <typename T>
    struct SharedPtrGT
    {
        bool operator()(std::shared_ptr<T> const& sp0, std::shared_ptr<T> const& sp1) const
        {
            return SharedPtrLT<T>()(sp1, sp0);
        }
    };

    // sp0 >= sp1
    template <typename T>
    struct SharedPtrGTE
    {
        bool operator()(std::shared_ptr<T> const& sp0, std::shared_ptr<T> const& sp1) const
        {
            return !SharedPtrLT<T>()(sp0, sp1);
        }
    };
}
