// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.11.16

#pragma once

#include <Mathematics/Delaunay2.h>
#include <Mathematics/ContScribeCircle2.h>
#include <Mathematics/DistPointAlignedBox.h>

// Quadratic interpolation of a network of triangles whose vertices are of
// the form (x,y,f(x,y)).  This code is an implementation of the algorithm
// found in
//
//   Zoltan J. Cendes and Steven H. Wong,
//   C1 quadratic interpolation over arbitrary point sets,
//   IEEE Computer Graphics & Applications,
//   pp. 8-16, 1987
//
// The TriangleMesh interface must support the following:
//   int GetNumVertices() const;
//   int GetNumTriangles() const;
//   Vector2<Real> const* GetVertices() const;
//   int const* GetIndices() const;
//   bool GetVertices(int, std::array<Vector2<Real>, 3>&) const;
//   bool GetIndices(int, std::array<int, 3>&) const;
//   bool GetAdjacencies(int, std::array<int, 3>&) const;
//   bool GetBarycentrics(int, Vector2<Real> const&,
//      std::array<Real, 3>&) const;
//   int GetContainingTriangle(Vector2<Real> const&) const;

namespace gte
{
    template <typename Real, typename TriangleMesh>
    class IntpQuadraticNonuniform2
    {
    public:
        // Construction.
        //
        // The first constructor requires only F and a measure of the rate of
        // change of the function values relative to changes in the spatial
        // variables.  The df/dx and df/dy values are estimated at the sample
        // points using mesh normals and spatialDelta.
        //
        // The second constructor requires you to specify function values F
        // and first-order partial derivative values df/dx and df/dy.

        IntpQuadraticNonuniform2(TriangleMesh const& mesh, Real const* F, Real spatialDelta)
            :
            mMesh(&mesh),
            mF(F),
            mFX(nullptr),
            mFY(nullptr)
        {
            EstimateDerivatives(spatialDelta);
            ProcessTriangles();
        }

        IntpQuadraticNonuniform2(TriangleMesh const& mesh, Real const* F, Real const* FX, Real const* FY)
            :
            mMesh(&mesh),
            mF(F),
            mFX(FX),
            mFY(FY)
        {
            ProcessTriangles();
        }

        // Quadratic interpolation.  The return value is 'true' if and only if
        // the input point is in the convex hull of the input vertices, in
        // which case the interpolation is valid.
        bool operator()(Vector2<Real> const& P, Real& F, Real& FX, Real& FY) const
        {
            auto t = mMesh->GetContainingTriangle(P);
            if (t == mMesh->GetInvalidIndex())
            {
                // The point is outside the triangulation.
                return false;
            }

            // Get the vertices of the triangle.
            std::array<Vector2<Real>, 3> V;
            mMesh->GetVertices(t, V);

            // Get the additional information for the triangle.
            TriangleData const& tData = mTData[t];

            // Determine which of the six subtriangles contains the target
            // point.  Theoretically, P must be in one of these subtriangles.
            Vector2<Real> sub0 = tData.center;
            Vector2<Real> sub1;
            Vector2<Real> sub2 = tData.intersect[2];
            Vector3<Real> bary;
            int index;

            Real const zero = (Real)0, one = (Real)1;
            AlignedBox3<Real> barybox({ zero, zero, zero }, { one, one, one });
            DCPQuery<Real, Vector3<Real>, AlignedBox3<Real>> pbQuery;
            int minIndex = 0;
            Real minDistance = (Real)-1;
            Vector3<Real> minBary{ (Real)0, (Real)0, (Real)0 };
            Vector2<Real> minSub0{ (Real)0, (Real)0 };
            Vector2<Real> minSub1{ (Real)0, (Real)0 };
            Vector2<Real> minSub2{ (Real)0, (Real)0 };

            for (index = 1; index <= 6; ++index)
            {
                sub1 = sub2;
                if (index % 2)
                {
                    sub2 = V[index / 2];
                }
                else
                {
                    sub2 = tData.intersect[index / 2 - 1];
                }

                bool valid = ComputeBarycentrics(P, sub0, sub1, sub2, &bary[0]);
                if (valid
                    && zero <= bary[0] && bary[0] <= one
                    && zero <= bary[1] && bary[1] <= one
                    && zero <= bary[2] && bary[2] <= one)
                {
                    // P is in triangle <Sub0,Sub1,Sub2>
                    break;
                }

                // When computing with floating-point arithmetic, rounding
                // errors can cause us to reach this code when, theoretically,
                // the point is in the subtriangle.  Keep track of the
                // (b0,b1,b2) that is closest to the barycentric cube [0,1]^3
                // and choose the triangle corresponding to it when all 6
                // tests previously fail.
                Real distance = pbQuery(bary, barybox).distance;
                if (minIndex == 0 || distance < minDistance)
                {
                    minDistance = distance;
                    minIndex = index;
                    minBary = bary;
                    minSub0 = sub0;
                    minSub1 = sub1;
                    minSub2 = sub2;
                }
            }

            // If the subtriangle was not found, rounding errors caused
            // problems.  Choose the barycentric point closest to the box.
            if (index > 6)
            {
                index = minIndex;
                bary = minBary;
                sub0 = minSub0;
                sub1 = minSub1;
                sub2 = minSub2;
            }

            // Fetch Bezier control points.
            Real bez[6] =
            {
                tData.coeff[0],
                tData.coeff[12 + index],
                tData.coeff[13 + (index % 6)],
                tData.coeff[index],
                tData.coeff[6 + index],
                tData.coeff[1 + (index % 6)]
            };

            // Evaluate Bezier quadratic.
            F = bary[0] * (bez[0] * bary[0] + bez[1] * bary[1] + bez[2] * bary[2]) +
                bary[1] * (bez[1] * bary[0] + bez[3] * bary[1] + bez[4] * bary[2]) +
                bary[2] * (bez[2] * bary[0] + bez[4] * bary[1] + bez[5] * bary[2]);

            // Evaluate barycentric derivatives of F.
            Real FU = ((Real)2) * (bez[0] * bary[0] + bez[1] * bary[1] +
                bez[2] * bary[2]);
            Real FV = ((Real)2) * (bez[1] * bary[0] + bez[3] * bary[1] +
                bez[4] * bary[2]);
            Real FW = ((Real)2) * (bez[2] * bary[0] + bez[4] * bary[1] +
                bez[5] * bary[2]);
            Real duw = FU - FW;
            Real dvw = FV - FW;

            // Convert back to (x,y) coordinates.
            Real m00 = sub0[0] - sub2[0];
            Real m10 = sub0[1] - sub2[1];
            Real m01 = sub1[0] - sub2[0];
            Real m11 = sub1[1] - sub2[1];
            Real inv = ((Real)1) / (m00 * m11 - m10 * m01);

            FX = inv * (m11 * duw - m10 * dvw);
            FY = inv * (m00 * dvw - m01 * duw);
            return true;
        }

    private:
        void EstimateDerivatives(Real spatialDelta)
        {
            auto numVertices = mMesh->GetNumVertices();
            Vector2<Real> const* vertices = mMesh->GetVertices();
            auto numTriangles = mMesh->GetNumTriangles();
            int const* indices = mMesh->GetIndices();

            mFXStorage.resize(numVertices);
            mFYStorage.resize(numVertices);
            std::vector<Real> FZ(numVertices);
            std::fill(mFXStorage.begin(), mFXStorage.end(), (Real)0);
            std::fill(mFYStorage.begin(), mFYStorage.end(), (Real)0);
            std::fill(FZ.begin(), FZ.end(), (Real)0);

            mFX = &mFXStorage[0];
            mFY = &mFYStorage[0];

            // Accumulate normals at spatial locations (averaging process).
            for (auto t = 0; t < numTriangles; ++t)
            {
                // Get three vertices of triangle.
                int v0 = *indices++;
                int v1 = *indices++;
                int v2 = *indices++;

                // Compute normal vector of triangle (with positive
                // z-component).
                Real dx1 = vertices[v1][0] - vertices[v0][0];
                Real dy1 = vertices[v1][1] - vertices[v0][1];
                Real dz1 = mF[v1] - mF[v0];
                Real dx2 = vertices[v2][0] - vertices[v0][0];
                Real dy2 = vertices[v2][1] - vertices[v0][1];
                Real dz2 = mF[v2] - mF[v0];
                Real nx = dy1 * dz2 - dy2 * dz1;
                Real ny = dz1 * dx2 - dz2 * dx1;
                Real nz = dx1 * dy2 - dx2 * dy1;
                if (nz < (Real)0)
                {
                    nx = -nx;
                    ny = -ny;
                    nz = -nz;
                }

                mFXStorage[v0] += nx;  mFYStorage[v0] += ny;  FZ[v0] += nz;
                mFXStorage[v1] += nx;  mFYStorage[v1] += ny;  FZ[v1] += nz;
                mFXStorage[v2] += nx;  mFYStorage[v2] += ny;  FZ[v2] += nz;
            }

            // Scale the normals to form (x,y,-1).
            for (auto i = 0; i < numVertices; ++i)
            {
                if (FZ[i] != (Real)0)
                {
                    Real inv = -spatialDelta / FZ[i];
                    mFXStorage[i] *= inv;
                    mFYStorage[i] *= inv;
                }
                else
                {
                    mFXStorage[i] = (Real)0;
                    mFYStorage[i] = (Real)0;
                }
            }
        }

        void ProcessTriangles()
        {
            // Add degenerate triangles to boundary triangles so that
            // interpolation at the boundary can be treated in the same way
            // as interpolation in the interior.

            // Compute centers of inscribed circles for triangles.
            Vector2<Real> const* vertices = mMesh->GetVertices();
            auto numTriangles = mMesh->GetNumTriangles();
            int const* indices = mMesh->GetIndices();
            mTData.resize(numTriangles);
            for (auto t = 0; t < numTriangles; ++t)
            {
                int v0 = *indices++;
                int v1 = *indices++;
                int v2 = *indices++;
                Circle2<Real> circle;
                Inscribe(vertices[v0], vertices[v1], vertices[v2], circle);
                mTData[t].center = circle.center;
            }

            // Compute cross-edge intersections.
            for (auto t = 0; t < numTriangles; ++t)
            {
                ComputeCrossEdgeIntersections(t);
            }

            // Compute Bezier coefficients.
            for (auto t = 0; t < numTriangles; ++t)
            {
                ComputeCoefficients(t);
            }
        }

        void ComputeCrossEdgeIntersections(int t)
        {
            // Get the vertices of the triangle.
            std::array<Vector2<Real>, 3> V;
            mMesh->GetVertices(t, V);

            // Get the centers of adjacent triangles.
            TriangleData& tData = mTData[t];
            std::array<int, 3> adjacencies = { 0, 0, 0 };
            mMesh->GetAdjacencies(t, adjacencies);
            for (int j0 = 2, j1 = 0; j1 < 3; j0 = j1++)
            {
                int a = adjacencies[j0];
                if (a >= 0)
                {
                    // Get center of adjacent triangle's inscribing circle.
                    Vector2<Real> U = mTData[a].center;
                    Real m00 = V[j0][1] - V[j1][1];
                    Real m01 = V[j1][0] - V[j0][0];
                    Real m10 = tData.center[1] - U[1];
                    Real m11 = U[0] - tData.center[0];
                    Real r0 = m00 * V[j0][0] + m01 * V[j0][1];
                    Real r1 = m10 * tData.center[0] + m11 * tData.center[1];
                    Real invDet = ((Real)1) / (m00 * m11 - m01 * m10);
                    tData.intersect[j0][0] = (m11 * r0 - m01 * r1) * invDet;
                    tData.intersect[j0][1] = (m00 * r1 - m10 * r0) * invDet;
                }
                else
                {
                    // No adjacent triangle, use center of edge.
                    tData.intersect[j0] = (Real)0.5 * (V[j0] + V[j1]);
                }
            }
        }

        void ComputeCoefficients(int t)
        {
            // Get the vertices of the triangle.
            std::array<Vector2<Real>, 3> V;
            mMesh->GetVertices(t, V);

            // Get the additional information for the triangle.
            TriangleData& tData = mTData[t];

            // Get the sample data at main triangle vertices.
            std::array<int, 3> indices = { 0, 0, 0 };
            mMesh->GetIndices(t, indices);
            std::array<Jet, 3> jet;
            for (int j = 0; j < 3; ++j)
            {
                int k = indices[j];
                jet[j].F = mF[k];
                jet[j].FX = mFX[k];
                jet[j].FY = mFY[k];
            }

            // Get centers of adjacent triangles.
            std::array<int, 3> adjacencies = { 0, 0, 0 };
            mMesh->GetAdjacencies(t, adjacencies);
            Vector2<Real> U[3];
            for (int j0 = 2, j1 = 0; j1 < 3; j0 = j1++)
            {
                int a = adjacencies[j0];
                if (a >= 0)
                {
                    // Get center of adjacent triangle's circumscribing 
                    // circle.
                    U[j0] = mTData[a].center;
                }
                else
                {
                    // No adjacent triangle, use center of edge.
                    U[j0] = ((Real)0.5) * (V[j0] + V[j1]);
                }
            }

            // Compute intermediate terms.
            std::array<Real, 3> cenT, cen0, cen1, cen2;
            mMesh->GetBarycentrics(t, tData.center, cenT);
            mMesh->GetBarycentrics(t, U[0], cen0);
            mMesh->GetBarycentrics(t, U[1], cen1);
            mMesh->GetBarycentrics(t, U[2], cen2);

            Real alpha = (cenT[1] * cen1[0] - cenT[0] * cen1[1]) / (cen1[0] - cenT[0]);
            Real beta = (cenT[2] * cen2[1] - cenT[1] * cen2[2]) / (cen2[1] - cenT[1]);
            Real gamma = (cenT[0] * cen0[2] - cenT[2] * cen0[0]) / (cen0[2] - cenT[2]);
            Real oneMinusAlpha = (Real)1 - alpha;
            Real oneMinusBeta = (Real)1 - beta;
            Real oneMinusGamma = (Real)1 - gamma;

            Real const zero = static_cast<Real>(0);
            std::array<Real, 9> A = { zero, zero, zero, zero, zero, zero, zero, zero, zero };
            std::array<Real, 9> B = { zero, zero, zero, zero, zero, zero, zero, zero, zero };

            Real tmp = cenT[0] * V[0][0] + cenT[1] * V[1][0] + cenT[2] * V[2][0];
            A[0] = (Real)0.5 * (tmp - V[0][0]);
            A[1] = (Real)0.5 * (tmp - V[1][0]);
            A[2] = (Real)0.5 * (tmp - V[2][0]);
            A[3] = (Real)0.5 * beta * (V[2][0] - V[0][0]);
            A[4] = (Real)0.5 * oneMinusGamma * (V[1][0] - V[0][0]);
            A[5] = (Real)0.5 * gamma * (V[0][0] - V[1][0]);
            A[6] = (Real)0.5 * oneMinusAlpha * (V[2][0] - V[1][0]);
            A[7] = (Real)0.5 * alpha * (V[1][0] - V[2][0]);
            A[8] = (Real)0.5 * oneMinusBeta * (V[0][0] - V[2][0]);

            tmp = cenT[0] * V[0][1] + cenT[1] * V[1][1] + cenT[2] * V[2][1];
            B[0] = (Real)0.5 * (tmp - V[0][1]);
            B[1] = (Real)0.5 * (tmp - V[1][1]);
            B[2] = (Real)0.5 * (tmp - V[2][1]);
            B[3] = (Real)0.5 * beta * (V[2][1] - V[0][1]);
            B[4] = (Real)0.5 * oneMinusGamma * (V[1][1] - V[0][1]);
            B[5] = (Real)0.5 * gamma * (V[0][1] - V[1][1]);
            B[6] = (Real)0.5 * oneMinusAlpha * (V[2][1] - V[1][1]);
            B[7] = (Real)0.5 * alpha * (V[1][1] - V[2][1]);
            B[8] = (Real)0.5 * oneMinusBeta * (V[0][1] - V[2][1]);

            // Compute Bezier coefficients.
            tData.coeff[2] = jet[0].F;
            tData.coeff[4] = jet[1].F;
            tData.coeff[6] = jet[2].F;

            tData.coeff[14] = jet[0].F + A[0] * jet[0].FX + B[0] * jet[0].FY;
            tData.coeff[7] = jet[0].F + A[3] * jet[0].FX + B[3] * jet[0].FY;
            tData.coeff[8] = jet[0].F + A[4] * jet[0].FX + B[4] * jet[0].FY;
            tData.coeff[16] = jet[1].F + A[1] * jet[1].FX + B[1] * jet[1].FY;
            tData.coeff[9] = jet[1].F + A[5] * jet[1].FX + B[5] * jet[1].FY;
            tData.coeff[10] = jet[1].F + A[6] * jet[1].FX + B[6] * jet[1].FY;
            tData.coeff[18] = jet[2].F + A[2] * jet[2].FX + B[2] * jet[2].FY;
            tData.coeff[11] = jet[2].F + A[7] * jet[2].FX + B[7] * jet[2].FY;
            tData.coeff[12] = jet[2].F + A[8] * jet[2].FX + B[8] * jet[2].FY;

            tData.coeff[5] = alpha * tData.coeff[10] + oneMinusAlpha * tData.coeff[11];
            tData.coeff[17] = alpha * tData.coeff[16] + oneMinusAlpha * tData.coeff[18];
            tData.coeff[1] = beta * tData.coeff[12] + oneMinusBeta * tData.coeff[7];
            tData.coeff[13] = beta * tData.coeff[18] + oneMinusBeta * tData.coeff[14];
            tData.coeff[3] = gamma * tData.coeff[8] + oneMinusGamma * tData.coeff[9];
            tData.coeff[15] = gamma * tData.coeff[14] + oneMinusGamma * tData.coeff[16];
            tData.coeff[0] = cenT[0] * tData.coeff[14] + cenT[1] * tData.coeff[16] + cenT[2] * tData.coeff[18];
        }

        class TriangleData
        {
        public:
            Vector2<Real> center;
            std::array<Vector2<Real>, 3> intersect;
            std::array<Real, 19> coeff;
        };

        class Jet
        {
        public:
            Jet()
                :
                F(static_cast<Real>(0)),
                FX(static_cast<Real>(0)),
                FY(static_cast<Real>(0))
            {
            }

            Real F, FX, FY;
        };

        TriangleMesh const* mMesh;
        Real const* mF;
        Real const* mFX;
        Real const* mFY;
        std::vector<Real> mFXStorage;
        std::vector<Real> mFYStorage;
        std::vector<TriangleData> mTData;
    };
}
