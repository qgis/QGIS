// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Math.h>

namespace gte
{
    // Distance and closest-point queries.
    template <typename Real, typename Type0, typename Type1>
    class DCPQuery
    {
    public:
        struct Result
        {
            // A DCPQuery-base class B must define a B::Result struct with
            // member 'Real distance'.  A DCPQuery-derived class D must also
            // derive a D::Result from B:Result but may have no members.  The
            // idea is to allow Result to store closest-point information in
            // addition to the distance.  The operator() is non-const so that
            // specific implementations can use internal data to support the
            // queries.  The implementations can also use static functions as
            // necessary.
        };

        Result operator()(Type0 const& primitive0, Type1 const& primitive1);
    };
}
