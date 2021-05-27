// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/NURBSSurface.h>
#include <Mathematics/Vector3.h>
#include <functional>

// The algorithm for representing a circle as a NURBS curve or a sphere as a
// NURBS surface is described in
//   https://www.geometrictools.com/Documentation/NURBSCircleSphere.pdf
// The implementations are related to the documents as shown next.
//   NURBSEighthSphereDegree4 implements Section 3.1.2 (triangular domain)
//   NURBSHalfSphereDegree3 implements Section 3.2 (rectangular domain)
//   NURBSFullSphereDegree3 implements Section 2.3 (rectangular domain)
// TODO: The class NURBSSurface currently assumes a rectangular domain.
// Once support is added for triangular domains, make that new class a
// base class of the sphere-representing NURBS.  This will allow sharing
// of the NURBS basis functions and evaluation framework.

namespace gte
{
    template <typename Real>
    class NURBSEighthSphereDegree4
    {
    public:
        // Construction.  The eigth sphere is x^2 + y^2 + z^2 = 1 for x >= 0,
        // y >= 0 and z >= 0.
        NURBSEighthSphereDegree4()
        {
            Real const sqrt2 = std::sqrt((Real)2);
            Real const sqrt3 = std::sqrt((Real)3);
            Real const a0 = (sqrt3 - (Real)1) / sqrt3;
            Real const a1 = (sqrt3 + (Real)1) / ((Real)2 * sqrt3);
            Real const a2 = (Real)1 - ((Real)5 - sqrt2) * ((Real)7 - sqrt3) / (Real)46;
            Real const b0 = (Real)4 * sqrt3 * (sqrt3 - (Real)1);
            Real const b1 = (Real)3 * sqrt2;
            Real const b2 = (Real)4;
            Real const b3 = sqrt2 * ((Real)3 + (Real)2 * sqrt2 - sqrt3) / sqrt3;

            mControls[0][0] = { (Real)0, (Real)0, (Real)1 };   // P004
            mControls[0][1] = { (Real)0, a0,   (Real)1 };   // P013
            mControls[0][2] = { (Real)0, a1,   a1 };     // P022
            mControls[0][3] = { (Real)0, (Real)1, a0 };     // P031
            mControls[0][4] = { (Real)0, (Real)1, (Real)0 };   // P040

            mControls[1][0] = { a0,   (Real)0, (Real)1 };   // P103
            mControls[1][1] = { a2,   a2,   (Real)1 };   // P112
            mControls[1][2] = { a2,   (Real)1, a2 };     // P121
            mControls[1][3] = { a0,   (Real)1, (Real)0 };   // P130
            mControls[1][4] = { (Real)0, (Real)0, (Real)0 };   // unused

            mControls[2][0] = { a1,   (Real)0, a1 };     // P202
            mControls[2][1] = { (Real)1, a2,   a2 };     // P211
            mControls[2][2] = { a1,   a1,   (Real)0 };   // P220
            mControls[2][3] = { (Real)0, (Real)0, (Real)0 };   // unused
            mControls[2][4] = { (Real)0, (Real)0, (Real)0 };   // unused

            mControls[3][0] = { (Real)1, (Real)0, a0 };     // P301
            mControls[3][1] = { (Real)1, a0,   (Real)0 };   // P310
            mControls[3][2] = { (Real)0, (Real)0, (Real)0 };   // unused
            mControls[3][3] = { (Real)0, (Real)0, (Real)0 };   // unused
            mControls[3][4] = { (Real)0, (Real)0, (Real)0 };   // unused

            mControls[4][0] = { (Real)1, (Real)0, (Real)0 };   // P400
            mControls[4][1] = { (Real)0, (Real)0, (Real)0 };   // unused
            mControls[4][2] = { (Real)0, (Real)0, (Real)0 };   // unused
            mControls[4][3] = { (Real)0, (Real)0, (Real)0 };   // unused
            mControls[4][4] = { (Real)0, (Real)0, (Real)0 };   // unused

            mWeights[0][0] = b0;    // w004
            mWeights[0][1] = b1;    // w013
            mWeights[0][2] = b2;    // w022
            mWeights[0][3] = b1;    // w031
            mWeights[0][4] = b0;    // w040

            mWeights[1][0] = b1;    // w103
            mWeights[1][1] = b3;    // w112
            mWeights[1][2] = b3;    // w121
            mWeights[1][3] = b1;    // w130
            mWeights[1][4] = (Real)0;  // unused

            mWeights[2][0] = b2;    // w202
            mWeights[2][1] = b3;    // w211
            mWeights[2][2] = b2;    // w220
            mWeights[2][3] = (Real)0;  // unused
            mWeights[2][4] = (Real)0;  // unused

            mWeights[3][0] = b1;    // w301
            mWeights[3][1] = b1;    // w310
            mWeights[3][2] = (Real)0;  // unused
            mWeights[3][3] = (Real)0;  // unused
            mWeights[3][4] = (Real)0;  // unused

            mWeights[4][0] = b0;    // w400
            mWeights[4][1] = (Real)0;  // unused
            mWeights[4][2] = (Real)0;  // unused
            mWeights[4][3] = (Real)0;  // unused
            mWeights[4][4] = (Real)0;  // unused
        }

        // Evaluation of the surface.  The function supports derivative
        // calculation through order 2; that is, maxOrder <= 2 is required.
        // If you want only the position, pass in maxOrder of 0.  If you want
        // the position and first-order derivatives, pass in maxOrder of 1,
        // and so on.  The output 'values' are ordered as: position X;
        // first-order derivatives dX/du, dX/dv; second-order derivatives
        // d2X/du2, d2X/dudv, d2X/dv2.
        void Evaluate(Real u, Real v, unsigned int maxOrder, Vector<3, Real> values[6]) const
        {
            // TODO: Some of the polynomials are used in other polynomials.
            // Optimize the code by eliminating the redundant computations.

            Real w = (Real)1 - u - v;
            Real uu = u * u, uv = u * v, uw = u * w, vv = v * v, vw = v * w, ww = w * w;

            // Compute the order-0 polynomials.  Only the elements to be used
            // are filled in.  The other terms are uninitialized but never
            // used.
            Real B[5][5];
            B[0][0] = ww * ww;
            B[0][1] = (Real)4 * vw * ww;
            B[0][2] = (Real)6 * vv * ww;
            B[0][3] = (Real)4 * vv * vw;
            B[0][4] = vv * vv;
            B[1][0] = (Real)4 * uw * ww;
            B[1][1] = (Real)12 * uv * ww;
            B[1][2] = (Real)12 * uv * vw;
            B[1][3] = (Real)4 * uv * vv;
            B[2][0] = (Real)6 * uu * ww;
            B[2][1] = (Real)12 * uu * vw;
            B[2][2] = (Real)6 * uu * vv;
            B[3][0] = (Real)4 * uu * uw;
            B[3][1] = (Real)4 * uu * uv;
            B[4][0] = uu * uu;

            // Compute the NURBS position.
            Vector<3, Real> N{ (Real)0, (Real)0, (Real)0 };
            Real D(0);
            for (int j1 = 0; j1 <= 4; ++j1)
            {
                for (int j0 = 0; j0 <= 4 - j1; ++j0)
                {
                    Real product = mWeights[j1][j0] * B[j1][j0];
                    N += product * mControls[j1][j0];
                    D += product;
                }
            }
            values[0] = N / D;

            if (maxOrder >= 1)
            {
                // Compute the order-1 polynomials.  Only the elements to be
                // used are filled in.  The other terms are uninitialized but
                // never used.
                Real WmU = w - u;
                Real WmTwoU = WmU - u;
                Real WmThreeU = WmTwoU - u;
                Real TwoWmU = w + WmU;
                Real ThreeWmU = w + TwoWmU;
                Real WmV = w - v;
                Real WmTwoV = WmV - v;
                Real WmThreeV = WmTwoV - v;
                Real TwoWmV = w + WmV;
                Real ThreeWmV = w + TwoWmV;
                Real Dsqr = D * D;

                Real Bu[5][5];
                Bu[0][0] = (Real)-4 * ww * w;
                Bu[0][1] = (Real)-12 * v * ww;
                Bu[0][2] = (Real)-12 * vv * w;
                Bu[0][3] = (Real)-4 * v * vv;
                Bu[0][4] = (Real)0;
                Bu[1][0] = (Real)4 * ww * WmThreeU;
                Bu[1][1] = (Real)12 * vw * WmTwoU;
                Bu[1][2] = (Real)12 * vv * WmU;
                Bu[1][3] = (Real)4 * vv;
                Bu[2][0] = (Real)12 * uw * WmU;
                Bu[2][1] = (Real)12 * uv * TwoWmU;
                Bu[2][2] = (Real)12 * u * vv;
                Bu[3][0] = (Real)4 * uu * ThreeWmU;
                Bu[3][1] = (Real)12 * uu * v;
                Bu[4][0] = (Real)4 * uu * u;

                Real Bv[5][5];
                Bv[0][0] = (Real)-4 * ww * w;
                Bv[0][1] = (Real)4 * ww * WmThreeV;
                Bv[0][2] = (Real)12 * vw * WmV;
                Bv[0][3] = (Real)4 * vv * ThreeWmV;
                Bv[0][4] = (Real)4 * vv * v;
                Bv[1][0] = (Real)-12 * u * ww;
                Bv[1][1] = (Real)12 * uw * WmTwoV;
                Bv[1][2] = (Real)12 * uv * TwoWmV;
                Bv[1][3] = (Real)12 * u * vv;
                Bv[2][0] = (Real)-12 * uu * w;
                Bv[2][1] = (Real)12 * uu * WmV;
                Bv[2][2] = (Real)12 * uu * v;
                Bv[3][0] = (Real)-4 * uu * u;
                Bv[3][1] = (Real)4 * uu * u;
                Bv[4][0] = (Real)0;

                Vector<3, Real> Nu{ (Real)0, (Real)0, (Real)0 };
                Vector<3, Real> Nv{ (Real)0, (Real)0, (Real)0 };
                Real Du(0), Dv(0);
                for (int j1 = 0; j1 <= 4; ++j1)
                {
                    for (int j0 = 0; j0 <= 4 - j1; ++j0)
                    {
                        Real product = mWeights[j1][j0] * Bu[j1][j0];
                        Nu += product * mControls[j1][j0];
                        Du += product;
                        product = mWeights[j1][j0] * Bv[j1][j0];
                        Nv += product * mControls[j1][j0];
                        Dv += product;
                    }
                }
                Vector<3, Real> numerDU = D * Nu - Du * N;
                Vector<3, Real> numerDV = D * Nv - Dv * N;
                values[1] = numerDU / Dsqr;
                values[2] = numerDV / Dsqr;

                if (maxOrder >= 2)
                {
                    // Compute the order-2 polynomials.  Only the elements to
                    // be used are filled in.  The other terms are
                    // uninitialized but never used.
                    Real Dcub = Dsqr * D;

                    Real Buu[5][5];
                    Buu[0][0] = (Real)12 * ww;
                    Buu[0][1] = (Real)24 * vw;
                    Buu[0][2] = (Real)12 * vv;
                    Buu[0][3] = (Real)0;
                    Buu[0][4] = (Real)0;
                    Buu[1][0] = (Real)-24 * w * WmU;
                    Buu[1][1] = (Real)-24 * v * TwoWmU;
                    Buu[1][2] = (Real)-24 * vv;
                    Buu[1][3] = (Real)0;
                    Buu[2][0] = (Real)12 * (ww - (Real)4 * uw + uu);
                    Buu[2][1] = (Real)24 * v * WmTwoU;
                    Buu[2][2] = (Real)12 * vv;
                    Buu[3][0] = (Real)24 * u * WmU;
                    Buu[3][1] = (Real)24 * uv;
                    Buu[4][0] = (Real)12 * uu;

                    Real Buv[5][5];
                    Buv[0][0] = (Real)12 * ww;
                    Buv[0][1] = (Real)-12 * w * WmTwoV;
                    Buv[0][2] = (Real)-12 * v * TwoWmV;
                    Buv[0][3] = (Real)-12 * vv;
                    Buv[0][4] = (Real)0;
                    Buv[1][0] = (Real)-12 * w * WmTwoU;
                    Buv[1][1] = (Real)12 * (ww + (Real)2 * (uv - uw - vw));
                    Buv[1][2] = (Real)12 * v * ((Real)2 * WmU - v);
                    Buv[1][3] = (Real)12 * vv;
                    Buv[2][0] = (Real)-12 * u * TwoWmU;
                    Buv[2][1] = (Real)12 * u * ((Real)2 * WmV - u);
                    Buv[2][2] = (Real)24 * uv;
                    Buv[3][0] = (Real)-12 * uu;
                    Buv[3][1] = (Real)12 * uu;
                    Buv[4][0] = (Real)0;

                    Real Bvv[5][5];
                    Bvv[0][0] = (Real)12 * ww;
                    Bvv[0][1] = (Real)-24 * w * WmV;
                    Bvv[0][2] = (Real)12 * (ww - (Real)4 * vw + vv);
                    Bvv[0][3] = (Real)24 * v * WmV;
                    Bvv[0][4] = (Real)12 * vv;
                    Bvv[1][0] = (Real)24 * uw;
                    Bvv[1][1] = (Real)-24 * u * TwoWmV;
                    Bvv[1][2] = (Real)24 * u * WmTwoV;
                    Bvv[1][3] = (Real)24 * uv;
                    Bvv[2][0] = (Real)12 * uu;
                    Bvv[2][1] = (Real)-24 * uu;
                    Bvv[2][2] = (Real)12 * uu;
                    Bvv[3][0] = (Real)0;
                    Bvv[3][1] = (Real)0;
                    Bvv[4][0] = (Real)0;

                    Vector<3, Real> Nuu{ (Real)0, (Real)0, (Real)0 };
                    Vector<3, Real> Nuv{ (Real)0, (Real)0, (Real)0 };
                    Vector<3, Real> Nvv{ (Real)0, (Real)0, (Real)0 };
                    Real Duu(0), Duv(0), Dvv(0);
                    for (int j1 = 0; j1 <= 4; ++j1)
                    {
                        for (int j0 = 0; j0 <= 4 - j1; ++j0)
                        {
                            Real product = mWeights[j1][j0] * Buu[j1][j0];
                            Nuu += product * mControls[j1][j0];
                            Duu += product;
                            product = mWeights[j1][j0] * Buv[j1][j0];
                            Nuv += product * mControls[j1][j0];
                            Duv += product;
                            product = mWeights[j1][j0] * Bvv[j1][j0];
                            Nvv += product * mControls[j1][j0];
                            Dvv += product;
                        }
                    }
                    Vector<3, Real> termDuu = D * (D * Nuu - Duu * N);
                    Vector<3, Real> termDuv = D * (D * Nuv - Duv * N - Du * Nv - Dv * Nu);
                    Vector<3, Real> termDvv = D * (D * Nvv - Dvv * N);
                    values[3] = (D * termDuu - (Real)2 * Du * numerDU) / Dcub;
                    values[4] = (D * termDuv + (Real)2 * Du * Dv * N) / Dcub;
                    values[5] = (D * termDvv - (Real)2 * Dv * numerDV) / Dcub;
                }
            }
        }

    private:
        // For simplicity of the implementation, 2-dimensional arrays
        // of size 5-by-5 are used.  Only array[r][c] is used where
        // 0 <= r <= 4 and 0 <= c < 4 - r.
        Vector3<Real> mControls[5][5];
        Real mWeights[5][5];
    };

    template <typename Real>
    class NURBSHalfSphereDegree3 : public NURBSSurface<3, Real>
    {
    public:
        NURBSHalfSphereDegree3()
            :
            NURBSSurface<3, Real>(BasisFunctionInput<Real>(4, 3),
                BasisFunctionInput<Real>(4, 3), nullptr, nullptr)
        {
            // weight[j][i] is mWeights[i + 4 * j], 0 <= i < 4, 0 <= j < 4
            Real const oneThird = (Real)1 / (Real)3;
            Real const oneNinth = (Real)1 / (Real)9;
            this->mWeights[0] = (Real)1;
            this->mWeights[1] = oneThird;
            this->mWeights[2] = oneThird;
            this->mWeights[3] = (Real)1;
            this->mWeights[4] = oneThird;
            this->mWeights[5] = oneNinth;
            this->mWeights[6] = oneNinth;
            this->mWeights[7] = oneThird;
            this->mWeights[8] = oneThird;
            this->mWeights[9] = oneNinth;
            this->mWeights[10] = oneNinth;
            this->mWeights[11] = oneThird;
            this->mWeights[12] = (Real)1;
            this->mWeights[13] = oneThird;
            this->mWeights[14] = oneThird;
            this->mWeights[15] = (Real)1;

            // control[j][i] is mControls[i + 4 * j], 0 <= i < 4, 0 <= j < 4
            this->mControls[0] = { (Real)0, (Real)0, (Real)1 };
            this->mControls[1] = { (Real)0, (Real)0, (Real)1 };
            this->mControls[2] = { (Real)0, (Real)0, (Real)1 };
            this->mControls[3] = { (Real)0, (Real)0, (Real)1 };
            this->mControls[4] = { (Real)2, (Real)0, (Real)1 };
            this->mControls[5] = { (Real)2, (Real)4, (Real)1 };
            this->mControls[6] = { (Real)-2, (Real)4, (Real)1 };
            this->mControls[7] = { (Real)-2, (Real)0, (Real)1 };
            this->mControls[8] = { (Real)2, (Real)0, (Real)-1 };
            this->mControls[9] = { (Real)2, (Real)4, (Real)-1 };
            this->mControls[10] = { (Real)-2, (Real)4, (Real)-1 };
            this->mControls[11] = { (Real)-2, (Real)0, (Real)-1 };
            this->mControls[12] = { (Real)0, (Real)0, (Real)-1 };
            this->mControls[13] = { (Real)0, (Real)0, (Real)-1 };
            this->mControls[14] = { (Real)0, (Real)0, (Real)-1 };
            this->mControls[15] = { (Real)0, (Real)0, (Real)-1 };
        }
    };

    template <typename Real>
    class NURBSFullSphereDegree3 : public NURBSSurface<3, Real>
    {
    public:
        NURBSFullSphereDegree3()
            :
            NURBSSurface<3, Real>(BasisFunctionInput<Real>(4, 3),
                CreateBasisFunctionInputV(), nullptr, nullptr)
        {
            // weight[j][i] is mWeights[i + 4 * j], 0 <= i < 4, 0 <= j < 7
            Real const oneThird = (Real)1 / (Real)3;
            Real const oneNinth = (Real)1 / (Real)9;
            this->mWeights[0] = (Real)1;
            this->mWeights[4] = oneThird;
            this->mWeights[8] = oneThird;
            this->mWeights[12] = (Real)1;
            this->mWeights[16] = oneThird;
            this->mWeights[20] = oneThird;
            this->mWeights[24] = (Real)1;
            this->mWeights[1] = oneThird;
            this->mWeights[5] = oneNinth;
            this->mWeights[9] = oneNinth;
            this->mWeights[13] = oneThird;
            this->mWeights[17] = oneNinth;
            this->mWeights[21] = oneNinth;
            this->mWeights[25] = oneThird;
            this->mWeights[2] = oneThird;
            this->mWeights[6] = oneNinth;
            this->mWeights[10] = oneNinth;
            this->mWeights[14] = oneThird;
            this->mWeights[18] = oneNinth;
            this->mWeights[22] = oneNinth;
            this->mWeights[26] = oneThird;
            this->mWeights[3] = (Real)1;
            this->mWeights[7] = oneThird;
            this->mWeights[11] = oneThird;
            this->mWeights[15] = (Real)1;
            this->mWeights[19] = oneThird;
            this->mWeights[23] = oneThird;
            this->mWeights[27] = (Real)1;

            // control[j][i] is mControls[i + 4 * j], 0 <= i < 4, 0 <= j < 7
            this->mControls[0] = { (Real)0, (Real)0, (Real)1 };
            this->mControls[4] = { (Real)0, (Real)0, (Real)1 };
            this->mControls[8] = { (Real)0, (Real)0, (Real)1 };
            this->mControls[12] = { (Real)0, (Real)0, (Real)1 };
            this->mControls[16] = { (Real)0, (Real)0, (Real)1 };
            this->mControls[20] = { (Real)0, (Real)0, (Real)1 };
            this->mControls[24] = { (Real)0, (Real)0, (Real)1 };
            this->mControls[1] = { (Real)2, (Real)0, (Real)1 };
            this->mControls[5] = { (Real)2, (Real)4, (Real)1 };
            this->mControls[9] = { (Real)-2, (Real)4, (Real)1 };
            this->mControls[13] = { (Real)-2, (Real)0, (Real)1 };
            this->mControls[17] = { (Real)-2, (Real)-4, (Real)1 };
            this->mControls[21] = { (Real)2, (Real)-4, (Real)1 };
            this->mControls[25] = { (Real)2, (Real)0, (Real)1 };
            this->mControls[2] = { (Real)2, (Real)0, (Real)-1 };
            this->mControls[6] = { (Real)2, (Real)4, (Real)-1 };
            this->mControls[10] = { (Real)-2, (Real)4, (Real)-1 };
            this->mControls[14] = { (Real)-2, (Real)0, (Real)-1 };
            this->mControls[18] = { (Real)-2, (Real)-4, (Real)-1 };
            this->mControls[22] = { (Real)2, (Real)-4, (Real)-1 };
            this->mControls[26] = { (Real)2, (Real)0, (Real)-1 };
            this->mControls[3] = { (Real)0, (Real)0, (Real)-1 };
            this->mControls[7] = { (Real)0, (Real)0, (Real)-1 };
            this->mControls[11] = { (Real)0, (Real)0, (Real)-1 };
            this->mControls[15] = { (Real)0, (Real)0, (Real)-1 };
            this->mControls[19] = { (Real)0, (Real)0, (Real)-1 };
            this->mControls[23] = { (Real)0, (Real)0, (Real)-1 };
            this->mControls[27] = { (Real)0, (Real)0, (Real)-1 };
        }

    private:
        static BasisFunctionInput<Real> CreateBasisFunctionInputV()
        {
            BasisFunctionInput<Real> input;
            input.numControls = 7;
            input.degree = 3;
            input.uniform = true;
            input.periodic = false;
            input.numUniqueKnots = 3;
            input.uniqueKnots.resize(input.numUniqueKnots);
            input.uniqueKnots[0] = { (Real)0, 4 };
            input.uniqueKnots[1] = { (Real)0.5, 3 };
            input.uniqueKnots[2] = { (Real)1, 4 };
            return input;
        }
    };
}
