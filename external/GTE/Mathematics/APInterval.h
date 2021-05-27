// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.1.2020.09.08

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/ArbitraryPrecision.h>

// The interval [e0,e1] must satisfy e0 <= e1. Expose this define to trap
// invalid construction where e0 > e1.
#define GTE_THROW_ON_INVALID_APINTERVAL

namespace gte
{
    // The APType must be an arbitrary-precision type.
    template <typename APType>
    class APInterval
    {
    public:
        // Construction. This is the only way to create an interval. All such
        // intervals are immutable once created. The constructor
        // APInterval(APType) is used to create the degenerate interval [e,e].
        APInterval()
            :
            mEndpoints{ static_cast<APType>(0), static_cast<APType>(0) }
        {
            static_assert(is_arbitrary_precision<APType>::value, "Invalid type.");
        }

        APInterval(APInterval const& other)
            :
            mEndpoints(other.mEndpoints)
        {
            static_assert(is_arbitrary_precision<APType>::value, "Invalid type.");
        }

        explicit APInterval(APType e)
            :
            mEndpoints{ e, e }
        {
            static_assert(is_arbitrary_precision<APType>::value, "Invalid type.");
        }

        APInterval(APType e0, APType e1)
            :
            mEndpoints{ e0, e1 }
        {
            static_assert(is_arbitrary_precision<APType>::value, "Invalid type.");
#if defined(GTE_THROW_ON_INVALID_APINTERVAL)
            LogAssert(mEndpoints[0] <= mEndpoints[1], "Invalid interval.");
#endif
        }

        APInterval(std::array<APType, 2> const& endpoint)
            :
            mEndpoints(endpoint)
        {
            static_assert(is_arbitrary_precision<APType>::value, "Invalid type.");
#if defined(GTE_THROW_ON_INVALID_APINTERVAL)
            LogAssert(mEndpoints[0] <= mEndpoints[1], "Invalid interval.");
#endif
        }

        APInterval& operator=(APInterval const& other)
        {
            static_assert(is_arbitrary_precision<APType>::value, "Invalid type.");
            mEndpoints = other.mEndpoints;
            return *this;
        }

        // Member access. It is only possible to read the endpoints. You
        // cannot modify the endpoints outside the arithmetic operations.
        inline APType operator[](size_t i) const
        {
            return mEndpoints[i];
        }

        inline std::array<APType, 2> GetEndpoints() const
        {
            return mEndpoints;
        }

        // Arithmetic operations to compute intervals at the leaf nodes of
        // an expression tree. Such nodes correspond to the raw floating-point
        // variables of the expression. The non-class operators defined after
        // the class definition are used to compute intervals at the interior
        // nodes of the expression tree.
        inline static APInterval Add(APType u, APType v)
        {
            APInterval w;
            w.mEndpoints[0] = u + v;
            w.mEndpoints[1] = w.mEndpoints[0];
            return w;
        }

        inline static APInterval Sub(APType u, APType v)
        {
            APInterval w;
            w.mEndpoints[0] = u - v;
            w.mEndpoints[1] = w.mEndpoints[0];
            return w;
        }

        inline static APInterval Mul(APType u, APType v)
        {
            APInterval w;
            w.mEndpoints[0] = u * v;
            w.mEndpoints[1] = w.mEndpoints[0];
            return w;
        }

        template <typename Dummy = APType>
        inline static
        typename std::enable_if<has_division_operator<Dummy>::value, APInterval>::type
        Div(APType u, APType v)
        {
            APType const zero = static_cast<APType>(0);
            if (v != zero)
            {
                APInterval w;
                w.mEndpoints[0] = u / v;
                w.mEndpoints[1] = w.mEndpoints[0];
                return w;
            }
            else
            {
                // Division by zero does not lead to a determinate interval.
                // Just return the entire set of real numbers.
                return Reals();
            }
        }

    private:
        std::array<APType, 2> mEndpoints;

    public:
        // FOR INTERNAL USE ONLY. These are used by the non-class operators
        // defined after the class definition.
        inline static APInterval Add(APType u0, APType u1, APType v0, APType v1)
        {
            APInterval w;
            w.mEndpoints[0] = u0 + v0;
            w.mEndpoints[1] = u1 + v1;
            return w;
        }

        inline static APInterval Sub(APType u0, APType u1, APType v0, APType v1)
        {
            APInterval w;
            w.mEndpoints[0] = u0 - v1;
            w.mEndpoints[1] = u1 - v0;
            return w;
        }

        inline static APInterval Mul(APType u0, APType u1, APType v0, APType v1)
        {
            APInterval w;
            w.mEndpoints[0] = u0 * v0;
            w.mEndpoints[1] = u1 * v1;
            return w;
        }

        inline static APInterval Mul2(APType u0, APType u1, APType v0, APType v1)
        {
            APType u0mv1 = u0 * v1;
            APType u1mv0 = u1 * v0;
            APType u0mv0 = u0 * v0;
            APType u1mv1 = u1 * v1;
            return APInterval<APType>(std::min(u0mv1, u1mv0), std::max(u0mv0, u1mv1));
        }

        template <typename Dummy = APType>
        inline static
        typename std::enable_if<has_division_operator<Dummy>::value, APInterval>::type
        Div(APType u0, APType u1, APType v0, APType v1)
        {
            APInterval w;
            w.mEndpoints[0] = u0 / v1;
            w.mEndpoints[1] = u1 / v0;
            return w;
        }

        template <typename Dummy = APType>
        inline static
        typename std::enable_if<has_division_operator<Dummy>::value, APInterval>::type
        Reciprocal(APType v0, APType v1)
        {
            APType const one = static_cast<APType>(1);
            APInterval w;
            w.mEndpoints[0] = one / v1;
            w.mEndpoints[1] = one / v0;
            return w;
        }

        template <typename Dummy = APType>
        inline static
        typename std::enable_if<has_division_operator<Dummy>::value, APInterval>::type
        ReciprocalDown(APType v)
        {
            APType recpv = static_cast<APType>(1) / v;
            APType posinf(0);
            posinf.SetSign(+2);
            return APInterval<APType>(recpv, posinf);
        }

        template <typename Dummy = APType>
        inline static
        typename std::enable_if<has_division_operator<Dummy>::value, APInterval>::type
        ReciprocalUp(APType v)
        {
            APType recpv = static_cast<APType>(1) / v;
            APType neginf(0);
            neginf.SetSign(-2);
            return APInterval<APType>(neginf, recpv);
        }

        inline static APInterval Reals()
        {
            APType posinf(0), neginf(0);
            posinf.SetSign(+2);
            neginf.SetSign(-2);
            return APInterval(neginf, posinf);
        }
    };

    // Unary operations. Negation of [e0,e1] produces [-e1,-e0]. This
    // operation needs to be supported in the sense of negating a
    // "number" in an arithmetic expression.
    template <typename APType>
    APInterval<APType> operator+(APInterval<APType> const& u)
    {
        return u;
    }

    template <typename APType>
    APInterval<APType> operator-(APInterval<APType> const& u)
    {
        return APInterval<APType>(-u[1], -u[0]);
    }

    // Addition operations.
    template <typename APType>
    APInterval<APType> operator+(APType const& u, APInterval<APType> const& v)
    {
        return APInterval<APType>::Add(u, u, v[0], v[1]);
    }

    template <typename APType>
    APInterval<APType> operator+(APInterval<APType> const& u, APType const& v)
    {
        return APInterval<APType>::Add(u[0], u[1], v, v);
    }

    template <typename APType>
    APInterval<APType> operator+(APInterval<APType> const& u, APInterval<APType> const& v)
    {
        return APInterval<APType>::Add(u[0], u[1], v[0], v[1]);
    }

    template <typename APType>
    APInterval<APType>& operator+=(APInterval<APType>& u, APType const& v)
    {
        u = u + v;
        return u;
    }

    template <typename APType>
    APInterval<APType>& operator+=(APInterval<APType>& u, APInterval<APType> const& v)
    {
        u = u + v;
        return u;
    }

    // Subtraction operations.
    template <typename APType>
    APInterval<APType> operator-(APType const& u, APInterval<APType> const& v)
    {
        return APInterval<APType>::Sub(u, u, v[0], v[1]);
    }

    template <typename APType>
    APInterval<APType> operator-(APInterval<APType> const& u, APType const& v)
    {
        return APInterval<APType>::Sub(u[0], u[1], v, v);
    }

    template <typename APType>
    APInterval<APType> operator-(APInterval<APType> const& u, APInterval<APType> const& v)
    {
        return APInterval<APType>::Sub(u[0], u[1], v[0], v[1]);
    }

    template <typename APType>
    APInterval<APType>& operator-=(APInterval<APType>& u, APType const& v)
    {
        u = u - v;
        return u;
    }

    template <typename APType>
    APInterval<APType>& operator-=(APInterval<APType>& u, APInterval<APType> const& v)
    {
        u = u - v;
        return u;
    }

    // Multiplication operations.
    template <typename APType>
    APInterval<APType> operator*(APType const& u, APInterval<APType> const& v)
    {
        APType const zero = static_cast<APType>(0);
        if (u >= zero)
        {
            return APInterval<APType>::Mul(u, u, v[0], v[1]);
        }
        else
        {
            return APInterval<APType>::Mul(u, u, v[1], v[0]);
        }
    }

    template <typename APType>
    APInterval<APType> operator*(APInterval<APType> const& u, APType const& v)
    {
        APType const zero = static_cast<APType>(0);
        if (v >= zero)
        {
            return APInterval<APType>::Mul(u[0], u[1], v, v);
        }
        else
        {
            return APInterval<APType>::Mul(u[1], u[0], v, v);
        }
    }

    template <typename APType>
    APInterval<APType> operator*(APInterval<APType> const& u, APInterval<APType> const& v)
    {
        APType const zero = static_cast<APType>(0);
        if (u[0] >= zero)
        {
            if (v[0] >= zero)
            {
                return APInterval<APType>::Mul(u[0], u[1], v[0], v[1]);
            }
            else if (v[1] <= zero)
            {
                return APInterval<APType>::Mul(u[1], u[0], v[0], v[1]);
            }
            else // v[0] < 0 < v[1]
            {
                return APInterval<APType>::Mul(u[1], u[1], v[0], v[1]);
            }
        }
        else if (u[1] <= zero)
        {
            if (v[0] >= zero)
            {
                return APInterval<APType>::Mul(u[0], u[1], v[1], v[0]);
            }
            else if (v[1] <= zero)
            {
                return APInterval<APType>::Mul(u[1], u[0], v[1], v[0]);
            }
            else // v[0] < 0 < v[1]
            {
                return APInterval<APType>::Mul(u[0], u[0], v[1], v[0]);
            }
        }
        else // u[0] < 0 < u[1]
        {
            if (v[0] >= zero)
            {
                return APInterval<APType>::Mul(u[0], u[1], v[1], v[1]);
            }
            else if (v[1] <= zero)
            {
                return APInterval<APType>::Mul(u[1], u[0], v[0], v[0]);
            }
            else // v[0] < 0 < v[1]
            {
                return APInterval<APType>::Mul2(u[0], u[1], v[0], v[1]);
            }
        }
    }

    template <typename APType>
    APInterval<APType>& operator*=(APInterval<APType>& u, APType const& v)
    {
        u = u * v;
        return u;
    }

    template <typename APType>
    APInterval<APType>& operator*=(APInterval<APType>& u, APInterval<APType> const& v)
    {
        u = u * v;
        return u;
    }

    // Division operations. If the divisor interval is [v0,v1] with
    // v0 < 0 < v1, then the returned interval is (-infinity,+infinity)
    // instead of Union((-infinity,1/v0),(1/v1,+infinity)). An application
    // should try to avoid this case by branching based on [v0,0] and [0,v1].
    template <typename APType>
    APInterval<APType> operator/(APType const& u, APInterval<APType> const& v)
    {
        APType const zero = static_cast<APType>(0);
        if (v[0] > zero || v[1] < zero)
        {
            return u * APInterval<APType>::Reciprocal(v[0], v[1]);
        }
        else
        {
            if (v[0] == zero)
            {
                return u * APInterval<APType>::ReciprocalDown(v[1]);
            }
            else if (v[1] == zero)
            {
                return u * APInterval<APType>::ReciprocalUp(v[0]);
            }
            else // v[0] < 0 < v[1]
            {
                return APInterval<APType>::Reals();
            }
        }
    }

    template <typename APType>
    APInterval<APType> operator/(APInterval<APType> const& u, APType const& v)
    {
        APType const zero = static_cast<APType>(0);
        if (v > zero)
        {
            return APInterval<APType>::Div(u[0], u[1], v, v);
        }
        else if (v < zero)
        {
            return APInterval<APType>::Div(u[1], u[0], v, v);
        }
        else // v = 0
        {
            return APInterval<APType>::Reals();
        }
    }

    template <typename APType>
    APInterval<APType> operator/(APInterval<APType> const& u, APInterval<APType> const& v)
    {
        APType const zero = static_cast<APType>(0);
        if (v[0] > zero || v[1] < zero)
        {
            return u * APInterval<APType>::Reciprocal(v[0], v[1]);
        }
        else
        {
            if (v[0] == zero)
            {
                return u * APInterval<APType>::ReciprocalDown(v[1]);
            }
            else if (v[1] == zero)
            {
                return u * APInterval<APType>::ReciprocalUp(v[0]);
            }
            else // v[0] < 0 < v[1]
            {
                return APInterval<APType>::Reals();
            }
        }
    }

    template <typename APType>
    APInterval<APType>& operator/=(APInterval<APType>& u, APType const& v)
    {
        u = u / v;
        return u;
    }

    template <typename APType>
    APInterval<APType>& operator/=(APInterval<APType>& u, APInterval<APType> const& v)
    {
        u = u / v;
        return u;
    }
}
