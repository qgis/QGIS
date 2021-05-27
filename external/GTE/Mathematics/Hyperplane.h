// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.04.23

#pragma once

#include <Mathematics/Matrix.h>
#include <Mathematics/SingularValueDecomposition.h>
#include <Mathematics/Vector3.h>

// The plane is represented as Dot(U,X) = c where U is a unit-length normal
// vector, c is the plane constant, and X is any point on the plane.  The user
// must ensure that the normal vector is unit length.

namespace gte
{
    template <int N, typename Real>
    class Hyperplane
    {
    public:
        // Construction and destruction.  The default constructor sets the
        // normal to (0,...,0,1) and the constant to zero (plane z = 0).
        Hyperplane()
            :
            constant((Real)0)
        {
            normal.MakeUnit(N - 1);
        }

        // Specify U and c directly.
        Hyperplane(Vector<N, Real> const& inNormal, Real inConstant)
            :
            normal(inNormal),
            constant(inConstant)
        {
        }

        // U is specified, c = Dot(U,p) where p is a point on the hyperplane.
        Hyperplane(Vector<N, Real> const& inNormal, Vector<N, Real> const& p)
            :
            normal(inNormal),
            constant(Dot(inNormal, p))
        {
        }

        // U is a unit-length vector in the orthogonal complement of the set
        // {p[1]-p[0],...,p[n-1]-p[0]} and c = Dot(U,p[0]), where the p[i] are
        // pointson the hyperplane.
        Hyperplane(std::array<Vector<N, Real>, N> const& p)
        {
            ComputeFromPoints<N>(p);
        }

        // Public member access.
        Vector<N, Real> normal;
        Real constant;

    public:
        // Comparisons to support sorted containers.
        bool operator==(Hyperplane const& hyperplane) const
        {
            return normal == hyperplane.normal && constant == hyperplane.constant;
        }

        bool operator!=(Hyperplane const& hyperplane) const
        {
            return !operator==(hyperplane);
        }

        bool operator< (Hyperplane const& hyperplane) const
        {
            if (normal < hyperplane.normal)
            {
                return true;
            }

            if (normal > hyperplane.normal)
            {
                return false;
            }

            return constant < hyperplane.constant;
        }

        bool operator<=(Hyperplane const& hyperplane) const
        {
            return !hyperplane.operator<(*this);
        }

        bool operator> (Hyperplane const& hyperplane) const
        {
            return hyperplane.operator<(*this);
        }

        bool operator>=(Hyperplane const& hyperplane) const
        {
            return !operator<(hyperplane);
        }

    private:
        // TODO: This is used in the
        // Hyperplane(std::array<Vector<N, Real>, N> const&) constructor to
        // have separate implementations for N = 3 and N != 3. A bug report
        // was filed for that constructor with code executed on a QEMU/KVM
        // virtual machine, which indicated the singular value decomposition
        // was producing inaccurate results. I am unable to reproduce the
        // problem on a non-virtual machine; the SVD works correctly for the
        // dataset included in the bug report. I need to determine what the
        // virtual machine is doing that causes such inaccurate results when
        // using floating-point arithmetic.
        template <int Dimension = N>
        typename std::enable_if<Dimension != 3, void>::type
        ComputeFromPoints(std::array<Vector<Dimension, Real>, Dimension> const& p)
        {
            Matrix<Dimension, Dimension - 1, Real> edge;
            for (int i = 0; i < Dimension - 1; ++i)
            {
                edge.SetCol(i, p[i + 1] - p[0]);
            }

            // Compute the 1-dimensional orthogonal complement of the edges of
            // the simplex formed by the points p[].
            SingularValueDecomposition<Real> svd(Dimension, Dimension - 1, 32);
            svd.Solve(&edge[0], -1);
            svd.GetUColumn(Dimension - 1, &normal[0]);

            constant = Dot(normal, p[0]);
        }

        template <int Dimension = N>
        typename std::enable_if<Dimension == 3, void>::type
        ComputeFromPoints(std::array<Vector<Dimension, Real>, Dimension> const& p)
        {
            Vector<Dimension, Real> edge0 = p[1] - p[0];
            Vector<Dimension, Real> edge1 = p[2] - p[0];
            normal = UnitCross(edge0, edge1);
            constant = Dot(normal, p[0]);
        }
    };

    // Template alias for convenience.
    template <typename Real>
    using Plane3 = Hyperplane<3, Real>;
}
