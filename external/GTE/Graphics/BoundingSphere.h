// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Matrix4x4.h>
#include <Graphics/CullingPlane.h>
#include <cstdint>

namespace gte
{
    template <typename Real>
    class BoundingSphere
    {
    public:
        // Construction and destruction.  The default constructor sets the
        // center to the origin (0,0,0) and the radius to 0.  A radius of 0
        // denotes an invalid bound.
        BoundingSphere()
            :
            mTuple{ (Real)0, (Real)0, (Real)0, (Real)0 }
        {
        }

        BoundingSphere(BoundingSphere const& sphere)
            :
            mTuple(sphere.mTuple)
        {
        }

        ~BoundingSphere()
        {
        }

        // Assignment.
        BoundingSphere& operator=(BoundingSphere const& sphere)
        {
            mTuple = sphere.mTuple;
            return *this;
        }

        // Member access.  The radius must be nonnegative.  When negative,
        // it is clamped to zero.
        inline void SetCenter(Vector3<Real> const& center)
        {
            mTuple[0] = center[0];
            mTuple[1] = center[1];
            mTuple[2] = center[2];
        }

        inline void SetRadius(Real radius)
        {
            mTuple[3] = (radius >= (Real)0 ? radius : (Real)0);
        }

        inline Vector3<Real> GetCenter() const
        {
            return Vector3<Real>{ mTuple[0], mTuple[1], mTuple[2] };
        }

        inline Real GetRadius() const
        {
            return mTuple[3];
        }

        // The "positive side" of the plane is the half space to which the
        // plane normal is directed.  The "negative side" is the other half
        // space.  The function returns +1 when the sphere is fully on the
        // positive side, -1 when the sphere is fully on the negative side, or
        // 0 when the sphere is transversely cut by the plane (sphere volume 
        // on each side of plane is positive).
        int WhichSide(CullingPlane<Real> const& plane) const
        {
            Vector4<Real> hcenter = HLift(GetCenter(), (Real)1);
            Real signedDistance = plane.DistanceTo(hcenter);
            Real radius = GetRadius();

            if (signedDistance <= -radius)
            {
                return -1;
            }

            if (signedDistance >= radius)
            {
                return +1;
            }

            return 0;
        }

        // Increase 'this' to contain the input sphere.
        void GrowToContain(BoundingSphere const& sphere)
        {
            Real radius1 = sphere.GetRadius();
            if (radius1 == (Real)0)
            {
                // The incoming bound is invalid and cannot affect growth.
                return;
            }

            Real radius0 = GetRadius();
            if (radius0 == (Real)0)
            {
                // The current bound is invalid, so just assign the incoming
                // bound.
                mTuple = sphere.mTuple;
                return;
            }

            Vector3<Real> center0 = GetCenter();
            Vector3<Real> center1 = sphere.GetCenter();
            Vector3<Real> centerDiff = center1 - center0;
            Real lengthSqr = Dot(centerDiff, centerDiff);
            Real radiusDiff = radius1 - radius0;
            Real radiusDiffSqr = radiusDiff * radiusDiff;

            if (radiusDiffSqr >= lengthSqr)
            {
                if (radiusDiff >= (Real)0)
                {
                    mTuple = sphere.mTuple;
                }
                return;
            }

            Real length = std::sqrt(lengthSqr);
            if (length > (Real)0)
            {
                Real coeff = (length + radiusDiff) / ((Real)2 * length);
                SetCenter(center0 + coeff * centerDiff);
            }

            SetRadius((Real)0.5 * (length + radius0 + radius1));
        }

        // Transform the sphere.  If the transform has nonuniform scaling, the
        // resulting object is an ellipsoid.  A sphere is generated to contain
        // the ellipsoid.
        void TransformBy(Matrix4x4<Real> const& hmatrix, BoundingSphere& sphere) const
        {
            // The spectral norm (maximum absolute value of the eigenvalues)
            // is smaller or equal to max-row-sum and max-col-sum norm.
            // Therefore, 'norm' is an approximation to the maximum scale.
#if defined (GTE_USE_MAT_VEC)
            Vector4<Real> hcenter = hmatrix * HLift(GetCenter(), (Real)1);
            sphere.SetCenter(HProject(hcenter));

            // Use the max-row-sum matrix norm.
            Real r0 = std::fabs(hmatrix(0, 0)) + std::fabs(hmatrix(0, 1)) + std::fabs(hmatrix(0, 2));
            Real r1 = std::fabs(hmatrix(1, 0)) + std::fabs(hmatrix(1, 1)) + std::fabs(hmatrix(1, 2));
            Real r2 = std::fabs(hmatrix(2, 0)) + std::fabs(hmatrix(2, 1)) + std::fabs(hmatrix(2, 2));
            Real norm = std::max(std::max(r0, r1), r2);
#else
            Vector4<Real> hcenter = HLift(GetCenter(), (Real)1) * hmatrix;
            sphere.SetCenter(HProject(hcenter));

            // Use the max-col-sum matrix norm.
            Real r0 = std::fabs(hmatrix(0, 0)) + std::fabs(hmatrix(1, 0)) + std::fabs(hmatrix(2, 0));
            Real r1 = std::fabs(hmatrix(0, 1)) + std::fabs(hmatrix(1, 1)) + std::fabs(hmatrix(2, 1));
            Real r2 = std::fabs(hmatrix(0, 2)) + std::fabs(hmatrix(1, 2)) + std::fabs(hmatrix(2, 2));
            Real norm = std::max(std::max(r0, r1), r2);
#endif
            sphere.SetRadius(norm * GetRadius());
        }

        // This function is valid only for 3-channel points (x,y,z) or
        // 4-channel vectors (x,y,z,0) or 4-channel points (x,y,z,1).  In all
        // cases, the function accesses only the (x,y,z) values.  The stride
        // allows you to pass in vertex buffer data.  Set the stride to zero
        // when the points are contiguous in memory.  The 'data' pointer must
        // be to the first point (offset 0).
        void ComputeFromData(uint32_t numVertices, uint32_t vertexSize, char const* data)
        {
            // The center is the average of the positions.
            Real sum[3] = { (Real)0, (Real)0, (Real)0 };
            for (uint32_t i = 0; i < numVertices; ++i)
            {
                Real const* position = reinterpret_cast<Real const*>(data + i * vertexSize);
                sum[0] += position[0];
                sum[1] += position[1];
                sum[2] += position[2];
            }
            Real invNumVertices = (Real)1 / static_cast<Real>(numVertices);
            mTuple[0] = sum[0] * invNumVertices;
            mTuple[1] = sum[1] * invNumVertices;
            mTuple[2] = sum[2] * invNumVertices;

            // The radius is the largest distance from the center to the
            // positions.
            mTuple[3] = (Real)0;
            for (uint32_t i = 0; i < numVertices; ++i)
            {
                Real const* position = reinterpret_cast<Real const*>(data + i * vertexSize);
                Real diff[3] =
                {
                    position[0] - mTuple[0],
                    position[1] - mTuple[1],
                    position[2] - mTuple[2]
                };
                Real radiusSqr = diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2];
                if (radiusSqr > mTuple[3])
                {
                    mTuple[3] = radiusSqr;
                }
            }

            mTuple[3] = std::sqrt(mTuple[3]);
        }

        // Test for intersection of linear component and bound (points of
        // intersection not computed).  The linear component is parameterized
        // by P + t*D, where P is a point on the component (the origin) and D
        // is a unit-length direction vector.  The interval [tmin,tmax] is
        //   line:     tmin = -INFINITY, tmax = INFINITY
        //   ray:      tmin = 0.0f, tmax = INFINITY
        //   segment:  tmin >= 0.0f, tmax > tmin
        // where INFINITY is std::numeric_limits<Real>::max().
        bool TestIntersection(Vector3<Real> const& origin, Vector3<Real> const& direction,
            Real tmin, Real tmax) const
        {
            Real radius = GetRadius();
            if (radius == (Real)0)
            {
                // The bound is invalid and cannot be intersected.
                LogWarning("Invalid bound. Did you forget to call UpdateModelBound()?");
                return false;
            }

            Vector3<Real> center = GetCenter();
            Real const infinity = std::numeric_limits<Real>::max();
            Vector3<Real> diff;
            Real a0, a1, discr;

            if (tmin == -infinity)
            {
                LogAssert(tmax == infinity, "tmax must be infinity for a line.");

                // Test for sphere-line intersection.
                diff = origin - center;
                a0 = Dot(diff, diff) - radius * radius;
                a1 = Dot(direction, diff);
                discr = a1 * a1 - a0;
                return discr >= (Real)0;
            }

            if (tmax == infinity)
            {
                LogAssert(tmin == (Real)0, "tmin must be zero for a ray.");

                // Test for sphere-ray intersection.
                diff = origin - center;
                a0 = Dot(diff, diff) - radius * radius;
                if (a0 <= (Real)0)
                {
                    // The ray origin is inside the sphere.
                    return true;
                }
                // else: The ray origin is outside the sphere.

                a1 = Dot(direction, diff);
                if (a1 >= (Real)0)
                {
                    // The ray forms an acute angle with diff, and so the ray
                    // is directed from the sphere.  Thus, the ray origin is
                    // outside the sphere, and points P+t*D for t >= 0 are
                    // even farther away from the sphere.
                    return false;
                }

                discr = a1 * a1 - a0;
                return discr >= (Real)0;
            }

            LogAssert(tmax > tmin, "tmin < tmax is required for a segment.");

            // Test for sphere-segment intersection.
            Real taverage = (Real)0.5 * (tmin + tmax);
            Vector3<Real> segOrigin = origin + taverage * direction;
            Real segExtent = (Real)0.5 * (tmax - tmin);

            diff = segOrigin - GetCenter();
            a0 = Dot(diff, diff) - radius * radius;
            if (a0 <= (Real)0)
            {
                // The segment center is inside the sphere.
                return true;
            }

            a1 = Dot(direction, diff);
            discr = a1 * a1 - a0;
            if (discr <= (Real)0)
            {
                // The line is outside the sphere, which implies the segment
                // is also.
                return false;
            }

            // See "3D Game Engine Design (2nd edition)", Section 15.4.3 for
            // the details of the test-intersection query for a segment and a
            // sphere.  In the book, 'qval' is the same as
            // '(segment.e - |a1|)^2 - discr'.
            Real absA1 = std::fabs(a1);
            Real tmp = segExtent - absA1;
            return tmp * tmp <= discr || segExtent >= absA1;
        }

        // Test for intersection of the two stationary spheres.
        bool TestIntersection(BoundingSphere const& sphere) const
        {
            if (sphere.GetRadius() == (Real)0 || GetRadius() == (Real)0)
            {
                // One of the bounds is invalid and cannot be intersected.
                LogWarning("Invalid bound. Did you forget to call UpdateModelBound()?");
                return false;
            }

            // Test for staticSphere-staticSphere intersection.
            Vector3<Real> diff = GetCenter() - sphere.GetCenter();
            Real rSum = GetRadius() + sphere.GetRadius();
            return Dot(diff, diff) <= rSum * rSum;
        }

        // Test for intersection of the two moving spheres.  The velocity0 is
        // for 'this' and the velocity1 is the for the input bound.
        bool TestIntersection(BoundingSphere const& sphere, Real tmax,
            Vector3<Real> const& velocity0, Vector3<Real> const& velocity1) const
        {
            if (sphere.GetRadius() == (Real)0 || GetRadius() == (Real)0)
            {
                // One of the bounds is invalid and cannot be intersected.
                LogWarning("Invalid bound. Did you forget to call UpdateModelBound()?");
                return false;
            }

            // Test for movingSphere-movingSphere intersection.
            Vector3<Real> relVelocity = velocity1 - velocity0;
            Vector3<Real> cenDiff = sphere.GetCenter() - GetCenter();
            Real a = Dot(relVelocity, relVelocity);
            Real c = Dot(cenDiff, cenDiff);
            Real rSum = sphere.GetRadius() + GetRadius();
            Real rSumSqr = rSum * rSum;

            if (a > (Real)0)
            {
                Real b = Dot(cenDiff, relVelocity);
                if (b <= (Real)0)
                {
                    if (-tmax * a <= b)
                    {
                        return a * c - b * b <= a * rSumSqr;
                    }
                    else
                    {
                        return tmax * (tmax * a + (Real)2 * b) + c <= rSumSqr;
                    }
                }
            }

            return c <= rSumSqr;
        }

    private:
        // (center, radius) = (c0, c1, c2, r)
        std::array<Real, 4> mTuple;
    };
}
