// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.1.2020.11.16

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Math.h>
#include <array>

// The FPInterval [e0,e1] must satisfy e0 <= e1. Expose this define to trap
// invalid construction where e0 > e1.
#define GTE_THROW_ON_INVALID_INTERVAL

namespace gte
{
    // The FPType must be 'float' or 'double'.
    template <typename FPType>
    class FPInterval
    {
    public:
        // Construction. This is the only way to create an interval. All such
        // intervals are immutable once created. The constructor
        // FPInterval(FPType) is used to create the degenerate interval [e,e].
        FPInterval()
            :
            mEndpoints{ static_cast<FPType>(0), static_cast<FPType>(0) }
        {
            static_assert(std::is_floating_point<FPType>::value, "Invalid type.");
        }

        FPInterval(FPInterval const& other)
            :
            mEndpoints(other.mEndpoints)
        {
            static_assert(std::is_floating_point<FPType>::value, "Invalid type.");
        }

        explicit FPInterval(FPType e)
            :
            mEndpoints{ e, e }
        {
            static_assert(std::is_floating_point<FPType>::value, "Invalid type.");
        }

        FPInterval(FPType e0, FPType e1)
            :
            mEndpoints{ e0, e1 }
        {
            static_assert(std::is_floating_point<FPType>::value, "Invalid type.");
#if defined(GTE_THROW_ON_INVALID_INTERVAL)
            LogAssert(mEndpoints[0] <= mEndpoints[1], "Invalid FPInterval.");
#endif
        }

        FPInterval(std::array<FPType, 2> const& endpoint)
            :
            mEndpoints(endpoint)
        {
            static_assert(std::is_floating_point<FPType>::value, "Invalid type.");
#if defined(GTE_THROW_ON_INVALID_INTERVAL)
            LogAssert(mEndpoints[0] <= mEndpoints[1], "Invalid FPInterval.");
#endif
        }

        FPInterval& operator=(FPInterval const& other)
        {
            static_assert(std::is_floating_point<FPType>::value, "Invalid type.");
            mEndpoints = other.mEndpoints;
            return *this;
        }

        // Member access. It is only possible to read the endpoints. You
        // cannot modify the endpoints outside the arithmetic operations.
        inline FPType operator[](size_t i) const
        {
            return mEndpoints[i];
        }

        inline std::array<FPType, 2> GetEndpoints() const
        {
            return mEndpoints;
        }

        // Arithmetic operations to compute intervals at the leaf nodes of
        // an expression tree. Such nodes correspond to the raw floating-point
        // variables of the expression. The non-class operators defined after
        // the class definition are used to compute intervals at the interior
        // nodes of the expression tree.
        inline static FPInterval Add(FPType u, FPType v)
        {
            FPInterval w;
            auto saveMode = std::fegetround();
            std::fesetround(FE_DOWNWARD);
            w.mEndpoints[0] = u + v;
            std::fesetround(FE_UPWARD);
            w.mEndpoints[1] = u + v;
            std::fesetround(saveMode);
            return w;
        }

        inline static FPInterval Sub(FPType u, FPType v)
        {
            FPInterval w;
            auto saveMode = std::fegetround();
            std::fesetround(FE_DOWNWARD);
            w.mEndpoints[0] = u - v;
            std::fesetround(FE_UPWARD);
            w.mEndpoints[1] = u - v;
            std::fesetround(saveMode);
            return w;
        }

        inline static FPInterval Mul(FPType u, FPType v)
        {
            FPInterval w;
            auto saveMode = std::fegetround();
            std::fesetround(FE_DOWNWARD);
            w.mEndpoints[0] = u * v;
            std::fesetround(FE_UPWARD);
            w.mEndpoints[1] = u * v;
            std::fesetround(saveMode);
            return w;
        }

        inline static FPInterval Div(FPType u, FPType v)
        {
            FPType const zero = static_cast<FPType>(0);
            if (v != zero)
            {
                FPInterval w;
                auto saveMode = std::fegetround();
                std::fesetround(FE_DOWNWARD);
                w.mEndpoints[0] = u / v;
                std::fesetround(FE_UPWARD);
                w.mEndpoints[1] = u / v;
                std::fesetround(saveMode);
                return w;
            }
            else
            {
                // Division by zero does not lead to a determinate FPInterval.
                // Just return the entire set of real numbers.
                return Reals();
            }
        }

        // This function is called to compute the lower bound on the product
        // of two intervals. Before calling the function, you need to call
        // std::fesetround(FE_DOWNWARD). The idea is to compute lower bounds
        // in batch mode (multiple calls of ProductLowerBound) in order to
        // minimize FPU control word state changes.
        static FPType ProductLowerBound(std::array<FPType, 2> const& u,
            std::array<FPType, 2> const& v)
        {
            FPType const zero = static_cast<FPType>(0);
            FPType w0;
            if (u[0] >= zero)
            {
                if (v[0] >= zero)
                {
                    w0 = u[0] * v[0];
                }
                else if (v[1] <= zero)
                {
                    w0 = u[1] * v[0];
                }
                else
                {
                    w0 = u[1] * v[0];
                }
            }
            else if (u[1] <= zero)
            {
                if (v[0] >= zero)
                {
                    w0 = u[0] * v[1];
                }
                else if (v[1] <= zero)
                {
                    w0 = u[1] * v[1];
                }
                else
                {
                    w0 = u[0] * v[1];
                }
            }
            else
            {
                if (v[0] >= zero)
                {
                    w0 = u[0] * v[1];
                }
                else if (v[1] <= zero)
                {
                    w0 = u[1] * v[0];
                }
                else
                {
                    w0 = u[0] * v[0];
                }
            }
            return w0;
        }

        // This function is called to compute the upper bound on the product
        // of two intervals. Before calling the function, you need to call
        // std::fesetround(FE_UPWARD). The idea is to compute lower bounds
        // inbatch mode (multiple calls of ProductUpperBound) in order to
        // minimize FPU control word state changes.
        static FPType ProductUpperBound(std::array<FPType, 2> const& u,
            std::array<FPType, 2> const& v)
        {
            FPType const zero = static_cast<FPType>(0);
            FPType w1;
            if (u[0] >= zero)
            {
                if (v[0] >= zero)
                {
                    w1 = u[1] * v[1];
                }
                else if (v[1] <= zero)
                {
                    w1 = u[0] * v[1];
                }
                else
                {
                    w1 = u[1] * v[1];
                }
            }
            else if (u[1] <= zero)
            {
                if (v[0] >= zero)
                {
                    w1 = u[1] * v[0];
                }
                else if (v[1] <= zero)
                {
                    w1 = u[0] * v[0];
                }
                else
                {
                    w1 = u[0] * v[0];
                }
            }
            else
            {
                if (v[0] >= zero)
                {
                    w1 = u[1] * v[1];
                }
                else if (v[1] <= zero)
                {
                    w1 = u[0] * v[0];
                }
                else
                {
                    w1 = u[1] * v[1];
                }
            }
            return w1;
        }

    private:
        std::array<FPType, 2> mEndpoints;

    public:
        // FOR INTERNAL USE ONLY. These are used by the non-class operators
        // defined after the class definition.
        inline static FPInterval Add(FPType u0, FPType u1, FPType v0, FPType v1)
        {
            FPInterval w;
            auto saveMode = std::fegetround();
            std::fesetround(FE_DOWNWARD);
            w.mEndpoints[0] = u0 + v0;
            std::fesetround(FE_UPWARD);
            w.mEndpoints[1] = u1 + v1;
            std::fesetround(saveMode);
            return w;
        }

        inline static FPInterval Sub(FPType u0, FPType u1, FPType v0, FPType v1)
        {
            FPInterval w;
            auto saveMode = std::fegetround();
            std::fesetround(FE_DOWNWARD);
            w.mEndpoints[0] = u0 - v1;
            std::fesetround(FE_UPWARD);
            w.mEndpoints[1] = u1 - v0;
            std::fesetround(saveMode);
            return w;
        }

        inline static FPInterval Mul(FPType u0, FPType u1, FPType v0, FPType v1)
        {
            FPInterval w;
            auto saveMode = std::fegetround();
            std::fesetround(FE_DOWNWARD);
            w.mEndpoints[0] = u0 * v0;
            std::fesetround(FE_UPWARD);
            w.mEndpoints[1] = u1 * v1;
            std::fesetround(saveMode);
            return w;
        }

        inline static FPInterval Mul2(FPType u0, FPType u1, FPType v0, FPType v1)
        {
            auto saveMode = std::fegetround();
            std::fesetround(FE_DOWNWARD);
            FPType u0mv1 = u0 * v1;
            FPType u1mv0 = u1 * v0;
            std::fesetround(FE_UPWARD);
            FPType u0mv0 = u0 * v0;
            FPType u1mv1 = u1 * v1;
            std::fesetround(saveMode);
            return FPInterval<FPType>(std::min(u0mv1, u1mv0), std::max(u0mv0, u1mv1));
        }

        inline static FPInterval Div(FPType u0, FPType u1, FPType v0, FPType v1)
        {
            FPInterval w;
            auto saveMode = std::fegetround();
            std::fesetround(FE_DOWNWARD);
            w.mEndpoints[0] = u0 / v1;
            std::fesetround(FE_UPWARD);
            w.mEndpoints[1] = u1 / v0;
            std::fesetround(saveMode);
            return w;
        }

        inline static FPInterval Reciprocal(FPType v0, FPType v1)
        {
            FPType const one = static_cast<FPType>(1);
            FPInterval w;
            auto saveMode = std::fegetround();
            std::fesetround(FE_DOWNWARD);
            w.mEndpoints[0] = one / v1;
            std::fesetround(FE_UPWARD);
            w.mEndpoints[1] = one / v0;
            std::fesetround(saveMode);
            return w;
        }

        inline static FPInterval ReciprocalDown(FPType v)
        {
            auto saveMode = std::fegetround();
            std::fesetround(FE_DOWNWARD);
            FPType recpv = static_cast<FPType>(1) / v;
            std::fesetround(saveMode);
            FPType const inf = std::numeric_limits<FPType>::infinity();
            return FPInterval<FPType>(recpv, +inf);
        }

        inline static FPInterval ReciprocalUp(FPType v)
        {
            auto saveMode = std::fegetround();
            std::fesetround(FE_UPWARD);
            FPType recpv = static_cast<FPType>(1) / v;
            std::fesetround(saveMode);
            FPType const inf = std::numeric_limits<FPType>::infinity();
            return FPInterval<FPType>(-inf, recpv);
        }

        inline static FPInterval Reals()
        {
            FPType const inf = std::numeric_limits<FPType>::infinity();
            return FPInterval(-inf, +inf);
        }
    };

    // Unary operations. Negation of [e0,e1] produces [-e1,-e0]. This
    // operation needs to be supported in the sense of negating a
    // "number" in an arithmetic expression.
    template <typename FPType>
    FPInterval<FPType> operator+(FPInterval<FPType> const& u)
    {
        return u;
    }

    template <typename FPType>
    FPInterval<FPType> operator-(FPInterval<FPType> const& u)
    {
        return FPInterval<FPType>(-u[1], -u[0]);
    }

    // Addition operations.
    template <typename FPType>
    FPInterval<FPType> operator+(FPType u, FPInterval<FPType> const& v)
    {
        return FPInterval<FPType>::Add(u, u, v[0], v[1]);
    }

    template <typename FPType>
    FPInterval<FPType> operator+(FPInterval<FPType> const& u, FPType v)
    {
        return FPInterval<FPType>::Add(u[0], u[1], v, v);
    }

    template <typename FPType>
    FPInterval<FPType> operator+(FPInterval<FPType> const& u, FPInterval<FPType> const& v)
    {
        return FPInterval<FPType>::Add(u[0], u[1], v[0], v[1]);
    }

    template <typename FPType>
    FPInterval<FPType>& operator+=(FPInterval<FPType>& u, FPType v)
    {
        u = u + v;
        return u;
    }

    template <typename FPType>
    FPInterval<FPType>& operator+=(FPInterval<FPType>& u, FPInterval<FPType> const& v)
    {
        u = u + v;
        return u;
    }

    // Subtraction operations.
    template <typename FPType>
    FPInterval<FPType> operator-(FPType u, FPInterval<FPType> const& v)
    {
        return FPInterval<FPType>::Sub(u, u, v[0], v[1]);
    }

    template <typename FPType>
    FPInterval<FPType> operator-(FPInterval<FPType> const& u, FPType v)
    {
        return FPInterval<FPType>::Sub(u[0], u[1], v, v);
    }

    template <typename FPType>
    FPInterval<FPType> operator-(FPInterval<FPType> const& u, FPInterval<FPType> const& v)
    {
        return FPInterval<FPType>::Sub(u[0], u[1], v[0], v[1]);
    }

    template <typename FPType>
    FPInterval<FPType>& operator-=(FPInterval<FPType>& u, FPType v)
    {
        u = u - v;
        return u;
    }

    template <typename FPType>
    FPInterval<FPType>& operator-=(FPInterval<FPType>& u, FPInterval<FPType> const& v)
    {
        u = u - v;
        return u;
    }

    // Multiplication operations.
    template <typename FPType>
    FPInterval<FPType> operator*(FPType u, FPInterval<FPType> const& v)
    {
        FPType const zero = static_cast<FPType>(0);
        if (u >= zero)
        {
            return FPInterval<FPType>::Mul(u, u, v[0], v[1]);
        }
        else
        {
            return FPInterval<FPType>::Mul(u, u, v[1], v[0]);
        }
    }

    template <typename FPType>
    FPInterval<FPType> operator*(FPInterval<FPType> const& u, FPType v)
    {
        FPType const zero = static_cast<FPType>(0);
        if (v >= zero)
        {
            return FPInterval<FPType>::Mul(u[0], u[1], v, v);
        }
        else
        {
            return FPInterval<FPType>::Mul(u[1], u[0], v, v);
        }
    }

    template <typename FPType>
    FPInterval<FPType> operator*(FPInterval<FPType> const& u, FPInterval<FPType> const& v)
    {
        FPType const zero = static_cast<FPType>(0);
        if (u[0] >= zero)
        {
            if (v[0] >= zero)
            {
                return FPInterval<FPType>::Mul(u[0], u[1], v[0], v[1]);
            }
            else if (v[1] <= zero)
            {
                return FPInterval<FPType>::Mul(u[1], u[0], v[0], v[1]);
            }
            else // v[0] < 0 < v[1]
            {
                return FPInterval<FPType>::Mul(u[1], u[1], v[0], v[1]);
            }
        }
        else if (u[1] <= zero)
        {
            if (v[0] >= zero)
            {
                return FPInterval<FPType>::Mul(u[0], u[1], v[1], v[0]);
            }
            else if (v[1] <= zero)
            {
                return FPInterval<FPType>::Mul(u[1], u[0], v[1], v[0]);
            }
            else // v[0] < 0 < v[1]
            {
                return FPInterval<FPType>::Mul(u[0], u[0], v[1], v[0]);
            }
        }
        else // u[0] < 0 < u[1]
        {
            if (v[0] >= zero)
            {
                return FPInterval<FPType>::Mul(u[0], u[1], v[1], v[1]);
            }
            else if (v[1] <= zero)
            {
                return FPInterval<FPType>::Mul(u[1], u[0], v[0], v[0]);
            }
            else // v[0] < 0 < v[1]
            {
                return FPInterval<FPType>::Mul2(u[0], u[1], v[0], v[1]);
            }
        }
    }

    template <typename FPType>
    FPInterval<FPType>& operator*=(FPInterval<FPType>& u, FPType v)
    {
        u = u * v;
        return u;
    }

    template <typename FPType>
    FPInterval<FPType>& operator*=(FPInterval<FPType>& u, FPInterval<FPType> const& v)
    {
        u = u * v;
        return u;
    }

    // Division operations. If the divisor FPInterval is [v0,v1] with
    // v0 < 0 < v1, then the returned FPInterval is (-infinity,+infinity)
    // instead of Union((-infinity,1/v0),(1/v1,+infinity)). An application
    // should try to avoid this case by branching based on [v0,0] and [0,v1].
    template <typename FPType>
    FPInterval<FPType> operator/(FPType u, FPInterval<FPType> const& v)
    {
        FPType const zero = static_cast<FPType>(0);
        if (v[0] > zero || v[1] < zero)
        {
            return u * FPInterval<FPType>::Reciprocal(v[0], v[1]);
        }
        else
        {
            if (v[0] == zero)
            {
                return u * FPInterval<FPType>::ReciprocalDown(v[1]);
            }
            else if (v[1] == zero)
            {
                return u * FPInterval<FPType>::ReciprocalUp(v[0]);
            }
            else // v[0] < 0 < v[1]
            {
                return FPInterval<FPType>::Reals();
            }
        }
    }

    template <typename FPType>
    FPInterval<FPType> operator/(FPInterval<FPType> const& u, FPType v)
    {
        FPType const zero = static_cast<FPType>(0);
        if (v > zero)
        {
            return FPInterval<FPType>::Div(u[0], u[1], v, v);
        }
        else if (v < zero)
        {
            return FPInterval<FPType>::Div(u[1], u[0], v, v);
        }
        else // v = 0
        {
            return FPInterval<FPType>::Reals();
        }
    }

    template <typename FPType>
    FPInterval<FPType> operator/(FPInterval<FPType> const& u, FPInterval<FPType> const& v)
    {
        FPType const zero = static_cast<FPType>(0);
        if (v[0] > zero || v[1] < zero)
        {
            return u * FPInterval<FPType>::Reciprocal(v[0], v[1]);
        }
        else
        {
            if (v[0] == zero)
            {
                return u * FPInterval<FPType>::ReciprocalDown(v[1]);
            }
            else if (v[1] == zero)
            {
                return u * FPInterval<FPType>::ReciprocalUp(v[0]);
            }
            else // v[0] < 0 < v[1]
            {
                return FPInterval<FPType>::Reals();
            }
        }
    }

    template <typename FPType>
    FPInterval<FPType>& operator/=(FPInterval<FPType>& u, FPType v)
    {
        u = u / v;
        return u;
    }
    template <typename FPType>
    FPInterval<FPType>& operator/=(FPInterval<FPType>& u, FPInterval<FPType> const& v)
    {
        u = u / v;
        return u;
    }
}
