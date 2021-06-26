// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 4.5.2020.01.10

#pragma once

#include <Mathematics/Math.h>
#include <Mathematics/Matrix.h>
#include <vector>
using namespace gte;

class FitASin
{
public:
    FitASin();

    template <int Degree>
    void Generate(std::vector<double>& poly, double& error) const;

private:
    // f(x) = (pi/2 - asin(x))/sqrt(1 - x)
    // lim_{x->1} f(x) = sqrt(2)
    double F(double x) const;

    // g(x) = f(x) - p(x)
    double G(double x, int const degree, double const* p) const;

    // (1-x)*f'(x) = -1/sqrt(1 + x) + f(x)/2
    // lim_{x->1 f'(x) = -pow(2,-1.5)/3
    double FDer(double x) const;

    // g'(x) = f'(x) - p'(x)
    double GDer(double x, int const degree, double const* p) const;

    int SignG(double x, int const degree, double const* p) const;
    int SignGDer(double x, int const degree, double const* p) const;
};


FitASin::FitASin()
{
}

double FitASin::F(double x) const
{
    if (x < 1.0)
    {
        return (GTE_C_HALF_PI - std::asin(x)) / std::sqrt(1.0 - x);
    }
    else
    {
        return GTE_C_SQRT_2;
    }
}

double FitASin::FDer(double x) const
{
    if (x < 1.0)
    {
        return (-1.0 / std::sqrt(1.0 + x) + 0.5 * F(x)) / (1.0 - x);
    }
    else
    {
        return -std::pow(2.0, -1.5) / 3.0;
    }
}

double FitASin::G(double x, int const degree, double const* p) const
{
    int i = degree;
    double result = p[i];
    while (--i >= 0)
    {
        result = x*result + p[i];
    }
    result = F(x) - result;
    return result;
}

double FitASin::GDer(double x, int const degree, double const* p) const
{
    int i = degree;
    double result = i*p[i];
    while (--i >= 1)
    {
        result = x*result + i*p[i];
    }
    result = FDer(x) - result;
    return result;
}

int FitASin::SignG(double x, int const degree, double const* p) const
{
    double g = G(x, degree, p);
    return (g > 0.0 ? 1 : (g < 0.0 ? -1 : 0));
}

int FitASin::SignGDer(double x, int const degree, double const* p) const
{
    double gder = GDer(x, degree, p);
    return (gder > 0.0 ? 1 : (gder < 0.0 ? -1 : 0));
}

template <int Degree>
void FitASin::Generate(std::vector<double>& poly, double& error) const
{
    Vector<Degree + 1, double> f0root;
    f0root[0] = 0.0;
    for (int i = 1; i < Degree; ++i)
    {
        f0root[i] = static_cast<double>(i) / static_cast<double>(Degree);
    }
    f0root[Degree] = 1.0;

    // Initial guess at the polynomial, gives you oscillation.
    Matrix<Degree + 1, Degree + 1, double> A;
    Vector<Degree + 1, double> B;
    A(0, 0) = 1.0;
    for (int c = 1; c <= Degree; ++c)
    {
        A(0, c) = 0.0;
    }
    B[0] = GTE_C_HALF_PI;
    for (int r = 1; r <= Degree; ++r)
    {
        A(r, 0) = 1.0;
        for (int c = 1; c <= Degree; ++c)
        {
            A(r, c) = f0root[r] * A(r, c - 1);
        }
        B[r] = F(f0root[r]);
    }
    Vector<Degree + 1, double> p = Inverse(A) * B;

    double absError[Degree], e;

    // Perturb away from zero.
    for (int iter = 0; iter < 16; ++iter)
    {
        // Find local extremum on [f0root[i],f0root[i+1]].
        Vector<Degree, double> f1root;
        int k = 0;
        for (int i = 0; i < Degree; ++i)
        {
            double x0 = f0root[i];
            double x1 = f0root[i + 1];
            int s0 = SignGDer(x0, Degree, &p[0]);
            int s1 = SignGDer(x1, Degree, &p[0]);
            LogAssert(s0*s1 < 0, "Unexpected condition.");
            int j;
            for (j = 0; j < 1024; ++j)
            {
                double xmid = 0.5*(x0 + x1);
                if (x0 != xmid && x1 != xmid)
                {
                    int smid = SignGDer(xmid, Degree, &p[0]);
                    if (smid == s0)
                    {
                        x0 = xmid;
                    }
                    else if (smid == s1)
                    {
                        x1 = xmid;
                    }
                    else
                    {
                        f1root[k++] = xmid;
                        break;
                    }
                }
                else
                {
                    f1root[k++] = xmid;
                    break;
                }

            }
            LogAssert(j < 1024, "Max j-iterations not large enough.");
        }

        for (int i = 0; i < Degree; ++i)
        {
            absError[i] = std::abs(G(f1root[i], Degree, &p[0]));
        }

        // Solve p(x[i]) + (-1)^{i}*e = f(x[i]) for e and p[j].
        double sign = 1.0;
        for (int r = 0; r < Degree; ++r, sign = -sign)
        {
            A(r, 0) = f1root[r];
            for (int c = 1; c < Degree; ++c)
            {
                A(r, c) = f1root[r] * A(r, c - 1);
            }
            A(r, Degree) = sign;
            B[r] = F(f1root[r]) - GTE_C_HALF_PI;  // F(x) - p[0]
        }
        for (int c = 0; c < Degree; ++c)
        {
            A(Degree, c) = 1.0;
        }
        A(Degree, Degree) = 0.0;
        B[Degree] = F(1.0) - GTE_C_HALF_PI;

        Vector<Degree + 1, double> X = Inverse(A)*B;
        for (int i = 0; i < Degree; ++i)
        {
            p[i + 1] = X[i];
        }
        e = X[Degree];

        k = 1;
        for (int i = 0; i < Degree - 1; ++i)
        {
            double x0 = f1root[i];
            double x1 = f1root[i + 1];
            int s0 = SignG(x0, Degree, &p[0]);
            int s1 = SignG(x1, Degree, &p[0]);
            LogAssert(s0*s1 < 0, "Unexpected condition.");
            int j;
            for (j = 0; j < 1024; ++j)
            {
                double xmid = 0.5*(x0 + x1);
                if (x0 != xmid && x1 != xmid)
                {
                    int smid = SignG(xmid, Degree, &p[0]);
                    if (smid == s0)
                    {
                        x0 = xmid;
                    }
                    else if (smid == s1)
                    {
                        x1 = xmid;
                    }
                    else
                    {
                        f0root[k++] = xmid;
                        break;
                    }
                }
                else
                {
                    f0root[k++] = (x0 == xmid ? x0 : x1);
                    break;
                }

            }
            LogAssert(j < 1024, "Max j-iterations not large enough.");
        }
    }

    poly.resize(Degree + 1);
    for (int i = 0; i <= Degree; ++i)
    {
        poly[i] = p[i];
    }
    error = e;
}

