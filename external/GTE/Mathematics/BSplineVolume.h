// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/BasisFunction.h>
#include <Mathematics/Vector.h>

namespace gte
{
    template <int N, typename Real>
    class BSplineVolume
    {
    public:
        // Construction.  If the input controls is non-null, a copy is made of
        // the controls.  To defer setting the control points, pass a null
        // pointer and later access the control points via GetControls() or
        // SetControl() member functions.  The input 'controls' must be stored
        // in lexicographical order,
        // control[i0+numControls0*(i1+numControls1*i2)].  As a 3D array, this
        // corresponds to control3D[i2][i1][i0].
        BSplineVolume(BasisFunctionInput<Real> const input[3], Vector<N, Real> const* controls)
            :
            mConstructed(false)
        {
            for (int i = 0; i < 3; ++i)
            {
                mNumControls[i] = input[i].numControls;
                mBasisFunction[i].Create(input[i]);
            }

            // The replication of control points for periodic splines is
            // avoided by wrapping the i-loop index in Evaluate.
            int numControls = mNumControls[0] * mNumControls[1] * mNumControls[2];
            mControls.resize(numControls);
            if (controls)
            {
                std::copy(controls, controls + numControls, mControls.begin());
            }
            else
            {
                Vector<N, Real> zero{ (Real)0 };
                std::fill(mControls.begin(), mControls.end(), zero);
            }
            mConstructed = true;
        }

        // To validate construction, create an object as shown:
        //     BSplineVolume<N, Real> volume(parameters);
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

        void SetControl(int i0, int i1, int i2, Vector<N, Real> const& control)
        {
            if (0 <= i0 && i0 < GetNumControls(0)
                && 0 <= i1 && i1 < GetNumControls(1)
                && 0 <= i2 && i2 < GetNumControls(2))
            {
                mControls[i0 + mNumControls[0] * (i1 + mNumControls[1] * i2)] = control;
            }
        }

        Vector<N, Real> const& GetControl(int i0, int i1, int i2) const
        {
            if (0 <= i0 && i0 < GetNumControls(0)
                && 0 <= i1 && i1 < GetNumControls(1)
                && 0 <= i2 && i2 < GetNumControls(2))
            {
                return mControls[i0 + mNumControls[0] * (i1 + mNumControls[1] * i2)];
            }
            else
            {
                return mControls[0];
            }
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
                // Return a zero-valued jet for invalid state.
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
            jet[0] = Compute(0, 0, 0, iumin, iumax, ivmin, ivmax, iwmin, iwmax);
            if (order >= 1)
            {
                // Compute first-order derivatives.
                jet[1] = Compute(1, 0, 0, iumin, iumax, ivmin, ivmax, iwmin, iwmax);
                jet[2] = Compute(0, 1, 0, iumin, iumax, ivmin, ivmax, iwmin, iwmax);
                jet[3] = Compute(0, 0, 1, iumin, iumax, ivmin, ivmax, iwmin, iwmax);
                if (order >= 2)
                {
                    // Compute second-order derivatives.
                    jet[4] = Compute(2, 0, 0, iumin, iumax, ivmin, ivmax, iwmin, iwmax);
                    jet[5] = Compute(0, 2, 0, iumin, iumax, ivmin, ivmax, iwmin, iwmax);
                    jet[6] = Compute(0, 0, 2, iumin, iumax, ivmin, ivmax, iwmin, iwmax);
                    jet[7] = Compute(1, 1, 0, iumin, iumax, ivmin, ivmax, iwmin, iwmax);
                    jet[8] = Compute(1, 0, 1, iumin, iumax, ivmin, ivmax, iwmin, iwmax);
                    jet[9] = Compute(0, 1, 1, iumin, iumax, ivmin, ivmax, iwmin, iwmax);
                }
            }
        }

    private:
        // Support for Evaluate(...).
        Vector<N, Real> Compute(unsigned int uOrder, unsigned int vOrder,
            unsigned int wOrder, int iumin, int iumax, int ivmin, int ivmax,
            int iwmin, int iwmax) const
        {
            // The j*-indices introduce a tiny amount of overhead in order to
            // handle both aperiodic and periodic splines.  For aperiodic
            // splines, j* = i* always.

            int const numControls0 = mNumControls[0];
            int const numControls1 = mNumControls[1];
            int const numControls2 = mNumControls[2];
            Vector<N, Real> result;
            result.MakeZero();
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
                        result += (tmpu * tmpvw) *
                            mControls[ju + numControls0 * (jv + numControls1 * jw)];
                    }
                }
            }
            return result;
        }

        std::array<BasisFunction<Real>, 3> mBasisFunction;
        std::array<int, 3> mNumControls;
        std::vector<Vector<N, Real>> mControls;
        bool mConstructed;
    };
}
