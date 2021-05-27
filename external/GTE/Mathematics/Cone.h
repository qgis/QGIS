// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.05.22

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Math.h>
#include <Mathematics/Ray.h>
#include <Mathematics/Vector2.h>
#include <Mathematics/Matrix3x3.h>
#include <Mathematics/UniqueVerticesSimplices.h>

// An infinite cone is defined by a vertex V, a unit-length direction D and an
// angle A with 0 < A < pi/2. A point X is on the cone when
//   Dot(D, X - V) = |X - V| * cos(A)
// A solid cone includes points on the cone and in the region that contains
// the cone ray V + h * D for h >= 0.  It is defined by
//   Dot(D, X - V) >= |X - V| * cos(A)
// The height of any point Y in space relative to the cone is defined by
// h = Dot(D, Y - V), which is the signed length of the projection of X - V
// onto the cone axis. Observe that we have restricted the cone definition to
// an acute angle A, so |X - V| * cos(A) >= 0; therefore, points on or inside
// the cone have nonnegative heights: Dot(D, X - V) >= 0. I will refer to the
// infinite solid cone as the "positive cone," which means that the non-vertex
// points inside the cone have positive heights. Although rare in computer
// graphics, one might also want to consider the "negative cone," which is
// defined by
//   -Dot(D, X - V) <= -|X - V| * cos(A)
// The non-vertex points inside this cone have negative heights.
//
// For many of the geometric queries involving cones, we can avoid the square
// root computation implied by |X - V|. The positive cone is defined by
//   Dot(D, X - V)^2 >= |X - V|^2 * cos(A)^2,
// which is a quadratic inequality, but the squaring of the terms leads to an
// inequality that includes points X in the negative cone. When using the
// quadratic inequality for the positive cone, we need to include also the
// constraint Dot(D, X - V) >= 0.
//
// I define four different types of cones. They all involve V, D and A. The
// differences are based on restrictions to the heights of the cone points.
// The height range is defined to be the interval of possible heights, say,
// [hmin,hmax] with 0 <= hmin < hmax <= +infinity.
//     1. infinite cone: hmin = 0, hmax = +infinity
//     2. infinite truncated cone:  hmin > 0, hmax = +infinity
//     3. finite cone:  hmin >= 0, hmax < +infinity
//     4. frustum of a cone:  hmin > 0, hmax < +infinity
// The infinite truncated cone is truncated for h-minimum; the radius of the
// disk at h-minimum is rmin = hmin * tan(A). The finite cone is truncated for
// h-maximum; the radius of the disk at h-maximum is rmax = hmax * tan(A).
// The frustum of a cone is truncated both for h-minimum and h-maximum.
//
// A technical problem when creating a data structure to represent a cone is
// deciding how to represent +infinity in the height range. When the template
// type T is 'float' or 'double', we could represent it as
// std::numeric_limits<T>::infinity(). The geometric queries must be
// structured properly to conform to the semantics associated with the
// floating-point infinity. We could also use the largest finite
// floating-point number, std::numeric_limits<T>::max(). Either choice is
// problematic when instead T is an arbitrary precision type that does not
// have a representation for infinity; this is the case for the types
// BSNumber<T> and BSRational<T>, where T is UIntegerAP or UIntegerFP<N>.
//
// The introduction of representations of infinities for the arbitrary
// precision types would require modifying the arithmetic operations to test
// whether the number is finite or infinite. This leads to a greater
// computational cost for all queries, even when those queries do not require
// manipulating infinities. In the case of a cone, the height manipulations
// are nearly always for comparisons of heights. I choose to represent
// +infinity by setting the maxHeight member to -1. The member functions
// IsFinite() and IsInfinite() compare maxHeight to -1 and report the correct
// state.
//
// My choice of representation has the main consequence that comparisons
// between heights requires extra logic. This can make geometric queries
// cumbersome to implement. For example, the point-in-cone test using the
// quadratic inequality is shown in the pseudocode
//   Vector point = <some point>;
//   Cone cone = <some cone>;
//   Vector delta = point - cone.V;
//   T h = Dot(cone.D, delta);
//   bool pointInCone =
//       cone.hmin <= h &&
//       h <= cone.hmax &&
//       h * h >= Dot(delta, delta) * cone.cosAngleSqr;
// In the event the cone is infinite and we choose cone.hmax = -1 to
// represent this, the test 'h <= cone.hmax' must be revised,
//   bool pointInCone =
//       cone.hmin <= h &&
//       (cone.hmax == -1 ? true : (h <= cone.hmax)) &&
//       h * h >= Dot(delta, delta) * cone.cosAngleSqr;
// To encapsulate the comparisons against height extremes, use the member
// function HeightInRange(h); that is
//   bool pointInCone =
//       cone.HeightInRange(h) &&
//       h * h >= Dot(delta, delta) * cone.cosAngleSqr;
// The modification is not that complicated here, but consider a more
// sophisticated query such as determining the interval of intersection
// of two height intervals [h0,h1] and [cone.hmin,cone.hmax]. The file
// GteIntrIntervals.h provides implementations for computing the
// intersection of two intervals, where either or both intervals are
// semi-infinite.

namespace gte
{
    template <int N, typename T>
    class Cone
    {
    public:
        // Create an infinite cone with
        //   vertex = (0,...,0)
        //   axis = (0,...,0,1)
        //   angle = pi/4
        //   minimum height = 0
        //   maximum height = +infinity
        Cone()
        {
            ray.origin.MakeZero();
            ray.direction.MakeUnit(N - 1);
            SetAngle((T)GTE_C_QUARTER_PI);
            MakeInfiniteCone();
        }

        // Create an infinite cone with the specified vertex, axis direction,
        // angle and with heights
        //   minimum height = 0
        //   maximum height = +infinity
        Cone(Ray<N, T> const& inRay, T const& inAngle)
            :
            ray(inRay)
        {
            SetAngle(inAngle);
            MakeInfiniteCone();
        }

        // Create an infinite truncated cone with the specified vertex, axis
        // direction, angle and positive minimum height. The maximum height
        // is +infinity. If you specify a minimum height of 0, you get the 
        // equivalent of calling the constructor for an infinite cone.
        Cone(Ray<N, T> const& inRay, T const& inAngle, T const& inMinHeight)
            :
            ray(inRay)
        {
            SetAngle(inAngle);
            MakeInfiniteTruncatedCone(inMinHeight);
        }

        // Create a finite cone or a frustum of a cone with all parameters
        // specified. If you specify a minimum height of 0, you get a finite
        // cone. If you specify a positive minimum height, you get a frustum
        // of a cone.
        Cone(Ray<N, T> const& inRay, T inAngle, T inMinHeight, T inMaxHeight)
            :
            ray(inRay)
        {
            SetAngle(inAngle);
            MakeConeFrustum(inMinHeight, inMaxHeight);
        }

        // The angle must be in (0,pi/2). The function sets 'angle' and
        // computes 'cosAngle', 'sinAngle', 'tanAngle', 'cosAngleSqr',
        // 'sinAngleSqr' and 'invSinAngle'.
        void SetAngle(T const& inAngle)
        {
            LogAssert((T)0 < inAngle && inAngle < (T)GTE_C_HALF_PI, "Invalid angle.");
            angle = inAngle;
            cosAngle = std::cos(angle);
            sinAngle = std::sin(angle);
            tanAngle = std::tan(angle);
            cosAngleSqr = cosAngle * cosAngle;
            sinAngleSqr = sinAngle * sinAngle;
            invSinAngle = (T)1 / sinAngle;
        }

        // Set the heights to obtain one of the four types of cones. Be aware
        // that an infinite cone has maxHeight set to -1. Be careful not to
        // use maxHeight without understanding this interpretation.
        void MakeInfiniteCone()
        {
            mMinHeight = (T)0;
            mMaxHeight = (T)-1;
        }

        void MakeInfiniteTruncatedCone(T const& inMinHeight)
        {
            LogAssert(inMinHeight >= (T)0, "Invalid minimum height.");
            mMinHeight = inMinHeight;
            mMaxHeight = (T)-1;
        }

        void MakeFiniteCone(T const& inMaxHeight)
        {
            LogAssert(inMaxHeight > (T)0, "Invalid maximum height.");
            mMinHeight = (T)0;
            mMaxHeight = inMaxHeight;
        }

        void MakeConeFrustum(T const& inMinHeight, T const& inMaxHeight)
        {
            LogAssert(inMinHeight >= (T)0 && inMaxHeight > inMinHeight,
                "Invalid minimum or maximum height.");
            mMinHeight = inMinHeight;
            mMaxHeight = inMaxHeight;
        }

        // Get the height extremes. For an infinite cone, maxHeight is set
        // to -1. For a finite cone, maxHeight is set to a positive number.
        // Be careful not to use maxHeight without understanding this
        // interpretation.
        inline T GetMinHeight() const
        {
            return mMinHeight;
        }

        inline T GetMaxHeight() const
        {
            return mMaxHeight;
        }

        inline bool HeightInRange(T const& h) const
        {
            return mMinHeight <= h && (mMaxHeight != (T)-1 ? h <= mMaxHeight : true);
        }

        inline bool HeightLessThanMin(T const& h) const
        {
            return h < mMinHeight;
        }

        inline bool HeightGreaterThanMax(T const& h) const
        {
            return (mMaxHeight != (T)-1 ? h > mMaxHeight : false);
        }

        inline bool IsFinite() const
        {
            return mMaxHeight != (T)-1;
        }

        inline bool IsInfinite() const
        {
            return mMaxHeight == (T)-1;
        }

        // The cone axis direction (ray.direction) must be unit length.
        Ray<N, T> ray;

        // The angle must be in (0,pi/2). The other members are derived from
        // angle to avoid calling trigonometric functions in geometric queries
        // (for speed). You may set the angle and compute these by calling
        // SetAngle(inAngle).
        T angle;
        T cosAngle, sinAngle, tanAngle;
        T cosAngleSqr, sinAngleSqr, invSinAngle;

    private:
        // The heights must satisfy 0 <= minHeight < maxHeight <= +infinity.
        // For an infinite cone, maxHeight is set to -1.  For a finite cone,
        // maxHeight is set to a positive number.  Be careful not to use
        // maxHeight without understanding this interpretation.
        T mMinHeight, mMaxHeight;

    public:
        // Comparisons to support sorted containers.  These based only on
        // 'ray', 'angle', 'minHeight' and 'maxHeight'.
        bool operator==(Cone const& cone) const
        {
            return ray == cone.ray
                && angle == cone.angle
                && mMinHeight == cone.mMinHeight
                && mMaxHeight == cone.mMaxHeight;
        }

        bool operator!=(Cone const& cone) const
        {
            return !operator==(cone);
        }

        bool operator< (Cone const& cone) const
        {
            if (ray < cone.ray)
            {
                return true;
            }

            if (ray > cone.ray)
            {
                return false;
            }

            if (angle < cone.angle)
            {
                return true;
            }

            if (angle > cone.angle)
            {
                return false;
            }

            if (mMinHeight < cone.mMinHeight)
            {
                return true;
            }

            if (mMinHeight > cone.mMinHeight)
            {
                return false;
            }

            return mMaxHeight < cone.mMaxHeight;
        }

        bool operator<=(Cone const& cone) const
        {
            return !cone.operator<(*this);
        }

        bool operator> (Cone const& cone) const
        {
            return cone.operator<(*this);
        }

        bool operator>=(Cone const& cone) const
        {
            return !operator<(cone);
        }

    public:
        // Support for visualization.
        template <int Dummy = N>
        typename std::enable_if<(Dummy == 3), void>::type
        CreateMesh(size_t numMinVertices, bool inscribed,
            std::vector<Vector<3, T>>& vertices, std::vector<int32_t>& indices)
        {
            LogAssert(IsFinite(), "Meshes can be generated only for finite cones.");

            T hMin = GetMinHeight();
            T hMax = GetMaxHeight();
            T rMin = hMin * tanAngle;
            T rMax = hMax * tanAngle;
            T tNumExtra = static_cast<T>(0.5) * rMax/ rMin - static_cast<T>(1);
            size_t numExtra = 0;
            if (tNumExtra > static_cast<T>(0))
            {
                numExtra = static_cast<size_t>(std::ceil(tNumExtra));
            }
            size_t numMaxVertices = 2 * numMinVertices * (1 + numExtra);
            vertices.clear();
            indices.clear();

            std::vector<Vector<2, T>> polygonMin, polygonMax;
            if (inscribed)
            {
                GenerateInscribed(numMinVertices, rMin, polygonMin);
                GenerateInscribed(numMaxVertices, rMax, polygonMax);
            }
            else
            {
                GenerateCircumscribed(numMinVertices, rMin, polygonMin);
                GenerateCircumscribed(numMaxVertices, rMax, polygonMax);
            }

            if (hMin > static_cast<T>(0))
            {
                CreateConeFrustumMesh(numMinVertices, numMaxVertices, numExtra,
                    hMin, hMax, polygonMin, polygonMax, vertices, indices);
            }
            else
            {
                // TODO:
                // CreateFiniteTruncatedConeMesh(numMaxVertices, numExtra,
                //     hMax, polygonMax, vertices, indices);
            }

            // Transform to the coordinate system of the cone.
            std::array<Vector<3, T>, 3> basis{};
            basis[0] = ray.direction;
            ComputeOrthogonalComplement(1, basis.data());
            Matrix<3, 3, T> rotate{};
            rotate.SetCol(0, basis[1]);
            rotate.SetCol(1, basis[2]);
            rotate.SetCol(2, basis[0]);
            for (size_t i = 0; i < vertices.size(); ++i)
            {
                vertices[i] = rotate * vertices[i] + ray.origin;
            }
        }

    private:
        template <int Dummy = N>
        static typename std::enable_if<(Dummy == 3), void>::type
        GenerateInscribed(size_t numVertices, T const& radius,
            std::vector<Vector<2, T>>& polygon)
        {
            T theta = static_cast<T>(GTE_C_TWO_PI) / static_cast<T>(numVertices);
            polygon.resize(numVertices + 1);
            for (size_t i = 0; i < numVertices; ++i)
            {
                T angle = static_cast<T>(i) * theta;
                polygon[i][0] = radius * std::cos(angle);
                polygon[i][1] = radius * std::sin(angle);
            }
            polygon.back() = polygon[0];
        }

        template <int Dummy = N>
        static typename std::enable_if<(Dummy == 3), void>::type
        GenerateCircumscribed(size_t numVertices, T const& radius,
            std::vector<Vector<2, T>>& polygon)
        {
            T theta = static_cast<T>(GTE_C_TWO_PI) / static_cast<T>(numVertices);
            std::vector<Vector<2, T>> inscribed(numVertices + 1);
            for (size_t i = 0; i < numVertices; ++i)
            {
                T angle = static_cast<T>(i) * theta;
                inscribed[i][0] = radius * std::cos(angle);
                inscribed[i][1] = radius * std::sin(angle);
            }
            inscribed.back() = inscribed[0];

            T divisor = static_cast<T>(1) + std::cos(theta);
            polygon.resize(numVertices + 1);
            for (size_t i = 0, ip1 = 1; i < numVertices; ++i, ++ip1)
            {
                polygon[i] = (inscribed[i] + inscribed[ip1]) / divisor;
            }
            polygon.back() = polygon[0];
        }

        template <int Dummy = N>
        static typename std::enable_if<(Dummy == 3), void>::type
        CreateConeFrustumMesh(size_t numMinVertices, size_t numMaxVertices,
            size_t numExtra, T const& hMin, T const& hMax,
            std::vector<Vector<2, T>> const& polygonMin,
            std::vector<Vector<2, T>> const& polygonMax,
            std::vector<Vector<3, T>>& vertices, std::vector<int32_t>& indices)
        {
            std::vector<Vector<3, T>> vertexPool;
            Vector<3, T> V0, V1, V2;
            for (size_t i0 = 0, i1 = 1; i0 < numMinVertices; i0 = i1++)
            {
                size_t j0 = 2 * (numExtra + 1) * i0;
                V0 = HLift(polygonMin[i0], hMin);
                for (size_t k0 = 0, k1 = 1; k0 <= numExtra; k0 = k1++)
                {
                    V1 = HLift(polygonMax[j0 + k1], hMax);
                    V2 = HLift(polygonMax[j0 + k0], hMax);
                    vertexPool.push_back(V0);
                    vertexPool.push_back(V1);
                    vertexPool.push_back(V2);
                }

                size_t j1 = 2 * (numExtra + 1) * i1;
                V0 = HLift(polygonMin[i1], hMin);
                for (size_t k0 = 0, k1 = 1; k0 <= numExtra; k0 = k1++)
                {
                    V1 = HLift(polygonMax[j1 - k0], hMax);
                    V2 = HLift(polygonMax[j1 - k1], hMax);
                    vertexPool.push_back(V0);
                    vertexPool.push_back(V1);
                    vertexPool.push_back(V2);
                }

                size_t jmid = j0 + (numExtra + 1);
                V0 = HLift(polygonMax[jmid], hMax);
                V1 = HLift(polygonMin[i0], hMin);
                V2 = HLift(polygonMin[i1], hMin);
                vertexPool.push_back(V0);
                vertexPool.push_back(V1);
                vertexPool.push_back(V2);
            }

            V0 = { static_cast<T>(0), static_cast<T>(0), hMin };
            for (size_t i0 = 0, i1 = 1; i0 < numMinVertices; i0 = i1++)
            {
                V1 = HLift(polygonMin[i1], hMin);
                V2 = HLift(polygonMin[i0], hMin);
                vertexPool.push_back(V0);
                vertexPool.push_back(V1);
                vertexPool.push_back(V2);
            }

            V0 = { static_cast<T>(0), static_cast<T>(0), hMax };
            for (size_t i0 = 0, i1 = 1; i0 < numMaxVertices; i0 = i1++)
            {
                V1 = HLift(polygonMax[i0], hMax);
                V2 = HLift(polygonMax[i1], hMax);
                vertexPool.push_back(V0);
                vertexPool.push_back(V1);
                vertexPool.push_back(V2);
            }

            UniqueVerticesSimplices<Vector<3, T>, int32_t, 3> uvs;
            uvs.GenerateIndexedSimplices(vertexPool, vertices, indices);
        }
    };

    // Template alias for convenience.
    template <typename T>
    using Cone3 = Cone<3, T>;
}
