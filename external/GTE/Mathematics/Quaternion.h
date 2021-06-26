// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.04.23

#pragma once

#include <Mathematics/Vector.h>
#include <Mathematics/Matrix.h>
#include <Mathematics/ChebyshevRatio.h>

// A quaternion is of the form
//   q = x * i + y * j + z * k + w * 1 = x * i + y * j + z * k + w
// where w, x, y, and z are real numbers.  The scalar and vector parts are
//   Vector(q) = x * i + y * j + z * k
//   Scalar(q) = w
//   q = Vector(q) + Scalar(q)
// I assume that you are familiar with the arithmetic and algebraic properties
// of quaternions.  See
// https://www.geometrictools.com/Documentation/Quaternions.pdf

namespace gte
{
    template <typename Real>
    class Quaternion
    {
    public:
        // The quaternions are of the form q = x*i + y*j + z*k + w.  In tuple
        // form, q = (x,y,z,w).

        // Construction.  The default constructor does not initialize the
        // members.
        Quaternion() = default;

        Quaternion(Real x, Real y, Real z, Real w)
        {
            mTuple[0] = x;
            mTuple[1] = y;
            mTuple[2] = z;
            mTuple[3] = w;
        }

        // Member access.
        inline Real const& operator[](int i) const
        {
            return mTuple[i];
        }

        inline Real& operator[](int i)
        {
            return mTuple[i];
        }

        // Comparisons.
        inline bool operator==(Quaternion const& q) const
        {
            return mTuple == q.mTuple;
        }

        inline bool operator!=(Quaternion const& q) const
        {
            return mTuple != q.mTuple;
        }

        inline bool operator<(Quaternion const& q) const
        {
            return mTuple < q.mTuple;
        }

        inline bool operator<=(Quaternion const& q) const
        {
            return mTuple <= q.mTuple;
        }

        inline bool operator>(Quaternion const& q) const
        {
            return mTuple > q.mTuple;
        }

        inline bool operator>=(Quaternion const& q) const
        {
            return mTuple >= q.mTuple;
        }

        // Special quaternions.

        // z = 0*i + 0*j + 0*k + 0
        static Quaternion Zero()
        {
            return Quaternion((Real)0, (Real)0, (Real)0, (Real)0);
        }

        // i = 1*i + 0*j + 0*k + 0
        static Quaternion I()
        {
            return Quaternion((Real)1, (Real)0, (Real)0, (Real)0);
        }

        // j = 0*i + 1*j + 0*k + 0
        static Quaternion J()
        {
            return Quaternion((Real)0, (Real)1, (Real)0, (Real)0);
        }

        // k = 0*i + 0*j + 1*k + 0
        static Quaternion K()
        {
            return Quaternion((Real)0, (Real)0, (Real)1, (Real)0);
        }

        // 1 = 0*i + 0*j + 0*k + 1
        static Quaternion Identity()
        {
            return Quaternion((Real)0, (Real)0, (Real)0, (Real)1);
        }

    protected:
        std::array<Real, 4> mTuple;
    };

    // Unary operations.
    template <typename Real>
    Quaternion<Real> operator+(Quaternion<Real> const& q)
    {
        return q;
    }

    template <typename Real>
    Quaternion<Real> operator-(Quaternion<Real> const& q)
    {
        Quaternion<Real> result;
        for (int i = 0; i < 4; ++i)
        {
            result[i] = -q[i];
        }
        return result;
    }

    // Linear algebraic operations.
    template <typename Real>
    Quaternion<Real> operator+(Quaternion<Real> const& q0, Quaternion<Real> const& q1)
    {
        Quaternion<Real> result = q0;
        return result += q1;
    }

    template <typename Real>
    Quaternion<Real> operator-(Quaternion<Real> const& q0, Quaternion<Real> const& q1)
    {
        Quaternion<Real> result = q0;
        return result -= q1;
    }

    template <typename Real>
    Quaternion<Real> operator*(Quaternion<Real> const& q, Real scalar)
    {
        Quaternion<Real> result = q;
        return result *= scalar;
    }

    template <typename Real>
    Quaternion<Real> operator*(Real scalar, Quaternion<Real> const& q)
    {
        Quaternion<Real> result = q;
        return result *= scalar;
    }

    template <typename Real>
    Quaternion<Real> operator/(Quaternion<Real> const& q, Real scalar)
    {
        Quaternion<Real> result = q;
        return result /= scalar;
    }

    template <typename Real>
    Quaternion<Real>& operator+=(Quaternion<Real>& q0, Quaternion<Real> const& q1)
    {
        for (int i = 0; i < 4; ++i)
        {
            q0[i] += q1[i];
        }
        return q0;
    }

    template <typename Real>
    Quaternion<Real>& operator-=(Quaternion<Real>& q0, Quaternion<Real> const& q1)
    {
        for (int i = 0; i < 4; ++i)
        {
            q0[i] -= q1[i];
        }
        return q0;
    }

    template <typename Real>
    Quaternion<Real>& operator*=(Quaternion<Real>& q, Real scalar)
    {
        for (int i = 0; i < 4; ++i)
        {
            q[i] *= scalar;
        }
        return q;
    }

    template <typename Real>
    Quaternion<Real>& operator/=(Quaternion<Real>& q, Real scalar)
    {
        if (scalar != (Real)0)
        {
            for (int i = 0; i < 4; ++i)
            {
                q[i] /= scalar;
            }
        }
        else
        {
            for (int i = 0; i < 4; ++i)
            {
                q[i] = (Real)0;
            }
        }
        return q;
    }

    // Geometric operations.
    template <typename Real>
    Real Dot(Quaternion<Real> const& q0, Quaternion<Real> const& q1)
    {
        Real dot = q0[0] * q1[0];
        for (int i = 1; i < 4; ++i)
        {
            dot += q0[i] * q1[i];
        }
        return dot;
    }

    template <typename Real>
    Real Length(Quaternion<Real> const& q)
    {
        return std::sqrt(Dot(q, q));
    }

    template <typename Real>
    Real Normalize(Quaternion<Real>& q)
    {
        Real length = std::sqrt(Dot(q, q));
        if (length > (Real)0)
        {
            q /= length;
        }
        else
        {
            for (int i = 0; i < 4; ++i)
            {
                q[i] = (Real)0;
            }
        }
        return length;
    }

    // Multiplication of quaternions.  This operation is not generally
    // commutative; that is, q0*q1 and q1*q0 are not usually the same value.
    // (x0*i + y0*j + z0*k + w0)*(x1*i + y1*j + z1*k + w1)
    // =
    // i*(+x0*w1 + y0*z1 - z0*y1 + w0*x1) +
    // j*(-x0*z1 + y0*w1 + z0*x1 + w0*y1) +
    // k*(+x0*y1 - y0*x1 + z0*w1 + w0*z1) +
    // 1*(-x0*x1 - y0*y1 - z0*z1 + w0*w1)
    template <typename Real>
    Quaternion<Real> operator*(Quaternion<Real> const& q0, Quaternion<Real> const& q1)
    {
        // (x0*i + y0*j + z0*k + w0)*(x1*i + y1*j + z1*k + w1)
        // =
        // i*(+x0*w1 + y0*z1 - z0*y1 + w0*x1) +
        // j*(-x0*z1 + y0*w1 + z0*x1 + w0*y1) +
        // k*(+x0*y1 - y0*x1 + z0*w1 + w0*z1) +
        // 1*(-x0*x1 - y0*y1 - z0*z1 + w0*w1)

        return Quaternion<Real>
        (
            +q0[0] * q1[3] + q0[1] * q1[2] - q0[2] * q1[1] + q0[3] * q1[0],
            -q0[0] * q1[2] + q0[1] * q1[3] + q0[2] * q1[0] + q0[3] * q1[1],
            +q0[0] * q1[1] - q0[1] * q1[0] + q0[2] * q1[3] + q0[3] * q1[2],
            -q0[0] * q1[0] - q0[1] * q1[1] - q0[2] * q1[2] + q0[3] * q1[3]
        );
    }

    // For a nonzero quaternion q = (x,y,z,w), inv(q) = (-x,-y,-z,w)/|q|^2,
    // where |q| is the length of the quaternion.  When q is zero, the
    // function returns zero, which is considered to be an improbable case.
    template <typename Real>
    Quaternion<Real> Inverse(Quaternion<Real> const& q)
    {
        Real sqrLen = Dot(q, q);
        if (sqrLen > (Real)0)
        {
            Quaternion<Real> inverse = Conjugate(q) / sqrLen;
            return inverse;
        }
        else
        {
            return Quaternion<Real>::Zero();
        }
    }

    // The conjugate of q = (x,y,z,w) is conj(q) = (-x,-y,-z,w).
    template <typename Real>
    Quaternion<Real> Conjugate(Quaternion<Real> const& q)
    {
        return Quaternion<Real>(-q[0], -q[1], -q[2], +q[3]);
    }

    // Rotate a 3D vector v = (v0,v1,v2) using quaternion multiplication. The
    // input quaternion must be unit length. If R is the rotation matrix
    // corresponding to the quaternion q, the rotated vector u corresponding
    // to v is u = R*v when GTE_USE_MAT_VEC is defined (the default for
    // projects) or u = v*R when GTE_USE_MAT_VEC is not defined.
    template <typename Real>
    Vector<3, Real> Rotate(Quaternion<Real> const& q, Vector<3, Real> const& v)
    {
        Quaternion<Real> input(v[0], v[1], v[2], (Real)0);
#if defined(GTE_USE_MAT_VEC)
        Quaternion<Real> output = q * input * Conjugate(q);
#else
        Quaternion<Real> output = Conjugate(q) * input * q;
#endif
        Vector<3, Real> u{ output[0], output[1], output[2] };
        return u;
    }

    // Rotate a 3D vector, represented as a homogeneous 4D vector
    // v = (v0,v1,v2,0), using quaternion multiplication. The input quaternion
    // must be unit length. If R is the rotation matrix corresponding to the
    // quaternion q, the rotated vector u corresponding to v is u = R*v when
    // GTE_USE_MAT_VEC is defined (the default for projects) or u = v*R when
    // GTE_USE_MAT_VEC is not defined.
    template <typename Real>
    Vector<4, Real> Rotate(Quaternion<Real> const& q, Vector<4, Real> const& v)
    {
        Quaternion<Real> input(v[0], v[1], v[2], (Real)0);
#if defined(GTE_USE_MAT_VEC)
        Quaternion<Real> output = q * input * Conjugate(q);
#else
        Quaternion<Real> output = Conjugate(q) * input * q;
#endif
        Vector<4, Real> u{ output[0], output[1], output[2], (Real)0 };
        return u;
    }

    // The spherical linear interpolation (slerp) of unit-length quaternions
    // q0 and q1 for t in [0,1] is
    //     slerp(t,q0,q1) = [sin(t*theta)*q0 + sin((1-t)*theta)*q1]/sin(theta)
    // where theta is the angle between q0 and q1 [cos(theta) = Dot(q0,q1)].
    // This function is a parameterization of the great spherical arc between
    // q0 and q1 on the unit hypersphere.  Moreover, the parameterization is
    // one of normalized arclength--a particle traveling along the arc through
    // time t does so with constant speed.
    //
    // When using slerp in animations involving sequences of quaternions, it
    // is typical that the quaternions are preprocessed so that consecutive
    // ones form an acute angle A in [0,pi/2].  Other preprocessing can help
    // with performance.  See the function comments below.
    //
    // See GteSlerpEstimate.{h,inl} for various approximations, including
    // SLERP<Real>::EstimateRPH that gives good performance and accurate
    // results for preprocessed quaternions.

    // The angle between q0 and q1 is in [0,pi).  There are no angle
    // restrictions and nothing is precomputed.
    template <typename Real>
    Quaternion<Real> Slerp(Real t, Quaternion<Real> const& q0, Quaternion<Real> const& q1)
    {
        Real cosA = Dot(q0, q1);
        Real sign;
        if (cosA >= (Real)0)
        {
            sign = (Real)1;
        }
        else
        {
            cosA = -cosA;
            sign = (Real)-1;
        }

        Real f0, f1;
        ChebyshevRatio<Real>::Get(t, cosA, f0, f1);
        return q0 * f0 + q1 * (sign * f1);
    }

    // The angle between q0 and q1 must be in [0,pi/2].  The suffix R is for
    // 'Restricted'.  The preprocessing code is
    //   Quaternion<Real> q[n];  // assuming initialized
    //   for (i0 = 0, i1 = 1; i1 < n; i0 = i1++)
    //   {
    //       cosA = Dot(q[i0], q[i1]);
    //       if (cosA < 0)
    //       {
    //           q[i1] = -q[i1];  // now Dot(q[i0], q[i]1) >= 0
    //       }
    //   }
    template <typename Real>
    Quaternion<Real> SlerpR(Real t, Quaternion<Real> const& q0, Quaternion<Real> const& q1)
    {
        Real f0, f1;
        ChebyshevRatio<Real>::Get(t, Dot(q0, q1), f0, f1);
        return q0 * f0 + q1 * f1;
    }

    // The angle between q0 and q1 must be in [0,pi/2].  The suffix R is for
    // 'Restricted' and the suffix P is for 'Preprocessed'.  The preprocessing
    // code is
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
    //       // for Quaterion<Real>::SlerpRP
    //       cosA[i0] = cs;
    //
    //       // for SLERP<Real>::EstimateRP
    //       omcosA[i0] = 1 - cs;
    //   }
    template <typename Real>
    Quaternion<Real> SlerpRP(Real t, Quaternion<Real> const& q0, Quaternion<Real> const& q1, Real cosA)
    {
        Real f0, f1;
        ChebyshevRatio<Real>::Get(t, cosA, f0, f1);
        return q0 * f0 + q1 * f1;
    }

    // The angle between q0 and q1 is A and must be in [0,pi/2].  The suffix R
    // is for 'Restricted', the suffix P is for 'Preprocessed' and the suffix
    // H is for 'Half' (the quaternion qh halfway between q0 and q1 is
    // precomputed).  Quaternion qh is slerp(1/2,q0,q1) = (q0+q1)/|q0+q1|, so
    // the angle between q0 and qh is A/2 and the angle between qh and q1 is
    // A/2.  The preprocessing code is
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
    //
    //       // for Quaternion<Real>::SlerpRPH and SLERP<Real>::EstimateRPH
    //       cosAH[i0] = sqrt((1+cosA)/2);
    //       qh[i0] = (q0 + q1) / (2 * cosAH[i0]);
    //
    //       // for SLERP<Real>::EstimateRPH
    //       omcosAH[i0] = 1 - cosAH[i0];
    //   }
    template <typename Real>
    Quaternion<Real> SlerpRPH(Real t, Quaternion<Real> const& q0, Quaternion<Real> const& q1,
        Quaternion<Real> const& qh, Real cosAH)
    {
        Real f0, f1;
        Real twoT = static_cast<Real>(2) * t;
        if (twoT <= static_cast<Real>(1))
        {
            ChebyshevRatio<Real>::Get(twoT, cosAH, f0, f1);
            return q0 * f0 + qh * f1;
        }
        else
        {
            ChebyshevRatio<Real>::Get(twoT - static_cast<Real>(1), cosAH, f0, f1);
            return qh * f0 + q1 * f1;
        }
    }
}
