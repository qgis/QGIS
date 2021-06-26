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
    using Vector2 = Vector<2, Real>;

    // Compute the perpendicular using the formal determinant,
    //   perp = det{{e0,e1},{x0,x1}} = (x1,-x0)
    // where e0 = (1,0), e1 = (0,1), and v = (x0,x1).
    template <typename Real>
    Vector2<Real> Perp(Vector2<Real> const& v)
    {
        return Vector2<Real>{ v[1], -v[0] };
    }

    // Compute the normalized perpendicular.
    template <typename Real>
    Vector2<Real> UnitPerp(Vector2<Real> const& v, bool robust = false)
    {
        Vector2<Real> unitPerp{ v[1], -v[0] };
        Normalize(unitPerp, robust);
        return unitPerp;
    }

    // Compute Dot((x0,x1),Perp(y0,y1)) = x0*y1 - x1*y0, where v0 = (x0,x1)
    // and v1 = (y0,y1).
    template <typename Real>
    Real DotPerp(Vector2<Real> const& v0, Vector2<Real> const& v1)
    {
        return Dot(v0, Perp(v1));
    }

    // Compute a right-handed orthonormal basis for the orthogonal complement
    // of the input vectors.  The function returns the smallest length of the
    // unnormalized vectors computed during the process.  If this value is
    // nearly zero, it is possible that the inputs are linearly dependent
    // (within numerical round-off errors).  On input, numInputs must be 1 and
    // v[0] must be initialized.  On output, the vectors v[0] and v[1] form an
    // orthonormal set.
    template <typename Real>
    Real ComputeOrthogonalComplement(int numInputs, Vector2<Real>* v, bool robust = false)
    {
        if (numInputs == 1)
        {
            v[1] = -Perp(v[0]);
            return Orthonormalize<2, Real>(2, v, robust);
        }

        return (Real)0;
    }

    // Compute the barycentric coordinates of the point P with respect to the
    // triangle <V0,V1,V2>, P = b0*V0 + b1*V1 + b2*V2, where b0 + b1 + b2 = 1.
    // The return value is 'true' iff {V0,V1,V2} is a linearly independent
    // set.  Numerically, this is measured by |det[V0 V1 V2]| <= epsilon.  The
    // values bary[] are valid only when the return value is 'true' but set to
    // zero when the return value is 'false'.
    template <typename Real>
    bool ComputeBarycentrics(Vector2<Real> const& p, Vector2<Real> const& v0,
        Vector2<Real> const& v1, Vector2<Real> const& v2, Real bary[3],
        Real epsilon = (Real)0)
    {
        // Compute the vectors relative to V2 of the triangle.
        Vector2<Real> diff[3] = { v0 - v2, v1 - v2, p - v2 };

        Real det = DotPerp(diff[0], diff[1]);
        if (det < -epsilon || det > epsilon)
        {
            Real invDet = (Real)1 / det;
            bary[0] = DotPerp(diff[2], diff[1]) * invDet;
            bary[1] = DotPerp(diff[0], diff[2]) * invDet;
            bary[2] = (Real)1 - bary[0] - bary[1];
            return true;
        }

        for (int i = 0; i < 3; ++i)
        {
            bary[i] = (Real)0;
        }
        return false;
    }

    // Get intrinsic information about the input array of vectors.  The return
    // value is 'true' iff the inputs are valid (numVectors > 0, v is not
    // null, and epsilon >= 0), in which case the class members are valid.
    template <typename Real>
    class IntrinsicsVector2
    {
    public:
        // The constructor sets the class members based on the input set.
        IntrinsicsVector2(int numVectors, Vector2<Real> const* v, Real inEpsilon)
            :
            epsilon(inEpsilon),
            dimension(0),
            maxRange((Real)0),
            origin{ (Real)0, (Real)0 },
            extremeCCW(false)
        {
            min[0] = (Real)0;
            min[1] = (Real)0;
            direction[0] = { (Real)0, (Real)0 };
            direction[1] = { (Real)0, (Real)0 };
            extreme[0] = 0;
            extreme[1] = 0;
            extreme[2] = 0;

            if (numVectors > 0 && v && epsilon >= (Real)0)
            {
                // Compute the axis-aligned bounding box for the input
                // vectors.  Keep track of the indices into 'vectors' for the
                // current min and max.
                int j, indexMin[2], indexMax[2];
                for (j = 0; j < 2; ++j)
                {
                    min[j] = v[0][j];
                    max[j] = min[j];
                    indexMin[j] = 0;
                    indexMax[j] = 0;
                }

                int i;
                for (i = 1; i < numVectors; ++i)
                {
                    for (j = 0; j < 2; ++j)
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

                // The origin is either the vector of minimum x0-value or
                // vector of minimum x1-value.
                origin = v[extreme[0]];

                // Test whether the vector set is (nearly) a vector.
                if (maxRange <= epsilon)
                {
                    dimension = 0;
                    for (j = 0; j < 2; ++j)
                    {
                        extreme[j + 1] = extreme[0];
                    }
                    return;
                }

                // Test whether the vector set is (nearly) a line segment.  We
                // need direction[1] to span the orthogonal complement of
                // direction[0].
                direction[0] = v[extreme[1]] - origin;
                Normalize(direction[0], false);
                direction[1] = -Perp(direction[0]);

                // Compute the maximum distance of the points from the line
                // origin+t*direction[0].
                Real maxDistance = (Real)0;
                Real maxSign = (Real)0;
                extreme[2] = extreme[0];
                for (i = 0; i < numVectors; ++i)
                {
                    Vector2<Real> diff = v[i] - origin;
                    Real distance = Dot(direction[1], diff);
                    Real sign = (distance > (Real)0 ? (Real)1 :
                        (distance < (Real)0 ? (Real)-1 : (Real)0));
                    distance = std::fabs(distance);
                    if (distance > maxDistance)
                    {
                        maxDistance = distance;
                        maxSign = sign;
                        extreme[2] = i;
                    }
                }

                if (maxDistance <= epsilon * maxRange)
                {
                    // The points are (nearly) on the line
                    // origin + t * direction[0].
                    dimension = 1;
                    extreme[2] = extreme[1];
                    return;
                }

                dimension = 2;
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
        // the larger of max[0]-min[0] and max[1]-min[1].
        Real min[2], max[2];
        Real maxRange;

        // Coordinate system.  The origin is valid for any dimension d.  The
        // unit-length direction vector is valid only for 0 <= i < d.  The
        // extreme index is relative to the array of input points, and is also
        // valid only for 0 <= i < d.  If d = 0, all points are effectively
        // the same, but the use of an epsilon may lead to an extreme index
        // that is not zero.  If d = 1, all points effectively lie on a line
        // segment.  If d = 2, the points are not collinear.
        Vector2<Real> origin;
        Vector2<Real> direction[2];

        // The indices that define the maximum dimensional extents.  The
        // values extreme[0] and extreme[1] are the indices for the points
        // that define the largest extent in one of the coordinate axis
        // directions.  If the dimension is 2, then extreme[2] is the index
        // for the point that generates the largest extent in the direction
        // perpendicular to the line through the points corresponding to
        // extreme[0] and extreme[1].  The triangle formed by the points
        // V[extreme[0]], V[extreme[1]], and V[extreme[2]] is clockwise or
        // counterclockwise, the condition stored in extremeCCW.
        int extreme[3];
        bool extremeCCW;
    };
}
