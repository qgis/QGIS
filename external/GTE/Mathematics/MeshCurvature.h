// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Matrix2x2.h>
#include <Mathematics/Matrix3x3.h>

// The MeshCurvature class estimates principal curvatures and principal
// directions at the vertices of a manifold triangle mesh.  The algorithm
// is described in
// https://www.geometrictools.com/Documentation/MeshDifferentialGeometry.pdf

namespace gte
{
    template <typename Real>
    class MeshCurvature
    {
    public:
        MeshCurvature() = default;

        // The input to operator() is a triangle mesh with the specified
        // vertex buffer and index buffer.  The number of elements of
        // 'indices' must be a multiple of 3, each triple of indices
        // (3*t, 3*t+1, 3*t+2) representing the triangle with vertices
        // (vertices[3*t], vertices[3*t+1], vertices[3*t+2]).  The
        // singularity threshold is a small nonnegative number.  It is
        // used to characterize whether the DWTrn matrix is singular.  In
        // theory, set the threshold to zero.  In practice you might have
        // to set this to a small positive number.

        void operator()(
            size_t numVertices, Vector3<Real> const* vertices,
            size_t numTriangles, unsigned int const* indices,
            Real singularityThreshold)
        {
            mNormals.resize(numVertices);
            mMinCurvatures.resize(numVertices);
            mMaxCurvatures.resize(numVertices);
            mMinDirections.resize(numVertices);
            mMaxDirections.resize(numVertices);

            // Compute the normal vectors for the vertices as an
            // area-weighted sum of the triangles sharing a vertex.
            Vector3<Real> vzero{ (Real)0, (Real)0, (Real)0 };
            std::fill(mNormals.begin(), mNormals.end(), vzero);
            unsigned int const* currentIndex = indices;
            for (size_t i = 0; i < numTriangles; ++i)
            {
                // Get vertex indices.
                unsigned int v0 = *currentIndex++;
                unsigned int v1 = *currentIndex++;
                unsigned int v2 = *currentIndex++;

                // Compute the normal (length provides a weighted sum).
                Vector3<Real> edge1 = vertices[v1] - vertices[v0];
                Vector3<Real> edge2 = vertices[v2] - vertices[v0];
                Vector3<Real> normal = Cross(edge1, edge2);

                mNormals[v0] += normal;
                mNormals[v1] += normal;
                mNormals[v2] += normal;
            }
            for (size_t i = 0; i < numVertices; ++i)
            {
                Normalize(mNormals[i]);
            }

            // Compute the matrix of normal derivatives.
            Matrix3x3<Real> mzero;
            std::vector<Matrix3x3<Real>> DNormal(numVertices, mzero);
            std::vector<Matrix3x3<Real>> WWTrn(numVertices, mzero);
            std::vector<Matrix3x3<Real>> DWTrn(numVertices, mzero);
            std::vector<bool> DWTrnZero(numVertices, false);

            currentIndex = indices;
            for (size_t i = 0; i < numTriangles; ++i)
            {
                // Get vertex indices.
                unsigned int v[3];
                v[0] = *currentIndex++;
                v[1] = *currentIndex++;
                v[2] = *currentIndex++;

                for (size_t j = 0; j < 3; j++)
                {
                    unsigned int v0 = v[j];
                    unsigned int v1 = v[(j + 1) % 3];
                    unsigned int v2 = v[(j + 2) % 3];

                    // Compute the edge direction from vertex v0 to vertex v1,
                    // project it to the tangent plane of vertex v0 and
                    // compute the difference of adjacent normals.
                    Vector3<Real> E = vertices[v1] - vertices[v0];
                    Vector3<Real> W = E - Dot(E, mNormals[v0]) * mNormals[v0];
                    Vector3<Real> D = mNormals[v1] - mNormals[v0];
                    for (int row = 0; row < 3; ++row)
                    {
                        for (int col = 0; col < 3; ++col)
                        {
                            WWTrn[v0](row, col) += W[row] * W[col];
                            DWTrn[v0](row, col) += D[row] * W[col];
                        }
                    }

                    // Compute the edge direction from vertex v0 to vertex v2,
                    // project it to the tangent plane of vertex v0 and
                    // compute the difference of adjacent normals.
                    E = vertices[v2] - vertices[v0];
                    W = E - Dot(E, mNormals[v0]) * mNormals[v0];
                    D = mNormals[v2] - mNormals[v0];
                    for (int row = 0; row < 3; ++row)
                    {
                        for (int col = 0; col < 3; ++col)
                        {
                            WWTrn[v0](row, col) += W[row] * W[col];
                            DWTrn[v0](row, col) += D[row] * W[col];
                        }
                    }
                }
            }

            // Add in N*N^T to W*W^T for numerical stability.  In theory 0*0^T
            // is added to D*W^T, but of course no update is needed in the
            // implementation.  Compute the matrix of normal derivatives.
            for (size_t i = 0; i < numVertices; ++i)
            {
                for (int row = 0; row < 3; ++row)
                {
                    for (int col = 0; col < 3; ++col)
                    {
                        WWTrn[i](row, col) = (Real)0.5 * WWTrn[i](row, col) +
                            mNormals[i][row] * mNormals[i][col];
                        DWTrn[i](row, col) *= (Real)0.5;
                    }
                }

                // Compute the max-abs entry of D*W^T.  If this entry is
                // (nearly) zero, flag the DNormal matrix as singular.
                Real maxAbs = (Real)0;
                for (int row = 0; row < 3; ++row)
                {
                    for (int col = 0; col < 3; ++col)
                    {
                        Real absEntry = std::fabs(DWTrn[i](row, col));
                        if (absEntry > maxAbs)
                        {
                            maxAbs = absEntry;
                        }
                    }
                }
                if (maxAbs < singularityThreshold)
                {
                    DWTrnZero[i] = true;
                }

                DNormal[i] = DWTrn[i] * Inverse(WWTrn[i]);
            }

            // If N is a unit-length normal at a vertex, let U and V be
            // unit-length tangents so that {U, V, N} is an orthonormal set.
            // Define the matrix J = [U | V], a 3-by-2 matrix whose columns
            // are U and V.  Define J^T to be the transpose of J, a 2-by-3
            // matrix.  Let dN/dX denote the matrix of first-order derivatives
            // of the normal vector field.  The shape matrix is
            //   S = (J^T * J)^{-1} * J^T * dN/dX * J = J^T * dN/dX * J
            // where the superscript of -1 denotes the inverse; the formula
            // allows for J to be created from non-perpendicular vectors. The
            // matrix S is 2-by-2.  The principal curvatures are the
            // eigenvalues of S.  If k is a principal curvature and W is the
            // 2-by-1 eigenvector corresponding to it, then S*W = k*W (by
            // definition).  The corresponding 3-by-1 tangent vector at the
            // vertex is a principal direction for k and is J*W.
            for (size_t i = 0; i < numVertices; ++i)
            {
                // Compute U and V given N.
                Vector3<Real> basis[3];
                basis[0] = mNormals[i];
                ComputeOrthogonalComplement(1, basis);
                Vector3<Real> const& U = basis[1];
                Vector3<Real> const& V = basis[2];

                if (DWTrnZero[i])
                {
                    // At a locally planar point.
                    mMinCurvatures[i] = (Real)0;
                    mMaxCurvatures[i] = (Real)0;
                    mMinDirections[i] = U;
                    mMaxDirections[i] = V;
                    continue;
                }

                // Compute S = J^T * dN/dX * J.  In theory S is symmetric, but
                // because dN/dX is estimated, we must ensure that the
                // computed S is symmetric.
                Real s00 = Dot(U, DNormal[i] * U);
                Real s01 = Dot(U, DNormal[i] * V);
                Real s10 = Dot(V, DNormal[i] * U);
                Real s11 = Dot(V, DNormal[i] * V);
                Real avr = (Real)0.5 * (s01 + s10);
                Matrix2x2<Real> S{ s00, avr, avr, s11 };

                // Compute the eigenvalues of S (min and max curvatures).
                Real trace = S(0, 0) + S(1, 1);
                Real det = S(0, 0) * S(1, 1) - S(0, 1) * S(1, 0);
                Real discr = trace * trace - (Real)4.0 * det;
                Real rootDiscr = std::sqrt(std::max(discr, (Real)0));
                mMinCurvatures[i] = (Real)0.5* (trace - rootDiscr);
                mMaxCurvatures[i] = (Real)0.5* (trace + rootDiscr);

                // Compute the eigenvectors of S.
                Vector2<Real> W0{ S(0, 1), mMinCurvatures[i] - S(0, 0) };
                Vector2<Real> W1{ mMinCurvatures[i] - S(1, 1), S(1, 0) };
                if (Dot(W0, W0) >= Dot(W1, W1))
                {
                    Normalize(W0);
                    mMinDirections[i] = W0[0] * U + W0[1] * V;
                }
                else
                {
                    Normalize(W1);
                    mMinDirections[i] = W1[0] * U + W1[1] * V;
                }

                W0 = Vector2<Real>{ S(0, 1), mMaxCurvatures[i] - S(0, 0) };
                W1 = Vector2<Real>{ mMaxCurvatures[i] - S(1, 1), S(1, 0) };
                if (Dot(W0, W0) >= Dot(W1, W1))
                {
                    Normalize(W0);
                    mMaxDirections[i] = W0[0] * U + W0[1] * V;
                }
                else
                {
                    Normalize(W1);
                    mMaxDirections[i] = W1[0] * U + W1[1] * V;
                }
            }
        }

        void operator()(
            std::vector<Vector3<Real>> const& vertices,
            std::vector<unsigned int> const& indices,
            Real singularityThreshold)
        {
            operator()(vertices.size(), vertices.data(), indices.size() / 3,
                indices.data(), singularityThreshold);
        }

        inline std::vector<Vector3<Real>> const& GetNormals() const
        {
            return mNormals;
        }

        inline std::vector<Real> const& GetMinCurvatures() const
        {
            return mMinCurvatures;
        }

        inline std::vector<Real> const& GetMaxCurvatures() const
        {
            return mMaxCurvatures;
        }

        inline std::vector<Vector3<Real>> const& GetMinDirections() const
        {
            return mMinDirections;
        }

        inline std::vector<Vector3<Real>> const& GetMaxDirections() const
        {
            return mMaxDirections;
        }

    private:
        std::vector<Vector3<Real>> mNormals;
        std::vector<Real> mMinCurvatures;
        std::vector<Real> mMaxCurvatures;
        std::vector<Vector3<Real>> mMinDirections;
        std::vector<Vector3<Real>> mMaxDirections;
    };
}
