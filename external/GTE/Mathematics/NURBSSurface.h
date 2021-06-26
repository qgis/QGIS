// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.12.28

#pragma once

#include <Mathematics/BasisFunction.h>
#include <Mathematics/ParametricSurface.h>
#include <Mathematics/Vector.h>

namespace gte
{
    template <int N, typename Real>
    class NURBSSurface : public ParametricSurface<N, Real>
    {
    public:
        // Construction.  If the input controls is non-null, a copy is made of
        // the controls.  To defer setting the control points or weights, pass
        // null pointers and later access the control points or weights via
        // GetControls(), GetWeights(), SetControl(), or SetWeight() member
        // functions.  The 'controls' and 'weights' must be stored in
        // row-major order, attribute[i0 + numControls0*i1].  As a 2D array,
        // this corresponds to attribute2D[i1][i0].
        NURBSSurface(BasisFunctionInput<Real> const& input0,
            BasisFunctionInput<Real> const& input1,
            Vector<N, Real> const* controls, Real const* weights)
            :
            ParametricSurface<N, Real>((Real)0, (Real)1, (Real)0, (Real)1, true)
        {
            BasisFunctionInput<Real> const* input[2] = { &input0, &input1 };
            for (int i = 0; i < 2; ++i)
            {
                mNumControls[i] = input[i]->numControls;
                mBasisFunction[i].Create(*input[i]);
            }

            // The mBasisFunction stores the domain but so does
            // ParametricSurface.
            this->mUMin = mBasisFunction[0].GetMinDomain();
            this->mUMax = mBasisFunction[0].GetMaxDomain();
            this->mVMin = mBasisFunction[1].GetMinDomain();
            this->mVMax = mBasisFunction[1].GetMaxDomain();

            // The replication of control points for periodic splines is
            // avoided by wrapping the i-loop index in Evaluate.
            int numControls = mNumControls[0] * mNumControls[1];
            mControls.resize(numControls);
            mWeights.resize(numControls);
            if (controls)
            {
                std::copy(controls, controls + numControls, mControls.begin());
            }
            else
            {
                Vector<N, Real> zero{ (Real)0 };
                std::fill(mControls.begin(), mControls.end(), zero);
            }
            if (weights)
            {
                std::copy(weights, weights + numControls, mWeights.begin());
            }
            else
            {
                std::fill(mWeights.begin(), mWeights.end(), (Real)0);
            }
            this->mConstructed = true;
        }

        // Member access.  The index 'dim' must be in {0,1}.
        inline BasisFunction<Real> const& GetBasisFunction(int dim) const
        {
            return mBasisFunction[dim];
        }

        inline int GetNumControls(int dim) const
        {
            return mNumControls[dim];
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

        void SetControl(int i0, int i1, Vector<N, Real> const& control)
        {
            if (0 <= i0 && i0 < GetNumControls(0) && 0 <= i1 && i1 < GetNumControls(1))
            {
                mControls[i0 + mNumControls[0] * i1] = control;
            }
        }

        Vector<N, Real> const& GetControl(int i0, int i1) const
        {
            if (0 <= i0 && i0 < GetNumControls(0) && 0 <= i1 && i1 < GetNumControls(1))
            {
                return mControls[i0 + mNumControls[0] * i1];
            }
            else
            {
                return mControls[0];
            }
        }

        void SetWeight(int i0, int i1, Real weight)
        {
            if (0 <= i0 && i0 < GetNumControls(0) && 0 <= i1 && i1 < GetNumControls(1))
            {
                mWeights[i0 + mNumControls[0] * i1] = weight;
            }
        }

        Real const& GetWeight(int i0, int i1) const
        {
            if (0 <= i0 && i0 < GetNumControls(0) && 0 <= i1 && i1 < GetNumControls(1))
            {
                return mWeights[i0 + mNumControls[0] * i1];
            }
            else
            {
                return mWeights[0];
            }
        }

        // Evaluation of the surface.  The function supports derivative
        // calculation through order 2; that is, order <= 2 is required.  If
        // you want only the position, pass in order of 0.  If you want the
        // position and first-order derivatives, pass in order of 1, and so
        // on.  The output array 'jet' must have enough storage to support the
        // maximum order.  The values are ordered as: position X; first-order
        // derivatives dX/du, dX/dv; second-order derivatives d2X/du2,
        // d2X/dudv, d2X/dv2.
        virtual void Evaluate(Real u, Real v, unsigned int order, Vector<N, Real>* jet) const override
        {
            unsigned int const supOrder = ParametricSurface<N, Real>::SUP_ORDER;
            if (!this->mConstructed || order >= supOrder)
            {
                // Return a zero-valued jet for invalid state.
                for (unsigned int i = 0; i < supOrder; ++i)
                {
                    jet[i].MakeZero();
                }
                return;
            }

            int iumin, iumax, ivmin, ivmax;
            mBasisFunction[0].Evaluate(u, order, iumin, iumax);
            mBasisFunction[1].Evaluate(v, order, ivmin, ivmax);

            // Compute position.
            Vector<N, Real> X;
            Real w;
            Compute(0, 0, iumin, iumax, ivmin, ivmax, X, w);
            Real invW = (Real)1 / w;
            jet[0] = invW * X;

            if (order >= 1)
            {
                // Compute first-order derivatives.
                Vector<N, Real> XDerU;
                Real wDerU;
                Compute(1, 0, iumin, iumax, ivmin, ivmax, XDerU, wDerU);
                jet[1] = invW * (XDerU - wDerU * jet[0]);

                Vector<N, Real> XDerV;
                Real wDerV;
                Compute(0, 1, iumin, iumax, ivmin, ivmax, XDerV, wDerV);
                jet[2] = invW * (XDerV - wDerV * jet[0]);

                if (order >= 2)
                {
                    // Compute second-order derivatives.
                    Vector<N, Real> XDerUU;
                    Real wDerUU;
                    Compute(2, 0, iumin, iumax, ivmin, ivmax, XDerUU, wDerUU);
                    jet[3] = invW * (XDerUU - (Real)2 * wDerU * jet[1] - wDerUU * jet[0]);

                    Vector<N, Real> XDerUV;
                    Real wDerUV;
                    Compute(1, 1, iumin, iumax, ivmin, ivmax, XDerUV, wDerUV);
                    jet[4] = invW * (XDerUV - wDerU * jet[2] - wDerV * jet[1]
                        - wDerUV * jet[0]);

                    Vector<N, Real> XDerVV;
                    Real wDerVV;
                    Compute(0, 2, iumin, iumax, ivmin, ivmax, XDerVV, wDerVV);
                    jet[5] = invW * (XDerVV - (Real)2 * wDerV * jet[2] - wDerVV * jet[0]);
                }
            }
        }

    protected:
        // Support for Evaluate(...).
        void Compute(unsigned int uOrder, unsigned int vOrder, int iumin,
            int iumax, int ivmin, int ivmax, Vector<N, Real>& X, Real& w) const
        {
            // The j*-indices introduce a tiny amount of overhead in order to handle
            // both aperiodic and periodic splines.  For aperiodic splines, j* = i*
            // always.

            int const numControls0 = mNumControls[0];
            int const numControls1 = mNumControls[1];
            X.MakeZero();
            w = (Real)0;
            for (int iv = ivmin; iv <= ivmax; ++iv)
            {
                Real tmpv = mBasisFunction[1].GetValue(vOrder, iv);
                int jv = (iv >= numControls1 ? iv - numControls1 : iv);
                for (int iu = iumin; iu <= iumax; ++iu)
                {
                    Real tmpu = mBasisFunction[0].GetValue(uOrder, iu);
                    int ju = (iu >= numControls0 ? iu - numControls0 : iu);
                    int index = ju + numControls0 * jv;
                    Real tmp = tmpu * tmpv * mWeights[index];
                    X += tmp * mControls[index];
                    w += tmp;
                }
            }
        }

        std::array<BasisFunction<Real>, 2> mBasisFunction;
        std::array<int, 2> mNumControls;
        std::vector<Vector<N, Real>> mControls;
        std::vector<Real> mWeights;
    };
}
