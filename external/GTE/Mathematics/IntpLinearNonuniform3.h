// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Vector3.h>

// Linear interpolation of a network of triangles whose vertices are of the
// form (x,y,z,f(x,y,z)).  The function samples are F[i] and represent
// f(x[i],y[i],z[i]), where i is the index of the input vertex
// (x[i],y[i],z[i]) to Delaunay3.
//
// The TetrahedronMesh interface must support the following:
//   int GetContainingTetrahedron(Vector3<Real> const&) const;
//   bool GetIndices(int, std::array<int, 4>&) const;
//   bool GetBarycentrics(int, Vector3<Real> const&, Real[4]) const;

namespace gte
{
    template <typename Real, typename TetrahedronMesh>
    class IntpLinearNonuniform3
    {
    public:
        // Construction.
        IntpLinearNonuniform3(TetrahedronMesh const& mesh, Real const* F)
            :
            mMesh(&mesh),
            mF(F)
        {
            LogAssert(mF != nullptr, "Invalid input.");
        }

        // Linear interpolation.  The return value is 'true' if and only if
        // the input point is in the convex hull of the input vertices, in
        // which case the interpolation is valid.
        bool operator()(Vector3<Real> const& P, Real& F) const
        {
            int t = mMesh->GetContainingTetrahedron(P);
            if (t == -1)
            {
                // The point is outside the tetrahedralization.
                return false;
            }

            // Get the barycentric coordinates of P with respect to the tetrahedron,
            // P = b0*V0 + b1*V1 + b2*V2 + b3*V3, where b0 + b1 + b2 + b3 = 1.
            std::array<Real, 4> bary;
            if (!mMesh->GetBarycentrics(t, P, bary))
            {
                // TODO: Throw an exception or allow this as valid behavior?
                // P is in a needle-like, flat, or degenerate tetrahedron.
                return false;
            }

            // The result is a barycentric combination of function values.
            std::array<int, 4> indices;
            mMesh->GetIndices(t, indices);
            F = bary[0] * mF[indices[0]] + bary[1] * mF[indices[1]] +
                bary[2] * mF[indices[2]] + bary[3] * mF[indices[4]];
            return true;
        }

    private:
        TetrahedronMesh const* mMesh;
        Real const* mF;
    };
}
