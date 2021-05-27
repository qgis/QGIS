// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/BasisFunction.h>
#include <Mathematics/ParametricCurve.h>

namespace gte
{
    template <int N, typename Real>
    class NURBSCurve : public ParametricCurve<N, Real>
    {
    public:
        // Construction.  If the input controls is non-null, a copy is made of
        // the controls.  To defer setting the control points or weights, pass
        // null pointers and later access the control points or weights via
        // GetControls(), GetWeights(), SetControl(), or SetWeight() member
        // functions.  The domain is t in [t[d],t[n]], where t[d] and t[n] are
        // knots with d the degree and n the number of control points.
        NURBSCurve(BasisFunctionInput<Real> const& input,
            Vector<N, Real> const* controls, Real const* weights)
            :
            ParametricCurve<N, Real>((Real)0, (Real)1),
            mBasisFunction(input)
        {
            // The mBasisFunction stores the domain but so does
            // ParametricCurve.
            this->mTime.front() = mBasisFunction.GetMinDomain();
            this->mTime.back() = mBasisFunction.GetMaxDomain();

            // The replication of control points for periodic splines is
            // avoided by wrapping the i-loop index in Evaluate.
            mControls.resize(input.numControls);
            mWeights.resize(input.numControls);
            if (controls)
            {
                std::copy(controls, controls + input.numControls, mControls.begin());
            }
            else
            {
                Vector<N, Real> zero{ (Real)0 };
                std::fill(mControls.begin(), mControls.end(), zero);
            }
            if (weights)
            {
                std::copy(weights, weights + input.numControls, mWeights.begin());
            }
            else
            {
                std::fill(mWeights.begin(), mWeights.end(), (Real)0);
            }
            this->mConstructed = true;
        }

        // Member access.
        inline BasisFunction<Real> const& GetBasisFunction() const
        {
            return mBasisFunction;
        }

        inline int GetNumControls() const
        {
            return static_cast<int>(mControls.size());
        }

        inline Vector<N, Real> const* GetControls() const
        {
            return mControls.data();
        }

        inline Vector<N, Real>* GetControls()
        {
            return mControls.data();
        }

        inline Real const* GetWeights() const
        {
            return mWeights.data();
        }

        inline Real* GetWeights()
        {
            return mWeights.data();
        }

        void SetControl(int i, Vector<N, Real> const& control)
        {
            if (0 <= i && i < GetNumControls())
            {
                mControls[i] = control;
            }
        }

        Vector<N, Real> const& GetControl(int i) const
        {
            if (0 <= i && i < GetNumControls())
            {
                return mControls[i];
            }
            else
            {
                // Invalid index, return something.
                return mControls[0];
            }
        }

        void SetWeight(int i, Real weight)
        {
            if (0 <= i && i < GetNumControls())
            {
                mWeights[i] = weight;
            }
        }

        Real const& GetWeight(int i) const
        {
            if (0 <= i && i < GetNumControls())
            {
                return mWeights[i];
            }
            else
            {
                // Invalid index, return something.
                return mWeights[0];
            }
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
            unsigned int const supOrder = ParametricCurve<N, Real>::SUP_ORDER;
            if (!this->mConstructed || order >= supOrder)
            {
                // Return a zero-valued jet for invalid state.
                for (unsigned int i = 0; i < supOrder; ++i)
                {
                    jet[i].MakeZero();
                }
                return;
            }

            int imin, imax;
            mBasisFunction.Evaluate(t, order, imin, imax);

            // Compute position.
            Vector<N, Real> X;
            Real w;
            Compute(0, imin, imax, X, w);
            Real invW = (Real)1 / w;
            jet[0] = invW * X;

            if (order >= 1)
            {
                // Compute first derivative.
                Vector<N, Real> XDer1;
                Real wDer1;
                Compute(1, imin, imax, XDer1, wDer1);
                jet[1] = invW * (XDer1 - wDer1 * jet[0]);

                if (order >= 2)
                {
                    // Compute second derivative.
                    Vector<N, Real> XDer2;
                    Real wDer2;
                    Compute(2, imin, imax, XDer2, wDer2);
                    jet[2] = invW * (XDer2 - (Real)2 * wDer1 * jet[1] - wDer2 * jet[0]);

                    if (order == 3)
                    {
                        // Compute third derivative.
                        Vector<N, Real> XDer3;
                        Real wDer3;
                        Compute(3, imin, imax, XDer3, wDer3);
                        jet[3] = invW * (XDer3 - (Real)3 * wDer1 * jet[2] -
                            (Real)3 * wDer2 * jet[1] - wDer3 * jet[0]);
                    }
                }
            }
        }

    protected:
        // Support for Evaluate(...).
        void Compute(unsigned int order, int imin, int imax, Vector<N, Real>& X, Real& w) const
        {
            // The j-index introduces a tiny amount of overhead in order to
            // handle both aperiodic and periodic splines.  For aperiodic
            // splines, j = i always.

            int numControls = GetNumControls();
            X.MakeZero();
            w = (Real)0;
            for (int i = imin; i <= imax; ++i)
            {
                int j = (i >= numControls ? i - numControls : i);
                Real tmp = mBasisFunction.GetValue(order, i) * mWeights[j];
                X += tmp * mControls[j];
                w += tmp;
            }
        }

        BasisFunction<Real> mBasisFunction;
        std::vector<Vector<N, Real>> mControls;
        std::vector<Real> mWeights;
    };
}
