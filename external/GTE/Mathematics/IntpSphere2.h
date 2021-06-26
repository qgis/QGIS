// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.11.16

#pragma once

#include <Mathematics/Delaunay2Mesh.h>
#include <Mathematics/IntpQuadraticNonuniform2.h>
#include <memory>

// Interpolation of a scalar-valued function defined on a sphere.  Although
// the sphere lives in 3D, the interpolation is a 2D method whose input
// points are angles (theta,phi) from spherical coordinates.  The domains of
// the angles are -pi <= theta <= pi and 0 <= phi <= pi.

namespace gte
{
    template <typename T, typename...>
    class IntpSphere2 {};
}

namespace gte
{
    // The InputType is 'float' or 'double'. The ComputeType can be a
    // floating-point type or BSNumber<*> type, because it does not require
    // divisions. The RationalType requires division, so you can use
    // BSRational<*>.

    template <typename InputType, typename ComputeType, typename RationalType>
    class // [[deprecated("Use IntpSphere2<InputType> instead.")]]
        IntpSphere2<InputType, ComputeType, RationalType>
    {
    public:
        // Construction and destruction.  For complete spherical coverage,
        // include the two antipodal (theta,phi) points (-pi,0,F(-pi,0)) and
        // (-pi,pi,F(-pi,pi)) in the input data.  These correspond to the
        // sphere poles x = 0, y = 0, and |z| = 1.
        ~IntpSphere2() = default;

        IntpSphere2(int numPoints, InputType const* theta, InputType const* phi, InputType const* F)
            :
            mMesh(mDelaunay)
        {
            // Copy the input data.  The larger arrays are used to support
            // wrap-around in the Delaunay triangulation for the interpolator.
            int totalPoints = 3 * numPoints;
            mWrapAngles.resize(totalPoints);
            mWrapF.resize(totalPoints);
            for (int i = 0; i < numPoints; ++i)
            {
                mWrapAngles[i][0] = theta[i];
                mWrapAngles[i][1] = phi[i];
                mWrapF[i] = F[i];
            }

            // Use periodicity to get wrap-around in the Delaunay
            // triangulation.
            int i0 = 0, i1 = numPoints, i2 = 2 * numPoints;
            for (/**/; i0 < numPoints; ++i0, ++i1, ++i2)
            {
                mWrapAngles[i1][0] = mWrapAngles[i0][0] + (InputType)GTE_C_TWO_PI;
                mWrapAngles[i2][0] = mWrapAngles[i0][0] - (InputType)GTE_C_TWO_PI;
                mWrapAngles[i1][1] = mWrapAngles[i0][1];
                mWrapAngles[i2][1] = mWrapAngles[i0][1];
                mWrapF[i1] = mWrapF[i0];
                mWrapF[i2] = mWrapF[i0];
            }

            mDelaunay(totalPoints, &mWrapAngles[0], (ComputeType)0);
            mInterp = std::make_unique<IntpQuadraticNonuniform2<InputType, TriangleMesh>>(
                mMesh, &mWrapF[0], (InputType)1);
        }

        // Spherical coordinates are
        //   x = cos(theta)*sin(phi)
        //   y = sin(theta)*sin(phi)
        //   z = cos(phi)
        // for -pi <= theta <= pi, 0 <= phi <= pi.  The application can use
        // this function to convert unit length vectors (x,y,z) to (theta,phi).
        static void GetSphericalCoordinates(InputType x, InputType y, InputType z,
            InputType& theta, InputType& phi)
        {
            // Assumes (x,y,z) is unit length.  Returns -pi <= theta <= pi and
            // 0 <= phiAngle <= pi.

            if (z < (InputType)1)
            {
                if (z > -(InputType)1)
                {
                    theta = std::atan2(y, x);
                    phi = std::acos(z);
                }
                else
                {
                    theta = -(InputType)GTE_C_PI;
                    phi = (InputType)GTE_C_PI;
                }
            }
            else
            {
                theta = -(InputType)GTE_C_PI;
                phi = (InputType)0;
            }
        }

        // The return value is 'true' if and only if the input point is in the
        // convex hull of the input (theta,pi) array, in which case the
        // interpolation is valid.
        bool operator()(InputType theta, InputType phi, InputType& F) const
        {
            Vector2<InputType> angles{ theta, phi };
            InputType thetaDeriv = static_cast<InputType>(0);
            InputType phiDeriv = static_cast<InputType>(0);
            return (*mInterp)(angles, F, thetaDeriv, phiDeriv);
        }

    private:
        typedef Delaunay2Mesh<InputType, ComputeType, RationalType> TriangleMesh;

        std::vector<Vector2<InputType>> mWrapAngles;
        Delaunay2<InputType, ComputeType> mDelaunay;
        TriangleMesh mMesh;
        std::vector<InputType> mWrapF;
        std::unique_ptr<IntpQuadraticNonuniform2<InputType, TriangleMesh>> mInterp;
    };
}

namespace gte
{
    // The input type T is 'float' or 'double'.

    template <typename T>
    class IntpSphere2<T>
    {
    public:
        // Construction and destruction.  For complete spherical coverage,
        // include the two antipodal (theta,phi) points (-pi,0,F(-pi,0)) and
        // (-pi,pi,F(-pi,pi)) in the input data.  These correspond to the
        // sphere poles x = 0, y = 0, and |z| = 1.
        ~IntpSphere2() = default;

        IntpSphere2(int numPoints, T const* theta, T const* phi, T const* F)
            :
            mMesh(mDelaunay)
        {
            // Copy the input data.  The larger arrays are used to support
            // wrap-around in the Delaunay triangulation for the interpolator.
            int totalPoints = 3 * numPoints;
            mWrapAngles.resize(totalPoints);
            mWrapF.resize(totalPoints);
            for (int i = 0; i < numPoints; ++i)
            {
                mWrapAngles[i][0] = theta[i];
                mWrapAngles[i][1] = phi[i];
                mWrapF[i] = F[i];
            }

            // Use periodicity to get wrap-around in the Delaunay
            // triangulation.
            int i0 = 0, i1 = numPoints, i2 = 2 * numPoints;
            for (/**/; i0 < numPoints; ++i0, ++i1, ++i2)
            {
                mWrapAngles[i1][0] = mWrapAngles[i0][0] + static_cast<T>(GTE_C_TWO_PI);
                mWrapAngles[i2][0] = mWrapAngles[i0][0] - static_cast<T>(GTE_C_TWO_PI);
                mWrapAngles[i1][1] = mWrapAngles[i0][1];
                mWrapAngles[i2][1] = mWrapAngles[i0][1];
                mWrapF[i1] = mWrapF[i0];
                mWrapF[i2] = mWrapF[i0];
            }

            mDelaunay(mWrapAngles);
            mInterp = std::make_unique<IntpQuadraticNonuniform2<T, TriangleMesh>>(
                mMesh, &mWrapF[0], static_cast<T>(1));
        }

        // Spherical coordinates are
        //   x = cos(theta)*sin(phi)
        //   y = sin(theta)*sin(phi)
        //   z = cos(phi)
        // for -pi <= theta <= pi, 0 <= phi <= pi.  The application can use
        // this function to convert unit length vectors (x,y,z) to (theta,phi).
        static void GetSphericalCoordinates(T x, T y, T z,
            T& theta, T& phi)
        {
            // Assumes (x,y,z) is unit length.  Returns -pi <= theta <= pi and
            // 0 <= phiAngle <= pi.
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const pi = static_cast<T>(GTE_C_PI);

            if (z < one)
            {
                if (z > -one)
                {
                    theta = std::atan2(y, x);
                    phi = std::acos(z);
                }
                else
                {
                    theta = -pi;
                    phi = pi;
                }
            }
            else
            {
                theta = -pi;
                phi = zero;
            }
        }

        // The return value is 'true' if and only if the input point is in the
        // convex hull of the input (theta,pi) array, in which case the
        // interpolation is valid.
        bool operator()(T theta, T phi, T& F) const
        {
            Vector2<T> angles{ theta, phi };
            T thetaDeriv = static_cast<T>(0);
            T phiDeriv = static_cast<T>(0);
            return (*mInterp)(angles, F, thetaDeriv, phiDeriv);
        }

    private:
        using TriangleMesh = Delaunay2Mesh<T>;

        std::vector<Vector2<T>> mWrapAngles;
        Delaunay2<T> mDelaunay;
        TriangleMesh mMesh;
        std::vector<T> mWrapF;
        std::unique_ptr<IntpQuadraticNonuniform2<T, TriangleMesh>> mInterp;
    };
}
