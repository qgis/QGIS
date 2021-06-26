// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Quaternion.h>

// The spherical linear interpolation (slerp) of unit-length quaternions
// q0 and q1 for t in [0,1] and theta in (0,pi) is
//   slerp(t,q0,q1) = [sin((1-t)*theta)*q0 + sin(theta)*q1]/sin(theta)
// where theta is the angle between q0 and q1 [cos(theta) = Dot(q0,q1)].
// This function is a parameterization of the great spherical arc between
// q0 and q1 on the unit hypersphere.  Moreover, the parameterization is
// one of normalized arclength--a particle traveling along the arc through
// time t does so with constant speed.
//
// Read the comments in GteChebyshevRatio.h regarding estimates for the
// ratio sin(t*theta)/sin(theta).
//
// When using slerp in animations involving sequences of quaternions, it is
// typical that the quaternions are preprocessed so that consecutive ones
// form an acute angle A in [0,pi/2].  Other preprocessing can help with
// performance.  See the function comments in the SLERP class.

namespace gte
{
    template <typename Real>
    class SLERP
    {
    public:
        // The angle between q0 and q1 is in [0,pi).  There are no angle
        // restrictions and nothing is precomputed.
        template <int N>
        inline static Quaternion<Real> Estimate(Real t, Quaternion<Real> const& q0, Quaternion<Real> const& q1)
        {
            static_assert(1 <= N && N <= 16, "Invalid degree.");

            Real cs = Dot(q0, q1);
            Real sign;
            if (cs >= (Real)0)
            {
                sign = (Real)1;
            }
            else
            {
                cs = -cs;
                sign = (Real)-1;
            }

            Real f0, f1;
            ChebyshevRatio<Real>::template GetEstimate<N>(t, (Real)1 - cs, f0, f1);
            return q0 * f0 + q1 * (sign * f1);
        }


        // The angle between q0 and q1 must be in [0,pi/2].  The suffix R is
        // for 'Restricted'.  The preprocessing code is
        //   Quaternion<Real> q[n];  // assuming initialized
        //   for (i0 = 0, i1 = 1; i1 < n; i0 = i1++)
        //   {
        //       cosA = Dot(q[i0], q[i1]);
        //       if (cosA < 0)
        //       {
        //           q[i1] = -q[i1];  // now Dot(q[i0], q[i]1) >= 0
        //       }
        //   }
        template <int N>
        inline static Quaternion<Real> EstimateR(Real t, Quaternion<Real> const& q0, Quaternion<Real> const& q1)
        {
            static_assert(1 <= N && N <= 16, "Invalid degree.");

            Real f0, f1;
            ChebyshevRatio<Real>::template GetEstimate<N>(t, (Real)1 - Dot(q0, q1), f0, f1);
            return q0 * f0 + q1 * f1;
        }

        // The angle between q0 and q1 must be in [0,pi/2].  The suffix R is
        // for 'Restricted' and the suffix P is for 'Preprocessed'.  The
        // preprocessing code is
        //   Quaternion<Real> q[n];  // assuming initialized
        //   Real cosA[n-1], omcosA[n-1];  // to be precomputed
        //   for (i0 = 0, i1 = 1; i1 < n; i0 = i1++)
        //   {
        //       cs = Dot(q[i0], q[i1]);
        //       if (cosA[i0] < 0)
        //       {
        //           q[i1] = -q[i1];
        //           cs = -cs;
        //       }
        //
        //       // for Quaterion<T>::SlerpRP
        //       cosA[i0] = cs;
        //
        //       // for SLERP<T>::EstimateRP
        //       omcosA[i0] = 1 - cs;
        //   }
        template <int N>
        inline static Quaternion<Real> EstimateRP(Real t, Quaternion<Real> const& q0, Quaternion<Real> const& q1,
            Real omcosA)
        {
            static_assert(1 <= N && N <= 16, "Invalid degree.");

            Real f0, f1;
            ChebyshevRatio<Real>::template GetEstimate<N>(t, omcosA, f0, f1);
            return q0 * f0 + q1 * f1;
        }

        // The angle between q0 and q1 is A and must be in [0,pi/2].
        // Quaternion qh is slerp(1/2,q0,q1) = (q0+q1)/|q0+q1|, so the angle
        // between q0 and qh is A/2 and the angle between qh and q1 is A/2.
        // The preprocessing code is
        //   Quaternion<Real> q[n];  // assuming initialized
        //   Quaternion<Real> qh[n-1];  // to be precomputed
        //   Real omcosAH[n-1];  // to be precomputed
        //   for (i0 = 0, i1 = 1; i1 < n; i0 = i1++)
        //   {
        //       cosA = Dot(q[i0], q[i1]);
        //       if (cosA < 0)
        //       {
        //           q[i1] = -q[i1];
        //           cosA = -cosA;
        //       }
        //       cosAH = sqrt((1 + cosA)/2);
        //       qh[i0] = (q0 + q1) / (2 * cosAH[i0]);
        //       omcosAH[i0] = 1 - cosAH;
        //   }
        template <int N>
        inline static Quaternion<Real> EstimateRPH(Real t, Quaternion<Real> const& q0, Quaternion<Real> const& q1,
            Quaternion<Real> const& qh, Real omcosAH)
        {
            static_assert(1 <= N && N <= 16, "Invalid degree.");

            Real f0, f1;
            Real twoT = t * (Real)2;
            if (twoT <= (Real)1)
            {
                ChebyshevRatio<Real>::template GetEstimate<N>(twoT, omcosAH, f0, f1);
                return q0 * f0 + qh * f1;
            }
            else
            {
                ChebyshevRatio<Real>::template GetEstimate<N>(twoT - (Real)1, omcosAH, f0, f1);
                return qh * f0 + q1 * f1;
            }
        }
    };
}
