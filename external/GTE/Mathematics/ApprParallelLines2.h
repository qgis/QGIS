// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.3.2019.12.27

#pragma once

// Least-squares fit of two parallel lines to points that presumably are
// clustered on the lines. The algorithm is described in
//   https://www.geometrictools.com/Documentation/FitParallelLinesToPoints2D.pdf

#include <Mathematics/Polynomial1.h>
#include <Mathematics/RootsPolynomial.h>
#include <Mathematics/Vector2.h>

namespace gte
{
    template <typename Real>
    class ApprParallelLines2
    {
    public:
        ApprParallelLines2()
            :
            mR0(0), mR1(1), mR2(2), mR3(3), mR4(4), mR5(5), mR6(6)
        {
            // The constants are set here in case Real is a rational type,
            // which avoids construction costs for those types.
        }

        void Fit(std::vector<Vector2<Real>> const& P, unsigned int maxIterations,
            Vector2<Real>& C, Vector2<Real>& V, Real& radius)
        {
            // Compute the average of the samples.
            size_t const n = P.size();
            Real const invN = static_cast<Real>(1) / static_cast<Real>(n);
            std::vector<Vector2<Real>> PAdjust = P;
            Vector2<Real> A{ mR0, mR0 };
            for (auto const& sample : PAdjust)
            {
                A += sample;
            }
            A *= invN;

            // Subtract the average from the samples so that the replacement
            // points have zero average.
            for (auto& sample : PAdjust)
            {
                sample -= A;
            }

            // Compute the Zpq terms.
            ZValues data(PAdjust);

            // Compute F(sigma,gamma) = f0(sigma) + gamma * f1(sigma).
            Polynomial1<Real> f0, f1;
            ComputeF(data, f0, f1);
            Polynomial1<Real> freduced0(4), freduced1(3);
            for (int i = 0; i <= 4; ++i)
            {
                freduced0[i] = f0[2 * i];
            }
            for (int i = 0; i <= 3; ++i)
            {
                freduced1[i] = f1[2 * i + 1];
            }

            // Evaluate the error function at any (sigma,gamma). Choose (0,1)
            // so that we do not have to process a root sigma=0 later.
            Real minSigma = mR0, minGamma = mR1;
            Real minK = data.Z03 / (mR2 * data.Z02);
            Real minKSqr = minK * minK;
            Real minRSqr = minKSqr + data.Z02;
            Real minError = data.Z04 - mR4 * minK * data.Z03 + (mR4 * minKSqr - data.Z02) * data.Z02;

            if (f1 != Polynomial1<Real>{ mR0 })
            {
                Polynomial1<Real> sigmaSqrPoly{ mR0, mR0, mR1 };
                Polynomial1<Real> f0Sqr = f0 * f0, f1Sqr = f1 * f1;
                Polynomial1<Real> h = sigmaSqrPoly * f1Sqr + (f0Sqr - f1Sqr);
                Polynomial1<Real> hreduced(8);
                for (int i = 0; i <= 8; ++i)
                {
                    hreduced[i] = h[2 * i];
                }

                std::array<Real, 8> roots;
                int numRoots = RootsPolynomial<Real>::Find(8, &hreduced[0],
                    maxIterations, roots.data());
                for (int i = 0; i < numRoots; ++i)
                {
                    Real sigmaSqr = roots[i];
                    if (sigmaSqr > mR0)
                    {
                        Real sigma = std::sqrt(sigmaSqr);
                        Real gamma = -freduced0(sigmaSqr) / (sigma * freduced1(sigmaSqr));
                        UpdateParameters(data, sigma, sigmaSqr, gamma,
                            minSigma, minGamma, minK, minRSqr, minError);
                    }
                }
            }
            else
            {
                Polynomial1<Real> hreduced(4);
                for (int i = 0; i <= 4; ++i)
                {
                    hreduced[i] = f0[2 * i];
                }

                std::array<Real, 4> roots;
                int numRoots = RootsPolynomial<Real>::Find(4, &hreduced[0],
                    maxIterations, roots.data());
                for (int i = 0; i < numRoots; ++i)
                {
                    Real sigmaSqr = roots[i];
                    if (sigmaSqr > mR0)
                    {
                        Real sigma = std::sqrt(sigmaSqr);
                        Real gamma = std::sqrt(sigma);
                        UpdateParameters(data, sigma, sigmaSqr, gamma,
                            minSigma, minGamma, minK, minRSqr, minError);

                        gamma = -gamma;
                        UpdateParameters(data, sigma, sigmaSqr, gamma,
                            minSigma, minGamma, minK, minRSqr, minError);
                    }
                }
            }

            // Compute the minimizers V, C and radius. The center minK*U must have
            // A added to it because the inputs P had A subtracted from them. The
            // addition no longer guarantees that Dot(V,C) = 0, so the V-component
            // of A+minK*U is projected out so that the returned center has only a
            // U-component.
            V = Vector2<Real>{ minGamma, minSigma };
            C = A + minK * Vector2<Real>{ -minSigma, minGamma };
            C -= Dot(C, V) * V;
            radius = std::sqrt(minRSqr);
        }
    private:
        struct ZValues
        {
            ZValues(std::vector<Vector2<Real>> const& P)
                :
                Z20(0), Z11(0), Z02(0),
                Z30(0), Z21(0), Z12(0), Z03(0),
                Z40(0), Z31(0), Z22(0), Z13(0), Z04(0)
            {
                Real const invN = static_cast<Real>(1) / static_cast<Real>(P.size());
                for (auto const& sample : P)
                {
                    Real xx = sample[0] * sample[0];
                    Real xy = sample[0] * sample[1];
                    Real yy = sample[1] * sample[1];
                    Real xxx = xx * sample[0];
                    Real xxy = xy * sample[0];
                    Real xyy = xy * sample[1];
                    Real yyy = yy * sample[1];
                    Real xxxx = xxx * sample[0];
                    Real xxxy = xxx * sample[1];
                    Real xxyy = xx * yy;
                    Real xyyy = yyy * sample[0];
                    Real yyyy = yyy * sample[1];
                    Z20 += xx;
                    Z11 += xy;
                    Z02 += yy;
                    Z30 += xxx;
                    Z21 += xxy;
                    Z12 += xyy;
                    Z03 += yyy;
                    Z40 += xxxx;
                    Z31 += xxxy;
                    Z22 += xxyy;
                    Z13 += xyyy;
                    Z04 += yyyy;
                }
                Z20 *= invN;
                Z11 *= invN;
                Z02 *= invN;
                Z30 *= invN;
                Z21 *= invN;
                Z12 *= invN;
                Z03 *= invN;
                Z40 *= invN;
                Z31 *= invN;
                Z22 *= invN;
                Z13 *= invN;
                Z04 *= invN;
            }

            Real Z20, Z11, Z02;
            Real Z30, Z21, Z12, Z03;
            Real Z40, Z31, Z22, Z13, Z04;
        };

        // Given two polynomials A0+gamma*B0 and A1+gamma*B1, the product is
        // [A0*A1+(1-sigma^2)*B0*B1] + gamma*[A0*B1+B0*A1] = A2+gamma*B2.
        void ComputeProduct(
            Polynomial1<Real> const& A0, Polynomial1<Real> const& B0,
            Polynomial1<Real> const& A1, Polynomial1<Real> const& B1,
            Polynomial1<Real>& A2, Polynomial1<Real>& B2)
        {
            Polynomial1<Real> gammaSqr{ mR1, mR0, -mR1 };
            A2 = A0 * A1 + gammaSqr * B0 * B1;
            B2 = A0 * B1 + B0 * A1;
        }

        void ComputeF(ZValues const& data, Polynomial1<Real>& f0, Polynomial1<Real>& f1)
        {
            // Compute the apq and bpq terms.
            Polynomial1<Real> a11(2);
            a11[0] = data.Z11;
            a11[2] = -mR2 * data.Z11;

            Polynomial1<Real> b11(1);
            b11[1] = data.Z02 - data.Z20;

            Polynomial1<Real> a20(2);
            a20[0] = data.Z02;
            a20[2] = data.Z20 - data.Z02;

            Polynomial1<Real> b20(1);
            b20[1] = -mR2 * data.Z11;

            Polynomial1<Real> a30(3);
            a30[1] = -mR3;
            a30[3] = mR3 * data.Z12 - data.Z30;

            Polynomial1<Real> b30(2);
            b30[0] = data.Z03;
            b30[2] = mR3 * data.Z21 - data.Z03;

            Polynomial1<Real> a21(3);
            a21[1] = data.Z03 - mR2 * data.Z21;
            a21[3] = mR3 * data.Z21 - data.Z03;

            Polynomial1<Real> b21(2);
            b21[0] = data.Z12;
            b21[2] = data.Z30 - mR3 * data.Z12;

            Polynomial1<Real> a40(4);
            a40[0] = data.Z04;
            a40[2] = mR6 * data.Z22 - mR2 * data.Z04;
            a40[4] = data.Z40 - mR6 * data.Z22 + data.Z04;

            Polynomial1<Real> b40(3);
            b40[1] = -mR4 * data.Z13;
            b40[3] = mR4 * (data.Z13 - data.Z31);

            Polynomial1<Real> a31(4);
            a31[0] = data.Z13;
            a31[2] = mR3 * data.Z31 - mR5 * data.Z13;
            a31[4] = mR4 * (data.Z13 - data.Z31);

            Polynomial1<Real> b31(3);
            b31[1] = data.Z04 - mR3 * data.Z22;
            b31[3] = mR6 * data.Z22 - data.Z40 - data.Z04;

            // Compute S20^2 = c0 + gamma*d0.
            Polynomial1<Real> c0, d0;
            ComputeProduct(a20, b20, a20, b20, c0, d0);

            // Compute S31 * S20^2 = c1 + gamma*d1.
            Polynomial1<Real> c1, d1;
            ComputeProduct(a31, b31, c0, d0, c1, d1);

            // Compute S21 * S20 = c2 + gamma*d2.
            Polynomial1<Real> c2, d2;
            ComputeProduct(a21, b21, a20, b20, c2, d2);

            // Compute S30 * (S21 * S20) = c3 + gamma*d3.
            Polynomial1<Real> c3, d3;
            ComputeProduct(a30, b30, c2, d2, c3, d3);

            // Compute S30 * S11 = c4 + gamma*d4.
            Polynomial1<Real> c4, d4;
            ComputeProduct(a30, b30, a11, b11, c4, d4);

            // Compute S30 * (S30 * S11) = c5 + gamma*d5.
            Polynomial1<Real> c5, d5;
            ComputeProduct(a30, b30, c4, d4, c5, d5);

            // Compute S20^2 * S11 = c6 + gamma*d6.
            Polynomial1<Real> c6, d6;
            ComputeProduct(c0, d0, a11, b11, c6, d6);

            // Compute S20 * (S20^2 * S11) = c7 + gamma*d7.
            Polynomial1<Real> c7, d7;
            ComputeProduct(a20, b20, c6, d6, c7, d7);

            // Compute F = 2*S31*S20^2 - 3*S30*S21*S20 + S30^2*S11
            // - 2*S20^3*S11 = f0 + gamma*f1, where f0 is even of degree 8
            // and f1 is odd of degree 7.
            f0 = mR2 * (c1 - c7) - mR3 * c3 + c5;
            f1 = mR2 * (d1 - d7) - mR3 * d3 + d5;
        }

        void UpdateParameters(ZValues const& data, Real const& sigma, Real const& sigmaSqr,
            Real const& gamma, Real& minSigma, Real& minGamma, Real& minK,
            Real& minRSqr, Real& minError)
        {
            // Rather than evaluate apq(sigma) and bpq(sigma), the
            // polynomials are evaluated at sigmaSqr to avoid the
            // rounding errors that are inherent by computing
            // s = sqrt(ssqr); ssqr = s * s;
            Real A20 = data.Z02 + (data.Z20 - data.Z02) * sigmaSqr;
            Real B20 = -mR2 * data.Z11 * sigma;
            Real S20 = A20 + gamma * B20;
            Real A30 = -sigma * (mR3 * data.Z12 + (data.Z30 - mR3 * data.Z12) * sigmaSqr);
            Real B30 = data.Z03 + (mR3 * data.Z21 - data.Z03) * sigmaSqr;
            Real S30 = A30 + gamma * B30;
            Real A40 = data.Z04 + ((mR6 * data.Z22 - mR2 * data.Z04)
                + (data.Z40 - mR6 * data.Z22 + data.Z04) * sigmaSqr) * sigmaSqr;
            Real B40 = -mR4 * sigma * (data.Z13 + (data.Z31 - data.Z13) * sigmaSqr);
            Real S40 = A40 + gamma * B40;
            Real k = S30 / (mR2 * S20);
            Real ksqr = k * k;
            Real rsqr = ksqr + S20;
            Real error = S40 - mR4 * k * S30 + (mR4 * ksqr - S20) * S20;
            if (error < minError)
            {
                minSigma = sigma;
                minGamma = gamma;
                minK = k;
                minRSqr = rsqr;
                minError = error;
            }
        }

        Real const mR0, mR1, mR2, mR3, mR4, mR5, mR6;
    };
}
