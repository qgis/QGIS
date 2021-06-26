// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrRay2OrientedBox2.h>
#include <Mathematics/Cone.h>

// The queries consider the box and cone to be solids.
//
// Define V = cone.ray.origin, D = cone.ray.direction, and cs = cone.cosAngle.
// Define C = box.center, U0 = box.axis[0], U1 = box.axis[1],
// e0 = box.extent[0], and e1 = box.extent[1].  A box point is
// P = C + x*U0 + y*U1 where |x| <= e0 and |y| <= e1.  Define the function
//   F(P) = Dot(D, (P-V)/Length(P-V)) = F(x,y)
//     = Dot(D, (x*U0 + y*U1 + (C-V))/|x*U0 + y*U1 + (C-V)|
//     = (a0*x + a1*y + a2)/(x^2 + y^2 + 2*b0*x + 2*b1*y + b2)^{1/2}
// The function has an essential singularity when P = V.  The box intersects
// the cone (with positive-area overlap) when at least one of the four box
// corners is strictly inside the cone.  It is necessary that the numerator
// of F(P) be positive at such a corner.  The (interior of the) solid cone
// is defined by the quadratic inequality
//   (Dot(D,P-V))^2 > |P-V|^2*(cone.cosAngle)^2
// This inequality is inexpensive to compute.  In summary, overlap occurs
// when there is a box corner P for which
//   F(P) > 0 and (Dot(D,P-V))^2 > |P-V|^2*(cone.cosAngle)^2

namespace gte
{
    template <typename Real>
    class TIQuery<Real, OrientedBox<2, Real>, Cone<2, Real>>
    {
    public:
        struct Result
        {
            // The value of 'intersect' is true when there is a box point that
            // is strictly inside the cone.  If the box just touches the cone
            // from the outside, an intersection is not reported, which
            // supports the common operation of culling objects outside a
            // cone.
            bool intersect;
        };

        Result operator()(OrientedBox<2, Real> const& box, Cone<2, Real>& cone)
        {
            Result result;

            TIQuery<Real, Ray<2, Real>, OrientedBox<2, Real>> rbQuery;
            auto rbResult = rbQuery(cone.ray, box);
            if (rbResult.intersect)
            {
                // The cone intersects the box.
                result.intersect = true;
                return result;
            }

            // Define V = cone.ray.origin, D = cone.ray.direction, and
            // cs = cone.cosAngle.  Define C = box.center, U0 = box.axis[0],
            // U1 = box.axis[1], e0 = box.extent[0], and e1 = box.extent[1].
            // A box point is P = C + x*U0 + y*U1 where |x| <= e0 and
            // |y| <= e1.  Define the function
            //   F(x,y) = Dot(D, (P-V)/Length(P-V))
            //   = Dot(D, (x*U0 + y*U1 + (C-V))/|x*U0 + y*U1 + (C-V)|
            //   = (a0*x + a1*y + a2)/(x^2 + y^2 + 2*b0*x + 2*b1*y + b2)^{1/2}
            // The function has an essential singularity when P = V.
            Vector<2, Real> diff = box.center - cone.ray.origin;
            Real a0 = Dot(cone.ray.direction, box.axis[0]);
            Real a1 = Dot(cone.ray.direction, box.axis[1]);
            Real a2 = Dot(cone.ray.direction, diff);
            Real b0 = Dot(box.axis[0], diff);
            Real b1 = Dot(box.axis[1], diff);
            Real b2 = Dot(diff, diff);
            Real csSqr = cone.cosAngle * cone.cosAngle;

            for (int i1 = 0; i1 < 2; ++i1)
            {
                Real sign1 = i1 * (Real)2 - (Real)1;
                Real y = sign1 * box.extent[1];
                for (int i0 = 0; i0 < 2; ++i0)
                {
                    Real sign0 = i0 * (Real)2 - (Real)1;
                    Real x = sign0 * box.extent[0];
                    Real fNumerator = a0 * x + a1 * y + a2;
                    if (fNumerator > (Real)0)
                    {
                        Real dSqr = x * x + y * y + (b0 * x + b1 * y) * (Real)2 + b2;
                        Real nSqr = fNumerator * fNumerator;
                        if (nSqr > dSqr * csSqr)
                        {
                            result.intersect = true;
                            return result;
                        }
                    }
                }
            }

            result.intersect = false;
            return result;
        }
    };
}
