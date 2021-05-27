// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Vector3.h>

// A torus with origin (0,0,0), outer radius r0 and inner radius r1 (with
// (r0 >= r1) is defined implicitly as follows.  The point P0 = (x,y,z) is on
// the torus. Its projection onto the xy-plane is P1 = (x,y,0).  The circular
// cross section of the torus that contains the projection has radius r0 and
// center P2 = r0*(x,y,0)/sqrt(x^2+y^2).  The points triangle <P0,P1,P2> is a
// right triangle with right angle at P1.  The hypotenuse <P0,P2> has length
// r1, leg <P1,P2> has length z and leg <P0,P1> has length
// |r0 - sqrt(x^2+y^2)|.  The Pythagorean theorem says
// z^2 + |r0 - sqrt(x^2+y^2)|^2 = r1^2.  This can be algebraically
// manipulated to
//   (x^2 + y^2 + z^2 + r0^2 - r1^2)^2 - 4 * r0^2 * (x^2 + y^2) = 0
//
// A parametric form is
//   x = (r0 + r1 * cos(v)) * cos(u)
//   y = (r0 + r1 * cos(v)) * sin(u)
//   z = r1 * sin(v)
// for u in [0,2*pi) and v in [0,2*pi).
//
// Generally, let the torus center be C with plane of symmetry containing C
// and having directions D0 and D1.  The axis of symmetry is the line
// containing C and having direction N (the plane normal).  The radius from
// the center of the torus is r0 and the radius of the tube of the torus is
// r1.  A point P may be written as P = C + x*D0 + y*D1 + z*N, where matrix
// [D0 D1 N] is orthonormal and has determinant 1.  Thus, x = Dot(D0,P-C),
// y = Dot(D1,P-C) and z = Dot(N,P-C).  The implicit form is
//      [|P-C|^2 + r0^2 - r1^2]^2 - 4*r0^2*[|P-C|^2 - (Dot(N,P-C))^2] = 0
// Observe that D0 and D1 are not present in the equation, which is to be
// expected by the symmetry.  The parametric form is
//      P(u,v) = C + (r0 + r1*cos(v))*(cos(u)*D0 + sin(u)*D1) + r1*sin(v)*N
// for u in [0,2*pi) and v in [0,2*pi).
//
// In the class Torus3, the members are 'center' C, 'direction0' D0,
// 'direction1' D1, 'normal' N, 'radius0' r0 and 'radius1' r1.

namespace gte
{
    template <typename Real>
    class Torus3
    {
    public:
        // Construction and destruction.  The default constructor sets center
        // to (0,0,0), direction0 to (1,0,0), direction1 to (0,1,0), normal
        // to (0,0,1), radius0 to 2 and radius1 to 1.
        Torus3()
            :
            center(Vector3<Real>::Zero()),
            direction0(Vector3<Real>::Unit(0)),
            direction1(Vector3<Real>::Unit(1)),
            normal(Vector3<Real>::Unit(2)),
            radius0((Real)2),
            radius1((Real)1)
        {
        }

        Torus3(Vector3<Real> const& inCenter, Vector3<Real> const& inDirection0,
            Vector3<Real> const& inDirection1, Vector3<Real> const& inNormal,
            Real inRadius0, Real inRadius1)
            :
            center(inCenter),
            direction0(inDirection0),
            direction1(inDirection1),
            normal(inNormal),
            radius0(inRadius0),
            radius1(inRadius1)
        {
        }

        // Evaluation of the surface.  The function supports derivative
        // calculation through order 2; that is, maxOrder <= 2 is required.
        // If you want only the position, pass in maxOrder of 0.  If you want
        // the position and first-order derivatives, pass in maxOrder of 1,
        // and so on.  The output 'values' are ordered as: position X;
        // first-order derivatives dX/du, dX/dv; second-order derivatives
        // d2X/du2, d2X/dudv, d2X/dv2.  The input array 'jet' must have enough
        // storage for the specified order.
        void Evaluate(Real u, Real v, unsigned int maxOrder, Vector3<Real>* jet) const
        {
            // Compute position.
            Real csu = std::cos(u);
            Real snu = std::sin(u);
            Real csv = std::cos(v);
            Real snv = std::sin(v);
            Real r1csv = radius1 * csv;
            Real r1snv = radius1 * snv;
            Real r0pr1csv = radius0 + r1csv;
            Vector3<Real> combo0 = csu * direction0 + snu * direction1;
            Vector3<Real> r0pr1csvcombo0 = r0pr1csv * combo0;
            Vector3<Real> r1snvnormal = r1snv * normal;
            jet[0] = center + r0pr1csvcombo0 + r1snvnormal;

            if (maxOrder >= 1)
            {
                // Compute first-order derivatives.
                Vector3<Real> combo1 = -snu * direction0 + csu * direction1;
                jet[1] = r0pr1csv * combo1;
                jet[2] = -r1snv * combo0 + r1csv * normal;

                if (maxOrder == 2)
                {
                    // Compute second-order derivatives.
                    jet[3] = -r0pr1csvcombo0;
                    jet[4] = -r1snv * combo1;
                    jet[5] = -r1csv * combo0 - r1snvnormal;
                }
            }
        }

        // Reverse lookup of parameters from position.
        void GetParameters(Vector3<Real> const& X, Real& u, Real& v) const
        {
            Vector3<Real> delta = X - center;

            // (r0 + r1*cos(v))*cos(u)
            Real dot0 = Dot(direction0, delta);

            // (r0 + r1*cos(v))*sin(u)
            Real dot1 = Dot(direction1, delta);

            // r1*sin(v)
            Real dot2 = Dot(normal, delta);

            // r1*cos(v)
            Real r1csv = std::sqrt(dot0 * dot0 + dot1 * dot1) - radius0;

            u = std::atan2(dot1, dot0);
            v = std::atan2(dot2, r1csv);
        }

        Vector3<Real> center, direction0, direction1, normal;
        Real radius0, radius1;

    public:
        // Comparisons to support sorted containers.
        bool operator==(Torus3 const& torus) const
        {
            return center == torus.center
                && direction0 == torus.direction0
                && direction1 == torus.direction1
                && normal == torus.normal
                && radius0 == torus.radius0
                && radius1 == torus.radius1;
        }

        bool operator!=(Torus3 const& torus) const
        {
            return !operator==(torus);
        }

        bool operator< (Torus3 const& torus) const
        {
            if (center < torus.center)
            {
                return true;
            }

            if (center > torus.center)
            {
                return false;
            }

            if (direction0 < torus.direction0)
            {
                return true;
            }

            if (direction0 > torus.direction0)
            {
                return false;
            }

            if (direction1 < torus.direction1)
            {
                return true;
            }

            if (direction1 > torus.direction1)
            {
                return false;
            }

            if (normal < torus.normal)
            {
                return true;
            }

            if (normal > torus.normal)
            {
                return false;
            }

            if (radius0 < torus.radius0)
            {
                return true;
            }

            if (radius0 > torus.radius0)
            {
                return false;
            }

            return radius1 < torus.radius1;
        }

        bool operator<=(Torus3 const& torus) const
        {
            return !torus.operator<(*this);
        }

        bool operator> (Torus3 const& torus) const
        {
            return torus.operator<(*this);
        }

        bool operator>=(Torus3 const& torus) const
        {
            return !operator<(torus);
        }
    };
}
