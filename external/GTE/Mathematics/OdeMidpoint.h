// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/OdeSolver.h>

// The TVector template parameter allows you to create solvers with
// Vector<N,Real> when the dimension N is known at compile time or
// GVector<Real> when the dimension N is known at run time.  Both classes
// have 'int GetSize() const' that allow OdeSolver-derived classes to query
// for the dimension.

namespace gte
{
    template <typename Real, typename TVector>
    class OdeMidpoint : public OdeSolver<Real, TVector>
    {
    public:
        // Construction and destruction.
        virtual ~OdeMidpoint() = default;

        OdeMidpoint(Real tDelta, std::function<TVector(Real, TVector const&)> const& F)
            :
            OdeSolver<Real, TVector>(tDelta, F)
        {
        }

        // Estimate x(t + tDelta) from x(t) using dx/dt = F(t,x).  You may
        // allow xIn and xOut to be the same object.
        virtual void Update(Real tIn, TVector const& xIn, Real& tOut, TVector& xOut) override
        {
            // Compute the first step.
            Real halfTDelta = (Real)0.5 * this->mTDelta;
            TVector fVector = this->mFunction(tIn, xIn);
            TVector xTemp = xIn + halfTDelta * fVector;

            // Compute the second step.
            Real halfT = tIn + halfTDelta;
            fVector = this->mFunction(halfT, xTemp);
            tOut = tIn + this->mTDelta;
            xOut = xIn + this->mTDelta * fVector;
        }
    };
}
