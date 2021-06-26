// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/TransformController.h>
#include <memory>

namespace gte
{
    class BlendTransformController : public TransformController
    {
    public:
        // The transformations consist of translation, rotation, and uniform
        // scale.  The rotation and scale blending is either geometric or
        // arithmetic, as specified in the other constructor inputs.
        // Translation blending is always arithmetic.  Let {R0,S0,T0} and
        // {R1,S1,T1} be the transformation channels, and let weight w be in
        // [0,1].  Let {R,S,T} be the blended result.  Let q0, q1, and q be
        // quaternions corresponding to R0, R1, and R with Dot(q0,q1) >= 0
        // and A = angle(q0,q1) = acos(Dot(q0,q1)).
        //
        // Translation:  T = (1-w)*T0 + w*T1
        //
        // Arithmetic rotation:  q = Normalize((1-w)*q0 + w*q1)
        // Geometric rotation:
        //   q = Slerp(w,q0,q1)
        //     = (sin((1-w)*A)*q0 + sin(w*A)*q1)/sin(A)
        //
        // Arithmetic scale:  s = (1-w)*s0 + w*s1 for each channel s0, s1, s
        // Geometric scale:  s = sign(s0)*sign(s1)*pow(|s0|,1-w)*pow(|s1|,w)
        //    If either of s0 or s1 is zero, then s is zero.
        BlendTransformController(
            std::shared_ptr<TransformController> const& controller0,
            std::shared_ptr<TransformController> const& controller1,
            bool geometricRotation, bool geometricScale);

        // Member access.  The weight w is a number for which 0 <= w <= 1.
        inline std::shared_ptr<TransformController> const& GetController0() const
        {
            return mController0;
        }

        inline std::shared_ptr<TransformController> const& GetController1() const
        {
            return mController1;
        }

        inline void SetWeight(float weight)
        {
            mWeight = weight;
        }

        inline float GetWeight() const
        {
            return mWeight;
        }

        // The animation update.  The application time is in milliseconds.
        virtual bool Update(double applicationTime) override;

    protected:
        // Set the object for 'this' and for the managed controllers.
        virtual void SetObject(ControlledObject* object) override;

        std::shared_ptr<TransformController> mController0, mController1;
        float mWeight;
        bool mRSMatrices, mGeometricRotation, mGeometricScale;
    };
}
