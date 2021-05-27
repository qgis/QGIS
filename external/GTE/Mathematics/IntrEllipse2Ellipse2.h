// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.04.13

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Hyperellipsoid.h>
#include <Mathematics/RootsBisection.h>
#include <Mathematics/RootsPolynomial.h>
#include <Mathematics/SymmetricEigensolver2x2.h>
#include <Mathematics/Matrix2x2.h>

// The test-intersection and find-intersection queries implemented here are
// discussed in the document
//   https://www.geometrictools.com/Documentation/IntersectionOfEllipses.pdf
// The Real type should support exact rational arithmetic in order for the
// polynomial root construction to be robust.  The classification of the
// intersections depends on various sign tests of computed values.  If these
// values are computed with floating-point arithmetic, the sign tests can
// lead to misclassification.
//
// The area-of-intersection query is discussed in the document
//   https://www.geometrictools.com/Documentation/AreaIntersectingEllipses.pdf

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Ellipse2<Real>, Ellipse2<Real>>
    {
    public:
        // The query tests the relationship between the ellipses as solid
        // objects.
        enum
        {
            ELLIPSES_SEPARATED,
            ELLIPSES_OVERLAP,
            ELLIPSE0_OUTSIDE_ELLIPSE1_BUT_TANGENT,
            ELLIPSE0_STRICTLY_CONTAINS_ELLIPSE1,
            ELLIPSE0_CONTAINS_ELLIPSE1_BUT_TANGENT,
            ELLIPSE1_STRICTLY_CONTAINS_ELLIPSE0,
            ELLIPSE1_CONTAINS_ELLIPSE0_BUT_TANGENT,
            ELLIPSES_EQUAL
        };

        // The ellipse axes are already normalized, which most likely
        // introduced rounding errors.
        int operator()(Ellipse2<Real> const& ellipse0, Ellipse2<Real> const& ellipse1)
        {
            Real const zero = 0, one = 1;

            // Get the parameters of ellipe0.
            Vector2<Real> K0 = ellipse0.center;
            Matrix2x2<Real> R0;
            R0.SetCol(0, ellipse0.axis[0]);
            R0.SetCol(1, ellipse0.axis[1]);
            Matrix2x2<Real> D0{
                one / (ellipse0.extent[0] * ellipse0.extent[0]), zero,
                zero, one / (ellipse0.extent[1] * ellipse0.extent[1]) };

            // Get the parameters of ellipse1.
            Vector2<Real> K1 = ellipse1.center;
            Matrix2x2<Real> R1;
            R1.SetCol(0, ellipse1.axis[0]);
            R1.SetCol(1, ellipse1.axis[1]);
            Matrix2x2<Real> D1{
                one / (ellipse1.extent[0] * ellipse1.extent[0]), zero,
                zero, one / (ellipse1.extent[1] * ellipse1.extent[1]) };

            // Compute K2 = D0^{1/2}*R0^T*(K1-K0). In the GTE code, the
            // quantity U = R0^T*(K1-K0) is a 2x1 vector which can be computed
            // in the GTE code by U = Transpose(R0)*(K1-K0). However, to avoid
            // the creation of the matrix object Transpose(R0), you can use
            // U^T = V^T*R0 which can be computed in the GTE code by
            // W = (K1-K0)*R0. The output W is mathematically a 1x2 vector,
            // but as a Vector<?> object, it is a 2-tuple. You can then
            // compute K2 = D0Half*W, where the 2-tuple W is now thought of
            // as a 2x1 vector. See Matrix.h, the operator function
            // Vector<?> operator*(Vector<?> const&, Matrix<?> const&) which
            // computes V^T*M for a Vector<?> V and a Matrix<?> M.
            Matrix2x2<Real> D0NegHalf{
                ellipse0.extent[0], zero,
                zero, ellipse0.extent[1] };
            Matrix2x2<Real> D0Half{
                one / ellipse0.extent[0], zero,
                zero, one / ellipse0.extent[1] };
            Vector2<Real> K2 = D0Half * ((K1 - K0) * R0);

            // Compute M2.
            Matrix2x2<Real> R1TR0D0NegHalf = MultiplyATB(R1, R0 * D0NegHalf);
            Matrix2x2<Real> M2 = MultiplyATB(R1TR0D0NegHalf, D1) * R1TR0D0NegHalf;

            // Factor M2 = R*D*R^T.
            SymmetricEigensolver2x2<Real> es;
            std::array<Real, 2> D;
            std::array<std::array<Real, 2>, 2> evec;
            es(M2(0, 0), M2(0, 1), M2(1, 1), +1, D, evec);
            Matrix2x2<Real> R;
            R.SetCol(0, evec[0]);
            R.SetCol(1, evec[1]);

            // Compute K = R^T*K2.
            Vector2<Real> K = K2 * R;

            // Transformed ellipse0 is Z^T*Z = 1 and transformed ellipse1 is
            // (Z-K)^T*D*(Z-K) = 0.

            // The minimum and maximum squared distances from the origin of
            // points on transformed ellipse1 are used to determine whether
            // the ellipses intersect, are separated or one contains the
            // other.
            Real minSqrDistance = std::numeric_limits<Real>::max();
            Real maxSqrDistance = zero;

            if (K == Vector2<Real>::Zero())
            {
                // The special case of common centers must be handled
                // separately.  It is not possible for the ellipses to be
                // separated.
                for (int i = 0; i < 2; ++i)
                {
                    Real invD = one / D[i];
                    if (invD < minSqrDistance)
                    {
                        minSqrDistance = invD;
                    }
                    if (invD > maxSqrDistance)
                    {
                        maxSqrDistance = invD;
                    }
                }
                return Classify(minSqrDistance, maxSqrDistance, zero);
            }

            // The closest point P0 and farthest point P1 are solutions to
            // s0*D*(P0 - K) = P0 and s1*D1*(P1 - K) = P1 for some scalars s0
            // and s1 that are roots to the function
            //   f(s) = d0*k0^2/(d0*s-1)^2 + d1*k1^2/(d1*s-1)^2 - 1
            // where D = diagonal(d0,d1) and K = (k0,k1).
            Real d0 = D[0], d1 = D[1];
            Real c0 = K[0] * K[0], c1 = K[1] * K[1];

            // Sort the values so that d0 >= d1.  This allows us to bound the
            // roots of f(s), of which there are at most 4.
            std::vector<std::pair<Real, Real>> param(2);
            if (d0 >= d1)
            {
                param[0] = std::make_pair(d0, c0);
                param[1] = std::make_pair(d1, c1);
            }
            else
            {
                param[0] = std::make_pair(d1, c1);
                param[1] = std::make_pair(d0, c0);
            }

            std::vector<std::pair<Real, Real>> valid;
            valid.reserve(2);
            if (param[0].first > param[1].first)
            {
                // d0 > d1
                for (int i = 0; i < 2; ++i)
                {
                    if (param[i].second > zero)
                    {
                        valid.push_back(param[i]);
                    }
                }
            }
            else
            {
                // d0 = d1
                param[0].second += param[1].second;
                if (param[0].second > zero)
                {
                    valid.push_back(param[0]);
                }
            }

            size_t numValid = valid.size();
            int numRoots = 0;
            Real roots[4];
            if (numValid == 2)
            {
                GetRoots(valid[0].first, valid[1].first, valid[0].second,
                    valid[1].second, numRoots, roots);
            }
            else if (numValid == 1)
            {
                GetRoots(valid[0].first, valid[0].second, numRoots, roots);
            }
            // else: numValid cannot be zero because we already handled case
            // K = 0

            for (int i = 0; i < numRoots; ++i)
            {
                Real s = roots[i];
                Real p0 = d0 * K[0] * s / (d0 * s - (Real)1);
                Real p1 = d1 * K[1] * s / (d1 * s - (Real)1);
                Real sqrDistance = p0 * p0 + p1 * p1;
                if (sqrDistance < minSqrDistance)
                {
                    minSqrDistance = sqrDistance;
                }
                if (sqrDistance > maxSqrDistance)
                {
                    maxSqrDistance = sqrDistance;
                }
            }

            return Classify(minSqrDistance, maxSqrDistance, d0 * c0 + d1 * c1);
        }

    private:
        void GetRoots(Real d0, Real c0, int& numRoots, Real* roots)
        {
            // f(s) = d0*c0/(d0*s-1)^2 - 1
            Real const one = (Real)1;
            Real temp = std::sqrt(d0 * c0);
            Real inv = one / d0;
            numRoots = 2;
            roots[0] = (one - temp) * inv;
            roots[1] = (one + temp) * inv;
        }

        void GetRoots(Real d0, Real d1, Real c0, Real c1, int& numRoots, Real* roots)
        {
            // f(s) = d0*c0/(d0*s-1)^2 + d1*c1/(d1*s-1)^2 - 1 with d0 > d1

            Real const zero = 0, one = (Real)1;
            Real d0c0 = d0 * c0;
            Real d1c1 = d1 * c1;
            Real sum = d0c0 + d1c1;
            Real sqrtsum = std::sqrt(sum);

            std::function<Real(Real)> F = [&one, d0, d1, d0c0, d1c1](Real s)
            {
                Real invN0 = one / (d0 * s - one);
                Real invN1 = one / (d1 * s - one);
                Real term0 = d0c0 * invN0 * invN0;
                Real term1 = d1c1 * invN1 * invN1;
                Real f = term0 + term1 - one;
                return f;
            };

            std::function<Real(Real)> DF = [&one, d0, d1, d0c0, d1c1](Real s)
            {
                Real const two = 2;
                Real invN0 = one / (d0 * s - one);
                Real invN1 = one / (d1 * s - one);
                Real term0 = d0 * d0c0 * invN0 * invN0 * invN0;
                Real term1 = d1 * d1c1 * invN1 * invN1 * invN1;
                Real df = -two * (term0 + term1);
                return df;
            };

            unsigned int const maxIterations = static_cast<unsigned int>(
                3 + std::numeric_limits<Real>::digits -
                std::numeric_limits<Real>::min_exponent);
            unsigned int iterations;
            numRoots = 0;

            Real invD0 = one / d0;
            Real invD1 = one / d1;
            Real smin, smax, s, fval;

            // Compute root in (-infinity,1/d0).  Obtain a lower bound for the
            // root better than -std::numeric_limits<Real>::max().
            smax = invD0;
            fval = sum - one;
            if (fval > zero)
            {
                smin = (one - sqrtsum) * invD1;  // < 0
                fval = F(smin);
                LogAssert(fval <= zero, "Unexpected condition.");
            }
            else
            {
                smin = zero;
            }
            iterations = RootsBisection<Real>::Find(F, smin, smax, -one, one,
                maxIterations, s);
            fval = F(s);
            LogAssert(iterations > 0, "Unexpected condition.");
            roots[numRoots++] = s;

            // Compute roots (if any) in (1/d0,1/d1).  It is the case that
            //   F(1/d0) = +infinity, F'(1/d0) = -infinity
            //   F(1/d1) = +infinity, F'(1/d1) = +infinity
            //   F"(s) > 0 for all s in the domain of F
            // Compute the unique root r of F'(s) on (1/d0,1/d1).  The
            // bisector needs only the signs at the endpoints, so we pass -1
            // and +1 instead of the infinite values.  If F(r) < 0, F(s) has
            // two roots in the interval.  If F(r) = 0, F(s) has only one root
            // in the interval.
            Real const oneThird = (Real)1 / (Real)3;
            Real rho = std::pow(d0 * d0c0 / (d1 * d1c1), oneThird);
            Real smid = (one + rho) / (d0 + rho * d1);
            Real fmid = F(smid);
            if (fmid < zero)
            {
                // Pass in signs rather than infinities, because the bisector cares
                // only about the signs.
                iterations = RootsBisection<Real>::Find(F, invD0, smid, one, -one,
                    maxIterations, s);
                fval = F(s);
                LogAssert(iterations > 0, "Unexpected condition.");
                roots[numRoots++] = s;
                iterations = RootsBisection<Real>::Find(F, smid, invD1, -one, one,
                    maxIterations, s);
                fval = F(s);
                LogAssert(iterations > 0, "Unexpected condition.");
                roots[numRoots++] = s;
            }
            else if (fmid == zero)
            {
                roots[numRoots++] = smid;
            }

            // Compute root in (1/d1,+infinity).  Obtain an upper bound for
            // the root better than std::numeric_limits<Real>::max().
            smin = invD1;
            smax = (one + sqrtsum) * invD1;  // > 1/d1
            fval = F(smax);
            LogAssert(fval <= zero, "Unexpected condition.");
            iterations = RootsBisection<Real>::Find(F, smin, smax, one, -one,
                maxIterations, s);
            fval = F(s);
            LogAssert(iterations > 0, "Unexpected condition.");
            roots[numRoots++] = s;
        }

        int Classify(Real minSqrDistance, Real maxSqrDistance, Real d0c0pd1c1)
        {
            Real const one = (Real)1;

            if (maxSqrDistance < one)
            {
                return ELLIPSE0_STRICTLY_CONTAINS_ELLIPSE1;
            }
            else if (maxSqrDistance > one)
            {
                if (minSqrDistance < one)
                {
                    return ELLIPSES_OVERLAP;
                }
                else if (minSqrDistance > one)
                {
                    if (d0c0pd1c1 > one)
                    {
                        return ELLIPSES_SEPARATED;
                    }
                    else
                    {
                        return ELLIPSE1_STRICTLY_CONTAINS_ELLIPSE0;
                    }
                }
                else  // minSqrDistance = 1
                {
                    if (d0c0pd1c1 > one)
                    {
                        return ELLIPSE0_OUTSIDE_ELLIPSE1_BUT_TANGENT;
                    }
                    else
                    {
                        return ELLIPSE1_CONTAINS_ELLIPSE0_BUT_TANGENT;
                    }
                }
            }
            else  // maxSqrDistance = 1
            {
                if (minSqrDistance < one)
                {
                    return ELLIPSE0_CONTAINS_ELLIPSE1_BUT_TANGENT;
                }
                else // minSqrDistance = 1
                {
                    return ELLIPSES_EQUAL;
                }
            }
        }
    };

    template <typename Real>
    class FIQuery<Real, Ellipse2<Real>, Ellipse2<Real>>
    {
    public:
        // The queries find the intersections (if any) of the ellipses treated
        // as hollow objects.  The implementations use the same concepts.
        FIQuery()
            :
            mZero((Real)0),
            mOne((Real)1),
            mTwo((Real)2),
            mPi((Real)GTE_C_PI),
            mTwoPi((Real)GTE_C_TWO_PI)
        {
        }

        struct Result
        {
            // This value is true when the ellipses intersect in at least one
            // point.
            bool intersect;

            // If the ellipses are not the same, numPoints is 0 through 4 and
            // that number of elements of 'points' are valid.  If the ellipses
            // are the same, numPoints is set to maxInt and 'points' is
            // invalid.
            int numPoints;
            std::array<Vector2<Real>, 4> points;
            std::array<bool, 4> isTransverse;
        };

        // The ellipse axes are already normalized, which most likely
        // introducedrounding errors.
        Result operator()(Ellipse2<Real> const& ellipse0, Ellipse2<Real> const& ellipse1)
        {
            Vector2<Real> rCenter, rAxis[2], rSqrExtent;

            rCenter = { ellipse0.center[0], ellipse0.center[1] };
            rAxis[0] = { ellipse0.axis[0][0], ellipse0.axis[0][1] };
            rAxis[1] = { ellipse0.axis[1][0], ellipse0.axis[1][1] };
            rSqrExtent =
            {
                ellipse0.extent[0] * ellipse0.extent[0],
                ellipse0.extent[1] * ellipse0.extent[1]
            };
            ToCoefficients(rCenter, rAxis, rSqrExtent, mA);

            rCenter = { ellipse1.center[0], ellipse1.center[1] };
            rAxis[0] = { ellipse1.axis[0][0], ellipse1.axis[0][1] };
            rAxis[1] = { ellipse1.axis[1][0], ellipse1.axis[1][1] };
            rSqrExtent =
            {
                ellipse1.extent[0] * ellipse1.extent[0],
                ellipse1.extent[1] * ellipse1.extent[1]
            };
            ToCoefficients(rCenter, rAxis, rSqrExtent, mB);

            Result result;
            DoRootFinding(result);
            return result;
        }

        // The axis directions do not have to be unit length.  The quadratic
        // equations are constructed according to the details of the PDF
        // document about the intersection of ellipses.
        Result operator()(Vector2<Real> const& center0,
            Vector2<Real> const axis0[2], Vector2<Real> const& sqrExtent0,
            Vector2<Real> const& center1, Vector2<Real> const axis1[2],
            Vector2<Real> const& sqrExtent1)
        {
            Vector2<Real> rCenter, rAxis[2], rSqrExtent;

            rCenter = { center0[0], center0[1] };
            rAxis[0] = { axis0[0][0], axis0[0][1] };
            rAxis[1] = { axis0[1][0], axis0[1][1] };
            rSqrExtent = { sqrExtent0[0], sqrExtent0[1] };
            ToCoefficients(rCenter, rAxis, rSqrExtent, mA);

            rCenter = { center1[0], center1[1] };
            rAxis[0] = { axis1[0][0], axis1[0][1] };
            rAxis[1] = { axis1[1][0], axis1[1][1] };
            rSqrExtent = { sqrExtent1[0], sqrExtent1[1] };
            ToCoefficients(rCenter, rAxis, rSqrExtent, mB);

            Result result;
            DoRootFinding(result);
            return result;
        }


        // Compute the area of intersection of ellipses.
        struct AreaResult
        {
            // The configuration of the two ellipses.
            enum
            {
                ELLIPSES_ARE_EQUAL,
                ELLIPSES_ARE_SEPARATED,
                E0_CONTAINS_E1,
                E1_CONTAINS_E0,
                ONE_CHORD_REGION,
                FOUR_CHORD_REGION
            };

            // One of the enumerates, determined in the call to AreaDispatch.
            int configuration;

            // Information about the ellipse-ellipse intersection points.
            Result findResult;

            // The area of intersection of the ellipses.
            Real area;
        };

        // The ellipse axes are already normalized, which most likely
        // introduced rounding errors.
        AreaResult AreaOfIntersection(Ellipse2<Real> const& ellipse0,
            Ellipse2<Real> const& ellipse1)
        {
            EllipseInfo E0;
            E0.center = ellipse0.center;
            E0.axis = ellipse0.axis;
            E0.extent = ellipse0.extent;
            E0.sqrExtent = ellipse0.extent * ellipse0.extent;
            FinishEllipseInfo(E0);

            EllipseInfo E1;
            E1.center = ellipse1.center;
            E1.axis = ellipse1.axis;
            E1.extent = ellipse1.extent;
            E1.sqrExtent = ellipse1.extent * ellipse1.extent;
            FinishEllipseInfo(E1);

            AreaResult ar;
            ar.configuration = 0;
            ar.findResult = operator()(ellipse0, ellipse1);
            ar.area = mZero;
            AreaDispatch(E0, E1, ar);
            return ar;
        }

        // The axis directions do not have to be unit length.  The positive
        // definite matrices are constructed according to the details of the
        // PDF documentabout the intersection of ellipses.
        AreaResult AreaOfIntersection(Vector2<Real> const& center0,
            Vector2<Real> const axis0[2], Vector2<Real> const& sqrExtent0,
            Vector2<Real> const& center1, Vector2<Real> const axis1[2],
            Vector2<Real> const& sqrExtent1)
        {
            EllipseInfo E0;
            E0.center = center0;
            E0.axis = { axis0[0], axis0[1] };
            E0.extent =
            {
                std::sqrt(sqrExtent0[0]),
                std::sqrt(sqrExtent0[1])
            };
            E0.sqrExtent = sqrExtent0;
            FinishEllipseInfo(E0);

            EllipseInfo E1;
            E1.center = center1;
            E1.axis = { axis1[0], axis1[1] };
            E1.extent =
            {
                std::sqrt(sqrExtent1[0]),
                std::sqrt(sqrExtent1[1])
            };
            E1.sqrExtent = sqrExtent1;
            FinishEllipseInfo(E1);

            AreaResult ar;
            ar.configuration = 0;
            ar.findResult = operator()(center0, axis0, sqrExtent0, center1, axis1, sqrExtent1);
            ar.area = mZero;
            AreaDispatch(E0, E1, ar);
            return ar;
        }

    private:
        // Compute the coefficients of the quadratic equation but with the
        // y^2-coefficient of 1.
        void ToCoefficients(Vector2<Real> const& center, Vector2<Real> const axis[2],
            Vector2<Real> const& sqrExtent, std::array<Real, 5>& coeff)
        {
            Real denom0 = Dot(axis[0], axis[0]) * sqrExtent[0];
            Real denom1 = Dot(axis[1], axis[1]) * sqrExtent[1];
            Matrix2x2<Real> outer0 = OuterProduct(axis[0], axis[0]);
            Matrix2x2<Real> outer1 = OuterProduct(axis[1], axis[1]);
            Matrix2x2<Real> A = outer0 / denom0 + outer1 / denom1;
            Vector2<Real> product = A * center;
            Vector2<Real> B = -mTwo * product;
            Real const& denom = A(1, 1);
            coeff[0] = (Dot(center, product) - mOne) / denom;
            coeff[1] = B[0] / denom;
            coeff[2] = B[1] / denom;
            coeff[3] = A(0, 0) / denom;
            coeff[4] = mTwo * A(0, 1) / denom;
            // coeff[5] = A(1, 1) / denom = 1;
        }

        void DoRootFinding(Result& result)
        {
            bool allZero = true;
            for (int i = 0; i < 5; ++i)
            {
                mD[i] = mA[i] - mB[i];
                if (mD[i] != mZero)
                {
                    allZero = false;
                }
            }
            if (allZero)
            {
                result.intersect = false;
                result.numPoints = std::numeric_limits<int>::max();
                return;
            }

            result.numPoints = 0;

            mA2Div2 = mA[2] / mTwo;
            mA4Div2 = mA[4] / mTwo;
            mC[0] = mA[0] - mA2Div2 * mA2Div2;
            mC[1] = mA[1] - mA2Div2 * mA[4];
            mC[2] = mA[3] - mA4Div2 * mA4Div2;  // c[2] > 0
            mE[0] = mD[0] - mA2Div2 * mD[2];
            mE[1] = mD[1] - mA2Div2 * mD[4] - mA4Div2 * mD[2];
            mE[2] = mD[3] - mA4Div2 * mD[4];

            if (mD[4] != mZero)
            {
                Real xbar = -mD[2] / mD[4];
                Real ebar = mE[0] + xbar * (mE[1] + xbar * mE[2]);
                if (ebar != mZero)
                {
                    D4NotZeroEBarNotZero(result);
                }
                else
                {
                    D4NotZeroEBarZero(xbar, result);
                }
            }
            else if (mD[2] != mZero)  // d[4] = 0
            {
                if (mE[2] != mZero)
                {
                    D4ZeroD2NotZeroE2NotZero(result);
                }
                else
                {
                    D4ZeroD2NotZeroE2Zero(result);
                }
            }
            else  // d[2] = d[4] = 0
            {
                D4ZeroD2Zero(result);
            }

            result.intersect = (result.numPoints > 0);
        }

        void D4NotZeroEBarNotZero(Result& result)
        {
            // The graph of w = -e(x)/d(x) is a hyperbola.
            Real d2d2 = mD[2] * mD[2], d2d4 = mD[2] * mD[4], d4d4 = mD[4] * mD[4];
            Real e0e0 = mE[0] * mE[0], e0e1 = mE[0] * mE[1], e0e2 = mE[0] * mE[2];
            Real e1e1 = mE[1] * mE[1], e1e2 = mE[1] * mE[2], e2e2 = mE[2] * mE[2];
            std::array<Real, 5> f =
            {
                mC[0] * d2d2 + e0e0,
                mC[1] * d2d2 + mTwo * (mC[0] * d2d4 + e0e1),
                mC[2] * d2d2 + mC[0] * d4d4 + e1e1 + mTwo * (mC[1] * d2d4 + e0e2),
                mC[1] * d4d4 + mTwo * (mC[2] * d2d4 + e1e2),
                mC[2] * d4d4 + e2e2  // > 0
            };

            std::map<Real, int> rmMap;
            RootsPolynomial<Real>::template SolveQuartic<Real>(f[0], f[1], f[2],
                f[3], f[4], rmMap);

            // xbar cannot be a root of f(x), so d(x) != 0 and we can solve
            // directly for w = -e(x)/d(x).
            for (auto const& rm : rmMap)
            {
                Real const& x = rm.first;
                Real w = -(mE[0] + x * (mE[1] + x * mE[2])) / (mD[2] + mD[4] * x);
                Real y = w - (mA2Div2 + x * mA4Div2);
                result.points[result.numPoints] = { x, y };
                result.isTransverse[result.numPoints++] = (rm.second == 1);
            }
        }

        void D4NotZeroEBarZero(Real const& xbar, Result& result)
        {
            // Factor e(x) = (d2 + d4*x)*(h0 + h1*x).  The w-equation has
            // two solution components, x = xbar and w = -(h0 + h1*x).
            Real translate, w, y;

            // Compute intersection of x = xbar with ellipse.
            Real ncbar = -(mC[0] + xbar * (mC[1] + xbar * mC[2]));
            if (ncbar >= mZero)
            {
                translate = mA2Div2 + xbar * mA4Div2;
                w = std::sqrt(ncbar);
                y = w - translate;
                result.points[result.numPoints] = { xbar, y };
                if (w > mZero)
                {
                    result.isTransverse[result.numPoints++] = true;
                    w = -w;
                    y = w - translate;
                    result.points[result.numPoints] = { xbar, y };
                    result.isTransverse[result.numPoints++] = true;
                }
                else
                {
                    result.isTransverse[result.numPoints++] = false;
                }
            }

            // Compute intersections of w = -(h0 + h1*x) with ellipse.
            std::array<Real, 2> h;
            h[1] = mE[2] / mD[4];
            h[0] = (mE[1] - mD[2] * h[1]) / mD[4];
            std::array<Real, 3> f =
            {
                mC[0] + h[0] * h[0],
                mC[1] + mTwo * h[0] * h[1],
                mC[2] + h[1] * h[1]  // > 0
            };

            std::map<Real, int> rmMap;
            RootsPolynomial<Real>::template SolveQuadratic<Real>(f[0], f[1], f[2], rmMap);
            for (auto const& rm : rmMap)
            {
                Real const& x = rm.first;
                translate = mA2Div2 + x * mA4Div2;
                w = -(h[0] + x * h[1]);
                y = w - translate;
                result.points[result.numPoints] = { x, y };
                result.isTransverse[result.numPoints++] = (rm.second == 1);
            }
        }

        void D4ZeroD2NotZeroE2NotZero(Result& result)
        {
            Real d2d2 = mD[2] * mD[2];
            std::array<Real, 5> f =
            {
                mC[0] * d2d2 + mE[0] * mE[0],
                mC[1] * d2d2 + mTwo * mE[0] * mE[1],
                mC[2] * d2d2 + mE[1] * mE[1] + mTwo * mE[0] * mE[2],
                mTwo * mE[1] * mE[2],
                mE[2] * mE[2]  // > 0
            };

            std::map<Real, int> rmMap;
            RootsPolynomial<Real>::template SolveQuartic<Real>(f[0], f[1], f[2], f[3],
                f[4], rmMap);
            for (auto const& rm : rmMap)
            {
                Real const& x = rm.first;
                Real translate = mA2Div2 + x * mA4Div2;
                Real w = -(mE[0] + x * (mE[1] + x * mE[2])) / mD[2];
                Real y = w - translate;
                result.points[result.numPoints] = { x, y };
                result.isTransverse[result.numPoints++] = (rm.second == 1);
            }
        }

        void D4ZeroD2NotZeroE2Zero(Result& result)
        {
            Real d2d2 = mD[2] * mD[2];
            std::array<Real, 3> f =
            {
                mC[0] * d2d2 + mE[0] * mE[0],
                mC[1] * d2d2 + mTwo * mE[0] * mE[1],
                mC[2] * d2d2 + mE[1] * mE[1]
            };

            std::map<Real, int> rmMap;
            RootsPolynomial<Real>::template SolveQuadratic<Real>(f[0], f[1], f[2], rmMap);
            for (auto const& rm : rmMap)
            {
                Real const& x = rm.first;
                Real translate = mA2Div2 + x * mA4Div2;
                Real w = -(mE[0] + x * mE[1]) / mD[2];
                Real y = w - translate;
                result.points[result.numPoints] = { x, y };
                result.isTransverse[result.numPoints++] = (rm.second == 1);
            }
        }

        void D4ZeroD2Zero(Result& result)
        {
            // e(x) cannot be identically zero, because if it were, then all
            // d[i] = 0.  But we tested that case previously and exited.

            if (mE[2] != mZero)
            {
                // Make e(x) monic, f(x) = e(x)/e2 = x^2 + (e1/e2)*x + (e0/e2)
                // = x^2 + f1*x + f0.
                std::array<Real, 2> f = { mE[0] / mE[2], mE[1] / mE[2] };

                Real mid = -f[1] / mTwo;
                Real discr = mid * mid - f[0];
                if (discr > mZero)
                {
                    // The theoretical roots of e(x) are
                    // x = -f1/2 + s*sqrt(discr) where s in {-1,+1}.  For each
                    // root, determine exactly the sign of c(x).  We need
                    // c(x) <= 0 in order to solve for w^2 = -c(x).  At the
                    // root,
                    //  c(x) = c0 + c1*x + c2*x^2 = c0 + c1*x - c2*(f0 + f1*x)
                    //       = (c0 - c2*f0) + (c1 - c2*f1)*x
                    //       = g0 + g1*x
                    // We need g0 + g1*x <= 0.
                    Real sqrtDiscr = std::sqrt(discr);
                    std::array<Real, 2> g =
                    {
                        mC[0] - mC[2] * f[0],
                        mC[1] - mC[2] * f[1]
                    };

                    if (g[1] > mZero)
                    {
                        // We need s*sqrt(discr) <= -g[0]/g[1] + f1/2.
                        Real r = -g[0] / g[1] - mid;

                        // s = +1:
                        if (r >= mZero)
                        {
                            Real rsqr = r * r;
                            if (discr < rsqr)
                            {
                                SpecialIntersection(mid + sqrtDiscr, true, result);
                            }
                            else if (discr == rsqr)
                            {
                                SpecialIntersection(mid + sqrtDiscr, false, result);
                            }
                        }

                        // s = -1:
                        if (r > mZero)
                        {
                            SpecialIntersection(mid - sqrtDiscr, true, result);
                        }
                        else
                        {
                            Real rsqr = r * r;
                            if (discr > rsqr)
                            {
                                SpecialIntersection(mid - sqrtDiscr, true, result);
                            }
                            else if (discr == rsqr)
                            {
                                SpecialIntersection(mid - sqrtDiscr, false, result);
                            }
                        }
                    }
                    else if (g[1] < mZero)
                    {
                        // We need s*sqrt(discr) >= -g[0]/g[1] + f1/2.
                        Real r = -g[0] / g[1] - mid;

                        // s = -1:
                        if (r <= mZero)
                        {
                            Real rsqr = r * r;
                            if (discr < rsqr)
                            {
                                SpecialIntersection(mid - sqrtDiscr, true, result);
                            }
                            else
                            {
                                SpecialIntersection(mid - sqrtDiscr, false, result);
                            }
                        }

                        // s = +1:
                        if (r < mZero)
                        {
                            SpecialIntersection(mid + sqrtDiscr, true, result);
                        }
                        else
                        {
                            Real rsqr = r * r;
                            if (discr > rsqr)
                            {
                                SpecialIntersection(mid + sqrtDiscr, true, result);
                            }
                            else if (discr == rsqr)
                            {
                                SpecialIntersection(mid + sqrtDiscr, false, result);
                            }
                        }
                    }
                    else  // g[1] = 0
                    {
                        // The graphs of c(x) and f(x) are parabolas of the
                        // same shape.  One is a vertical translation of the
                        // other.
                        if (g[0] < mZero)
                        {
                            // The graph of f(x) is above that of c(x).
                            SpecialIntersection(mid - sqrtDiscr, true, result);
                            SpecialIntersection(mid + sqrtDiscr, true, result);
                        }
                        else if (g[0] == mZero)
                        {
                            // The graphs of c(x) and f(x) are the same parabola.
                            SpecialIntersection(mid - sqrtDiscr, false, result);
                            SpecialIntersection(mid + sqrtDiscr, false, result);
                        }
                    }
                }
                else if (discr == mZero)
                {
                    // The theoretical root of f(x) is x = -f1/2.
                    Real nchat = -(mC[0] + mid * (mC[1] + mid * mC[2]));
                    if (nchat > mZero)
                    {
                        SpecialIntersection(mid, true, result);
                    }
                    else if (nchat == mZero)
                    {
                        SpecialIntersection(mid, false, result);
                    }
                }
            }
            else if (mE[1] != mZero)
            {
                Real xhat = -mE[0] / mE[1];
                Real nchat = -(mC[0] + xhat * (mC[1] + xhat * mC[2]));
                if (nchat > mZero)
                {
                    SpecialIntersection(xhat, true, result);
                }
                else if (nchat == mZero)
                {
                    SpecialIntersection(xhat, false, result);
                }
            }
        }

        // Helper function for D4ZeroD2Zero.
        void SpecialIntersection(Real const& x, bool transverse, Result& result)
        {
            if (transverse)
            {
                Real translate = mA2Div2 + x * mA4Div2;
                Real nc = -(mC[0] + x * (mC[1] + x * mC[2]));
                if (nc < mZero)
                {
                    // Clamp to eliminate the rounding error, but duplicate
                    // the point because we know that it is a transverse
                    // intersection.
                    nc = mZero;
                }

                Real w = std::sqrt(nc);
                Real y = w - translate;
                result.points[result.numPoints] = { x, y };
                result.isTransverse[result.numPoints++] = true;
                w = -w;
                y = w - translate;
                result.points[result.numPoints] = { x, y };
                result.isTransverse[result.numPoints++] = true;
            }
            else
            {
                // The vertical line at the root is tangent to the ellipse.
                Real y = -(mA2Div2 + x * mA4Div2);  // w = 0
                result.points[result.numPoints] = { x, y };
                result.isTransverse[result.numPoints++] = false;
            }
        }


        // Helper functions for AreaOfIntersection.
        struct EllipseInfo
        {
            Vector2<Real> center;
            std::array<Vector2<Real>, 2> axis;
            Vector2<Real> extent, sqrExtent;
            Matrix2x2<Real> M;
            Real AB;        // extent[0] * extent[1]
            Real halfAB;    // extent[0] * extent[1] / 2
            Real BpA;       // extent[1] + extent[0]
            Real BmA;       // extent[1] - extent[0]
        };

        // The axis, extent and sqrExtent members of E must be set before
        // this function is called.
        void FinishEllipseInfo(EllipseInfo& E)
        {
            E.M = OuterProduct(E.axis[0], E.axis[0]) /
                (E.sqrExtent[0] * Dot(E.axis[0], E.axis[0]));
            E.M += OuterProduct(E.axis[1], E.axis[1]) /
                (E.sqrExtent[1] * Dot(E.axis[1], E.axis[1]));
            E.AB = E.extent[0] * E.extent[1];
            E.halfAB = E.AB / mTwo;
            E.BpA = E.extent[1] + E.extent[0];
            E.BmA = E.extent[1] - E.extent[0];
        }

        void AreaDispatch(EllipseInfo const& E0, EllipseInfo const& E1, AreaResult& ar)
        {
            if (ar.findResult.intersect)
            {
                if (ar.findResult.numPoints == 1)
                {
                    // Containment or separation.
                    AreaCS(E0, E1, ar);
                }
                else if (ar.findResult.numPoints == 2)
                {
                    if (ar.findResult.isTransverse[0])
                    {
                        // Both intersection points are transverse.
                        Area2(E0, E1, 0, 1, ar);
                    }
                    else
                    {
                        // Both intersection points are tangential, so one
                        // ellipse is contained in the other.
                        AreaCS(E0, E1, ar);
                    }
                }
                else if (ar.findResult.numPoints == 3)
                {
                    // The tangential intersection is irrelevant in the area
                    // computation.
                    if (!ar.findResult.isTransverse[0])
                    {
                        Area2(E0, E1, 1, 2, ar);
                    }
                    else if (!ar.findResult.isTransverse[1])
                    {
                        Area2(E0, E1, 2, 0, ar);
                    }
                    else  // ar.findResult.isTransverse[2] == false
                    {
                        Area2(E0, E1, 0, 1, ar);
                    }
                }
                else  // ar.findResult.numPoints == 4
                {
                    Area4(E0, E1, ar);
                }
            }
            else
            {
                // Containment, separation, or same ellipse.
                AreaCS(E0, E1, ar);
            }
        }

        void AreaCS(EllipseInfo const& E0, EllipseInfo const& E1, AreaResult& ar)
        {
            if (ar.findResult.numPoints <= 1)
            {
                Vector2<Real> diff = E0.center - E1.center;
                Real qform0 = Dot(diff, E0.M * diff);
                Real qform1 = Dot(diff, E1.M * diff);
                if (qform0 > mOne && qform1 > mOne)
                {
                    // Each ellipse center is outside the other ellipse, so
                    // the ellipses are separated (numPoints == 0) or outside
                    // each other and just touching (numPoints == 1).
                    ar.configuration = AreaResult::ELLIPSES_ARE_SEPARATED;
                    ar.area = mZero;
                }
                else
                {
                    // One ellipse is inside the other.  Determine this
                    // indirectly by comparing areas.
                    if (E0.AB < E1.AB)
                    {
                        ar.configuration = AreaResult::E1_CONTAINS_E0;
                        ar.area = mPi * E0.AB;
                    }
                    else
                    {
                        ar.configuration = AreaResult::E0_CONTAINS_E1;
                        ar.area = mPi * E1.AB;
                    }
                }
            }
            else
            {
                ar.configuration = AreaResult::ELLIPSES_ARE_EQUAL;
                ar.area = mPi * E0.AB;
            }
        }

        void Area2(EllipseInfo const& E0, EllipseInfo const& E1, int i0, int i1, AreaResult& ar)
        {
            ar.configuration = AreaResult::ONE_CHORD_REGION;

            // The endpoints of the chord.
            Vector2<Real> const& P0 = ar.findResult.points[i0];
            Vector2<Real> const& P1 = ar.findResult.points[i1];

            // Compute locations relative to the ellipses.
            Vector2<Real> P0mC0 = P0 - E0.center, P0mC1 = P0 - E1.center;
            Vector2<Real> P1mC0 = P1 - E0.center, P1mC1 = P1 - E1.center;

            // Compute the ellipse normal vectors at endpoint P0.  This is
            // sufficient information to determine chord endpoint order.
            Vector2<Real> N0 = E0.M * P0mC0, N1 = E1.M * P0mC1;
            Real dotperp = DotPerp(N1, N0);

            // Choose the endpoint order for the chord region associated
            // with E0.
            if (dotperp > mZero)
            {
                // The chord order for E0 is <P0,P1> and for E1 is <P1,P0>.
                ar.area =
                    ComputeAreaChordRegion(E0, P0mC0, P1mC0) +
                    ComputeAreaChordRegion(E1, P1mC1, P0mC1);
            }
            else
            {
                // The chord order for E0 is <P1,P0> and for E1 is <P0,P1>.
                ar.area =
                    ComputeAreaChordRegion(E0, P1mC0, P0mC0) +
                    ComputeAreaChordRegion(E1, P0mC1, P1mC1);
            }
        }

        void Area4(EllipseInfo const& E0, EllipseInfo const& E1, AreaResult& ar)
        {
            ar.configuration = AreaResult::FOUR_CHORD_REGION;

            // Select a counterclockwise ordering of the points of
            // intersection.  Use the polar coordinates for E0 to do this.
            // The multimap is used in the event that computing the
            // intersections involved numerical rounding errors that lead to
            // a duplicate intersection, even though the intersections are all
            // labeled as transverse.  [See the comment in the function
            // SpecialIntersection.]
            std::multimap<Real, int> ordering;
            int i;
            for (i = 0; i < 4; ++i)
            {
                Vector2<Real> PmC = ar.findResult.points[i] - E0.center;
                Real x = Dot(E0.axis[0], PmC);
                Real y = Dot(E0.axis[1], PmC);
                Real theta = std::atan2(y, x);
                ordering.insert(std::make_pair(theta, i));
            }

            std::array<int, 4> permute;
            i = 0;
            for (auto const& element : ordering)
            {
                permute[i++] = element.second;
            }

            // Start with the area of the convex quadrilateral.
            Vector2<Real> diag20 =
                ar.findResult.points[permute[2]] - ar.findResult.points[permute[0]];
            Vector2<Real> diag31 =
                ar.findResult.points[permute[3]] - ar.findResult.points[permute[1]];
            ar.area = std::fabs(DotPerp(diag20, diag31)) / mTwo;

            // Visit each pair of consecutive points.  The selection of
            // ellipse for the chord-region area calculation uses the "most
            // counterclockwise" tangent vector.
            for (int i0 = 3, i1 = 0; i1 < 4; i0 = i1++)
            {
                // Get a pair of consecutive points.
                Vector2<Real> const& P0 = ar.findResult.points[permute[i0]];
                Vector2<Real> const& P1 = ar.findResult.points[permute[i1]];

                // Compute locations relative to the ellipses.
                Vector2<Real> P0mC0 = P0 - E0.center, P0mC1 = P0 - E1.center;
                Vector2<Real> P1mC0 = P1 - E0.center, P1mC1 = P1 - E1.center;

                // Compute the ellipse normal vectors at endpoint P0.
                Vector2<Real> N0 = E0.M * P0mC0, N1 = E1.M * P0mC1;
                Real dotperp = DotPerp(N1, N0);
                if (dotperp > mZero)
                {
                    // The chord goes with ellipse E0.
                    ar.area += ComputeAreaChordRegion(E0, P0mC0, P1mC0);
                }
                else
                {
                    // The chord goes with ellipse E1.
                    ar.area += ComputeAreaChordRegion(E1, P0mC1, P1mC1);
                }
            }
        }

        Real ComputeAreaChordRegion(EllipseInfo const& E, Vector2<Real> const& P0mC, Vector2<Real> const& P1mC)
        {
            // Compute polar coordinates for P0 and P1 on the ellipse.
            Real x0 = Dot(E.axis[0], P0mC);
            Real y0 = Dot(E.axis[1], P0mC);
            Real theta0 = std::atan2(y0, x0);
            Real x1 = Dot(E.axis[0], P1mC);
            Real y1 = Dot(E.axis[1], P1mC);
            Real theta1 = std::atan2(y1, x1);

            // The arc straddles the atan2 discontinuity on the negative
            // x-axis.  Wrap the second angle to be larger than the first
            // angle.
            if (theta1 < theta0)
            {
                theta1 += mTwoPi;
            }

            // Compute the area portion of the sector due to the triangle.
            Real triArea = std::fabs(DotPerp(P0mC, P1mC)) / mTwo;

            // Compute the chord region area.
            Real dtheta = theta1 - theta0;
            Real F0, F1, sectorArea;
            if (dtheta <= mPi)
            {
                // Use the area formula directly.
                // area(theta0,theta1) = F(theta1)-F(theta0)-area(triangle)
                F0 = ComputeIntegral(E, theta0);
                F1 = ComputeIntegral(E, theta1);
                sectorArea = F1 - F0;
                return sectorArea - triArea;
            }
            else
            {
                // The angle of the elliptical sector is larger than pi
                // radians.  Use the area formula
                //   area(theta0,theta1) = pi*a*b - area(theta1,theta0)
                theta0 += mTwoPi;  // ensure theta0 > theta1
                F0 = ComputeIntegral(E, theta0);
                F1 = ComputeIntegral(E, theta1);
                sectorArea = F0 - F1;
                return mPi * E.AB - (sectorArea - triArea);
            }
        }

        Real ComputeIntegral(EllipseInfo const& E, Real const& theta)
        {
            Real twoTheta = mTwo * theta;
            Real sn = std::sin(twoTheta);
            Real cs = std::cos(twoTheta);
            Real arg = E.BmA * sn / (E.BpA + E.BmA * cs);
            return E.halfAB * (theta - std::atan(arg));
        }

        // Constants that are set up once (optimization for rational
        // arithmetic).
        Real mZero, mOne, mTwo, mPi, mTwoPi;

        // Various polynomial coefficients.  The short names are meant to
        // match the variable names used in the PDF documentation.
        std::array<Real, 5> mA, mB, mD, mF;
        std::array<Real, 3> mC, mE;
        Real mA2Div2, mA4Div2;
    };

    // Template aliases for convenience.
    template <typename Real>
    using TIEllipses2 = TIQuery<Real, Ellipse2<Real>, Ellipse2<Real>>;

    template <typename Real>
    using FIEllipses2 = FIQuery<Real, Ellipse2<Real>, Ellipse2<Real>>;
}
