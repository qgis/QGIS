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
    // must both have xBound*yBound elements and be stored in lexicographical
    // order.  The indexing is i = x + xBound * y.

    template <typename T>
    class FastGaussianBlur2
    {
    public:
        void Execute(int xBound, int yBound, T const* input, T* output,
            double scale, double logBase)
        {
            mXBound = xBound;
            mYBound = yBound;
            mInput = input;
            mOutput = output;

            int xBoundM1 = xBound - 1, yBoundM1 = yBound - 1;
            for (int y = 0; y < yBound; ++y)
            {
                double ryps = static_cast<double>(y) + scale;
                double ryms = static_cast<double>(y) - scale;
                int yp1 = static_cast<int>(std::floor(ryps));
                int ym1 = static_cast<int>(std::ceil(ryms));

                for (int x = 0; x < xBound; ++x)
                {
                    double rxps = x + scale;
                    double rxms = x - scale;
                    int xp1 = static_cast<int>(std::floor(rxps));
                    int xm1 = static_cast<int>(std::ceil(rxms));

                    double center = Input(x, y);
                    double xsum = -2.0 * center, ysum = xsum;

                    // x portion of second central difference
                    if (xp1 >= xBoundM1)  // use boundary value
                    {
                        xsum += Input(xBoundM1, y);
                    }
                    else  // linearly interpolate
                    {
                        double imgXp1 = Input(xp1, y);
                        double imgXp2 = Input(xp1 + 1, y);
                        double delta = rxps - static_cast<double>(xp1);
                        xsum += imgXp1 + delta * (imgXp2 - imgXp1);
                    }

                    if (xm1 <= 0)  // use boundary value
                    {
                        xsum += Input(0, y);
                    }
                    else  // linearly interpolate
                    {
                        double imgXm1 = Input(xm1, y);
                        double imgXm2 = Input(xm1 - 1, y);
                        double delta = rxms - static_cast<double>(xm1);
                        xsum += imgXm1 + delta * (imgXm1 - imgXm2);
                    }

                    // y portion of second central difference
                    if (yp1 >= yBoundM1)  // use boundary value
                    {
                        ysum += Input(x, yBoundM1);
                    }
                    else  // linearly interpolate
                    {
                        double imgYp1 = Input(x, yp1);
                        double imgYp2 = Input(x, yp1 + 1);
                        double delta = ryps - static_cast<double>(yp1);
                        ysum += imgYp1 + delta * (imgYp2 - imgYp1);
                    }

                    if (ym1 <= 0)  // use boundary value
                    {
                        ysum += Input(x, 0);
                    }
                    else  // linearly interpolate
                    {
                        double imgYm1 = Input(x, ym1);
                        double imgYm2 = Input(x, ym1 - 1);
                        double delta = ryms - static_cast<double>(ym1);
                        ysum += imgYm1 + delta * (imgYm1 - imgYm2);
                    }

                    Output(x, y) = static_cast<T>(center + logBase * (xsum + ysum));
                }
            }

            mXBound = 0;
            mYBound = 0;
            mInput = nullptr;
            mOutput = nullptr;
        }

    private:
        inline double Input(int x, int y) const
        {
            return static_cast<double>(mInput[x + mXBound * y]);
        }

        inline T& Output(int x, int y)
        {
            return mOutput[x + mXBound * y];
        }

        int mXBound, mYBound;
        T const* mInput;
        T* mOutput;
    };
}
