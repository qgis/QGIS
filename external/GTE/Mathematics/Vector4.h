// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Vector3.h>

namespace gte
{
    // Template alias for convenience.
    template <typename Real>
    using Vector4 = Vector<4, Real>;

    // In Vector3.h, the Vector3 Cross, UnitCross, and DotCross have a
    // template parameter N that should be 3 or 4.  The latter case supports
    // affine vectors in 4D (last component w = 0) when you want to use
    // 4-tuples and 4x4 matrices for affine algebra.  Thus, you may use those
    // template functions for Vector4.

    // Compute the hypercross product using the formal determinant:
    //   hcross = det{{e0,e1,e2,e3},{x0,x1,x2,x3},{y0,y1,y2,y3},{z0,z1,z2,z3}}
    // where e0 = (1,0,0,0), e1 = (0,1,0,0), e2 = (0,0,1,0), e3 = (0,0,0,1),
    // v0 = (x0,x1,x2,x3), v1 = (y0,y1,y2,y3), and v2 = (z0,z1,z2,z3).
    template <typename Real>
    Vector4<Real> HyperCross(Vector4<Real> const& v0, Vector4<Real> const& v1, Vector4<Real> const& v2)
    {
        Real m01 = v0[0] * v1[1] - v0[1] * v1[0];  // x0*y1 - y0*x1
        Real m02 = v0[0] * v1[2] - v0[2] * v1[0];  // x0*z1 - z0*x1
        Real m03 = v0[0] * v1[3] - v0[3] * v1[0];  // x0*w1 - w0*x1
        Real m12 = v0[1] * v1[2] - v0[2] * v1[1];  // y0*z1 - z0*y1
        Real m13 = v0[1] * v1[3] - v0[3] * v1[1];  // y0*w1 - w0*y1
        Real m23 = v0[2] * v1[3] - v0[3] * v1[2];  // z0*w1 - w0*z1
        return Vector4<Real>
        {
            +m23 * v2[1] - m13 * v2[2] + m12 * v2[3],  // +m23*y2 - m13*z2 + m12*w2
            -m23 * v2[0] + m03 * v2[2] - m02 * v2[3],  // -m23*x2 + m03*z2 - m02*w2
            +m13 * v2[0] - m03 * v2[1] + m01 * v2[3],  // +m13*x2 - m03*y2 + m01*w2
            -m12 * v2[0] + m02 * v2[1] - m01 * v2[2]   // -m12*x2 + m02*y2 - m01*z2
        };
    }

    // Compute the normalized hypercross product.
    template <typename Real>
    Vector4<Real> UnitHyperCross(Vector4<Real> const& v0,
        Vector4<Real> const& v1, Vector4<Real> const& v2, bool robust = false)
    {
        Vector4<Real> unitHyperCross = HyperCross(v0, v1, v2);
        Normalize(unitHyperCross, robust);
        return unitHyperCross;
    }

    // Compute Dot(HyperCross((x0,x1,x2,x3),(y0,y1,y2,y3),(z0,z1,z2,z3)),
    // (w0,w1,w2,w3)), where v0 = (x0,x1,x2,x3), v1 = (y0,y1,y2,y3),
    // v2 = (z0,z1,z2,z3), and v3 = (w0,w1,w2,w3).
    template <typename Real>
    Real DotHyperCross(Vector4<Real> const& v0, Vector4<Real> const& v1,
        Vector4<Real> const& v2, Vector4<Real> const& v3)
    {
        return Dot(HyperCross(v0, v1, v2), v3);
    }

    // Compute a right-handed orthonormal basis for the orthogonal complement
    // of the input vectors.  The function returns the smallest length of the
    // unnormalized vectors computed during the process.  If this value is
    // nearly zero, it is possible that the inputs are linearly dependent
    // (within numerical round-off errors).  On input, numInputs must be 1, 2
    // or 3, and v[0] through v[numInputs-1] must be initialized.  On output,
    // the vectors v[0] through v[3] form an orthonormal set.
    template <typename Real>
    Real ComputeOrthogonalComplement(int numInputs, Vector4<Real>* v, bool robust = false)
    {
        if (numInputs == 1)
        {
            int maxIndex = 0;
            Real maxAbsValue = std::fabs(v[0][0]);
            for (int i = 1; i < 4; ++i)
            {
                Real absValue = std::fabs(v[0][i]);
                if (absValue > maxAbsValue)
                {
                    maxIndex = i;
                    maxAbsValue = absValue;
                }
            }

            if (maxIndex < 2)
            {
                v[1][0] = -v[0][1];
                v[1][1] = +v[0][0];
                v[1][2] = (Real)0;
                v[1][3] = (Real)0;
            }
            else if (maxIndex == 3)
            {
                // Generally, you can skip this clause and swap the last two
                // components.  However, by swapping 2 and 3 in this case, we
                // allow the function to work properly when the inputs are 3D
                // vectors represented as 4D affine vectors (w = 0).
                v[1][0] = (Real)0;
                v[1][1] = +v[0][2];
                v[1][2] = -v[0][1];
                v[1][3] = (Real)0;
            }
            else
            {
                v[1][0] = (Real)0;
                v[1][1] = (Real)0;
                v[1][2] = -v[0][3];
                v[1][3] = +v[0][2];
            }

            numInputs = 2;
        }

        if (numInputs == 2)
        {
            Real det[6] =
            {
                v[0][0] * v[1][1] - v[1][0] * v[0][1],
                v[0][0] * v[1][2] - v[1][0] * v[0][2],
                v[0][0] * v[1][3] - v[1][0] * v[0][3],
                v[0][1] * v[1][2] - v[1][1] * v[0][2],
                v[0][1] * v[1][3] - v[1][1] * v[0][3],
                v[0][2] * v[1][3] - v[1][2] * v[0][3]
            };

            int maxIndex = 0;
            Real maxAbsValue = std::fabs(det[0]);
            for (int i = 1; i < 6; ++i)
            {
                Real absValue = std::fabs(det[i]);
                if (absValue > maxAbsValue)
                {
                    maxIndex = i;
                    maxAbsValue = absValue;
                }
            }

            if (maxIndex == 0)
            {
                v[2] = { -det[4], +det[2], (Real)0, -det[0] };
            }
            else if (maxIndex <= 2)
            {
                v[2] = { +det[5], (Real)0, -det[2], +det[1] };
            }
            else
            {
                v[2] = { (Real)0, -det[5], +det[4], -det[3] };
            }

            numInputs = 3;
        }

        if (numInputs == 3)
        {
            v[3] = HyperCross(v[0], v[1], v[2]);
            return Orthonormalize<4, Real>(4, v, robust);
        }

        return (Real)0;
    }
}
