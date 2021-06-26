// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Cone.h>
#include <Mathematics/Hypersphere.h>
#include <Mathematics/Vector3.h>

// The test-intersection query is based on the document
// https://www.geometrictools.com/Documentation/IntersectionSphereCone.pdf
//
// The find-intersection returns a single point in the set of intersection
// when that intersection is not empty.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Sphere3<Real>, Cone3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Sphere3<Real> const& sphere, Cone3<Real> const& cone)
        {
            Result result;
            if (cone.GetMinHeight() > (Real)0)
            {
                if (cone.IsFinite())
                {
                    result.intersect = DoQueryConeFrustum(sphere, cone);
                }
                else
                {
                    result.intersect = DoQueryInfiniteTruncatedCone(sphere, cone);
                }
            }
            else
            {
                if (cone.IsFinite())
                {
                    result.intersect = DoQueryFiniteCone(sphere, cone);
                }
                else
                {
                    result.intersect = DoQueryInfiniteCone(sphere, cone);
                }
            }
            return result;
        }

    private:
        bool DoQueryInfiniteCone(Sphere3<Real> const& sphere, Cone3<Real> const& cone)
        {
            Vector3<Real> U = cone.ray.origin - (sphere.radius * cone.invSinAngle) * cone.ray.direction;
            Vector3<Real> CmU = sphere.center - U;
            Real AdCmU = Dot(cone.ray.direction, CmU);
            if (AdCmU > (Real)0)
            {
                Real sqrLengthCmU = Dot(CmU, CmU);
                if (AdCmU * AdCmU >= sqrLengthCmU * cone.cosAngleSqr)
                {
                    Vector3<Real> CmV = sphere.center - cone.ray.origin;
                    Real AdCmV = Dot(cone.ray.direction, CmV);
                    if (AdCmV < -sphere.radius)
                    {
                        return false;
                    }

                    Real rSinAngle = sphere.radius * cone.sinAngle;
                    if (AdCmV >= -rSinAngle)
                    {
                        return true;
                    }

                    Real sqrLengthCmV = Dot(CmV, CmV);
                    return sqrLengthCmV <= sphere.radius * sphere.radius;
                }
            }

            return false;
        }

        bool DoQueryInfiniteTruncatedCone(Sphere3<Real> const& sphere, Cone3<Real> const& cone)
        {
            Vector3<Real> U = cone.ray.origin - (sphere.radius * cone.invSinAngle) * cone.ray.direction;
            Vector3<Real> CmU = sphere.center - U;
            Real AdCmU = Dot(cone.ray.direction, CmU);
            if (AdCmU > (Real)0)
            {
                Real sqrLengthCmU = Dot(CmU, CmU);
                if (AdCmU * AdCmU >= sqrLengthCmU * cone.cosAngleSqr)
                {
                    Vector3<Real> CmV = sphere.center - cone.ray.origin;
                    Real AdCmV = Dot(cone.ray.direction, CmV);
                    Real minHeight = cone.GetMinHeight();
                    if (AdCmV < minHeight - sphere.radius)
                    {
                        return false;
                    }

                    Real rSinAngle = sphere.radius * cone.sinAngle;
                    if (AdCmV >= -rSinAngle)
                    {
                        return true;
                    }

                    Vector3<Real> D = CmV - minHeight * cone.ray.direction;
                    Real lengthAxD = Length(Cross(cone.ray.direction, D));
                    Real hminTanAngle = minHeight * cone.tanAngle;
                    if (lengthAxD <= hminTanAngle)
                    {
                        return true;
                    }

                    Real AdD = AdCmV - minHeight;
                    Real diff = lengthAxD - hminTanAngle;
                    Real sqrLengthCmK = AdD * AdD + diff * diff;
                    return sqrLengthCmK <= sphere.radius * sphere.radius;
                }
            }

            return false;
        }

        bool DoQueryFiniteCone(Sphere3<Real> const& sphere, Cone3<Real> const& cone)
        {
            Vector3<Real> U = cone.ray.origin - (sphere.radius * cone.invSinAngle) * cone.ray.direction;
            Vector3<Real> CmU = sphere.center - U;
            Real AdCmU = Dot(cone.ray.direction, CmU);
            if (AdCmU > (Real)0)
            {
                Real sqrLengthCmU = Dot(CmU, CmU);
                if (AdCmU * AdCmU >= sqrLengthCmU * cone.cosAngleSqr)
                {
                    Vector3<Real> CmV = sphere.center - cone.ray.origin;
                    Real AdCmV = Dot(cone.ray.direction, CmV);
                    if (AdCmV < -sphere.radius)
                    {
                        return false;
                    }

                    Real maxHeight = cone.GetMaxHeight();
                    if (AdCmV > cone.GetMaxHeight() + sphere.radius)
                    {
                        return false;
                    }

                    Real rSinAngle = sphere.radius * cone.sinAngle;
                    if (AdCmV >= -rSinAngle)
                    {
                        if (AdCmV <= maxHeight - rSinAngle)
                        {
                            return true;
                        }
                        else
                        {
                            Vector3<Real> barD = CmV - maxHeight * cone.ray.direction;
                            Real lengthAxBarD = Length(Cross(cone.ray.direction, barD));
                            Real hmaxTanAngle = maxHeight * cone.tanAngle;
                            if (lengthAxBarD <= hmaxTanAngle)
                            {
                                return true;
                            }

                            Real AdBarD = AdCmV - maxHeight;
                            Real diff = lengthAxBarD - hmaxTanAngle;
                            Real sqrLengthCmBarK = AdBarD * AdBarD + diff * diff;
                            return sqrLengthCmBarK <= sphere.radius * sphere.radius;
                        }
                    }
                    else
                    {
                        Real sqrLengthCmV = Dot(CmV, CmV);
                        return sqrLengthCmV <= sphere.radius * sphere.radius;
                    }
                }
            }

            return false;
        }

        bool DoQueryConeFrustum(Sphere3<Real> const& sphere, Cone3<Real> const& cone)
        {
            Vector3<Real> U = cone.ray.origin - (sphere.radius * cone.invSinAngle) * cone.ray.direction;
            Vector3<Real> CmU = sphere.center - U;
            Real AdCmU = Dot(cone.ray.direction, CmU);
            if (AdCmU > (Real)0)
            {
                Real sqrLengthCmU = Dot(CmU, CmU);
                if (AdCmU * AdCmU >= sqrLengthCmU * cone.cosAngleSqr)
                {
                    Vector3<Real> CmV = sphere.center - cone.ray.origin;
                    Real AdCmV = Dot(cone.ray.direction, CmV);
                    Real minHeight = cone.GetMinHeight();
                    if (AdCmV < minHeight - sphere.radius)
                    {
                        return false;
                    }

                    Real maxHeight = cone.GetMaxHeight();
                    if (AdCmV > maxHeight + sphere.radius)
                    {
                        return false;
                    }

                    Real rSinAngle = sphere.radius * cone.sinAngle;
                    if (AdCmV >= minHeight - rSinAngle)
                    {
                        if (AdCmV <= maxHeight - rSinAngle)
                        {
                            return true;
                        }
                        else
                        {
                            Vector3<Real> barD = CmV - maxHeight * cone.ray.direction;
                            Real lengthAxBarD = Length(Cross(cone.ray.direction, barD));
                            Real hmaxTanAngle = maxHeight * cone.tanAngle;
                            if (lengthAxBarD <= hmaxTanAngle)
                            {
                                return true;
                            }

                            Real AdBarD = AdCmV - maxHeight;
                            Real diff = lengthAxBarD - hmaxTanAngle;
                            Real sqrLengthCmBarK = AdBarD * AdBarD + diff * diff;
                            return sqrLengthCmBarK <= sphere.radius * sphere.radius;
                        }
                    }
                    else
                    {
                        Vector3<Real> D = CmV - minHeight * cone.ray.direction;
                        Real lengthAxD = Length(Cross(cone.ray.direction, D));
                        Real hminTanAngle = minHeight * cone.tanAngle;
                        if (lengthAxD <= hminTanAngle)
                        {
                            return true;
                        }

                        Real AdD = AdCmV - minHeight;
                        Real diff = lengthAxD - hminTanAngle;
                        Real sqrLengthCmK = AdD * AdD + diff * diff;
                        return sqrLengthCmK <= sphere.radius * sphere.radius;
                    }
                }
            }

            return false;
        }
    };

    template <typename Real>
    class FIQuery<Real, Sphere3<Real>, Cone3<Real>>
    {
    public:
        struct Result
        {
            // If an intersection occurs, it is potentially an infinite set.
            // If the cone vertex is inside the sphere, 'point' is set to the
            // cone vertex.  If the sphere center is inside the cone, 'point'
            // is set to the sphere center. Otherwise, 'point' is set to the
            // cone point that is closest to the cone vertex and inside the
            // sphere.
            bool intersect;
            Vector3<Real> point;
        };

        Result operator()(Sphere3<Real> const& sphere, Cone3<Real> const& cone)
        {
            Result result;

            // Test whether the cone vertex is inside the sphere.
            Vector3<Real> diff = sphere.center - cone.ray.origin;
            Real rSqr = sphere.radius * sphere.radius;
            Real lenSqr = Dot(diff, diff);
            if (lenSqr <= rSqr)
            {
                // The cone vertex is inside the sphere, so the sphere and
                // cone intersect.
                result.intersect = true;
                result.point = cone.ray.origin;
                return result;
            }

            // Test whether the sphere center is inside the cone.
            Real dot = Dot(diff, cone.ray.direction);
            Real dotSqr = dot * dot;
            if (dotSqr >= lenSqr * cone.cosAngleSqr && dot > (Real)0)
            {
                // The sphere center is inside cone, so the sphere and cone
                // intersect.
                result.intersect = true;
                result.point = sphere.center;
                return result;
            }

            // The sphere center is outside the cone.  The problem now reduces
            // to computing an intersection between the circle and the ray in
            // the plane containing the cone vertex and spanned by the cone
            // axis and vector from the cone vertex to the sphere center.

            // The ray is parameterized by t * D + V with t >= 0, |D| = 1 and
            // dot(A,D) = cos(angle).  Also, D = e * A + f * (C - V).
            // Substituting the ray equation into the sphere equation yields
            // R^2 = |t * D + V - C|^2, so the quadratic for intersections is
            // t^2 - 2 * dot(D, C - V) * t + |C - V|^2 - R^2 = 0.  An
            // intersection occurs if and only if the discriminant is
            // nonnegative.  This test becomes
            //     dot(D, C - V)^2 >= dot(C - V, C - V) - R^2
            // Note that if the right-hand side is nonpositive, then the
            // inequality is true (the sphere contains V).  This is already
            // ruled out in the first block of code in this function.

            Real uLen = std::sqrt(std::max(lenSqr - dotSqr, (Real)0));
            Real test = cone.cosAngle * dot + cone.sinAngle * uLen;
            Real discr = test * test - lenSqr + rSqr;

            if (discr >= (Real)0 && test >= (Real)0)
            {
                // Compute the point of intersection closest to the cone
                // vertex.
                result.intersect = true;
                Real t = test - std::sqrt(std::max(discr, (Real)0));
                Vector3<Real> B = diff - dot * cone.ray.direction;
                Real tmp = cone.sinAngle / uLen;
                result.point = t * (cone.cosAngle * cone.ray.direction + tmp * B);
            }
            else
            {
                result.intersect = false;
            }

            return result;
        }
    };
}
