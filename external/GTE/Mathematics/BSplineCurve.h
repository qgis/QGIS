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
    class BSplineCurve : public ParametricCurve<N, Real>
    {
    public:
        // Construction.  If the input controls is non-null, a copy is made of
        // the controls.  To defer setting the control points, pass a null
        // pointer and later access the control points via GetControls() or
        // SetControl() member functions.  The domain is t in [t[d],t[n]],
        // where t[d] and t[n] are knots with d the degree and n the number of
        // control points.
        BSplineCurve(BasisFunctionInput<Real> const& input, Vector<N, Real> const* controls)
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
            if (controls)
            {
                std::copy(controls, controls + input.numControls, mControls.begin());
            }
            else
            {
                Vector<N, Real> zero{ (Real)0 };
                std::fill(mControls.begin(), mControls.end(), zero);
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
                return mControls[0];
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
            jet[0] = Compute(0, imin, imax);
            if (order >= 1)
            {
                // Compute first derivative.
                jet[1] = Compute(1, imin, imax);
                if (order >= 2)
                {
                    // Compute second derivative.
                    jet[2] = Compute(2, imin, imax);
                    if (order == 3)
                    {
                        jet[3] = Compute(3, imin, imax);
                    }
                }
            }
        }

    private:
        // Support for Evaluate(...).
        Vector<N, Real> Compute(unsigned int order, int imin, int imax) const
        {
            // The j-index introduces a tiny amount of overhead in order to handle
            // both aperiodic and periodic splines.  For aperiodic splines, j = i
            // always.

            int numControls = GetNumControls();
            Vector<N, Real> result;
            result.MakeZero();
            for (int i = imin; i <= imax; ++i)
            {
                Real tmp = mBasisFunction.GetValue(order, i);
                int j = (i >= numControls ? i - numControls : i);
                result += tmp * mControls[j];
            }
            return result;
        }

        BasisFunction<Real> mBasisFunction;
        std::vector<Vector<N, Real>> mControls;
    };
}
