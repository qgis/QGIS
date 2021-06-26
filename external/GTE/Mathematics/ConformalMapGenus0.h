// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.04.22

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/ETManifoldMesh.h>
#include <Mathematics/LinearSystem.h>
#include <Mathematics/Polynomial1.h>
#include <Mathematics/Vector2.h>
#include <Mathematics/Vector3.h>

// Conformally map a 2-dimensional manifold mesh with the topology of a sphere
// to a sphere.  The algorithm is an implementation of the one in the paper
//    S.Haker, S.Angenent, A.Tannenbaum, R.Kikinis, G.Sapiro, and M.Halle.
//    Conformal surface parameterization for texture mapping,
//    IEEE Transactions on Visualization and Computer Graphics,
//    Volume 6, Number 2, pages 181–189, 2000
// The paper is available at https://ieeexplore.ieee.org/document/856998 but
// is not freely downloadable.

namespace gte
{
    template <typename Real>
    class ConformalMapGenus0
    {
    public:
        // The input mesh should be a closed, manifold surface that has the
        // topology of a sphere (genus 0 surface).
        ConformalMapGenus0()
            :
            mSphereRadius(0.0f)
        {
        }

        ~ConformalMapGenus0()
        {
        }

        // The returned 'bool' value is 'true' whenever the conjugate gradient
        // algorithm converged.  Even if it did not, the results might still
        // be acceptable.
        bool operator()(int numPositions, Vector3<Real> const* positions,
            int numTriangles, int const* indices, int punctureTriangle)
        {
            bool converged = true;
            mPlaneCoordinates.resize(numPositions);
            mSphereCoordinates.resize(numPositions);

            // Construct a triangle-edge representation of mesh.
            ETManifoldMesh graph;
            int const* currentIndex = indices;
            int t;
            for (t = 0; t < numTriangles; ++t)
            {
                int v0 = *currentIndex++;
                int v1 = *currentIndex++;
                int v2 = *currentIndex++;
                graph.Insert(v0, v1, v2);
            }
            auto const& emap = graph.GetEdges();

            // Construct the nondiagonal entries of the sparse matrix A.
            typename LinearSystem<Real>::SparseMatrix A;
            int v0, v1, v2, i;
            Vector3<Real> E0, E1;
            Real value;
            for (auto const& element : emap)
            {
                v0 = element.first.V[0];
                v1 = element.first.V[1];

                value = (Real)0;
                for (int j = 0; j < 2; ++j)
                {
                    auto triangle = element.second->T[j];
                    for (i = 0; i < 3; ++i)
                    {
                        v2 = triangle->V[i];
                        if (v2 != v0 && v2 != v1)
                        {
                            E0 = positions[v0] - positions[v2];
                            E1 = positions[v1] - positions[v2];
                            value += Dot(E0, E1) / Length(Cross(E0, E1));
                        }
                    }
                }

                value *= -(Real)0.5;

                std::array<int, 2> lookup = { v0, v1 };
                A[lookup] = value;
            }

            // Construct the diagonal entries of the sparse matrix A.
            std::vector<Real> tmp(numPositions, (Real)0);
            for (auto const& element : A)
            {
                tmp[element.first[0]] -= element.second;
                tmp[element.first[1]] -= element.second;
            }
            for (i = 0; i < numPositions; ++i)
            {
                std::array<int, 2> lookup = { i, i };
                A[lookup] = tmp[i];
            }
            LogAssert(static_cast<size_t>(numPositions) + emap.size() == A.size(), "Mismatched sizes.");

            // Construct the sparse column vector B.
            currentIndex = &indices[3 * punctureTriangle];
            v0 = *currentIndex++;
            v1 = *currentIndex++;
            v2 = *currentIndex++;
            Vector3<Real> V0 = positions[v0];
            Vector3<Real> V1 = positions[v1];
            Vector3<Real> V2 = positions[v2];
            Vector3<Real> E10 = V1 - V0;
            Vector3<Real> E20 = V2 - V0;
            Vector3<Real> E12 = V1 - V2;
            Vector3<Real> normal = Cross(E20, E10);
            Real len10 = Length(E10);
            Real invLen10 = (Real)1 / len10;
            Real twoArea = Length(normal);
            Real invLenNormal = (Real)1 / twoArea;
            Real invProd = invLen10 * invLenNormal;
            Real re0 = -invLen10;
            Real im0 = invProd * Dot(E12, E10);
            Real re1 = invLen10;
            Real im1 = invProd * Dot(E20, E10);
            Real re2 = (Real)0;
            Real im2 = -len10 * invLenNormal;

            // Solve the sparse system for the real parts.
            unsigned int const maxIterations = 1024;
            Real const tolerance = 1e-06f;
            std::fill(tmp.begin(), tmp.end(), (Real)0);
            tmp[v0] = re0;
            tmp[v1] = re1;
            tmp[v2] = re2;
            std::vector<Real> result(numPositions);
            unsigned int iterations = LinearSystem<Real>().SolveSymmetricCG(
                numPositions, A, tmp.data(), result.data(), maxIterations, tolerance);
            if (iterations >= maxIterations)
            {
                LogWarning("Conjugate gradient solver did not converge.");
                converged = false;
            }
            for (i = 0; i < numPositions; ++i)
            {
                mPlaneCoordinates[i][0] = result[i];
            }

            // Solve the sparse system for the imaginary parts.
            std::fill(tmp.begin(), tmp.end(), (Real)0);
            tmp[v0] = -im0;
            tmp[v1] = -im1;
            tmp[v2] = -im2;
            iterations = LinearSystem<Real>().SolveSymmetricCG(numPositions, A,
                tmp.data(), result.data(), maxIterations, tolerance);
            if (iterations >= maxIterations)
            {
                LogWarning("Conjugate gradient solver did not converge.");
                converged = false;
            }
            for (i = 0; i < numPositions; ++i)
            {
                mPlaneCoordinates[i][1] = result[i];
            }

            // Scale to [-1,1]^2 for numerical conditioning in later steps.
            Real fmin = mPlaneCoordinates[0][0], fmax = fmin;
            for (i = 0; i < numPositions; i++)
            {
                if (mPlaneCoordinates[i][0] < fmin)
                {
                    fmin = mPlaneCoordinates[i][0];
                }
                else if (mPlaneCoordinates[i][0] > fmax)
                {
                    fmax = mPlaneCoordinates[i][0];
                }
                if (mPlaneCoordinates[i][1] < fmin)
                {
                    fmin = mPlaneCoordinates[i][1];
                }
                else if (mPlaneCoordinates[i][1] > fmax)
                {
                    fmax = mPlaneCoordinates[i][1];
                }
            }
            Real halfRange = (Real)0.5 * (fmax - fmin);
            Real invHalfRange = (Real)1 / halfRange;
            for (i = 0; i < numPositions; ++i)
            {
                mPlaneCoordinates[i][0] = (Real)-1 + invHalfRange * (mPlaneCoordinates[i][0] - fmin);
                mPlaneCoordinates[i][1] = (Real)-1 + invHalfRange * (mPlaneCoordinates[i][1] - fmin);
            }

            // Map the plane coordinates to the sphere using inverse
            // stereographic projection.  The main issue is selecting a
            // translation in (x,y) and a radius of the projection sphere.
            // Both factors strongly influence the final result.

            // Use the average as the south pole.  The points tend to be
            // clustered approximately in the middle of the conformally
            // mapped punctured triangle, so the average is a good choice
            // to place the pole.
            Vector2<Real> origin{ (Real)0, (Real)0 };
            for (i = 0; i < numPositions; ++i)
            {
                origin += mPlaneCoordinates[i];
            }
            origin /= (Real)numPositions;
            for (i = 0; i < numPositions; ++i)
            {
                mPlaneCoordinates[i] -= origin;
            }

            mMinPlaneCoordinate = mPlaneCoordinates[0];
            mMaxPlaneCoordinate = mPlaneCoordinates[0];
            for (i = 1; i < numPositions; ++i)
            {
                if (mPlaneCoordinates[i][0] < mMinPlaneCoordinate[0])
                {
                    mMinPlaneCoordinate[0] = mPlaneCoordinates[i][0];
                }
                else if (mPlaneCoordinates[i][0] > mMaxPlaneCoordinate[0])
                {
                    mMaxPlaneCoordinate[0] = mPlaneCoordinates[i][0];
                }

                if (mPlaneCoordinates[i][1] < mMinPlaneCoordinate[1])
                {
                    mMinPlaneCoordinate[1] = mPlaneCoordinates[i][1];
                }
                else if (mPlaneCoordinates[i][1] > mMaxPlaneCoordinate[1])
                {
                    mMaxPlaneCoordinate[1] = mPlaneCoordinates[i][1];
                }
            }

            // Select the radius of the sphere so that the projected punctured
            // triangle has an area whose fraction of total spherical area is
            // the same fraction as the area of the punctured triangle to the
            // total area of the original triangle mesh.
            Real twoTotalArea = (Real)0;
            currentIndex = indices;
            for (t = 0; t < numTriangles; ++t)
            {
                V0 = positions[*currentIndex++];
                V1 = positions[*currentIndex++];
                V2 = positions[*currentIndex++];
                E0 = V1 - V0;
                E1 = V2 - V0;
                twoTotalArea += Length(Cross(E0, E1));
            }
            ComputeSphereRadius(v0, v1, v2, twoArea / twoTotalArea);
            Real sqrSphereRadius = mSphereRadius * mSphereRadius;

            // Inverse stereographic projection to obtain sphere coordinates.
            // The sphere is centered at the origin and has radius 1.
            for (i = 0; i < numPositions; i++)
            {
                Real rSqr = Dot(mPlaneCoordinates[i], mPlaneCoordinates[i]);
                Real mult = (Real)1 / (rSqr + sqrSphereRadius);
                Real x = (Real)2 * mult * sqrSphereRadius * mPlaneCoordinates[i][0];
                Real y = (Real)2 * mult * sqrSphereRadius * mPlaneCoordinates[i][1];
                Real z = mult * mSphereRadius * (rSqr - sqrSphereRadius);
                mSphereCoordinates[i] = Vector3<Real>{ x, y, z } / mSphereRadius;
            }

            return converged;
        }

        // Conformal mapping of mesh to plane.  The array of coordinates has a
        // one-to-one correspondence with the input vertex array.
        inline std::vector<Vector2<Real>> const& GetPlaneCoordinates() const
        {
            return mPlaneCoordinates;
        }

        inline Vector2<Real> const& GetMinPlaneCoordinate() const
        {
            return mMinPlaneCoordinate;
        }

        inline Vector2<Real> const& GetMaxPlaneCoordinate() const
        {
            return mMaxPlaneCoordinate;
        }

        // Conformal mapping of mesh to sphere (centered at origin).  The array
        // of coordinates has a one-to-one correspondence with the input vertex
        // array.
        inline std::vector<Vector3<Real>> const& GetSphereCoordinates() const
        {
            return mSphereCoordinates;
        }

        inline Real GetSphereRadius() const
        {
            return mSphereRadius;
        }

    private:
        void ComputeSphereRadius(int v0, int v1, int v2, Real areaFraction)
        {
            Vector2<Real> V0 = mPlaneCoordinates[v0];
            Vector2<Real> V1 = mPlaneCoordinates[v1];
            Vector2<Real> V2 = mPlaneCoordinates[v2];

            Real r0Sqr = Dot(V0, V0);
            Real r1Sqr = Dot(V1, V1);
            Real r2Sqr = Dot(V2, V2);
            Real diffR10 = r1Sqr - r0Sqr;
            Real diffR20 = r2Sqr - r0Sqr;
            Real diffX10 = V1[0] - V0[0];
            Real diffY10 = V1[1] - V0[1];
            Real diffX20 = V2[0] - V0[0];
            Real diffY20 = V2[1] - V0[1];
            Real diffRX10 = V1[0] * r0Sqr - V0[0] * r1Sqr;
            Real diffRY10 = V1[1] * r0Sqr - V0[1] * r1Sqr;
            Real diffRX20 = V2[0] * r0Sqr - V0[0] * r2Sqr;
            Real diffRY20 = V2[1] * r0Sqr - V0[1] * r2Sqr;

            Real c0 = diffR20 * diffRY10 - diffR10 * diffRY20;
            Real c1 = diffR20 * diffY10 - diffR10 * diffY20;
            Real d0 = diffR10 * diffRX20 - diffR20 * diffRX10;
            Real d1 = diffR10 * diffX20 - diffR20 * diffX10;
            Real e0 = diffRX10 * diffRY20 - diffRX20 * diffRY10;
            Real e1 = diffRX10 * diffY20 - diffRX20 * diffY10;
            Real e2 = diffX10 * diffY20 - diffX20 * diffY10;

            Polynomial1<Real> poly0(6);
            poly0[0] = (Real)0;
            poly0[1] = (Real)0;
            poly0[2] = e0 * e0;
            poly0[3] = c0 * c0 + d0 * d0 + (Real)2 * e0 * e1;
            poly0[4] = (Real)2 * (c0 * c1 + d0 * d1 + e0 * e1) + e1 * e1;
            poly0[5] = c1 * c1 + d1 * d1 + (Real)2 * e1 * e2;
            poly0[6] = e2 * e2;

            Polynomial1<Real> qpoly0(1), qpoly1(1), qpoly2(1);
            qpoly0[0] = r0Sqr;
            qpoly0[1] = (Real)1;
            qpoly1[0] = r1Sqr;
            qpoly1[1] = (Real)1;
            qpoly2[0] = r2Sqr;
            qpoly2[1] = (Real)1;

            Real tmp = areaFraction * static_cast<Real>(GTE_C_PI);
            Real amp = tmp * tmp;

            Polynomial1<Real> poly1 = amp * qpoly0;
            poly1 = poly1 * qpoly0;
            poly1 = poly1 * qpoly0;
            poly1 = poly1 * qpoly0;
            poly1 = poly1 * qpoly1;
            poly1 = poly1 * qpoly1;
            poly1 = poly1 * qpoly2;
            poly1 = poly1 * qpoly2;

            Polynomial1<Real> poly2 = poly1 - poly0;
            LogAssert(poly2.GetDegree() <= 8, "Expecting degree no larger than 8.");

            // Bound a root near zero and apply bisection to find t.
            Real tmin = (Real)0, fmin = poly2(tmin);
            Real tmax = (Real)1, fmax = poly2(tmax);
            LogAssert(fmin > (Real)0 && fmax < (Real)0, "Expecting opposite-signed extremes.");

            // Determine the number of iterations to get 'digits' of accuracy.
            int const digits = 6;
            Real tmp0 = std::log(tmax - tmin);
            Real tmp1 = (Real)digits * static_cast<Real>(GTE_C_LN_10);
            Real arg = (tmp0 + tmp1) * static_cast<Real>(GTE_C_INV_LN_2);
            int maxIterations = static_cast<int>(arg + (Real)0.5);
            Real tmid = (Real)0, fmid;
            for (int i = 0; i < maxIterations; ++i)
            {
                tmid = (Real)0.5 * (tmin + tmax);
                fmid = poly2(tmid);
                Real product = fmid * fmin;
                if (product < (Real)0)
                {
                    tmax = tmid;
                    fmax = fmid;
                }
                else
                {
                    tmin = tmid;
                    fmin = fmid;
                }
            }

            mSphereRadius = std::sqrt(tmid);
        }

        // Conformal mapping to a plane.  The plane's (px,py) points
        // correspond to the mesh's (mx,my,mz) points.
        std::vector<Vector2<Real>> mPlaneCoordinates;
        Vector2<Real> mMinPlaneCoordinate, mMaxPlaneCoordinate;

        // Conformal mapping to a sphere.  The sphere's (sx,sy,sz) points
        // correspond to the mesh's (mx,my,mz) points.
        std::vector<Vector3<Real>> mSphereCoordinates;
        Real mSphereRadius;
    };
}
