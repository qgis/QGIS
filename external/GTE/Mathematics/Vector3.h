// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.10

#pragma once

#include <Mathematics/Vector.h>

namespace gte
{
    // Template alias for convenience.
    template <typename Real>
    using Vector3 = Vector<3, Real>;

    // Cross, UnitCross, and DotCross have a template parameter N that should
    // be 3 or 4.  The latter case supports affine vectors in 4D (last
    // component w = 0) when you want to use 4-tuples and 4x4 matrices for
    // affine algebra.

    // Compute the cross product using the formal determinant:
    //   cross = det{{e0,e1,e2},{x0,x1,x2},{y0,y1,y2}}
    //         = (x1*y2-x2*y1, x2*y0-x0*y2, x0*y1-x1*y0)
    // where e0 = (1,0,0), e1 = (0,1,0), e2 = (0,0,1), v0 = (x0,x1,x2), and
    // v1 = (y0,y1,y2).
    template <int N, typename Real>
    Vector<N, Real> Cross(Vector<N, Real> const& v0, Vector<N, Real> const& v1)
    {
        static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");

        Vector<N, Real> result;
        result.MakeZero();
        result[0] = v0[1] * v1[2] - v0[2] * v1[1];
        result[1] = v0[2] * v1[0] - v0[0] * v1[2];
        result[2] = v0[0] * v1[1] - v0[1] * v1[0];
        return result;
    }

    // Compute the normalized cross product.
    template <int N, typename Real>
    Vector<N, Real> UnitCross(Vector<N, Real> const& v0, Vector<N, Real> const& v1, bool robust = false)
    {
        static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");
        Vector<N, Real> unitCross = Cross(v0, v1);
        Normalize(unitCross, robust);
        return unitCross;
    }

    // Compute Dot((x0,x1,x2),Cross((y0,y1,y2),(z0,z1,z2)), the triple scalar
    // product of three vectors, where v0 = (x0,x1,x2), v1 = (y0,y1,y2), and
    // v2 is (z0,z1,z2).
    template <int N, typename Real>
    Real DotCross(Vector<N, Real> const& v0, Vector<N, Real> const& v1,
        Vector<N, Real> const& v2)
    {
        static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");
        return Dot(v0, Cross(v1, v2));
    }

    // Compute a right-handed orthonormal basis for the orthogonal complement
    // of the input vectors.  The function returns the smallest length of the
    // unnormalized vectors computed during the process.  If this value is
    // nearly zero, it is possible that the inputs are linearly dependent
    // (within numerical round-off errors).  On input, numInputs must be 1 or
    // 2 and v[0] through v[numInputs-1] must be initialized.  On output, the
    // vectors v[0] through v[2] form an orthonormal set.
    template <typename Real>
    Real ComputeOrthogonalComplement(int numInputs, Vector3<Real>* v, bool robust = false)
    {
        if (numInputs == 1)
        {
            if (std::fabs(v[0][0]) > std::fabs(v[0][1]))
            {
                v[1] = { -v[0][2], (Real)0, +v[0][0] };
            }
            else
            {
                v[1] = { (Real)0, +v[0][2], -v[0][1] };
            }
            numInputs = 2;
        }

        if (numInputs == 2)
        {
            v[2] = Cross(v[0], v[1]);
            return Orthonormalize<3, Real>(3, v, robust);
        }

        return (Real)0;
    }

    // Compute the barycentric coordinates of the point P with respect to the
    // tetrahedron <V0,V1,V2,V3>, P = b0*V0 + b1*V1 + b2*V2 + b3*V3, where
    // b0 + b1 + b2 + b3 = 1.  The return value is 'true' iff {V0,V1,V2,V3} is
    // a linearly independent set.  Numerically, this is measured by
    // |det[V0 V1 V2 V3]| <= epsilon.  The values bary[] are valid only when
    // the return value is 'true' but set to zero when the return value is
    // 'false'.
    template <typename Real>
    bool ComputeBarycentrics(Vector3<Real> const& p, Vector3<Real> const& v0,
        Vector3<Real> const& v1, Vector3<Real> const& v2, Vector3<Real> const& v3,
        Real bary[4], Real epsilon = (Real)0)
    {
        // Compute the vectors relative to V3 of the tetrahedron.
        Vector3<Real> diff[4] = { v0 - v3, v1 - v3, v2 - v3, p - v3 };

        Real det = DotCross(diff[0], diff[1], diff[2]);
        if (det < -epsilon || det > epsilon)
        {
            Real invDet = ((Real)1) / det;
            bary[0] = DotCross(diff[3], diff[1], diff[2]) * invDet;
            bary[1] = DotCross(diff[3], diff[2], diff[0]) * invDet;
            bary[2] = DotCross(diff[3], diff[0], diff[1]) * invDet;
            bary[3] = (Real)1 - bary[0] - bary[1] - bary[2];
            return true;
        }

        for (int i = 0; i < 4; ++i)
        {
            bary[i] = (Real)0;
        }
        return false;
    }

    // Get intrinsic information about the input array of vectors.  The return
    // value is 'true' iff the inputs are valid (numVectors > 0, v is not
    // null, and epsilon >= 0), in which case the class members are valid.
    template <typename Real>
    class IntrinsicsVector3
    {
    public:
        // The constructor sets the class members based on the input set.
        IntrinsicsVector3(int numVectors, Vector3<Real> const* v, Real inEpsilon)
            :
            epsilon(inEpsilon),
            dimension(0),
            maxRange((Real)0),
            origin{ (Real)0, (Real)0, (Real)0 },
            extremeCCW(false)
        {
            min[0] = (Real)0;
            min[1] = (Real)0;
            min[2] = (Real)0;
            direction[0] = { (Real)0, (Real)0, (Real)0 };
            direction[1] = { (Real)0, (Real)0, (Real)0 };
            direction[2] = { (Real)0, (Real)0, (Real)0 };
            extreme[0] = 0;
            extreme[1] = 0;
            extreme[2] = 0;
            extreme[3] = 0;

            if (numVectors > 0 && v && epsilon >= (Real)0)
            {
                // Compute the axis-aligned bounding box for the input vectors.
                // Keep track of the indices into 'vectors' for the current
                // min and max.
                int j, indexMin[3], indexMax[3];
                for (j = 0; j < 3; ++j)
                {
                    min[j] = v[0][j];
                    max[j] = min[j];
                    indexMin[j] = 0;
                    indexMax[j] = 0;
                }

                int i;
                for (i = 1; i < numVectors; ++i)
                {
                    for (j = 0; j < 3; ++j)
                    {
                        if (v[i][j] < min[j])
                        {
                            min[j] = v[i][j];
                            indexMin[j] = i;
                        }
                        else if (v[i][j] > max[j])
                        {
                            max[j] = v[i][j];
                            indexMax[j] = i;
                        }
                    }
                }

                // Determine the maximum range for the bounding box.
                maxRange = max[0] - min[0];
                extreme[0] = indexMin[0];
                extreme[1] = indexMax[0];
                Real range = max[1] - min[1];
                if (range > maxRange)
                {
                    maxRange = range;
                    extreme[0] = indexMin[1];
                    extreme[1] = indexMax[1];
                }
                range = max[2] - min[2];
                if (range > maxRange)
                {
                    maxRange = range;
                    extreme[0] = indexMin[2];
                    extreme[1] = indexMax[2];
                }

                // The origin is either the vector of minimum x0-value, vector
                // of minimum x1-value, or vector of minimum x2-value.
                origin = v[extreme[0]];

                // Test whether the vector set is (nearly) a vector.
                if (maxRange <= epsilon)
                {
                    dimension = 0;
                    for (j = 0; j < 3; ++j)
                    {
                        extreme[j + 1] = extreme[0];
                    }
                    return;
                }

                // Test whether the vector set is (nearly) a line segment.  We
                // need {direction[2],direction[3]} to span the orthogonal
                // complement of direction[0].
                direction[0] = v[extreme[1]] - origin;
                Normalize(direction[0], false);
                if (std::fabs(direction[0][0]) > std::fabs(direction[0][1]))
                {
                    direction[1][0] = -direction[0][2];
                    direction[1][1] = (Real)0;
                    direction[1][2] = +direction[0][0];
                }
                else
                {
                    direction[1][0] = (Real)0;
                    direction[1][1] = +direction[0][2];
                    direction[1][2] = -direction[0][1];
                }
                Normalize(direction[1], false);
                direction[2] = Cross(direction[0], direction[1]);

                // Compute the maximum distance of the points from the line
                // origin + t * direction[0].
                Real maxDistance = (Real)0;
                Real distance, dot;
                extreme[2] = extreme[0];
                for (i = 0; i < numVectors; ++i)
                {
                    Vector3<Real> diff = v[i] - origin;
                    dot = Dot(direction[0], diff);
                    Vector3<Real> proj = diff - dot * direction[0];
                    distance = Length(proj, false);
                    if (distance > maxDistance)
                    {
                        maxDistance = distance;
                        extreme[2] = i;
                    }
                }

                if (maxDistance <= epsilon * maxRange)
                {
                    // The points are (nearly) on the line
                    // origin + t * direction[0].
                    dimension = 1;
                    extreme[2] = extreme[1];
                    extreme[3] = extreme[1];
                    return;
                }

                // Test whether the vector set is (nearly) a planar polygon.
                // The point v[extreme[2]] is farthest from the line:
                // origin + t * direction[0].  The vector
                // v[extreme[2]] - origin is not necessarily perpendicular to
                // direction[0], so project out the direction[0] component so
                // that the result is perpendicular to direction[0].
                direction[1] = v[extreme[2]] - origin;
                dot = Dot(direction[0], direction[1]);
                direction[1] -= dot * direction[0];
                Normalize(direction[1], false);

                // We need direction[2] to span the orthogonal complement of
                // {direction[0],direction[1]}.
                direction[2] = Cross(direction[0], direction[1]);

                // Compute the maximum distance of the points from the plane
                // origin+t0 * direction[0] + t1 * direction[1].
                maxDistance = (Real)0;
                Real maxSign = (Real)0;
                extreme[3] = extreme[0];
                for (i = 0; i < numVectors; ++i)
                {
                    Vector3<Real> diff = v[i] - origin;
                    distance = Dot(direction[2], diff);
                    Real sign = (distance > (Real)0 ? (Real)1 :
                        (distance < (Real)0 ? (Real)-1 : (Real)0));
                    distance = std::fabs(distance);
                    if (distance > maxDistance)
                    {
                        maxDistance = distance;
                        maxSign = sign;
                        extreme[3] = i;
                    }
                }

                if (maxDistance <= epsilon * maxRange)
                {
                    // The points are (nearly) on the plane
                    // origin + t0 * direction[0] + t1 * direction[1].
                    dimension = 2;
                    extreme[3] = extreme[2];
                    return;
                }

                dimension = 3;
                extremeCCW = (maxSign > (Real)0);
                return;
            }
        }

        // A nonnegative tolerance that is used to determine the intrinsic
        // dimension of the set.
        Real epsilon;

        // The intrinsic dimension of the input set, computed based on the
        // nonnegative tolerance mEpsilon.
        int dimension;

        // Axis-aligned bounding box of the input set.  The maximum range is
        // the larger of max[0]-min[0], max[1]-min[1], and max[2]-min[2].
        Real min[3], max[3];
        Real maxRange;

        // Coordinate system.  The origin is valid for any dimension d.  The
        // unit-length direction vector is valid only for 0 <= i < d.  The
        // extreme index is relative to the array of input points, and is also
        // valid only for 0 <= i < d.  If d = 0, all points are effectively
        // the same, but the use of an epsilon may lead to an extreme index
        // that is not zero.  If d = 1, all points effectively lie on a line
        // segment.  If d = 2, all points effectively line on a plane.  If
        // d = 3, the points are not coplanar.
        Vector3<Real> origin;
        Vector3<Real> direction[3];

        // The indices that define the maximum dimensional extents.  The
        // values extreme[0] and extreme[1] are the indices for the points
        // that define the largest extent in one of the coordinate axis
        // directions.  If the dimension is 2, then extreme[2] is the index
        // for the point that generates the largest extent in the direction
        // perpendicular to the line through the points corresponding to
        // extreme[0] and extreme[1].  Furthermore, if the dimension is 3,
        // then extreme[3] is the index for the point that generates the
        // largest extent in the direction perpendicular to the triangle
        // defined by the other extreme points.  The tetrahedron formed by the
        // points V[extreme[0]], V[extreme[1]], V[extreme[2]], and
        // V[extreme[3]] is clockwise or counterclockwise, the condition
        // stored in extremeCCW.
        int extreme[4];
        bool extremeCCW;
    };
}
