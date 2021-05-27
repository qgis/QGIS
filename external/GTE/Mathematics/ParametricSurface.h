// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Vector.h>

namespace gte
{
    template <int N, typename Real>
    class ParametricSurface
    {
    protected:
        // Abstract base class for a parameterized surface X(u,v).  The
        // parametric domain is either rectangular or triangular.  Valid
        // (u,v) values for a rectangular domain satisfy
        //   umin <= u <= umax,  vmin <= v <= vmax
        // and valid (u,v) values for a triangular domain satisfy
        //   umin <= u <= umax,  vmin <= v <= vmax,
        //   (vmax-vmin)*(u-umin)+(umax-umin)*(v-vmax) <= 0
        ParametricSurface(Real umin, Real umax, Real vmin, Real vmax, bool rectangular)
            :
            mUMin(umin),
            mUMax(umax),
            mVMin(vmin),
            mVMax(vmax),
            mRectangular(rectangular)
        {
        }

    public:
        virtual ~ParametricSurface()
        {
        }

        // To validate construction, create an object as shown:
        //     DerivedClassSurface<Real> surface(parameters);
        //     if (!surface) { <constructor failed, handle accordingly>; }
        inline operator bool() const
        {
            return mConstructed;
        }

        // Member access.
        inline Real GetUMin() const
        {
            return mUMin;
        }

        inline Real GetUMax() const
        {
            return mUMax;
        }

        inline Real GetVMin() const
        {
            return mVMin;
        }

        inline Real GetVMax() const
        {
            return mVMax;
        }

        inline bool IsRectangular() const
        {
            return mRectangular;
        }

        // Evaluation of the surface.  The function supports derivative
        // calculation through order 2; that is, order <= 2 is required.  If
        // you want only the position, pass in order of 0.  If you want the
        // position and first-order derivatives, pass in order of 1, and so
        // on.  The output array 'jet' must have enough storage to support the
        // maximum order.  The values are ordered as: position X; first-order
        // derivatives dX/du, dX/dv; second-order derivatives d2X/du2,
        // d2X/dudv, d2X/dv2.
        enum { SUP_ORDER = 6 };
        virtual void Evaluate(Real u, Real v, unsigned int order, Vector<N, Real>* jet) const = 0;

        // Differential geometric quantities.
        Vector<N, Real> GetPosition(Real u, Real v) const
        {
            Vector<N, Real> position;
            Evaluate(u, v, 0, &position);
            return position;
        }

        Vector<N, Real> GetUTangent(Real u, Real v) const
        {
            std::array<Vector<N, Real>, 3> jet;
            Evaluate(u, v, 1, jet.data());
            Normalize(jet[1]);
            return jet[1];
        }

        Vector<N, Real> GetVTangent(Real u, Real v) const
        {
            std::array<Vector<N, Real>, 3> jet;
            Evaluate(u, v, 1, jet.data());
            Normalize(jet[2]);
            return jet[2];
        }

    protected:
        Real mUMin, mUMax, mVMin, mVMax;
        bool mRectangular;
        bool mConstructed;
    };
}
