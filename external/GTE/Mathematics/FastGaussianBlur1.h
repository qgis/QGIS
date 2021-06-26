// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Math.h>

// The algorithms here are based on solving the linear heat equation using
// finite differences in scale, not in time.  The following document has
// a brief summary of the concept,
//   https://www.geometrictools.com/Documentation/FastGaussianBlur.pdf
// The idea is to represent the blurred image as f(x,s) in terms of position
// x and scale s.  Gaussian blurring is accomplished by using the input image
// I(x,s0) as the initial image (of scale s0 > 0) for the partial differential
// equation
//   s*df/ds = s^2*Laplacian(f)
// where the Laplacian operator is
//   Laplacian = (d/dx)^2, dimension 1
//   Laplacian = (d/dx)^2+(d/dy)^2, dimension 2
//   Laplacian = (d/dx)^2+(d/dy)^2+(d/dz)^2, dimension 3
//
// The term s*df/ds is approximated by
//   s*df(x,s)/ds = (f(x,b*s)-f(x,s))/ln(b)
// for b > 1, but close to 1, where ln(b) is the natural logarithm of b.  If
// you take the limit of the right-hand side as b approaches 1, you get the
// left-hand side.
//
// The term s^2*((d/dx)^2)f is approximated by
//   s^2*((d/dx)^2)f = (f(x+h*s,s)-2*f(x,s)+f(x-h*s,s))/h^2
// for h > 0, but close to zero.
//
// Equating the approximations for the left-hand side and the right-hand side
// of the partial differential equation leads to the numerical method used in
// this code.
//
// For iterative application of these functions, the caller is responsible
// for constructing a geometric sequence of scales,
//   s0, s1 = s0*b, s2 = s1*b = s0*b^2, ...
// where the base b satisfies 1 < b < exp(0.5*d) where d is the dimension of
// the image.  The upper bound on b guarantees stability of the finite
// difference method used to approximate the partial differential equation.
// The method assumes a pixel size of h = 1.

namespace gte
{
    // The image type must be one of short, int, float or double.  The
    // computations are performed using double.  The input and output images
    // must both have xBound elements.

    template <typename T>
    class FastGaussianBlur1
    {
    public:
        void Execute(int xBound, T const* input, T* output,
            double scale, double logBase)
        {
            int xBoundM1 = xBound - 1;
            for (int x = 0; x < xBound; ++x)
            {
                double rxps = static_cast<double>(x) + scale;
                double rxms = static_cast<double>(x) - scale;
                int xp1 = static_cast<int>(std::floor(rxps));
                int xm1 = static_cast<int>(std::ceil(rxms));

                double center = static_cast<double>(input[x]);
                double xsum = -2.0 * center;

                if (xp1 >= xBoundM1)  // use boundary value
                {
                    xsum += static_cast<double>(input[xBoundM1]);
                }
                else  // linearly interpolate
                {
                    double imgXp1 = static_cast<double>(input[xp1]);
                    double imgXp2 = static_cast<double>(input[xp1 + 1]);
                    double delta = rxps - static_cast<double>(xp1);
                    xsum += imgXp1 + delta * (imgXp2 - imgXp1);
                }

                if (xm1 <= 0)  // use boundary value
                {
                    xsum += static_cast<double>(input[0]);
                }
                else  // linearly interpolate
                {
                    double imgXm1 = static_cast<double>(input[xm1]);
                    double imgXm2 = static_cast<double>(input[xm1 - 1]);
                    double delta = rxms - static_cast<double>(xm1);
                    xsum += imgXm1 + delta * (imgXm1 - imgXm2);
                }

                output[x] = static_cast<T>(center + logBase * xsum);
            }
        }
    };
}
