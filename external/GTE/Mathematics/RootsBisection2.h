// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.6.2020.02.05

#pragma once

#include <Mathematics/RootsBisection1.h>
#include <iomanip>
#include <sstream>

// Estimate a root to continuous functions F(x,y) and G(x,y) define on a
// rectangle [xMin,xMax]x[yMin,yMax]. The requirements are that for each
// y' in [yMin,yMax], A(x) = F(x,y') satisfies A(xMin) * A(xMax) < 0,
// which guarantees A(x) has a root. Also, for each x' in [xMin,xMax],
// B(y) = G(x',y) satisfies B(yMin) * B(yMax) < 0, which guarantees B(y)
// has a root. Bisection is performed in the x-direction for A(x). Let
// x' be the root. Bisection is then performed in the y-direction for
// B(y). Let y' be the root. The function value is A(x') = F(x',y').
// This effectively is a bisection of C(x) = F(x,h(x)) along the curve
// where G(x,h(x)) = 0.

namespace gte
{
    template <typename Real>
    class RootsBisection2
    {
    public:
        // Use this constructor when Real is a floating-point type.
        template <typename Dummy = Real>
        RootsBisection2(uint32_t xMaxIterations, uint32_t yMaxIterations,
            typename std::enable_if<!is_arbitrary_precision<Dummy>::value>::type* = nullptr)
            :
            mXBisector(xMaxIterations),
            mYBisector(yMaxIterations),
            mXRoot(0),
            mYRoot(0),
            mFAtRoot(0),
            mGAtRoot(0)
        {
            static_assert(!is_arbitrary_precision<Real>::value,
                "Template parameter is not a floating-point type.");
        }

        // Use this constructor when Real is an arbitrary-precision type.
        template <typename Dummy = Real>
            RootsBisection2(uint32_t precision, uint32_t xMaxIterations, uint32_t yMaxIterations,
                typename std::enable_if<is_arbitrary_precision<Dummy>::value>::type* = nullptr)
            :
            mXBisector(precision, xMaxIterations),
            mYBisector(precision, yMaxIterations),
            mXRoot(0),
            mYRoot(0),
            mFAtRoot(0),
            mGAtRoot(0)
        {
            static_assert(is_arbitrary_precision<Real>::value,
                "Template parameter is not an arbitrary-precision type.");
        }

        // Disallow copy and move semantics.
        RootsBisection2(RootsBisection2 const&) = delete;
        RootsBisection2(RootsBisection2&&) = delete;
        RootsBisection2& operator=(RootsBisection2 const&) = delete;
        RootsBisection2& operator=(RootsBisection2&&) = delete;

        uint32_t operator()(
            std::function<Real(Real const&, Real const&)> F,
            std::function<Real(Real const&, Real const&)> G, 
            Real const& xMin, Real const& xMax, Real const& yMin, Real const& yMax,
            Real& xRoot, Real& yRoot, Real& fAtRoot, Real& gAtRoot)
        {
            // XFunction(x) = F(x,y), where G(x,y) = 0.
            auto XFunction = [this, &F, &G, &yMin, &yMax](Real const& x)
            {
                // YFunction(y) = G(x,y)
                auto YFunction = [this, &G, &x](Real const& y)
                {
                    return G(x, y);
                };

                // Bisect in the y-variable to find the root of YFunction(y).
                uint32_t numYIterations = mYBisector(YFunction, yMin, yMax, mYRoot, mGAtRoot);
                if (numYIterations == 0)
                {
                    // The interval is not guaranteed to bound a root.
                    std::ostringstream message;
                    message << std::setprecision(20)
                        << "yMin = " << (double)yMin
                        << ", yMax = " << (double)yMax
                        << ", fMin = " << (double)YFunction(yMin)
                        << ", fMax = " << (double)YFunction(yMax)
                        << std::endl;
                    LogWarning(message.str());

                }
                return F(x, mYRoot);
            };

            // Bisect in the x-variable to find the root of XFunction(x).
            uint32_t numXIterations = mXBisector(XFunction, xMin, xMax, mXRoot, mFAtRoot);
            if (numXIterations == 0)
            {
                // The interval is not guaranteed to bound a root.
                std::ostringstream message;
                message << std::setprecision(20)
                    << "xMin = " << (double)xMin
                    << ", xMax = " << (double)xMax
                    << ", fMin = " << (double)XFunction(xMin)
                    << ", fMax = " << (double)XFunction(xMax)
                    << std::endl;
                LogWarning(message.str());

            }

            xRoot = mXRoot;
            yRoot = mYRoot;
            fAtRoot = mFAtRoot;
            gAtRoot = mGAtRoot;
            return numXIterations;
        }

    private:
        RootsBisection1<Real> mXBisector, mYBisector;
        Real mXRoot, mYRoot, mFAtRoot, mGAtRoot;
    };
}
