// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/GVector.h>
#include <functional>

// The differential equation is dx/dt = F(t,x).  The TVector template
// parameter allows you to create solvers with Vector<N,Real> when the
// dimension N is known at compile time or GVector<Real> when the dimension
// N is known at run time.  Both classes have 'int GetSize() const' that
// allow OdeSolver-derived classes to query for the dimension.

namespace gte
{
    template <typename Real, typename TVector>
    class OdeSolver
    {
    public:
        // Abstract base class.
        virtual ~OdeSolver() = default;
    protected:
        OdeSolver(Real tDelta, std::function<TVector(Real, TVector const&)> const& F)
            :
            mTDelta(tDelta),
            mFunction(F)
        {
        }

    public:
        // Member access.
        inline void SetTDelta(Real tDelta)
        {
            mTDelta = tDelta;
        }

        inline Real GetTDelta() const
        {
            return mTDelta;
        }

        // Estimate x(t + tDelta) from x(t) using dx/dt = F(t,x).  The
        // derived classes implement this so that it is possible for xIn and
        // xOut to be the same object.
        virtual void Update(Real tIn, TVector const& xIn, Real& tOut, TVector& xOut) = 0;

    protected:
        Real mTDelta;
        std::function<TVector(Real, TVector const&)> mFunction;
    };
}
