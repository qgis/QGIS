// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 5.5.2021.01.14

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Math.h>
#include <array>

// The SWInterval [e0,e1] must satisfy e0 <= e1. Expose this define to trap
// invalid construction where e0 > e1.
#define GTE_THROW_ON_INVALID_SWINTERVAL

namespace gte
{
    // The T must be 'float' or 'double'.
    template <typename T>
    class SWInterval
    {
    public:
        // Convenient constants.
        static T constexpr zero = 0;
        static T constexpr one = 1;
        static T constexpr max = std::numeric_limits<T>::max();
        static T constexpr inf = std::numeric_limits<T>::infinity();

        // Construction. This is the only way to create an interval. All such
        // intervals are immutable once created. The constructor SWInterval(T)
        // is used to create the degenerate interval [e,e].
        SWInterval()
            :
            mEndpoints{ static_cast<T>(0), static_cast<T>(0) }
        {
            static_assert(std::is_floating_point<T>::value, "Invalid type.");
        }

        SWInterval(SWInterval const& other)
            :
            mEndpoints(other.mEndpoints)
        {
            static_assert(std::is_floating_point<T>::value, "Invalid type.");
        }

        SWInterval(T e)
            :
            mEndpoints{ e, e }
        {
            static_assert(std::is_floating_point<T>::value, "Invalid type.");
        }

        SWInterval(T e0, T e1)
            :
            mEndpoints{ e0, e1 }
        {
            static_assert(std::is_floating_point<T>::value, "Invalid type.");
#if defined(GTE_THROW_ON_INVALID_SWINTERVAL)
            LogAssert(mEndpoints[0] <= mEndpoints[1], "Invalid SWInterval.");
#endif
        }

        SWInterval(std::array<T, 2> const& endpoint)
            :
            mEndpoints(endpoint)
        {
            static_assert(std::is_floating_point<T>::value, "Invalid type.");
#if defined(GTE_THROW_ON_INVALID_SWINTERVAL)
            LogAssert(mEndpoints[0] <= mEndpoints[1], "Invalid SWInterval.");
#endif
        }

        SWInterval& operator=(SWInterval const& other)
        {
            static_assert(std::is_floating_point<T>::value, "Invalid type.");
            mEndpoints = other.mEndpoints;
            return *this;
        }

        // Member access. It is only possible to read the endpoints. You
        // cannot modify the endpoints outside the arithmetic operations.
        inline T operator[](size_t i) const
        {
            return mEndpoints[i];
        }

        inline std::array<T, 2> GetEndpoints() const
        {
            return mEndpoints;
        }

        // Arithmetic operations to compute intervals at the leaf nodes of
        // an expression tree. Such nodes correspond to the raw floating-point
        // variables of the expression. The non-class operators defined after
        // the class definition are used to compute intervals at the interior
        // nodes of the expression tree.
        inline static SWInterval Add(T u, T v)
        {
            SWInterval w;
            T add = u + v;
            w.mEndpoints[0] = std::nextafter(add, -max);
            w.mEndpoints[1] = std::nextafter(add, +max);
            return w;
        }

        inline static SWInterval Sub(T u, T v)
        {
            SWInterval w;
            T sub = u - v;
            w.mEndpoints[0] = std::nextafter(sub, -max);
            w.mEndpoints[1] = std::nextafter(sub, +max);
            return w;
        }

        inline static SWInterval Mul(T u, T v)
        {
            SWInterval w;
            T mul = u * v;
            w.mEndpoints[0] = std::nextafter(mul, -max);
            w.mEndpoints[1] = std::nextafter(mul, +max);
            return w;
        }

        inline static SWInterval Div(T u, T v)
        {
            if (v != zero)
            {
                SWInterval w;
                T div = u / v;
                w.mEndpoints[0] = std::nextafter(div, -max);
                w.mEndpoints[1] = std::nextafter(div, +max);
                return w;
            }
            else
            {
                // Division by zero does not lead to a determinate SWInterval.
                // Return the entire set of real numbers.
                return Reals();
            }
        }

    private:
        std::array<T, 2> mEndpoints;

    public:
        // FOR INTERNAL USE ONLY. These are used by the non-class operators
        // defined after the class definition.
        inline static SWInterval Add(T u0, T u1, T v0, T v1)
        {
            SWInterval w;
            w.mEndpoints[0] = std::nextafter(u0 + v0, -max);
            w.mEndpoints[1] = std::nextafter(u1 + v1, +max);
            return w;
        }

        inline static SWInterval Sub(T u0, T u1, T v0, T v1)
        {
            SWInterval w;
            w.mEndpoints[0] = std::nextafter(u0 - v1, -max);
            w.mEndpoints[1] = std::nextafter(u1 - v0, +max);
            return w;
        }

        inline static SWInterval Mul(T u0, T u1, T v0, T v1)
        {
            SWInterval w;
            w.mEndpoints[0] = std::nextafter(u0 * v0, -max);
            w.mEndpoints[1] = std::nextafter(u1 * v1, +max);
            return w;
        }

        inline static SWInterval Mul2(T u0, T u1, T v0, T v1)
        {
            T u0mv1 = std::nextafter(u0 * v1, -max);
            T u1mv0 = std::nextafter(u1 * v0, -max);
            T u0mv0 = std::nextafter(u0 * v0, +max);
            T u1mv1 = std::nextafter(u1 * v1, +max);
            return SWInterval<T>(std::min(u0mv1, u1mv0), std::max(u0mv0, u1mv1));
        }

        inline static SWInterval Div(T u0, T u1, T v0, T v1)
        {
            SWInterval w;
            w.mEndpoints[0] = std::nextafter(u0 / v1, -max);
            w.mEndpoints[1] = std::nextafter(u1 / v0, +max);
            return w;
        }

        inline static SWInterval Reciprocal(T v0, T v1)
        {
            SWInterval w;
            w.mEndpoints[0] = std::nextafter(one / v1, -max);
            w.mEndpoints[1] = std::nextafter(one / v0, +max);
            return w;
        }

        inline static SWInterval ReciprocalDown(T v)
        {
            T recpv = std::nextafter(one / v, -max);
            return SWInterval<T>(recpv, +inf);
        }

        inline static SWInterval ReciprocalUp(T v)
        {
            T recpv = std::nextafter(one / v, +max);
            return SWInterval<T>(-inf, recpv);
        }

        inline static SWInterval Reals()
        {
            return SWInterval(-inf, +inf);
        }
    };

    // Unary operations. Negation of [e0,e1] produces [-e1,-e0]. This
    // operation needs to be supported in the sense of negating a
    // "number" in an arithmetic expression.
    template <typename T>
    SWInterval<T> operator+(SWInterval<T> const& u)
    {
        return u;
    }

    template <typename T>
    SWInterval<T> operator-(SWInterval<T> const& u)
    {
        return SWInterval<T>(-u[1], -u[0]);
    }

    // Addition operations.
    template <typename T>
    SWInterval<T> operator+(T u, SWInterval<T> const& v)
    {
        return SWInterval<T>::Add(u, u, v[0], v[1]);
    }

    template <typename T>
    SWInterval<T> operator+(SWInterval<T> const& u, T v)
    {
        return SWInterval<T>::Add(u[0], u[1], v, v);
    }

    template <typename T>
    SWInterval<T> operator+(SWInterval<T> const& u, SWInterval<T> const& v)
    {
        return SWInterval<T>::Add(u[0], u[1], v[0], v[1]);
    }

    template <typename T>
    SWInterval<T>& operator+=(SWInterval<T>& u, T v)
    {
        u = u + v;
        return u;
    }

    template <typename T>
    SWInterval<T>& operator+=(SWInterval<T>& u, SWInterval<T> const& v)
    {
        u = u + v;
        return u;
    }

    // Subtraction operations.
    template <typename T>
    SWInterval<T> operator-(T u, SWInterval<T> const& v)
    {
        return SWInterval<T>::Sub(u, u, v[0], v[1]);
    }

    template <typename T>
    SWInterval<T> operator-(SWInterval<T> const& u, T v)
    {
        return SWInterval<T>::Sub(u[0], u[1], v, v);
    }

    template <typename T>
    SWInterval<T> operator-(SWInterval<T> const& u, SWInterval<T> const& v)
    {
        return SWInterval<T>::Sub(u[0], u[1], v[0], v[1]);
    }

    template <typename T>
    SWInterval<T>& operator-=(SWInterval<T>& u, T v)
    {
        u = u - v;
        return u;
    }

    template <typename T>
    SWInterval<T>& operator-=(SWInterval<T>& u, SWInterval<T> const& v)
    {
        u = u - v;
        return u;
    }

    // Multiplication operations.
    template <typename T>
    SWInterval<T> operator*(T u, SWInterval<T> const& v)
    {
        if (u >= SWInterval<T>::zero)
        {
            return SWInterval<T>::Mul(u, u, v[0], v[1]);
        }
        else
        {
            return SWInterval<T>::Mul(u, u, v[1], v[0]);
        }
    }

    template <typename T>
    SWInterval<T> operator*(SWInterval<T> const& u, T v)
    {
        if (v >= SWInterval<T>::zero)
        {
            return SWInterval<T>::Mul(u[0], u[1], v, v);
        }
        else
        {
            return SWInterval<T>::Mul(u[1], u[0], v, v);
        }
    }

    template <typename T>
    SWInterval<T> operator*(SWInterval<T> const& u, SWInterval<T> const& v)
    {
        if (u[0] >= SWInterval<T>::zero)
        {
            if (v[0] >= SWInterval<T>::zero)
            {
                return SWInterval<T>::Mul(u[0], u[1], v[0], v[1]);
            }
            else if (v[1] <= SWInterval<T>::zero)
            {
                return SWInterval<T>::Mul(u[1], u[0], v[0], v[1]);
            }
            else // v[0] < 0 < v[1]
            {
                return SWInterval<T>::Mul(u[1], u[1], v[0], v[1]);
            }
        }
        else if (u[1] <= SWInterval<T>::zero)
        {
            if (v[0] >= SWInterval<T>::zero)
            {
                return SWInterval<T>::Mul(u[0], u[1], v[1], v[0]);
            }
            else if (v[1] <= SWInterval<T>::zero)
            {
                return SWInterval<T>::Mul(u[1], u[0], v[1], v[0]);
            }
            else // v[0] < 0 < v[1]
            {
                return SWInterval<T>::Mul(u[0], u[0], v[1], v[0]);
            }
        }
        else // u[0] < 0 < u[1]
        {
            if (v[0] >= SWInterval<T>::zero)
            {
                return SWInterval<T>::Mul(u[0], u[1], v[1], v[1]);
            }
            else if (v[1] <= SWInterval<T>::zero)
            {
                return SWInterval<T>::Mul(u[1], u[0], v[0], v[0]);
            }
            else // v[0] < 0 < v[1]
            {
                return SWInterval<T>::Mul2(u[0], u[1], v[0], v[1]);
            }
        }
    }

    template <typename T>
    SWInterval<T>& operator*=(SWInterval<T>& u, T v)
    {
        u = u * v;
        return u;
    }

    template <typename T>
    SWInterval<T>& operator*=(SWInterval<T>& u, SWInterval<T> const& v)
    {
        u = u * v;
        return u;
    }

    // Division operations. If the divisor SWInterval is [v0,v1] with
    // v0 < 0 < v1, then the returned SWInterval is (-inf,+inf) instead of
    // Union((-inf,1/v0),(1/v1,+inf)). An application should try to avoid
    // this case by branching based on [v0,0] and [0,v1].
    template <typename T>
    SWInterval<T> operator/(T u, SWInterval<T> const& v)
    {
        if (v[0] > SWInterval<T>::zero || v[1] < SWInterval<T>::zero)
        {
            return u * SWInterval<T>::Reciprocal(v[0], v[1]);
        }
        else
        {
            if (v[0] == SWInterval<T>::zero)
            {
                return u * SWInterval<T>::ReciprocalDown(v[1]);
            }
            else if (v[1] == SWInterval<T>::zero)
            {
                return u * SWInterval<T>::ReciprocalUp(v[0]);
            }
            else // v[0] < 0 < v[1]
            {
                return SWInterval<T>::Reals();
            }
        }
    }

    template <typename T>
    SWInterval<T> operator/(SWInterval<T> const& u, T v)
    {
        if (v > SWInterval<T>::zero)
        {
            return SWInterval<T>::Div(u[0], u[1], v, v);
        }
        else if (v < SWInterval<T>::zero)
        {
            return SWInterval<T>::Div(u[1], u[0], v, v);
        }
        else // v = 0
        {
            return SWInterval<T>::Reals();
        }
    }

    template <typename T>
    SWInterval<T> operator/(SWInterval<T> const& u, SWInterval<T> const& v)
    {
        if (v[0] > SWInterval<T>::zero || v[1] < SWInterval<T>::zero)
        {
            return u * SWInterval<T>::Reciprocal(v[0], v[1]);
        }
        else
        {
            if (v[0] == SWInterval<T>::zero)
            {
                return u * SWInterval<T>::ReciprocalDown(v[1]);
            }
            else if (v[1] == SWInterval<T>::zero)
            {
                return u * SWInterval<T>::ReciprocalUp(v[0]);
            }
            else // v[0] < 0 < v[1]
            {
                return SWInterval<T>::Reals();
            }
        }
    }

    template <typename T>
    SWInterval<T>& operator/=(SWInterval<T>& u, T v)
    {
        u = u / v;
        return u;
    }
    template <typename T>
    SWInterval<T>& operator/=(SWInterval<T>& u, SWInterval<T> const& v)
    {
        u = u / v;
        return u;
    }
}
