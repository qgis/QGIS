// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <algorithm>
#include <initializer_list>
#include <vector>

namespace gte
{
    template <typename Real>
    class Polynomial1
    {
    public:
        // Construction and destruction.  The first constructor creates a
        // polynomial of the specified degree but sets all coefficients to
        // zero (to ensure initialization).  You are responsible for setting
        // the coefficients, presumably with the degree-term set to a nonzero
        // number.  In the second constructor, the degree is the number of
        // initializers plus 1, but then adjusted so that coefficient[degree]
        // is not zero (unless all initializer values are zero).
        Polynomial1(unsigned int degree = 0)
            :
            mCoefficient(degree + 1, (Real)0)
        {
        }

        Polynomial1(std::initializer_list<Real> values)
        {
            // C++ 11 will call the default constructor for
            // Polynomial1<Real> p{}, so it is guaranteed that
            // values.size() > 0.
            mCoefficient.resize(values.size());
            std::copy(values.begin(), values.end(), mCoefficient.begin());
            EliminateLeadingZeros();
        }

        // Support for partial construction, where the default constructor is
        // used when the degree is not yet known.  The coefficients are
        // uninitialized.
        void SetDegree(unsigned int degree)
        {
            mCoefficient.resize(degree + 1);
        }

        // Set all coefficients to the specified value.
        void SetCoefficients(Real value)
        {
            std::fill(mCoefficient.begin(), mCoefficient.end(), value);
        }

        // Member access.
        inline unsigned int GetDegree() const
        {
            // By design, mCoefficient.size() > 0.
            return static_cast<unsigned int>(mCoefficient.size() - 1);
        }

        inline Real const& operator[](unsigned int i) const
        {
            return mCoefficient[i];
        }

        inline Real& operator[](unsigned int i)
        {
            return mCoefficient[i];
        }


        // Comparisons.
        inline bool operator==(Polynomial1<Real> const& p) const
        {
            return mCoefficient == p.mCoefficient;
        }

        inline bool operator!=(Polynomial1<Real> const& p) const
        {
            return mCoefficient != p.mCoefficient;
        }

        inline bool operator< (Polynomial1<Real> const& p) const
        {
            return mCoefficient < p.mCoefficient;
        }

        inline bool operator<=(Polynomial1<Real> const& p) const
        {
            return mCoefficient <= p.mCoefficient;
        }

        inline bool operator> (Polynomial1<Real> const& p) const
        {
            return mCoefficient > p.mCoefficient;
        }

        inline bool operator>=(Polynomial1<Real> const& p) const
        {
            return mCoefficient >= p.mCoefficient;
        }

        // Evaluate the polynomial.  If the polynomial is invalid, the
        // function returns zero.
        Real operator()(Real t) const
        {
            int i = static_cast<int>(mCoefficient.size());
            Real result = mCoefficient[--i];
            for (--i; i >= 0; --i)
            {
                result *= t;
                result += mCoefficient[i];
            }
            return result;
        }

        // Compute the derivative of the polynomial.
        Polynomial1 GetDerivative() const
        {
            unsigned int const degree = GetDegree();
            if (degree > 0)
            {
                Polynomial1 result(degree - 1);
                for (unsigned int i0 = 0, i1 = 1; i0 < degree; ++i0, ++i1)
                {
                    result.mCoefficient[i0] = mCoefficient[i1] * (Real)i1;
                }
                return result;
            }
            else
            {
                Polynomial1 result(0);
                result[0] = (Real)0;
                return result;
            }
        }

        // Inversion (invpoly[i] = poly[degree-i] for 0 <= i <= degree).
        Polynomial1 GetInversion() const
        {
            unsigned int const degree = GetDegree();
            Polynomial1 result(degree);
            for (unsigned int i = 0; i <= degree; ++i)
            {
                result.mCoefficient[i] = mCoefficient[degree - i];
            }
            return result;
        }

        // Tranlation.  If 'this' is p(t}, return p(t-t0).
        Polynomial1 GetTranslation(Real t0) const
        {
            Polynomial1<Real> factor{ -t0, (Real)1 };  // f(t) = t - t0
            unsigned int const degree = GetDegree();
            Polynomial1 result{ mCoefficient[degree] };
            for (unsigned int i = 1, j = degree - 1; i <= degree; ++i, --j)
            {
                result = mCoefficient[j] + factor * result;
            }
            return result;
        }

        // Eliminate any leading zeros in the polynomial, except in the case
        // the degree is 0 and the coefficient is 0.  The elimination is
        // necessary when arithmetic operations cause a decrease in the degree
        // of the result.  For example, (1 + x + x^2) + (1 + 2*x - x^2) =
        // (2 + 3*x).  The inputs both have degree 2, so the result is created
        // with degree 2.  After the addition we find that the degree is in
        // fact 1 and resize the array of coefficients.  This function is
        // called internally by the arithmetic operators, but it is exposed in
        // the public interface in case you need it for your own purposes.
        void EliminateLeadingZeros()
        {
            size_t size = mCoefficient.size();
            if (size > 1)
            {
                Real const zero = (Real)0;
                int leading;
                for (leading = static_cast<int>(size) - 1; leading > 0; --leading)
                {
                    if (mCoefficient[leading] != zero)
                    {
                        break;
                    }
                }

                mCoefficient.resize(++leading);
            }
        }

        // If 'this' is P(t) and the divisor is D(t) with
        // degree(P) >= degree(D), then P(t) = Q(t)*D(t)+R(t) where Q(t) is
        // the quotient with degree(Q) = degree(P) - degree(D) and R(t) is the
        // remainder with degree(R) < degree(D).  If this routine is called
        // with degree(P) < degree(D), then Q = 0 and R = P are returned.
        void Divide(Polynomial1 const& divisor, Polynomial1& quotient, Polynomial1& remainder) const
        {
            Real const zero = (Real)0;
            int divisorDegree = static_cast<int>(divisor.GetDegree());
            int quotientDegree = static_cast<int>(GetDegree()) - divisorDegree;
            if (quotientDegree >= 0)
            {
                quotient.SetDegree(quotientDegree);

                // Temporary storage for the remainder.
                Polynomial1 tmp = *this;

                // Do the division using the Euclidean algorithm.
                Real inv = ((Real)1) / divisor[divisorDegree];
                for (int i = quotientDegree; i >= 0; --i)
                {
                    int j = divisorDegree + i;
                    quotient[i] = inv * tmp[j];
                    for (j--; j >= i; j--)
                    {
                        tmp[j] -= quotient[i] * divisor[j - i];
                    }
                }

                // Calculate the correct degree for the remainder.
                if (divisorDegree >= 1)
                {
                    int remainderDegree = divisorDegree - 1;
                    while (remainderDegree > 0 && tmp[remainderDegree] == zero)
                    {
                        --remainderDegree;
                    }

                    remainder.SetDegree(remainderDegree);
                    for (int i = 0; i <= remainderDegree; ++i)
                    {
                        remainder[i] = tmp[i];
                    }
                }
                else
                {
                    remainder.SetDegree(0);
                    remainder[0] = zero;
                }
            }
            else
            {
                quotient.SetDegree(0);
                quotient[0] = zero;
                remainder = *this;
            }
        }

        // Scale the polynomial so the highest-degree term has coefficient 1.
        void MakeMonic()
        {
            EliminateLeadingZeros();
            Real const one(1);
            if (mCoefficient.back() != one)
            {
                unsigned int degree = GetDegree();
                Real invLeading = one / mCoefficient.back();
                mCoefficient.back() = one;
                for (unsigned int i = 0; i < degree; ++i)
                {
                    mCoefficient[i] *= invLeading;
                }
            }
        }

    protected:
        // The class is designed so that mCoefficient.size() >= 1.
        std::vector<Real> mCoefficient;
    };

    // Compute the greatest common divisor of two polynomials.  The returned
    // polynomial has leading coefficient 1 (except when zero-valued
    // polynomials are passed to the function.
    template <typename Real>
    Polynomial1<Real> GreatestCommonDivisor(Polynomial1<Real> const& p0, Polynomial1<Real> const& p1)
    {
        // The numerator should be the polynomial of larger degree.
        Polynomial1<Real> a, b;
        if (p0.GetDegree() >= p1.GetDegree())
        {
            a = p0;
            b = p1;
        }
        else
        {
            a = p1;
            b = p0;
        }

        Polynomial1<Real> const zero{ (Real)0 };
        if (a == zero || b == zero)
        {
            return (a != zero ? a : zero);
        }

        // Make the polynomials monic to keep the coefficients reasonable size
        // when computing with floating-point Real.
        a.MakeMonic();
        b.MakeMonic();

        Polynomial1<Real> q, r;
        for (;;)
        {
            a.Divide(b, q, r);
            if (r != zero)
            {
                // a = q * b + r, so gcd(a,b) = gcd(b, r)
                a = b;
                b = r;
                b.MakeMonic();
            }
            else
            {
                b.MakeMonic();
                break;
            }
        }

        return b;
    }

    // Factor f = factor[0]*factor[1]^2*factor[2]^3*...*factor[n-1]^n
    // according to the square-free factorization algorithm
    // https://en.wikipedia.org/wiki/Square-free_polynomial
    template <typename Real>
    void SquareFreeFactorization(Polynomial1<Real> const& f, std::vector<Polynomial1<Real>>& factors)
    {
        // In the call to Divide(...), we know that the divisor exactly
        // divides the numerator, so r = 0 after all such calls.
        Polynomial1<Real> fder = f.GetDerivative();
        Polynomial1<Real> a, b, c, d, q, r;

        a = GreatestCommonDivisor(f, fder);
        f.Divide(a, b, r);  // b = f / a
        fder.Divide(a, c, r);  // c = fder / a
        d = c - b.GetDerivative();

        do
        {
            a = GreatestCommonDivisor(b, d);
            factors.emplace_back(a);
            b.Divide(a, q, r);  // q = b / a
            b = std::move(q);
            d.Divide(a, c, r);  // c = d / a
            d = c - b.GetDerivative();
        } while (b.GetDegree() > 0);
    }

    // Unary operations.
    template <typename Real>
    Polynomial1<Real> operator+(Polynomial1<Real> const& p)
    {
        return p;
    }

    template <typename Real>
    Polynomial1<Real> operator-(Polynomial1<Real> const& p)
    {
        unsigned int const degree = p.GetDegree();
        Polynomial1<Real> result(degree);
        for (unsigned int i = 0; i <= degree; ++i)
        {
            result[i] = -p[i];
        }
        return result;
    }

    // Linear-algebraic operations.
    template <typename Real>
    Polynomial1<Real> operator+(Polynomial1<Real> const& p0, Polynomial1<Real> const& p1)
    {
        unsigned int const p0Degree = p0.GetDegree(), p1Degree = p1.GetDegree();
        unsigned int i;
        if (p0Degree >= p1Degree)
        {
            Polynomial1<Real> result(p0Degree);
            for (i = 0; i <= p1Degree; ++i)
            {
                result[i] = p0[i] + p1[i];
            }
            for (/**/; i <= p0Degree; ++i)
            {
                result[i] = p0[i];
            }
            result.EliminateLeadingZeros();
            return result;
        }
        else
        {
            Polynomial1<Real> result(p1Degree);
            for (i = 0; i <= p0Degree; ++i)
            {
                result[i] = p0[i] + p1[i];
            }
            for (/**/; i <= p1Degree; ++i)
            {
                result[i] = p1[i];
            }
            result.EliminateLeadingZeros();
            return result;
        }
    }

    template <typename Real>
    Polynomial1<Real> operator-(Polynomial1<Real> const& p0, Polynomial1<Real> const& p1)
    {
        unsigned int const p0Degree = p0.GetDegree(), p1Degree = p1.GetDegree();
        unsigned int i;
        if (p0Degree >= p1Degree)
        {
            Polynomial1<Real> result(p0Degree);
            for (i = 0; i <= p1Degree; ++i)
            {
                result[i] = p0[i] - p1[i];
            }
            for (/**/; i <= p0Degree; ++i)
            {
                result[i] = p0[i];
            }
            result.EliminateLeadingZeros();
            return result;
        }
        else
        {
            Polynomial1<Real> result(p1Degree);
            for (i = 0; i <= p0Degree; ++i)
            {
                result[i] = p0[i] - p1[i];
            }
            for (/**/; i <= p1Degree; ++i)
            {
                result[i] = -p1[i];
            }
            result.EliminateLeadingZeros();
            return result;
        }
    }

    template <typename Real>
    Polynomial1<Real> operator*(Polynomial1<Real> const& p0, Polynomial1<Real> const& p1)
    {
        unsigned int const p0Degree = p0.GetDegree(), p1Degree = p1.GetDegree();
        Polynomial1<Real> result(p0Degree + p1Degree);
        result.SetCoefficients((Real)0);
        for (unsigned int i0 = 0; i0 <= p0Degree; ++i0)
        {
            for (unsigned int i1 = 0; i1 <= p1Degree; ++i1)
            {
                result[i0 + i1] += p0[i0] * p1[i1];
            }
        }
        return result;
    }

    template <typename Real>
    Polynomial1<Real> operator+(Polynomial1<Real> const& p, Real scalar)
    {
        unsigned int const degree = p.GetDegree();
        Polynomial1<Real> result(degree);
        result[0] = p[0] + scalar;
        for (unsigned int i = 1; i <= degree; ++i)
        {
            result[i] = p[i];
        }
        return result;
    }

    template <typename Real>
    Polynomial1<Real> operator+(Real scalar, Polynomial1<Real> const& p)
    {
        unsigned int const degree = p.GetDegree();
        Polynomial1<Real> result(degree);
        result[0] = p[0] + scalar;
        for (unsigned int i = 1; i <= degree; ++i)
        {
            result[i] = p[i];
        }
        return result;
    }

    template <typename Real>
    Polynomial1<Real> operator-(Polynomial1<Real> const& p, Real scalar)
    {
        unsigned int const degree = p.GetDegree();
        Polynomial1<Real> result(degree);
        result[0] = p[0] - scalar;
        for (unsigned int i = 1; i <= degree; ++i)
        {
            result[i] = p[i];
        }
        return result;
    }

    template <typename Real>
    Polynomial1<Real> operator-(Real scalar, Polynomial1<Real> const& p)
    {
        unsigned int const degree = p.GetDegree();
        Polynomial1<Real> result(degree);
        result[0] = scalar - p[0];
        for (unsigned int i = 1; i <= degree; ++i)
        {
            result[i] = -p[i];
        }
        return result;
    }

    template <typename Real>
    Polynomial1<Real> operator*(Polynomial1<Real> const& p, Real scalar)
    {
        unsigned int const degree = p.GetDegree();
        Polynomial1<Real> result(degree);
        for (unsigned int i = 0; i <= degree; ++i)
        {
            result[i] = scalar * p[i];
        }
        return result;
    }

    template <typename Real>
    Polynomial1<Real> operator*(Real scalar, Polynomial1<Real> const& p)
    {
        unsigned int const degree = p.GetDegree();
        Polynomial1<Real> result(degree);
        for (unsigned int i = 0; i <= degree; ++i)
        {
            result[i] = scalar * p[i];
        }
        return result;
    }

    template <typename Real>
    Polynomial1<Real> operator/(Polynomial1<Real> const& p, Real scalar)
    {
        LogAssert(scalar != (Real)0, "Division by zero.");

        unsigned int const degree = p.GetDegree();
        Real invScalar = (Real)1 / scalar;
        Polynomial1<Real> result(degree);
        for (unsigned int i = 0; i <= degree; ++i)
        {
            result[i] = invScalar * p[i];
        }
        return result;
    }

    template <typename Real>
    Polynomial1<Real>& operator+=(Polynomial1<Real>& p0, Polynomial1<Real> const& p1)
    {
        p0 = p0 + p1;
        return p0;
    }

    template <typename Real>
    Polynomial1<Real>& operator-=(Polynomial1<Real>& p0, Polynomial1<Real> const& p1)
    {
        p0 = p0 - p1;
        return p0;
    }

    template <typename Real>
    Polynomial1<Real>& operator*=(Polynomial1<Real>& p0, Polynomial1<Real> const& p1)
    {
        p0 = p0 * p1;
        return p0;
    }

    template <typename Real>
    Polynomial1<Real>& operator+=(Polynomial1<Real>& p, Real scalar)
    {
        p[0] += scalar;
        return p;
    }

    template <typename Real>
    Polynomial1<Real>& operator-=(Polynomial1<Real>& p, Real scalar)
    {
        p[0] -= scalar;
        return p;
    }

    template <typename Real>
    Polynomial1<Real>& operator*=(Polynomial1<Real>& p, Real scalar)
    {
        p = p * scalar;
        return p;
    }

    template <typename Real>
    Polynomial1<Real> & operator/=(Polynomial1<Real>& p, Real scalar)
    {
        p = p / scalar;
        return p;
    }
}
