// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.10.17

#pragma once

#include <Mathematics/Vector2.h>

// Queries about the relation of a point to various geometric objects.  The
// choices for N when using UIntegerFP32<N> for either BSNumber of BSRational
// are determined in GeometricTools/GTEngine/Tools/PrecisionCalculator.  These
// N-values are worst case scenarios. Your specific input data might require
// much smaller N, in which case you can modify PrecisionCalculator to use the
// BSPrecision(int32_t,int32_t,int32_t,bool) constructors.

namespace gte
{
    template <typename Real>
    class PrimalQuery2
    {
    public:
        // The caller is responsible for ensuring that the array is not empty
        // before calling queries and that the indices passed to the queries
        // are valid.  The class does no range checking.
        PrimalQuery2()
            :
            mNumVertices(0),
            mVertices(nullptr)
        {
        }

        PrimalQuery2(int numVertices, Vector2<Real> const* vertices)
            :
            mNumVertices(numVertices),
            mVertices(vertices)
        {
        }

        // Member access.
        inline void Set(int numVertices, Vector2<Real> const* vertices)
        {
            mNumVertices = numVertices;
            mVertices = vertices;
        }

        inline int GetNumVertices() const
        {
            return mNumVertices;
        }

        inline Vector2<Real> const* GetVertices() const
        {
            return mVertices;
        }

        // In the following, point P refers to vertices[i] or 'test' and Vi
        // refers to vertices[vi].

        // For a line with origin V0 and direction <V0,V1>, ToLine returns
        //   +1, P on right of line
        //   -1, P on left of line
        //    0, P on the line
        //
        // Choice of N for UIntegerFP32<N>.
        //    input type | compute type | N
        //    -----------+--------------+----
        //    float      | BSNumber     |  18
        //    double     | BSNumber     | 132
        //    float      | BSRational   |  35
        //    double     | BSRational   | 263
        int ToLine(int i, int v0, int v1) const
        {
            return ToLine(mVertices[i], v0, v1);
        }

        int ToLine(Vector2<Real> const& test, int v0, int v1) const
        {
            Vector2<Real> const& vec0 = mVertices[v0];
            Vector2<Real> const& vec1 = mVertices[v1];

            Real x0 = test[0] - vec0[0];
            Real y0 = test[1] - vec0[1];
            Real x1 = vec1[0] - vec0[0];
            Real y1 = vec1[1] - vec0[1];
            Real x0y1 = x0 * y1;
            Real x1y0 = x1 * y0;
            Real det = x0y1 - x1y0;
            Real const zero(0);

            return (det > zero ? +1 : (det < zero ? -1 : 0));
        }

        // For a line with origin V0 and direction <V0,V1>, ToLine returns
        //   +1, P on right of line
        //   -1, P on left of line
        //    0, P on the line
        // The 'order' parameter is
        //   -3, points not collinear, P on left of line
        //   -2, P strictly left of V0 on the line
        //   -1, P = V0
        //    0, P interior to line segment [V0,V1]
        //   +1, P = V1
        //   +2, P strictly right of V0 on the line
        //
        // Choice of N for UIntegerFP32<N>.
        //    input type | compute type | N
        //    -----------+--------------+----
        //    float      | BSNumber     |  18
        //    double     | BSNumber     | 132
        //    float      | BSRational   |  35
        //    double     | BSRational   | 263
        // This is the same as the first-listed ToLine calls because the
        // worst-case path has the same computational complexity.
        int ToLine(int i, int v0, int v1, int& order) const
        {
            return ToLine(mVertices[i], v0, v1, order);
        }

        int ToLine(Vector2<Real> const& test, int v0, int v1, int& order) const
        {
            Vector2<Real> const& vec0 = mVertices[v0];
            Vector2<Real> const& vec1 = mVertices[v1];

            Real x0 = test[0] - vec0[0];
            Real y0 = test[1] - vec0[1];
            Real x1 = vec1[0] - vec0[0];
            Real y1 = vec1[1] - vec0[1];
            Real x0y1 = x0 * y1;
            Real x1y0 = x1 * y0;
            Real det = x0y1 - x1y0;
            Real const zero(0);

            if (det > zero)
            {
                order = +3;
                return +1;
            }

            if (det < zero)
            {
                order = -3;
                return -1;
            }

            Real x0x1 = x0 * x1;
            Real y0y1 = y0 * y1;
            Real dot = x0x1 + y0y1;
            if (dot == zero)
            {
                order = -1;
            }
            else if (dot < zero)
            {
                order = -2;
            }
            else
            {
                Real x0x0 = x0 * x0;
                Real y0y0 = y0 * y0;
                Real sqrLength = x0x0 + y0y0;
                if (dot == sqrLength)
                {
                    order = +1;
                }
                else if (dot > sqrLength)
                {
                    order = +2;
                }
                else
                {
                    order = 0;
                }
            }

            return 0;
        }

        // For a triangle with counterclockwise vertices V0, V1, and V2,
        // ToTriangle returns
        //   +1, P outside triangle
        //   -1, P inside triangle
        //    0, P on triangle
        //
        // Choice of N for UIntegerFP32<N>.
        //    input type | compute type | N
        //    -----------+--------------+-----
        //    float      | BSNumber     |   18
        //    double     | BSNumber     |  132
        //    float      | BSRational   |   35
        //    double     | BSRational   |  263
        // The query involves three calls to ToLine, so the numbers match
        // those of ToLine.
        int ToTriangle(int i, int v0, int v1, int v2) const
        {
            return ToTriangle(mVertices[i], v0, v1, v2);
        }

        int ToTriangle(Vector2<Real> const& test, int v0, int v1, int v2) const
        {
            int sign0 = ToLine(test, v1, v2);
            if (sign0 > 0)
            {
                return +1;
            }

            int sign1 = ToLine(test, v0, v2);
            if (sign1 < 0)
            {
                return +1;
            }

            int sign2 = ToLine(test, v0, v1);
            if (sign2 > 0)
            {
                return +1;
            }

            return ((sign0 && sign1 && sign2) ? -1 : 0);
        }

        // For a triangle with counterclockwise vertices V0, V1, and V2,
        // ToCircumcircle returns
        //   +1, P outside circumcircle of triangle
        //   -1, P inside circumcircle of triangle
        //    0, P on circumcircle of triangle
        //
        // Choice of N for UIntegerFP32<N>.
        //    input type | compute type | N
        //    -----------+--------------+----
        //    float      | BSNumber     |  35
        //    double     | BSNumber     | 263
        //    float      | BSRational   | 105
        //    double     | BSRational   | 788
        // The query involves three calls of ToLine, so the numbers match
        // those of ToLine.
        int ToCircumcircle(int i, int v0, int v1, int v2) const
        {
            return ToCircumcircle(mVertices[i], v0, v1, v2);
        }

        int ToCircumcircle(Vector2<Real> const& test, int v0, int v1, int v2) const
        {
            Vector2<Real> const& vec0 = mVertices[v0];
            Vector2<Real> const& vec1 = mVertices[v1];
            Vector2<Real> const& vec2 = mVertices[v2];

            Real x0 = vec0[0] - test[0];
            Real y0 = vec0[1] - test[1];
            Real s00 = vec0[0] + test[0];
            Real s01 = vec0[1] + test[1];
            Real t00 = s00 * x0;
            Real t01 = s01 * y0;
            Real z0 = t00 + t01;

            Real x1 = vec1[0] - test[0];
            Real y1 = vec1[1] - test[1];
            Real s10 = vec1[0] + test[0];
            Real s11 = vec1[1] + test[1];
            Real t10 = s10 * x1;
            Real t11 = s11 * y1;
            Real z1 = t10 + t11;

            Real x2 = vec2[0] - test[0];
            Real y2 = vec2[1] - test[1];
            Real s20 = vec2[0] + test[0];
            Real s21 = vec2[1] + test[1];
            Real t20 = s20 * x2;
            Real t21 = s21 * y2;
            Real z2 = t20 + t21;

            Real y0z1 = y0 * z1;
            Real y0z2 = y0 * z2;
            Real y1z0 = y1 * z0;
            Real y1z2 = y1 * z2;
            Real y2z0 = y2 * z0;
            Real y2z1 = y2 * z1;
            Real c0 = y1z2 - y2z1;
            Real c1 = y2z0 - y0z2;
            Real c2 = y0z1 - y1z0;
            Real x0c0 = x0 * c0;
            Real x1c1 = x1 * c1;
            Real x2c2 = x2 * c2;
            Real term = x0c0 + x1c1;
            Real det = term + x2c2;
            Real const zero(0);

            return (det < zero ? 1 : (det > zero ? -1 : 0));
        }

        // An extended classification of the relationship of a point to a line
        // segment.  For noncollinear points, the return value is
        //   ORDER_POSITIVE when <P,Q0,Q1> is a counterclockwise triangle
        //   ORDER_NEGATIVE when <P,Q0,Q1> is a clockwise triangle
        // For collinear points, the line direction is Q1-Q0.  The return
        // value is
        //   ORDER_COLLINEAR_LEFT when the line ordering is <P,Q0,Q1>
        //   ORDER_COLLINEAR_RIGHT when the line ordering is <Q0,Q1,P>
        //   ORDER_COLLINEAR_CONTAIN when the line ordering is <Q0,P,Q1>
        enum OrderType
        {
            ORDER_Q0_EQUALS_Q1,
            ORDER_P_EQUALS_Q0,
            ORDER_P_EQUALS_Q1,
            ORDER_POSITIVE,
            ORDER_NEGATIVE,
            ORDER_COLLINEAR_LEFT,
            ORDER_COLLINEAR_RIGHT,
            ORDER_COLLINEAR_CONTAIN
        };

        // Choice of N for UIntegerFP32<N>.
        //    input type | compute type | N
        //    -----------+--------------+----
        //    float      | BSNumber     |  18
        //    double     | BSNumber     | 132
        //    float      | BSRational   |  35
        //    double     | BSRational   | 263
        // This is the same as the first-listed ToLine calls because the
        // worst-case path has the same computational complexity.
        OrderType ToLineExtended(Vector2<Real> const& P, Vector2<Real> const& Q0, Vector2<Real> const& Q1) const
        {
            Real const zero(0);

            Real x0 = Q1[0] - Q0[0];
            Real y0 = Q1[1] - Q0[1];
            if (x0 == zero && y0 == zero)
            {
                return ORDER_Q0_EQUALS_Q1;
            }

            Real x1 = P[0] - Q0[0];
            Real y1 = P[1] - Q0[1];
            if (x1 == zero && y1 == zero)
            {
                return ORDER_P_EQUALS_Q0;
            }

            Real x2 = P[0] - Q1[0];
            Real y2 = P[1] - Q1[1];
            if (x2 == zero && y2 == zero)
            {
                return ORDER_P_EQUALS_Q1;
            }

            // The theoretical classification relies on computing exactly the
            // sign of the determinant.  Numerical roundoff errors can cause
            // misclassification.
            Real x0y1 = x0 * y1;
            Real x1y0 = x1 * y0;
            Real det = x0y1 - x1y0;

            if (det != zero)
            {
                if (det > zero)
                {
                    // The points form a counterclockwise triangle <P,Q0,Q1>.
                    return ORDER_POSITIVE;
                }
                else
                {
                    // The points form a clockwise triangle <P,Q1,Q0>.
                    return ORDER_NEGATIVE;
                }
            }
            else
            {
                // The points are collinear; P is on the line through
                // Q0 and Q1.
                Real x0x1 = x0 * x1;
                Real y0y1 = y0 * y1;
                Real dot = x0x1 + y0y1;
                if (dot < zero)
                {
                    // The line ordering is <P,Q0,Q1>.
                    return ORDER_COLLINEAR_LEFT;
                }

                Real x0x0 = x0 * x0;
                Real y0y0 = y0 * y0;
                Real sqrLength = x0x0 + y0y0;
                if (dot > sqrLength)
                {
                    // The line ordering is <Q0,Q1,P>.
                    return ORDER_COLLINEAR_RIGHT;
                }

                // The line ordering is <Q0,P,Q1> with P strictly between
                // Q0 and Q1.
                return ORDER_COLLINEAR_CONTAIN;
            }
        }

    private:
        int mNumVertices;
        Vector2<Real> const* mVertices;
    };
}
