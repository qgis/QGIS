// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.10.17

#pragma once

#include <Mathematics/Vector3.h>

// Queries about the relation of a point to various geometric objects.  The
// choices for N when using UIntegerFP32<N> for either BSNumber of BSRational
// are determined in GeometricTools/GTEngine/Tools/PrecisionCalculator.  These
// N-values are worst case scenarios. Your specific input data might require
// much smaller N, in which case you can modify PrecisionCalculator to use the
// BSPrecision(int32_t,int32_t,int32_t,bool) constructors.

namespace gte
{
    template <typename Real>
    class PrimalQuery3
    {
    public:
        // The caller is responsible for ensuring that the array is not empty
        // before calling queries and that the indices passed to the queries
        // are valid.  The class does no range checking.
        PrimalQuery3()
            :
            mNumVertices(0),
            mVertices(nullptr)
        {
        }

        PrimalQuery3(int numVertices, Vector3<Real> const* vertices)
            :
            mNumVertices(numVertices),
            mVertices(vertices)
        {
        }

        // Member access.
        inline void Set(int numVertices, Vector3<Real> const* vertices)
        {
            mNumVertices = numVertices;
            mVertices = vertices;
        }

        inline int GetNumVertices() const
        {
            return mNumVertices;
        }

        inline Vector3<Real> const* GetVertices() const
        {
            return mVertices;
        }

        // In the following, point P refers to vertices[i] or 'test' and Vi
        // refers to vertices[vi].

        // For a plane with origin V0 and normal N = Cross(V1-V0,V2-V0),
        // ToPlane returns
        //   +1, P on positive side of plane (side to which N points)
        //   -1, P on negative side of plane (side to which -N points)
        //    0, P on the plane
        //
        // Choice of N for UIntegerFP32<N>.
        //    input type | compute type | N
        //    -----------+--------------+----
        //    float      | BSNumber     |  27
        //    double     | BSNumber     | 197
        //    float      | BSRational   |  79
        //    double     | BSRational   | 591
        int ToPlane(int i, int v0, int v1, int v2) const
        {
            return ToPlane(mVertices[i], v0, v1, v2);
        }

        int ToPlane(Vector3<Real> const& test, int v0, int v1, int v2) const
        {
            Vector3<Real> const& vec0 = mVertices[v0];
            Vector3<Real> const& vec1 = mVertices[v1];
            Vector3<Real> const& vec2 = mVertices[v2];

            Real x0 = test[0] - vec0[0];
            Real y0 = test[1] - vec0[1];
            Real z0 = test[2] - vec0[2];
            Real x1 = vec1[0] - vec0[0];
            Real y1 = vec1[1] - vec0[1];
            Real z1 = vec1[2] - vec0[2];
            Real x2 = vec2[0] - vec0[0];
            Real y2 = vec2[1] - vec0[1];
            Real z2 = vec2[2] - vec0[2];
            Real y1z2 = y1 * z2;
            Real y2z1 = y2 * z1;
            Real y2z0 = y2 * z0;
            Real y0z2 = y0 * z2;
            Real y0z1 = y0 * z1;
            Real y1z0 = y1 * z0;
            Real c0 = y1z2 - y2z1;
            Real c1 = y2z0 - y0z2;
            Real c2 = y0z1 - y1z0;
            Real x0c0 = x0 * c0;
            Real x1c1 = x1 * c1;
            Real x2c2 = x2 * c2;
            Real term = x0c0 + x1c1;
            Real det = term + x2c2;
            Real const zero(0);

            return (det > zero ? +1 : (det < zero ? -1 : 0));
        }

        // For a tetrahedron with vertices ordered as described in the file
        // TetrahedronKey.h, the function returns
        //   +1, P outside tetrahedron
        //   -1, P inside tetrahedron
        //    0, P on tetrahedron
        //
        // Choice of N for UIntegerFP32<N>.
        //    input type | compute type | N
        //    -----------+--------------+----
        //    float      | BSNumber     |  27
        //    double     | BSNumber     | 197
        //    float      | BSRational   |  79
        //    double     | BSRational   | 591
        // The query involves four calls of ToPlane, so the numbers match
        // those of ToPlane.
        int ToTetrahedron(int i, int v0, int v1, int v2, int v3) const
        {
            return ToTetrahedron(mVertices[i], v0, v1, v2, v3);
        }

        int ToTetrahedron(Vector3<Real> const& test, int v0, int v1, int v2, int v3) const
        {
            int sign0 = ToPlane(test, v1, v2, v3);
            if (sign0 > 0)
            {
                return +1;
            }

            int sign1 = ToPlane(test, v0, v2, v3);
            if (sign1 < 0)
            {
                return +1;
            }

            int sign2 = ToPlane(test, v0, v1, v3);
            if (sign2 > 0)
            {
                return +1;
            }

            int sign3 = ToPlane(test, v0, v1, v2);
            if (sign3 < 0)
            {
                return +1;
            }

            return ((sign0 && sign1 && sign2 && sign3) ? -1 : 0);
        }

        // For a tetrahedron with vertices ordered as described in the file
        // TetrahedronKey.h, the function returns
        //   +1, P outside circumsphere of tetrahedron
        //   -1, P inside circumsphere of tetrahedron
        //    0, P on circumsphere of tetrahedron
        //
        // Choice of N for UIntegerFP32<N>.
        //    input type | compute type | N
        //    -----------+--------------+-----
        //    float      | BSNumber     |   44
        //    double     | BSNumber     |  329
        //    float      | BSNumber     |  262
        //    double     | BSRational   | 1969
        int ToCircumsphere(int i, int v0, int v1, int v2, int v3) const
        {
            return ToCircumsphere(mVertices[i], v0, v1, v2, v3);
        }

        int ToCircumsphere(Vector3<Real> const& test, int v0, int v1, int v2, int v3) const
        {
            Vector3<Real> const& vec0 = mVertices[v0];
            Vector3<Real> const& vec1 = mVertices[v1];
            Vector3<Real> const& vec2 = mVertices[v2];
            Vector3<Real> const& vec3 = mVertices[v3];

            Real x0 = vec0[0] - test[0];
            Real y0 = vec0[1] - test[1];
            Real z0 = vec0[2] - test[2];
            Real s00 = vec0[0] + test[0];
            Real s01 = vec0[1] + test[1];
            Real s02 = vec0[2] + test[2];
            Real t00 = s00 * x0;
            Real t01 = s01 * y0;
            Real t02 = s02 * z0;
            Real t00pt01 = t00 + t01;
            Real w0 = t00pt01 + t02;

            Real x1 = vec1[0] - test[0];
            Real y1 = vec1[1] - test[1];
            Real z1 = vec1[2] - test[2];
            Real s10 = vec1[0] + test[0];
            Real s11 = vec1[1] + test[1];
            Real s12 = vec1[2] + test[2];
            Real t10 = s10 * x1;
            Real t11 = s11 * y1;
            Real t12 = s12 * z1;
            Real t10pt11 = t10 + t11;
            Real w1 = t10pt11 + t12;

            Real x2 = vec2[0] - test[0];
            Real y2 = vec2[1] - test[1];
            Real z2 = vec2[2] - test[2];
            Real s20 = vec2[0] + test[0];
            Real s21 = vec2[1] + test[1];
            Real s22 = vec2[2] + test[2];
            Real t20 = s20 * x2;
            Real t21 = s21 * y2;
            Real t22 = s22 * z2;
            Real t20pt21 = t20 + t21;
            Real w2 = t20pt21 + t22;

            Real x3 = vec3[0] - test[0];
            Real y3 = vec3[1] - test[1];
            Real z3 = vec3[2] - test[2];
            Real s30 = vec3[0] + test[0];
            Real s31 = vec3[1] + test[1];
            Real s32 = vec3[2] + test[2];
            Real t30 = s30 * x3;
            Real t31 = s31 * y3;
            Real t32 = s32 * z3;
            Real t30pt31 = t30 + t31;
            Real w3 = t30pt31 + t32;

            Real x0y1 = x0 * y1;
            Real x0y2 = x0 * y2;
            Real x0y3 = x0 * y3;
            Real x1y0 = x1 * y0;
            Real x1y2 = x1 * y2;
            Real x1y3 = x1 * y3;
            Real x2y0 = x2 * y0;
            Real x2y1 = x2 * y1;
            Real x2y3 = x2 * y3;
            Real x3y0 = x3 * y0;
            Real x3y1 = x3 * y1;
            Real x3y2 = x3 * y2;
            Real a0 = x0y1 - x1y0;
            Real a1 = x0y2 - x2y0;
            Real a2 = x0y3 - x3y0;
            Real a3 = x1y2 - x2y1;
            Real a4 = x1y3 - x3y1;
            Real a5 = x2y3 - x3y2;

            Real z0w1 = z0 * w1;
            Real z0w2 = z0 * w2;
            Real z0w3 = z0 * w3;
            Real z1w0 = z1 * w0;
            Real z1w2 = z1 * w2;
            Real z1w3 = z1 * w3;
            Real z2w0 = z2 * w0;
            Real z2w1 = z2 * w1;
            Real z2w3 = z2 * w3;
            Real z3w0 = z3 * w0;
            Real z3w1 = z3 * w1;
            Real z3w2 = z3 * w2;
            Real b0 = z0w1 - z1w0;
            Real b1 = z0w2 - z2w0;
            Real b2 = z0w3 - z3w0;
            Real b3 = z1w2 - z2w1;
            Real b4 = z1w3 - z3w1;
            Real b5 = z2w3 - z3w2;
            Real a0b5 = a0 * b5;
            Real a1b4 = a1 * b4;
            Real a2b3 = a2 * b3;
            Real a3b2 = a3 * b2;
            Real a4b1 = a4 * b1;
            Real a5b0 = a5 * b0;
            Real term0 = a0b5 - a1b4;
            Real term1 = term0 + a2b3;
            Real term2 = term1 + a3b2;
            Real term3 = term2 - a4b1;
            Real det = term3 + a5b0;
            Real const zero(0);

            return (det > zero ? 1 : (det < zero ? -1 : 0));
        }

    private:
        int mNumVertices;
        Vector3<Real> const* mVertices;
    };
}
