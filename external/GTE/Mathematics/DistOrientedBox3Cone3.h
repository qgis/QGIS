// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 5.10.2021.05.22

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Cone.h>
#include <Mathematics/OrientedBox.h>
#include <Mathematics/LCPSolver.h>
#include <Mathematics/Matrix.h>
#include <Mathematics/Minimize1.h>
#include <Mathematics/Vector3.h>

// Compute the distance between an oriented box and a cone frustum. The
// frustum is part of a single-sided cone with heights measured along the
// axis direction. The single-sided cone heights h satisfy
// 0 <= h <= infinity. The cone frustum has heights that satisfy
// 0 <= hmin < h <= hmax < infinity. The algorithm is described in
// https://www.geometrictools.com/Documentation/DistanceBox3Cone3.pdf

namespace gte
{
    template <typename T>
    class DCPQuery<T, OrientedBox<3, T>, Cone<3, T>>
    {
    public:
        DCPQuery()
        {
            static_assert(
                std::is_floating_point<T>::value,
                "The input type must be a floating-point type.");
        }

        // Parameters used internally for controlling the minimizer.
        struct Control
        {
            Control(
                int32_t inMaxSubdivisions = 8,
                int32_t inMaxBisections = 128,
                T inEpsilon = static_cast<T>(1e-08),
                T inTolerance = static_cast<T>(1e-04))
                :
                maxSubdivisions(inMaxSubdivisions),
                maxBisections(inMaxBisections),
                epsilon(inEpsilon),
                tolerance(inTolerance)
            {
            }

            int32_t maxSubdivisions;
            int32_t maxBisections;
            T epsilon;
            T tolerance;
        };

        // The output of the query, which is the distance between the
        // objects and a pair of closest points, one from each object.
        struct Result
        {
            Result()
                :
                distance(std::numeric_limits<T>::max()),
                boxClosestPoint{},
                coneClosestPoint{}
            {
                boxClosestPoint.MakeZero();
                coneClosestPoint.MakeZero();
            }

            T distance;
            Vector<3, T> boxClosestPoint, coneClosestPoint;
        };

        // The default minimizer controls are reasonable choices generally,
        // in which case you can use
        //   using BCQuery = DCPQuery<T, OrientedBox<3, T>, Cone<3, T>>;
        //   BCQuery bcQuery{};
        //   BCQuery::Result bcResult = bcQuery(box, cone);
        // If your application requires specialized controls,
        //   using BCQuery = DCPQuery<T, OrientedBox<3, T>, Cone<3, T>>;
        //   BCQuery bcQuery{};
        //   BCQuery::Control bcControl(your_parameters);
        //   BCQuery::Result bcResult = bcQuery(box, cone, &bcControl);
        Result operator()(OrientedBox<3, T> const& box, Cone<3, T> const& cone,
            Control const* inControl = nullptr)
        {
            Control control{};
            if (inControl != nullptr)
            {
                control = *inControl;
            }

            // Compute a basis for the cone coordinate system.
            std::array<Vector<3, T>, 3> basis{};
            basis[0] = cone.ray.direction;
            ComputeOrthogonalComplement(1, basis.data());
            Vector<3, T> coneW0 = basis[1];
            Vector<3, T> coneW1 = basis[2];

            Result result{};
            result.distance = std::numeric_limits<T>::max();

            auto F = [this, &box, &cone, &coneW0, &coneW1, &result](T angle)
            {
                T distance = std::numeric_limits<T>::max();
                Vector<3, T> boxClosestPoint{}, quadClosestPoint{};
                DoBoxQuadQuery(box, cone, coneW0, coneW1, angle,
                    distance, boxClosestPoint, quadClosestPoint);

                if (distance < result.distance)
                {
                    result.distance = distance;
                    result.boxClosestPoint = boxClosestPoint;
                    result.coneClosestPoint = quadClosestPoint;
                }

                return distance;
            };

            Minimize1<T> minimizer(F, control.maxSubdivisions, control.maxBisections,
                control.epsilon, control.tolerance);
            T angle0 = static_cast<T>(-GTE_C_HALF_PI);
            T angle1 = static_cast<T>(+GTE_C_HALF_PI);
            T angleMin = static_cast<T>(0);
            T distanceMin = std::numeric_limits<T>::max();
            minimizer.GetMinimum(angle0, angle1, angleMin, distanceMin);
            LogAssert(
                distanceMin == result.distance,
                "Unexpected mismatch in minimum distance.");

            return result;
        }

    private:
        void DoBoxQuadQuery(OrientedBox<3, T> const& box, Cone<3, T> const& cone,
            Vector<3, T> const& coneW0, Vector<3, T> const& coneW1,
            T const& quadAngle, T& distance, Vector<3, T>& boxClosestPoint,
            Vector<3, T>& quadClosestPoint)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const two = static_cast<T>(2);

            Vector<3, T> K = box.center, ell{};
            for (int32_t i = 0; i < 3; ++i)
            {
                K -= box.extent[i] * box.axis[i];
                ell[i] = two * box.extent[i];
            }

            T cs = std::cos(quadAngle), sn = std::sin(quadAngle);
            Vector<3, T> term = cone.tanAngle * (cs * coneW0 + sn * coneW1);
            std::array<Vector<3, T>, 2> G{};
            G[0] = cone.ray.direction - term;
            G[1] = cone.ray.direction + term;

            Matrix<5, 5, T> A{};  // A is the zero matrix
            A(0, 0) = one;
            A(0, 1) = zero;
            A(0, 2) = zero;
            A(0, 3) = -Dot(box.axis[0], G[0]);
            A(0, 4) = -Dot(box.axis[0], G[1]);
            A(1, 0) = A(0, 1);
            A(1, 1) = one;
            A(1, 2) = zero;
            A(1, 3) = -Dot(box.axis[1], G[0]);
            A(1, 4) = -Dot(box.axis[1], G[1]);
            A(2, 0) = A(0, 2);
            A(2, 1) = A(1, 2);
            A(2, 2) = one;
            A(2, 3) = -Dot(box.axis[2], G[0]);
            A(2, 4) = -Dot(box.axis[2], G[1]);
            A(3, 0) = A(0, 3);
            A(3, 1) = A(1, 3);
            A(3, 2) = A(2, 3);
            A(3, 3) = Dot(G[0], G[0]);
            A(3, 4) = Dot(G[0], G[1]);
            A(4, 0) = A(0, 4);
            A(4, 1) = A(1, 4);
            A(4, 2) = A(2, 4);
            A(4, 3) = A(3, 4);
            A(4, 4) = Dot(G[1], G[1]);

            Vector<3, T> KmV = K - cone.ray.origin;
            Vector<5, T> b{};
            b[0] = Dot(box.axis[0], KmV);
            b[1] = Dot(box.axis[1], KmV);
            b[2] = Dot(box.axis[2], KmV);
            b[3] = -Dot(G[0], KmV);
            b[4] = -Dot(G[1], KmV);

            Matrix<5, 5, T> D{};  // D is the zero matrix
            D(0, 0) = -one;
            D(1, 1) = -one;
            D(2, 2) = -one;
            D(3, 3) = one;
            D(3, 4) = one;
            D(4, 3) = -one;
            D(4, 4) = -one;

            Vector<5, T> e{};
            e[0] = -ell[0];
            e[1] = -ell[1];
            e[2] = -ell[2];
            e[3] = cone.GetMinHeight();
            e[4] = -cone.GetMaxHeight();

            std::array<T, 10> q;
            for (int32_t i = 0; i < 5; ++i)
            {
                q[i] = b[i];
                q[i + 5] = -e[i];
            }

            std::array<std::array<T, 10>, 10> M;
            for (int32_t r = 0; r < 5; ++r)
            {
                for (int32_t c = 0; c < 5; ++c)
                {
                    M[r][c] = A(r, c);
                    M[r + 5][c] = D(r, c);
                    M[r][c + 5] = -D(c, r);
                    M[r + 5][c + 5] = zero;
                }
            }

            std::array<T, 10> w, z;
            if (mLCP.Solve(q, M, w, z))
            {
                boxClosestPoint = K;
                for (int32_t i = 0; i < 3; ++i)
                {
                    boxClosestPoint += z[i] * box.axis[i];
                }

                quadClosestPoint = cone.ray.origin;
                for (int32_t i = 0; i < 2; ++i)
                {
                    quadClosestPoint += z[i + 3] * G[i];
                }

                distance = Length(boxClosestPoint - quadClosestPoint);
            }
            else
            {
                boxClosestPoint.MakeZero();
                quadClosestPoint.MakeZero();
                distance = std::numeric_limits<T>::max();
            }
        }

        LCPSolver<T, 10> mLCP;
    };
}
