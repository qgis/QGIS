// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/ParametricCurve.h>
#include <Mathematics/Polynomial1.h>

namespace gte
{
    template <int N, typename Real>
    class PolynomialCurve : public ParametricCurve<N, Real>
    {
    public:
        // Construction and destruction.  The default constructor creates a
        // polynomial curve with all components set to the constant zero (all
        // degree-0 polynomials).  You can set these to other polynomials
        // using member accessors.
        PolynomialCurve(Real tmin, Real tmax)
            :
            ParametricCurve<N, Real>(tmin, tmax)
        {
        }

        PolynomialCurve(Real tmin, Real tmax,
            std::array<Polynomial1<Real>, N> const& components)
            :
            ParametricCurve<N, Real>(tmin, tmax)
        {
            for (int i = 0; i < N; ++i)
            {
                SetPolynomial(i, components[i]);
            }
        }

        virtual ~PolynomialCurve()
        {
        }

        // Member access.
        void SetPolynomial(int i, Polynomial1<Real> const& poly)
        {
            mPolynomial[i] = poly;
            mDer1Polynomial[i] = mPolynomial[i].GetDerivative();
            mDer2Polynomial[i] = mDer1Polynomial[i].GetDerivative();
            mDer3Polynomial[i] = mDer2Polynomial[i].GetDerivative();
        }

        inline Polynomial1<Real> const& GetPolynomial(int i) const
        {
            return mPolynomial[i];
        }

        inline Polynomial1<Real> const& GetDer1Polynomial(int i) const
        {
            return mDer1Polynomial[i];
        }

        inline Polynomial1<Real> const& GetDer2Polynomial(int i) const
        {
            return mDer2Polynomial[i];
        }

        inline Polynomial1<Real> const& GetDer3Polynomial(int i) const
        {
            return mDer3Polynomial[i];
        }

        // Evaluation of the curve.  The function supports derivative
        // calculation through order 3; that is, order <= 3 is required.  If
        // you want/ only the position, pass in order of 0.  If you want the
        // position and first derivative, pass in order of 1, and so on.  The
        // output array 'jet' must have enough storage to support the maximum
        // order.  The values are ordered as: position, first derivative,
        // second derivative, third derivative.
        virtual void Evaluate(Real t, unsigned int order, Vector<N, Real>* jet) const override
        {
            for (int i = 0; i < N; ++i)
            {
                jet[0][i] = mPolynomial[i](t);
            }

            if (order >= 1)
            {
                for (int i = 0; i < N; ++i)
                {
                    jet[1][i] = mDer1Polynomial[i](t);
                }

                if (order >= 2)
                {
                    for (int i = 0; i < N; ++i)
                    {
                        jet[2][i] = mDer2Polynomial[i](t);
                    }

                    if (order == 3)
                    {
                        for (int i = 0; i < N; ++i)
                        {
                            jet[3][i] = mDer3Polynomial[i](t);
                        }
                    }
                }
            }
        }

    protected:
        std::array<Polynomial1<Real>, N> mPolynomial;
        std::array<Polynomial1<Real>, N> mDer1Polynomial;
        std::array<Polynomial1<Real>, N> mDer2Polynomial;
        std::array<Polynomial1<Real>, N> mDer3Polynomial;
    };
}
