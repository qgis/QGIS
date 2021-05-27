// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.12.28

#pragma once

#include <Mathematics/BasisFunction.h>
#include <Mathematics/Vector.h>

namespace gte
{
    template <int N, typename Real>
    class NURBSVolume
    {
    public:
        // Construction.  If the input controls is non-null, a copy is made of
        // the controls.  To defer setting the control points or weights, pass
        // null pointers and later access the control points or weights via
        // GetControls(), GetWeights(), SetControl(), or SetWeight() member
        // functions.  The 'controls' and 'weights' must be stored in
        // lexicographical order,
        //   attribute[i0 + numControls0 * (i1 + numControls1 * i2)]
        // As a 3D array, this corresponds to attribute3D[i2][i1][i0].
        NURBSVolume(BasisFunctionInput<Real> const& input0,
            BasisFunctionInput<Real> const& input1,
            BasisFunctionInput<Real> const& input2,
            Vector<N, Real> const* controls, Real const* weights)
            :
            mConstructed(false)
        {
            BasisFunctionInput<Real> const* input[3] = { &input0, &input1, &input2 };
            for (int i = 0; i < 3; ++i)
            {
                mNumControls[i] = input[i]->numControls;
                mBasisFunction[i].Create(*input[i]);
            }

            // The replication of control points for periodic splines is
            // avoided by wrapping the i-loop index in Evaluate.
            int numControls = mNumControls[0] * mNumControls[1] * mNumControls[2];
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
            mConstructed = true;
        }

        // To validate construction, create an object as shown:
        //     NURBSVolume<N, Real> volume(parameters);
        //     if (!volume) { <constructor failed, handle accordingly>; }
        inline operator bool() const
        {
            return mConstructed;
        }

        // Member access.  The index 'dim' must be in {0,1,2}.
        inline BasisFunction<Real> const& GetBasisFunction(int dim) const
        {
            return mBasisFunction[dim];
        }

        inline Real GetMinDomain(int dim) const
        {
            return mBasisFunction[dim].GetMinDomain();
        }

        inline Real GetMaxDomain(int dim) const
        {
            return mBasisFunction[dim].GetMaxDomain();
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

        // Evaluation of the volume.  The function supports derivative
        // calculation through order 2; that is, order <= 2 is required.  If
        // you want only the position, pass in order of 0.  If you want the
        // position and first-order derivatives, pass in order of 1, and so
        // on.  The output array 'jet' muist have enough storage to support
        // the maximum order.  The values are ordered as: position X;
        // first-order derivatives dX/du, dX/dv, dX/dw; second-order
        // derivatives d2X/du2, d2X/dv2, d2X/dw2, d2X/dudv, d2X/dudw,
        // d2X/dvdw.
        enum { SUP_ORDER = 10 };
        void Evaluate(Real u, Real v, Real w, unsigned int order, Vector<N, Real>* jet) const
        {
            if (!mConstructed || order >= SUP_ORDER)
            {
                // Errors were already generated during construction.
                for (unsigned int i = 0; i < SUP_ORDER; ++i)
                {
                    jet[i].MakeZero();
                }
                return;
            }

            int iumin, iumax, ivmin, ivmax, iwmin, iwmax;
            mBasisFunction[0].Evaluate(u, order, iumin, iumax);
            mBasisFunction[1].Evaluate(v, order, ivmin, ivmax);
            mBasisFunction[2].Evaluate(w, order, iwmin, iwmax);

            // Compute position.
            Vector<N, Real> X;
            Real h;
            Compute(0, 0, 0, iumin, iumax, ivmin, ivmax, iwmin, iwmax, X, h);
            Real invH = (Real)1 / h;
            jet[0] = invH * X;

            if (order >= 1)
            {
                // Compute first-order derivatives.
                Vector<N, Real> XDerU;
                Real hDerU;
                Compute(1, 0, 0, iumin, iumax, ivmin, ivmax, iwmin, iwmax, XDerU, hDerU);
                jet[1] = invH * (XDerU - hDerU * jet[0]);

                Vector<N, Real> XDerV;
                Real hDerV;
                Compute(0, 1, 0, iumin, iumax, ivmin, ivmax, iwmin, iwmax, XDerV, hDerV);
                jet[2] = invH * (XDerV - hDerV * jet[0]);

                Vector<N, Real> XDerW;
                Real hDerW;
                Compute(0, 0, 1, iumin, iumax, ivmin, ivmax, iwmin, iwmax, XDerW, hDerW);
                jet[3] = invH * (XDerW - hDerW * jet[0]);

                if (order >= 2)
                {
                    // Compute second-order derivatives.
                    Vector<N, Real> XDerUU;
                    Real hDerUU;
                    Compute(2, 0, 0, iumin, iumax, ivmin, ivmax, iwmin, iwmax, XDerUU, hDerUU);
                    jet[4] = invH * (XDerUU - (Real)2 * hDerU * jet[1] - hDerUU * jet[0]);

                    Vector<N, Real> XDerVV;
                    Real hDerVV;
                    Compute(0, 2, 0, iumin, iumax, ivmin, ivmax, iwmin, iwmax, XDerVV, hDerVV);
                    jet[5] = invH * (XDerVV - (Real)2 * hDerV * jet[2] - hDerVV * jet[0]);

                    Vector<N, Real> XDerWW;
                    Real hDerWW;
                    Compute(0, 0, 2, iumin, iumax, ivmin, ivmax, iwmin, iwmax, XDerWW, hDerWW);
                    jet[6] = invH * (XDerWW - (Real)2 * hDerW * jet[3] - hDerWW * jet[0]);

                    Vector<N, Real> XDerUV;
                    Real hDerUV;
                    Compute(1, 1, 0, iumin, iumax, ivmin, ivmax, iwmin, iwmax, XDerUV, hDerUV);
                    jet[7] = invH * (XDerUV - hDerU * jet[2] - hDerV * jet[1] - hDerUV * jet[0]);

                    Vector<N, Real> XDerUW;
                    Real hDerUW;
                    Compute(1, 0, 1, iumin, iumax, ivmin, ivmax, iwmin, iwmax, XDerUW, hDerUW);
                    jet[8] = invH * (XDerUW - hDerU * jet[3] - hDerW * jet[1] - hDerUW * jet[0]);

                    Vector<N, Real> XDerVW;
                    Real hDerVW;
                    Compute(0, 1, 1, iumin, iumax, ivmin, ivmax, iwmin, iwmax, XDerVW, hDerVW);
                    jet[9] = invH * (XDerVW - hDerV * jet[3] - hDerW * jet[2] - hDerVW * jet[0]);
                }
            }
        }

    private:
        // Support for Evaluate(...).
        void Compute(unsigned int uOrder, unsigned int vOrder,
            unsigned int wOrder, int iumin, int iumax, int ivmin, int ivmax,
            int iwmin, int iwmax, Vector<N, Real>& X, Real& h) const
        {
            // The j*-indices introduce a tiny amount of overhead in order to
            // handle both aperiodic and periodic splines.  For aperiodic
            // splines, j* = i* always.

            int const numControls0 = mNumControls[0];
            int const numControls1 = mNumControls[1];
            int const numControls2 = mNumControls[2];
            X.MakeZero();
            h = (Real)0;
            for (int iw = iwmin; iw <= iwmax; ++iw)
            {
                Real tmpw = mBasisFunction[2].GetValue(wOrder, iw);
                int jw = (iw >= numControls2 ? iw - numControls2 : iw);
                for (int iv = ivmin; iv <= ivmax; ++iv)
                {
                    Real tmpv = mBasisFunction[1].GetValue(vOrder, iv);
                    Real tmpvw = tmpv * tmpw;
                    int jv = (iv >= numControls1 ? iv - numControls1 : iv);
                    for (int iu = iumin; iu <= iumax; ++iu)
                    {
                        Real tmpu = mBasisFunction[0].GetValue(uOrder, iu);
                        int ju = (iu >= numControls0 ? iu - numControls0 : iu);
                        int index = ju + numControls0 * (jv + numControls1 * jw);
                        Real tmp = (tmpu * tmpvw) * mWeights[index];
                        X += tmp * mControls[index];
                        h += tmp;
                    }
                }
            }
        }

        std::array<BasisFunction<Real>, 3> mBasisFunction;
        std::array<int, 3> mNumControls;
        std::vector<Vector<N, Real>> mControls;
        std::vector<Real> mWeights;
        bool mConstructed;
    };
}
