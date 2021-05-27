// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/Matrix3x3.h>

// A quadric surface is defined implicitly by
//
//   0 = a0 + a1*x[0] + a2*x[1] + a3*x[2] + a4*x[0]^2 + a5*x[0]*x[1] +
//       a6*x[0]*x[2] + a7*x[1]^2 + a8*x[1]*x[2] + a9*x[2]^2
//
//     = a0 + [a1 a2 a3]*X + X^T*[a4   a5/2 a6/2]*X
//                               [a5/2 a7   a8/2]
//                               [a6/2 a8/2 a9  ]
//     = C + B^T*X + X^T*A*X
//
// The matrix A is symmetric.

namespace gte
{
    template <typename Real>
    class QuadricSurface
    {
    public:
        // Construction and destruction.  The default constructor sets all
        // coefficients to zero.
        QuadricSurface()
        {
            mCoefficient.fill((Real)0);
            mC = (Real)0;
            mB.MakeZero();
            mA.MakeZero();
        }

        QuadricSurface(std::array<Real, 10> const& coefficient)
            :
            mCoefficient(coefficient)
        {
            mC = mCoefficient[0];
            mB[0] = mCoefficient[1];
            mB[1] = mCoefficient[2];
            mB[2] = mCoefficient[3];
            mA(0, 0) = mCoefficient[4];
            mA(0, 1) = (Real)0.5 * mCoefficient[5];
            mA(0, 2) = (Real)0.5 * mCoefficient[6];
            mA(1, 0) = mA(0, 1);
            mA(1, 1) = mCoefficient[7];
            mA(1, 2) = (Real)0.5 * mCoefficient[8];
            mA(2, 0) = mA(0, 2);
            mA(2, 1) = mA(1, 2);
            mA(2, 2) = mCoefficient[9];
        }

        // Member access.
        inline std::array<Real, 10> const& GetCoefficients() const
        {
            return mCoefficient;
        }

        inline Real const& GetC() const
        {
            return mC;
        }

        inline Vector3<Real> const& GetB() const
        {
            return mB;
        }

        inline Matrix3x3<Real> const& GetA() const
        {
            return mA;
        }

        // Evaluate the function.
        Real F(Vector3<Real> const& position) const
        {
            Real f = Dot(position, mA * position + mB) + mC;
            return f;
        }

        // Evaluate the first-order partial derivatives (gradient).
        Real FX(Vector3<Real> const& position) const
        {
            Real sum = mA(0, 0) * position[0] + mA(0, 1) * position[1] + mA(0, 2) * position[2];
            Real fx = (Real)2 * sum + mB[0];
            return fx;
        }

        Real FY(Vector3<Real> const& position) const
        {
            Real sum = mA(1, 0) * position[0] + mA(1, 1) * position[1] + mA(1, 2) * position[2];
            Real fy = (Real)2 * sum + mB[1];
            return fy;
        }

        Real FZ(Vector3<Real> const& position) const
        {
            Real sum = mA(2, 0) * position[0] + mA(2, 1) * position[1] + mA(2, 2) * position[2];
            Real fz = (Real)2 * sum + mB[2];
            return fz;
        }

        // Evaluate the second-order partial derivatives (Hessian).
        Real FXX(Vector3<Real> const&) const
        {
            Real fxx = (Real)2 * mA(0, 0);
            return fxx;
        }

        Real FXY(Vector3<Real> const&) const
        {
            Real fxy = (Real)2 * mA(0, 1);
            return fxy;
        }

        Real FXZ(Vector3<Real> const&) const
        {
            Real fxz = (Real)2 * mA(0, 2);
            return fxz;
        }

        Real FYY(Vector3<Real> const&) const
        {
            Real fyy = (Real)2 * mA(1, 1);
            return fyy;
        }

        Real FYZ(Vector3<Real> const&) const
        {
            Real fyz = (Real)2 * mA(1, 2);
            return fyz;
        }

        Real FZZ(Vector3<Real> const&) const
        {
            Real fzz = (Real)2 * mA(2, 2);
            return fzz;
        }

        // Classification of the quadric.  The implementation uses exact
        // rational arithmetic to avoid misclassification due to
        // floating-point rounding errors.
        enum Classification
        {
            QT_NONE,
            QT_POINT,
            QT_LINE,
            QT_PLANE,
            QT_TWO_PLANES,
            QT_PARABOLIC_CYLINDER,
            QT_ELLIPTIC_CYLINDER,
            QT_HYPERBOLIC_CYLINDER,
            QT_ELLIPTIC_PARABOLOID,
            QT_HYPERBOLIC_PARABOLOID,
            QT_ELLIPTIC_CONE,
            QT_HYPERBOLOID_ONE_SHEET,
            QT_HYPERBOLOID_TWO_SHEETS,
            QT_ELLIPSOID
        };

        Classification GetClassification() const
        {
            // Convert the coefficients to their rational representations and
            // compute various derived quantities.
            RReps reps(mCoefficient);

            // Use Sturm sequences to determine the signs of the roots.
            int positiveRoots, negativeRoots, zeroRoots;
            GetRootSigns(reps, positiveRoots, negativeRoots, zeroRoots);

            // Classify the solution set to the equation.
            Classification type = QT_NONE;
            switch (zeroRoots)
            {
            case 0:
                type = ClassifyZeroRoots0(reps, positiveRoots);
                break;
            case 1:
                type = ClassifyZeroRoots1(reps, positiveRoots);
                break;
            case 2:
                type = ClassifyZeroRoots2(reps, positiveRoots);
                break;
            case 3:
                type = ClassifyZeroRoots3(reps);
                break;
            }
            return type;
        }

    private:
        typedef BSRational<UIntegerAP32> Rational;
        typedef Vector<3, Rational> RVector3;

        class RReps
        {
        public:
            RReps(std::array<Real, 10> const& coefficient)
            {
                Rational half = (Real)0.5;

                c = Rational(coefficient[0]);
                b0 = Rational(coefficient[1]);
                b1 = Rational(coefficient[2]);
                b2 = Rational(coefficient[3]);
                a00 = Rational(coefficient[4]);
                a01 = half * Rational(coefficient[5]);
                a02 = half * Rational(coefficient[6]);
                a11 = Rational(coefficient[7]);
                a12 = half * Rational(coefficient[8]);
                a22 = Rational(coefficient[9]);

                sub00 = a11 * a22 - a12 * a12;
                sub01 = a01 * a22 - a12 * a02;
                sub02 = a01 * a12 - a02 * a11;
                sub11 = a00 * a22 - a02 * a02;
                sub12 = a00 * a12 - a02 * a01;
                sub22 = a00 * a11 - a01 * a01;

                k0 = a00 * sub00 - a01 * sub01 + a02 * sub02;
                k1 = sub00 + sub11 + sub22;
                k2 = a00 + a11 + a22;
                k3 = Rational(0);
                k4 = Rational(0);
                k5 = Rational(0);
            }

            // Quadratic coefficients.
            Rational a00, a01, a02, a11, a12, a22, b0, b1, b2, c;

            // 2-by-2 determinants
            Rational sub00, sub01, sub02, sub11, sub12, sub22;

            // Characteristic polynomial L^3 - k2 * L^2 + k1 * L - k0.
            Rational k0, k1, k2;

            // For Sturm sequences.
            Rational k3, k4, k5;
        };

        static void GetRootSigns(RReps& reps, int& positiveRoots, int& negativeRoots, int& zeroRoots)
        {
            // Use Sturm sequences to determine the signs of the roots.
            int signChangeMI, signChange0, signChangePI, distinctNonzeroRoots;
            std::array<Rational, 4> value;
            Rational const zero(0);
            if (reps.k0 != zero)
            {
                reps.k3 = Rational(2, 9) * reps.k2 * reps.k2 - Rational(2, 3) * reps.k1;
                reps.k4 = reps.k0 - Rational(1, 9) * reps.k1 * reps.k2;

                if (reps.k3 != zero)
                {
                    reps.k5 = -(reps.k1 + ((Rational(2) * reps.k2 * reps.k3 +
                        Rational(3) * reps.k4) * reps.k4) / (reps.k3 * reps.k3));

                    value[0] = 1;
                    value[1] = -reps.k3;
                    value[2] = reps.k5;
                    signChangeMI = 1 + GetSignChanges(3, value);

                    value[0] = -reps.k0;
                    value[1] = reps.k1;
                    value[2] = reps.k4;
                    value[3] = reps.k5;
                    signChange0 = GetSignChanges(4, value);

                    value[0] = 1;
                    value[1] = reps.k3;
                    value[2] = reps.k5;
                    signChangePI = GetSignChanges(3, value);
                }
                else
                {
                    value[0] = -reps.k0;
                    value[1] = reps.k1;
                    value[2] = reps.k4;
                    signChange0 = GetSignChanges(3, value);

                    value[0] = 1;
                    value[1] = reps.k4;
                    signChangePI = GetSignChanges(2, value);
                    signChangeMI = 1 + signChangePI;
                }

                positiveRoots = signChange0 - signChangePI;
                LogAssert(positiveRoots >= 0, "Unexpected condition.");
                negativeRoots = signChangeMI - signChange0;
                LogAssert(negativeRoots >= 0, "Unexpected condition.");
                zeroRoots = 0;

                distinctNonzeroRoots = positiveRoots + negativeRoots;
                if (distinctNonzeroRoots == 2)
                {
                    if (positiveRoots == 2)
                    {
                        positiveRoots = 3;
                    }
                    else if (negativeRoots == 2)
                    {
                        negativeRoots = 3;
                    }
                    else
                    {
                        // One root is positive and one is negative.  One root
                        // has multiplicity 2, the other of multiplicity 1.
                        // Distinguish between the two cases by computing the
                        // sign of the polynomial at the inflection point
                        // L = k2/3.
                        Rational X = Rational(1, 3) * reps.k2;
                        Rational poly = X * (X * (X - reps.k2) + reps.k1) - reps.k0;
                        if (poly > zero)
                        {
                            positiveRoots = 2;
                        }
                        else
                        {
                            negativeRoots = 2;
                        }
                    }
                }
                else if (distinctNonzeroRoots == 1)
                {
                    // Root of multiplicity 3.
                    if (positiveRoots == 1)
                    {
                        positiveRoots = 3;
                    }
                    else
                    {
                        negativeRoots = 3;
                    }
                }

                return;
            }

            if (reps.k1 != zero)
            {
                reps.k3 = Rational(1, 4) * reps.k2 * reps.k2 - reps.k1;

                value[0] = -1;
                value[1] = reps.k3;
                signChangeMI = 1 + GetSignChanges(2, value);

                value[0] = reps.k1;
                value[1] = -reps.k2;
                value[2] = reps.k3;
                signChange0 = GetSignChanges(3, value);

                value[0] = 1;
                value[1] = reps.k3;
                signChangePI = GetSignChanges(2, value);

                positiveRoots = signChange0 - signChangePI;
                LogAssert(positiveRoots >= 0, "Unexpected condition.");
                negativeRoots = signChangeMI - signChange0;
                LogAssert(negativeRoots >= 0, "Unexpected condition.");
                zeroRoots = 1;

                distinctNonzeroRoots = positiveRoots + negativeRoots;
                if (distinctNonzeroRoots == 1)
                {
                    positiveRoots = 2;
                }

                return;
            }

            if (reps.k2 != zero)
            {
                zeroRoots = 2;
                if (reps.k2 > zero)
                {
                    positiveRoots = 1;
                    negativeRoots = 0;
                }
                else
                {
                    positiveRoots = 0;
                    negativeRoots = 1;
                }
                return;
            }

            positiveRoots = 0;
            negativeRoots = 0;
            zeroRoots = 3;
        }

        static int GetSignChanges(int quantity, std::array<Rational, 4> const& value)
        {
            int signChanges = 0;
            Rational const zero(0);

            Rational prev = value[0];
            for (int i = 1; i < quantity; ++i)
            {
                Rational next = value[i];
                if (next != zero)
                {
                    if (prev * next < zero)
                    {
                        ++signChanges;
                    }

                    prev = next;
                }
            }

            return signChanges;
        }

        static Classification ClassifyZeroRoots0(RReps const& reps, int positiveRoots)
        {
            // The inverse matrix is
            // +-                      -+
            // |  sub00  -sub01   sub02 |
            // | -sub01   sub11  -sub12 | * (1/det)
            // |  sub02  -sub12   sub22 |
            // +-                      -+
            Rational fourDet = Rational(4) * reps.k0;

            Rational qForm = reps.b0 * (reps.sub00 * reps.b0 -
                reps.sub01 * reps.b1 + reps.sub02 * reps.b2) -
                reps.b1 * (reps.sub01 * reps.b0 - reps.sub11 * reps.b1 +
                reps.sub12 * reps.b2) + reps.b2 * (reps.sub02 * reps.b0 -
                reps.sub12 * reps.b1 + reps.sub22 * reps.b2);

            Rational r = Rational(1, 4) * qForm / fourDet - reps.c;
            Rational const zero(0);
            if (r > zero)
            {
                if (positiveRoots == 3)
                {
                    return QT_ELLIPSOID;
                }
                else if (positiveRoots == 2)
                {
                    return QT_HYPERBOLOID_ONE_SHEET;
                }
                else if (positiveRoots == 1)
                {
                    return QT_HYPERBOLOID_TWO_SHEETS;
                }
                else
                {
                    return QT_NONE;
                }
            }
            else if (r < zero)
            {
                if (positiveRoots == 3)
                {
                    return QT_NONE;
                }
                else if (positiveRoots == 2)
                {
                    return QT_HYPERBOLOID_TWO_SHEETS;
                }
                else if (positiveRoots == 1)
                {
                    return QT_HYPERBOLOID_ONE_SHEET;
                }
                else
                {
                    return QT_ELLIPSOID;
                }
            }

            // else r == 0
            if (positiveRoots == 3 || positiveRoots == 0)
            {
                return QT_POINT;
            }

            return QT_ELLIPTIC_CONE;
        }

        static Classification ClassifyZeroRoots1(RReps const& reps, int positiveRoots)
        {
            // Generate an orthonormal set {p0,p1,p2}, where p0 is an
            // eigenvector of A corresponding to eigenvalue zero.
            RVector3 P0, P1, P2;
            Rational const zero(0);

            if (reps.sub00 != zero || reps.sub01 != zero || reps.sub02 != zero)
            {
                // Rows 1 and 2 are linearly independent.
                P0 = { reps.sub00, -reps.sub01, reps.sub02 };
                P1 = { reps.a01, reps.a11, reps.a12 };
                P2 = Cross(P0, P1);
                return ClassifyZeroRoots1(reps, positiveRoots, P0, P1, P2);
            }

            if (reps.sub01 != zero || reps.sub11 != zero || reps.sub12 != zero)
            {
                // Rows 2 and 0 are linearly independent.
                P0 = { -reps.sub01, reps.sub11, -reps.sub12 };
                P1 = { reps.a02, reps.a12, reps.a22 };
                P2 = Cross(P0, P1);
                return ClassifyZeroRoots1(reps, positiveRoots, P0, P1, P2);
            }

            // Rows 0 and 1 are linearly independent.
            P0 = { reps.sub02, -reps.sub12, reps.sub22 };
            P1 = { reps.a00, reps.a01, reps.a02 };
            P2 = Cross(P0, P1);
            return ClassifyZeroRoots1(reps, positiveRoots, P0, P1, P2);
        }

        static Classification ClassifyZeroRoots1(RReps const& reps, int positiveRoots,
            RVector3 const& P0, RVector3 const& P1, RVector3 const& P2)
        {
            Rational const zero(0);
            Rational e0 = P0[0] * reps.b0 + P0[1] * reps.b1 + P0[2] * reps.b2;

            if (e0 != zero)
            {
                if (positiveRoots == 1)
                {
                    return QT_HYPERBOLIC_PARABOLOID;
                }
                else
                {
                    return QT_ELLIPTIC_PARABOLOID;
                }
            }

            // Matrix F.
            Rational f11 = P1[0] * (reps.a00 * P1[0] + reps.a01 * P1[1] +
                reps.a02 * P1[2]) + P1[1] * (reps.a01 * P1[0] +
                reps.a11 * P1[1] + reps.a12 * P1[2]) + P1[2] * (
                reps.a02 * P1[0] + reps.a12 * P1[1] + reps.a22 * P1[2]);

            Rational f12 = P2[0] * (reps.a00 * P1[0] + reps.a01 * P1[1] +
                reps.a02 * P1[2]) + P2[1] * (reps.a01 * P1[0] +
                reps.a11 * P1[1] + reps.a12 * P1[2]) + P2[2] * (
                reps.a02 * P1[0] + reps.a12 * P1[1] + reps.a22 * P1[2]);

            Rational f22 = P2[0] * (reps.a00 * P2[0] + reps.a01 * P2[1] +
                reps.a02 * P2[2]) + P2[1] * (reps.a01 * P2[0] +
                reps.a11 * P2[1] + reps.a12 * P2[2]) + P2[2] * (
                reps.a02 * P2[0] + reps.a12 * P2[1] + reps.a22 * P2[2]);

            // Vector g.
            Rational g1 = P1[0] * reps.b0 + P1[1] * reps.b1 + P1[2] * reps.b2;
            Rational g2 = P2[0] * reps.b0 + P2[1] * reps.b1 + P2[2] * reps.b2;

            // Compute g^T*F^{-1}*g/4 - c.
            Rational fourDet = Rational(4) * (f11 * f22 - f12 * f12);
            Rational r = (g1 * (f22 * g1 - f12 * g2) + g2 * (f11 * g2 - f12 * g1)) / fourDet - reps.c;

            if (r > zero)
            {
                if (positiveRoots == 2)
                {
                    return QT_ELLIPTIC_CYLINDER;
                }
                else if (positiveRoots == 1)
                {
                    return QT_HYPERBOLIC_CYLINDER;
                }
                else
                {
                    return QT_NONE;
                }
            }
            else if (r < zero)
            {
                if (positiveRoots == 2)
                {
                    return QT_NONE;
                }
                else if (positiveRoots == 1)
                {
                    return QT_HYPERBOLIC_CYLINDER;
                }
                else
                {
                    return QT_ELLIPTIC_CYLINDER;
                }
            }

            // else r == 0
            return (positiveRoots == 1 ? QT_TWO_PLANES : QT_LINE);
        }

        static Classification ClassifyZeroRoots2(const RReps& reps, int positiveRoots)
        {
            // Generate an orthonormal set {p0,p1,p2}, where p0 and p1 are
            // eigenvectors of A corresponding to eigenvalue zero.  The vector
            // p2 is an eigenvector of A corresponding to eigenvalue c2.
            Rational const zero(0);
            RVector3 P0, P1, P2;

            if (reps.a00 != zero || reps.a01 != zero || reps.a02 != zero)
            {
                // row 0 is not zero
                P2 = { reps.a00, reps.a01, reps.a02 };
            }
            else if (reps.a01 != zero || reps.a11 != zero || reps.a12 != zero)
            {
                // row 1 is not zero
                P2 = { reps.a01, reps.a11, reps.a12 };
            }
            else
            {
                // row 2 is not zero
                P2 = { reps.a02, reps.a12, reps.a22 };
            }

            if (P2[0] != zero)
            {
                P1[0] = P2[1];
                P1[1] = -P2[0];
                P1[2] = zero;
            }
            else
            {
                P1[0] = zero;
                P1[1] = P2[2];
                P1[2] = -P2[1];
            }
            P0 = Cross(P1, P2);

            return ClassifyZeroRoots2(reps, positiveRoots, P0, P1, P2);
        }

        static Classification ClassifyZeroRoots2(RReps const& reps, int positiveRoots,
            RVector3 const& P0, RVector3 const& P1, RVector3 const& P2)
        {
            Rational const zero(0);
            Rational e0 = P0[0] * reps.b0 + P0[1] * reps.b1 + P0[2] * reps.b1;
            if (e0 != zero)
            {
                return QT_PARABOLIC_CYLINDER;
            }

            Rational e1 = P1[0] * reps.b0 + P1[1] * reps.b1 + P1[2] * reps.b1;
            if (e1 != zero)
            {
                return QT_PARABOLIC_CYLINDER;
            }

            Rational f1 = reps.k2 * Dot(P2, P2);
            Rational e2 = P2[0] * reps.b0 + P2[1] * reps.b1 + P2[2] * reps.b1;
            Rational r = e2 * e2 / (Rational(4) * f1) - reps.c;
            if (r > zero)
            {
                if (positiveRoots == 1)
                {
                    return QT_TWO_PLANES;
                }
                else
                {
                    return QT_NONE;
                }
            }
            else if (r < zero)
            {
                if (positiveRoots == 1)
                {
                    return QT_NONE;
                }
                else
                {
                    return QT_TWO_PLANES;
                }
            }

            // else r == 0
            return QT_PLANE;
        }

        static Classification ClassifyZeroRoots3(RReps const& reps)
        {
            Rational const zero(0);
            if (reps.b0 != zero || reps.b1 != zero || reps.b2 != zero)
            {
                return QT_PLANE;
            }

            return QT_NONE;
        }

        std::array<Real, 10> mCoefficient;
        Real mC;
        Vector3<Real> mB;
        Matrix3x3<Real> mA;
    };
}
