// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.04.30

#pragma once

#include <Mathematics/Matrix3x3.h>
#include <Mathematics/Rotation.h>
#include <functional>

namespace gte
{
    template <typename Real>
    class RigidBody
    {
    public:
        // Construction and destruction.  The rigid body state is
        // uninitialized.  Use the set functions to initialize the state
        // before starting the simulation.
        virtual ~RigidBody() = default;

        RigidBody()
            :
            mMass(std::numeric_limits<Real>::max()),
            mInvMass((Real)0),
            mInertia(Matrix3x3<Real>::Identity()),
            mInvInertia(Matrix3x3<Real>::Zero()),
            mPosition(Vector3<Real>::Zero()),
            mQuatOrient(Quaternion<Real>::Identity()),
            mLinearMomentum(Vector3<Real>::Zero()),
            mAngularMomentum(Vector3<Real>::Zero()),
            mRotOrient(Matrix3x3<Real>::Identity()),
            mLinearVelocity(Vector3<Real>::Zero()),
            mAngularVelocity(Vector3<Real>::Zero())
        {
            // The default body is immovable.
        }

        // Set rigid body state.
        void SetMass(float mass)
        {
            if ((Real)0 < mass && mass < std::numeric_limits<Real>::max())
            {
                mMass = mass;
                mInvMass = (Real)1 / mass;
            }
            else
            {
                // Assume the body as immovable.
                mMass = std::numeric_limits<Real>::max();
                mInvMass = (Real)0;
                mInertia = Matrix3x3<Real>::Identity();
                mInvInertia = Matrix3x3<Real>::Zero();
                mQuatOrient = Quaternion<Real>::Identity();
                mLinearMomentum = Vector3<Real>::Zero();
                mAngularMomentum = Vector3<Real>::Zero();
                mRotOrient = Matrix3x3<Real>::Identity();
                mLinearVelocity = Vector3<Real>::Zero();
                mAngularVelocity = Vector3<Real>::Zero();
            }
        }

        void SetBodyInertia(Matrix3x3<Real> const& inertia)
        {
            mInertia = inertia;
            mInvInertia = Inverse(mInertia);
        }

        inline void SetPosition(Vector3<Real> const& position)
        {
            mPosition = position;
        }

        void SetQOrientation(Quaternion<Real> const& quatOrient)
        {
            mQuatOrient = quatOrient;
            mRotOrient = Rotation<3, Real>(mQuatOrient);
        }

        void SetLinearMomentum(Vector3<Real> const& linearMomentum)
        {
            mLinearMomentum = linearMomentum;
            mLinearVelocity = mInvMass * mLinearMomentum;
        }

        void SetAngularMomentum(Vector3<Real> const& angularMomentum)
        {
            mAngularMomentum = angularMomentum;

            // V = R^T*M
            mAngularVelocity = mAngularMomentum * mRotOrient;

            // V = J^{-1}*R^T*M
            mAngularVelocity = mInvInertia * mAngularVelocity;

            // V = R*J^{-1}*R^T*M
            mAngularVelocity = mRotOrient * mAngularVelocity;
        }

        void SetROrientation(Matrix3x3<Real> const& rotOrient)
        {
            mRotOrient = rotOrient;
            mQuatOrient = Rotation<3, Real>(mRotOrient);
        }

        void SetLinearVelocity(Vector3<Real> const& linearVelocity)
        {
            mLinearVelocity = linearVelocity;
            mLinearMomentum = mMass * mLinearVelocity;
        }

        void SetAngularVelocity(Vector3<Real> const& angularVelocity)
        {
            mAngularVelocity = angularVelocity;

            // M = R^T*V
            mAngularMomentum = mAngularVelocity * mRotOrient;

            // M = J*R^T*V
            mAngularMomentum = mInertia * mAngularMomentum;

            // M = R*J*R^T*V
            mAngularMomentum = mRotOrient * mAngularMomentum;
        }

        // Get rigid body state.
        inline Real GetMass() const
        {
            return mMass;
        }

        inline Real GetInverseMass() const
        {
            return mInvMass;
        }

        inline Matrix3x3<Real> const& GetBodyInertia() const
        {
            return mInertia;
        }

        inline Matrix3x3<Real> const& GetBodyInverseInertia() const
        {
            return mInvInertia;
        }

        Matrix3x3<Real> GetWorldInertia() const
        {
            return MultiplyABT(mRotOrient * mInertia, mRotOrient);  // R*J*R^T
        }

        Matrix3x3<Real> GetWorldInverseInertia() const
        {
            // R*J^{-1}*R^T
            return MultiplyABT(mRotOrient * mInvInertia, mRotOrient);
        }

        inline Vector3<Real> const& GetPosition() const
        {
            return mPosition;
        }

        Quaternion<Real> const& GetQOrientation() const
        {
            return mQuatOrient;
        }

        inline Vector3<Real> const& GetLinearMomentum() const
        {
            return mLinearMomentum;
        }

        inline Vector3<Real> const& GetAngularMomentum() const
        {
            return mAngularMomentum;
        }

        inline Matrix3x3<Real> const& GetROrientation() const
        {
            return mRotOrient;
        }

        inline Vector3<Real> const& GetLinearVelocity() const
        {
            return mLinearVelocity;
        }

        inline Vector3<Real> const& GetAngularVelocity() const
        {
            return mAngularVelocity;
        }

        // Force/torque function format.
        typedef std::function
            <
            Vector3<Real>
            (
                Real,                       // time of application
                Real,                       // mass
                Vector3<Real> const&,       // position
                Quaternion<Real> const&,    // orientation
                Vector3<Real> const&,       // linear momentum
                Vector3<Real> const&,       // angular momentum
                Matrix3x3<Real> const&,     // orientation
                Vector3<Real> const&,       // linear velocity
                Vector3<Real> const&        // angular velocity
                )
            >
            Function;

        // Force and torque functions.
        Function mForce;
        Function mTorque;

        // Runge-Kutta fourth-order differential equation solver
        void Update(Real t, Real dt)
        {
            // TODO: When GTE_MAT_VEC is not defined (i.e. use vec-mat),
            // test to see whether dq/dt = 0.5 * w * q (mat-vec convention)
            // needs to become a different equation.
            Real halfDT = (Real)0.5 * dt;
            Real sixthDT = dt / (Real)6;
            Real TpHalfDT = t + halfDT;
            Real TpDT = t + dt;

            Vector3<Real> newPosition, newLinearMomentum, newAngularMomentum;
            Vector3<Real> newLinearVelocity, newAngularVelocity;
            Quaternion<Real> newQuatOrient;
            Matrix3x3<Real> newRotOrient;

            // A1 = G(T,S0), B1 = S0 + (DT/2)*A1
            Vector3<Real> A1DXDT = mLinearVelocity;
            Quaternion<Real> W = Quaternion<Real>(mAngularVelocity[0],
                mAngularVelocity[1], mAngularVelocity[2], (Real)0);
            Quaternion<Real> A1DQDT = (Real)0.5 * W * mQuatOrient;

            Vector3<Real> A1DPDT = mForce(t, mMass, mPosition, mQuatOrient,
                mLinearMomentum, mAngularMomentum, mRotOrient, mLinearVelocity,
                mAngularVelocity);

            Vector3<Real> A1DLDT = mTorque(t, mMass, mPosition, mQuatOrient,
                mLinearMomentum, mAngularMomentum, mRotOrient, mLinearVelocity,
                mAngularVelocity);

            newPosition = mPosition + halfDT * A1DXDT;
            newQuatOrient = mQuatOrient + halfDT * A1DQDT;
            Normalize(newQuatOrient);
            newLinearMomentum = mLinearMomentum + halfDT * A1DPDT;
            newAngularMomentum = mAngularMomentum + halfDT * A1DLDT;
            newRotOrient = Rotation<3, Real>(newQuatOrient);
            newLinearVelocity = mInvMass * newLinearMomentum;
            newAngularVelocity = newAngularMomentum * newRotOrient;
            newAngularVelocity = mInvInertia * newAngularVelocity;
            newAngularVelocity = newRotOrient * newAngularVelocity;

            // A2 = G(T+DT/2,B1), B2 = S0 + (DT/2)*A2
            Vector3<Real> A2DXDT = newLinearVelocity;
            W = Quaternion<Real>(newAngularVelocity[0], newAngularVelocity[1],
                newAngularVelocity[2], (Real)0);
            Quaternion<Real> A2DQDT = (Real)0.5 * W * newQuatOrient;

            Vector3<Real> A2DPDT = mForce(TpHalfDT, mMass, newPosition,
                newQuatOrient, newLinearMomentum, newAngularMomentum, newRotOrient,
                newLinearVelocity, newAngularVelocity);

            Vector3<Real> A2DLDT = mTorque(TpHalfDT, mMass, newPosition,
                newQuatOrient, newLinearMomentum, newAngularMomentum, newRotOrient,
                newLinearVelocity, newAngularVelocity);

            newPosition = mPosition + halfDT * A2DXDT;
            newQuatOrient = mQuatOrient + halfDT * A2DQDT;
            Normalize(newQuatOrient);
            newLinearMomentum = mLinearMomentum + halfDT * A2DPDT;
            newAngularMomentum = mAngularMomentum + halfDT * A2DLDT;
            newRotOrient = Rotation<3, Real>(newQuatOrient);
            newLinearVelocity = mInvMass * newLinearMomentum;
            newAngularVelocity = newAngularMomentum * newRotOrient;
            newAngularVelocity = mInvInertia * newAngularVelocity;
            newAngularVelocity = newRotOrient * newAngularVelocity;

            // A3 = G(T+DT/2,B2), B3 = S0 + DT*A3
            Vector3<Real> A3DXDT = newLinearVelocity;
            W = Quaternion<Real>(newAngularVelocity[0], newAngularVelocity[1],
                newAngularVelocity[2], (Real)0);
            Quaternion<Real> A3DQDT = (Real)0.5 * W * newQuatOrient;

            Vector3<Real> A3DPDT = mForce(TpHalfDT, mMass, newPosition,
                newQuatOrient, newLinearMomentum, newAngularMomentum, newRotOrient,
                newLinearVelocity, newAngularVelocity);

            Vector3<Real> A3DLDT = mTorque(TpHalfDT, mMass, newPosition,
                newQuatOrient, newLinearMomentum, newAngularMomentum, newRotOrient,
                newLinearVelocity, newAngularVelocity);

            newPosition = mPosition + dt * A3DXDT;
            newQuatOrient = mQuatOrient + dt * A3DQDT;
            Normalize(newQuatOrient);
            newLinearMomentum = mLinearMomentum + dt * A3DPDT;
            newAngularMomentum = mAngularMomentum + dt * A3DLDT;
            newRotOrient = Rotation<3, Real>(newQuatOrient);
            newLinearVelocity = mInvMass * newLinearMomentum;
            newAngularVelocity = newAngularMomentum * newRotOrient;
            newAngularVelocity = mInvInertia * newAngularVelocity;
            newAngularVelocity = newRotOrient * newAngularVelocity;

            // A4 = G(T+DT,B3), S1 = S0 + (DT/6)*(A1+2*(A2+A3)+A4)
            Vector3<Real> A4DXDT = newLinearVelocity;
            W = Quaternion<Real>(newAngularVelocity[0], newAngularVelocity[1],
                newAngularVelocity[2], (Real)0);
            Quaternion<Real> A4DQDT = (Real)0.5 * W * newQuatOrient;

            Vector3<Real> A4DPDT = mForce(TpDT, mMass, newPosition,
                newQuatOrient, newLinearMomentum, newAngularMomentum, newRotOrient,
                newLinearVelocity, newAngularVelocity);

            Vector3<Real> A4DLDT = mTorque(TpDT, mMass, newPosition, newQuatOrient,
                newLinearMomentum, newAngularMomentum, newRotOrient,
                newLinearVelocity, newAngularVelocity);

            mPosition += sixthDT * (A1DXDT + (Real)2 * (A2DXDT + A3DXDT) + A4DXDT);
            mQuatOrient += sixthDT * (A1DQDT + (Real)2 * (A2DQDT + A3DQDT) + A4DQDT);
            mLinearMomentum += sixthDT * (A1DPDT + (Real)2 * (A2DPDT + A3DPDT) + A4DPDT);
            mAngularMomentum += sixthDT * (A1DLDT + (Real)2 * (A2DLDT + A3DLDT) + A4DLDT);

            Normalize(mQuatOrient);
            mRotOrient = Rotation<3, Real>(mQuatOrient);
            mLinearVelocity = mInvMass * mLinearMomentum;
            mAngularVelocity = mAngularMomentum * mRotOrient;
            mAngularVelocity = mInvInertia * mAngularVelocity;
            mAngularVelocity = mRotOrient * mAngularVelocity;
        }

    protected:
        // Constant quantities (matrices in body coordinates).
        Real mMass, mInvMass;
        Matrix3x3<Real> mInertia, mInvInertia;

        // State variables.
        Vector3<Real> mPosition;
        Quaternion<Real> mQuatOrient;
        Vector3<Real> mLinearMomentum;
        Vector3<Real> mAngularMomentum;

        // Derived state variables.
        Matrix3x3<Real> mRotOrient;
        Vector3<Real> mLinearVelocity;
        Vector3<Real> mAngularVelocity;
    };
}
