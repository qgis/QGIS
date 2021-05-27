// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.10

#pragma once

#include <Mathematics/Logger.h>

// Class QFNumber is an implementation for quadratic fields with N >= 1
// square root terms. The theory and details are provided in
// https://www.geometrictools.com/Documentation/QuadraticFields.pdf

// Enable this macro if you want the logging system to trap when arithmetic
// operations are performed on two quadratic elements that do not share the
// same value d
#define GTE_ASSERT_ON_QFNUMBER_MISMATCHED_D

namespace gte
{
    // Arithmetic for quadratic fields with N >= 2 square root terms. The
    // d-term is rational and the x-coefficients are elements in a quadratic
    // field with N-1 >= 1 square root terms.
    template <typename T, size_t N>
    class QFNumber
    {
    public:
        // The quadratic field numbers is x[0] + x[1] * sqrt(d).
        std::array<QFNumber<T, N - 1>, 2> x;
        T d;

        // Create z = 0 + 0 * sqrt(0), where the 0 coefficients are quadratic
        // field elements with N-1 d-terms all set to 0 and x-coefficients all
        // set to 0.
        QFNumber()
            :
            d(0)
        {
            static_assert(N >= 2, "Invalid number of root arguments.");
        }

        // Create z = 0 + 0 * sqrt(d), where the 0 coefficients are quadratic
        // field elements with N-1 d-terms all set to 0 and x-coefficients all
        // set to 0.
        explicit QFNumber(T const& inD)
            :
            d(inD)
        {
            static_assert(N >= 2, "Invalid number of root arguments.");
        }

        // Create z = x0 + x1 * sqrt(d), where the x-coefficients are
        // quadratic field elements with N-1 d-terms.
        QFNumber(QFNumber<T, N - 1> const& x0, QFNumber<T, N - 1> const& x1, T const& inD)
            :
            x{ x0, x1 },
            d(inD)
        {
            static_assert(N >= 2, "Invalid number of root arguments.");
        }

        // Create z = inX[0] + inX[1] * sqrt(inD), where the x-coefficients are
        // quadratic field elements with N-1 d-terms.
        QFNumber(std::array<QFNumber<T, N - 1>, 2> const& inX, T const& inD)
            :
            x(inX),
            d(inD)
        {
            static_assert(N >= 2, "Invalid number of root arguments.");
        }
    };

    // Arithmetic for quadratic fields with 1 square root term.
    template <typename T>
    class QFNumber<T, 1>
    {
    public:
        // The quadratic field number is x[0] + x[1] * sqrt(d).
        std::array<T, 2> x;
        T d;

        // Create z = 0. You can defer the setting of d until later.
        QFNumber()
            :
            x{ static_cast<T>(0), static_cast<T>(0) },
            d(static_cast<T>(0))
        {
        }

        // Create z = 0 + 0 * sqrt(d) = 0.
        explicit QFNumber(T const& inD)
            :
            x{ static_cast<T>(0), static_cast<T>(0) },
            d(inD)
        {
        }

        // Create z = x0 + x1 * sqrt(d).
        QFNumber(T const& x0, T const& x1, T const& inD)
            :
            x{ x0, x1 },
            d(inD)
        {
        }

        // Create z = inX[0] + inX[1] * sqrt(d).
        QFNumber(std::array<T, 2> const& inX, T const& inD)
            :
            x(inX),
            d(inD)
        {
        }
    };

    // Unary operations.
    template <typename T, size_t N>
    QFNumber<T, N> operator+(QFNumber<T, N> const& q)
    {
        static_assert(N >= 1, "Invalid number of d-terms.");
        return q;
    }

    template <typename T, size_t N>
    QFNumber<T, N> operator-(QFNumber<T, N> const& q)
    {
        static_assert(N >= 1, "Invalid number of d-terms.");
        return QFNumber<T, N>(-q.x[0], -q.x[1], q.d);
    }

    // Arithmetic operations between elements of a quadratic field must occur
    // only when the d-values are the same. To trap mismatches, read the
    // comments at the beginning of this file.
    template <typename T, size_t N>
    QFNumber<T, N> operator+(QFNumber<T, N> const& q0, QFNumber<T, N> const& q1)
    {
#if defined(GTE_ASSERT_ON_QFNUMBER_MISMATCHED_D)
        LogAssert(q0.d == q1.d, "Mismatched d-value.");
#endif
        return QFNumber<T, N>(q0.x[0] + q1.x[0], q0.x[1] + q1.x[1], q0.d);
    }

    template <typename T, size_t N>
    QFNumber<T, N> operator+(QFNumber<T, N> const& q, T const& s)
    {
        return QFNumber<T, N>(q.x[0] + s, q.x[1], q.d);
    }

    template <typename T, size_t N>
    QFNumber<T, N> operator+(T const& s, QFNumber<T, N> const& q)
    {
        return QFNumber<T, N>(s + q.x[0], q.x[1], q.d);
    }

    template <typename T, size_t N>
    QFNumber<T, N> operator-(QFNumber<T, N> const& q0, QFNumber<T, N> const& q1)
    {
#if defined(GTE_ASSERT_ON_QFNUMBER_MISMATCHED_D)
        LogAssert(q0.d == q1.d, "Mismatched d-value.");
#endif
        return QFNumber<T, N>(q0.x[0] - q1.x[0], q0.x[1] - q1.x[1], q0.d);
    }

    template <typename T, size_t N>
    QFNumber<T, N> operator-(QFNumber<T, N> const& q, T const& s)
    {
        return QFNumber<T, N>(q.x[0] - s, q.x[1], q.d);
    }

    template <typename T, size_t N>
    QFNumber<T, N> operator-(T const& s, QFNumber<T, N> const& q)
    {
        return QFNumber<T, N>(s - q.x[0], -q.x[1], q.d);
    }

    template <typename T, size_t N>
    QFNumber<T, N> operator*(QFNumber<T, N> const& q0, QFNumber<T, N> const& q1)
    {
#if defined(GTE_ASSERT_ON_QFNUMBER_MISMATCHED_D)
        LogAssert(q0.d == q1.d, "Mismatched d-value.");
#endif
        return QFNumber<T, N>(
            q0.x[0] * q1.x[0] + q0.x[1] * q1.x[1] * q0.d,
            q0.x[0] * q1.x[1] + q0.x[1] * q1.x[0],
            q0.d);
    }

    template <typename T, size_t N>
    QFNumber<T, N> operator*(QFNumber<T, N> const& q, T const& s)
    {
        return QFNumber<T, N>(q.x[0] * s, q.x[1] * s, q.d);
    }

    template <typename T, size_t N>
    QFNumber<T, N> operator*(T const& s, QFNumber<T, N> const& q)
    {
        return QFNumber<T, N>(s * q.x[0], s * q.x[1], q.d);
    }

    template <typename T, size_t N>
    QFNumber<T, N> operator/(QFNumber<T, N> const& q0, QFNumber<T, N> const& q1)
    {
#if defined(GTE_ASSERT_ON_QFNUMBER_MISMATCHED_D)
        LogAssert(q0.d == q1.d, "Mismatched d-value.");
#endif
        auto denom = q1.x[0] * q1.x[0] - q1.x[1] * q1.x[1] * q0.d;
        auto numer0 = q0.x[0] * q1.x[0] - q0.x[1] * q1.x[1] * q0.d;
        auto numer1 = q0.x[1] * q1.x[0] - q0.x[0] * q1.x[1];
        return QFNumber<T, N>(numer0 / denom, numer1 / denom, q0.d);
    }

    template <typename T, size_t N>
    QFNumber<T, N> operator/(QFNumber<T, N> const& q, T const& s)
    {
        return QFNumber<T, N>(q.x[0] / s, q.x[1] / s, q.d);
    }

    template <typename T, size_t N>
    QFNumber<T, N> operator/(T const& s, QFNumber<T, N> const& q)
    {
        auto denom = q.x[0] * q.x[0] - q.x[1] * q.x[1] * q.d;
        auto x0 = (s * q.x[0]) / denom;
        auto x1 = -(s * q.x[1]) / denom;
        return QFNumber<T, N>(x0, x1, q.d);
    }

    // Arithmetic updates between elements of a quadratic field must occur
    // only when the d-values are the same. To trap mismatches, read the
    // comments at the beginning of this file.
    template <typename T, size_t N>
    QFNumber<T, N>& operator+=(QFNumber<T, N>& q0, QFNumber<T, N> const& q1)
    {
#if defined(GTE_ASSERT_ON_QFNUMBER_MISMATCHED_D)
        LogAssert(q0.d == q1.d, "Mismatched d-value.");
#endif
        q0.x[0] += q1.x[0];
        q0.x[1] += q1.x[1];
        return q0;
    }

    template <typename T, size_t N>
    QFNumber<T, N>& operator+=(QFNumber<T, N>& q, T const& s)
    {
        q.x[0] += s;
        return q;
    }

    template <typename T, size_t N>
    QFNumber<T, N>& operator-=(QFNumber<T, N>& q0, QFNumber<T, N> const& q1)
    {
#if defined(GTE_ASSERT_ON_QFNUMBER_MISMATCHED_D)
        LogAssert(q0.d == q1.d, "Mismatched d-value.");
#endif
        q0.x[0] -= q1.x[0];
        q0.x[1] -= q1.x[1];
        return q0;
    }

    template <typename T, size_t N>
    QFNumber<T, N>& operator-=(QFNumber<T, N>& q, T const& s)
    {
        q.x[0] -= s;
        return q;
    }

    template <typename T, size_t N>
    QFNumber<T, N>& operator*=(QFNumber<T, N>& q0, QFNumber<T, N> const& q1)
    {
#if defined(GTE_ASSERT_ON_QFNUMBER_MISMATCHED_D)
        LogAssert(q0.d == q1.d, "Mismatched d-value.");
#endif
        auto x0 = q0.x[0] * q1.x[0] + q0.x[1] * q1.x[1] * q0.d;
        auto x1 = q0.x[0] * q1.x[1] + q0.x[1] * q1.x[0];
        q0.x[0] = x0;
        q0.x[1] = x1;
        return q0;
    }

    template <typename T, size_t N>
    QFNumber<T, N>& operator*=(QFNumber<T, N>& q, T const& s)
    {
        q.x[0] *= s;
        q.x[1] *= s;
        return q;
    }

    template <typename T, size_t N>
    QFNumber<T, N>& operator/=(QFNumber<T, N>& q0, QFNumber<T, N> const& q1)
    {
#if defined(GTE_ASSERT_ON_QFNUMBER_MISMATCHED_D)
        LogAssert(q0.d == q1.d, "Mismatched d-value.");
#endif
        auto denom = q1.x[0] * q1.x[0] - q1.x[1] * q1.x[1] * q0.d;
        auto numer0 = q0.x[0] * q1.x[0] - q0.x[1] * q1.x[1] * q0.d;
        auto numer1 = q0.x[1] * q1.x[0] - q0.x[0] * q1.x[1];
        q0.x[0] = numer0 / denom;
        q0.x[1] = numer1 / denom;
        return q0;
    }

    template <typename T, size_t N>
    QFNumber<T, N>& operator/=(QFNumber<T, N>& q, T const& s)
    {
        q.x[0] /= s;
        q.x[1] /= s;
        return q;
    }

    // Comparisons between numbers of a quadratic field must occur only when
    // the d-values are the same. To trap mismatches, read the comments at
    // the beginning of this file.
    template <typename T, size_t N>
    bool operator==(QFNumber<T, N> const& q0, QFNumber<T, N> const& q1)
    {
#if defined(GTE_ASSERT_ON_QFNUMBER_MISMATCHED_D)
        LogAssert(q0.d == q1.d, "Mismatched d-value.");
#endif
        if (q0.d == T(0) || q0.x[1] == q1.x[1])
        {
            return q0.x[0] == q1.x[0];
        }
        else if (q0.x[1] > q1.x[1])
        {
            if (q0.x[0] >= q1.x[0])
            {
                return false;
            }
            else // q0.x[0] < q1.x[0]
            {
                auto diff = q0 - q1;
                return diff.x[0] * diff.x[0] == diff.x[1] * diff.x[1] * diff.d;
            }
        }
        else // q0.x[1] < q1.x[1]
        {
            if (q0.x[0] <= q1.x[0])
            {
                return false;
            }
            else // q0.x[0] > q1.x[0]
            {
                auto diff = q0 - q1;
                return diff.x[0] * diff.x[0] == diff.x[1] * diff.x[1] * diff.d;
            }
        }
    }

    template <typename T, size_t N>
    bool operator!=(QFNumber<T, N> const& q0, QFNumber<T, N> const& q1)
    {
        return !operator==(q0, q1);
    }

    template <typename T, size_t N>
    bool operator<(QFNumber<T, N> const& q0, QFNumber<T, N> const& q1)
    {
#if defined(GTE_ASSERT_ON_QFNUMBER_MISMATCHED_D)
        LogAssert(q0.d == q1.d, "Mismatched d-value.");
#endif
        if (q0.d == T(0) || q0.x[1] == q1.x[1])
        {
            return q0.x[0] < q1.x[0];
        }
        else if (q0.x[1] > q1.x[1])
        {
            if (q0.x[0] >= q1.x[0])
            {
                return false;
            }
            else // q0.x[0] < q1.x[0]
            {
                auto diff = q0 - q1;
                return diff.x[0] * diff.x[0] > diff.x[1] * diff.x[1] * diff.d;
            }
        }
        else // q0.x[1] < q1.x[1]
        {
            if (q0.x[0] <= q1.x[0])
            {
                return true;
            }
            else // q0.x[0] > q1.x[0]
            {
                auto diff = q0 - q1;
                return diff.x[0] * diff.x[0] < diff.x[1] * diff.x[1] * diff.d;
            }
        }
    }

    template <typename T, size_t N>
    bool operator>(QFNumber<T, N> const& q0, QFNumber<T, N> const& q1)
    {
        return operator<(q1, q0);
    }

    template <typename T, size_t N>
    bool operator<=(QFNumber<T, N> const& q0, QFNumber<T, N> const& q1)
    {
        return !operator<(q1, q0);
    }

    template <typename T, size_t N>
    bool operator>=(QFNumber<T, N> const& q0, QFNumber<T, N> const& q1)
    {
        return !operator<(q0, q1);
    }
}
