// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Camera.h>
#include <Graphics/ConstantBuffer.h>
#include <map>

namespace gte
{
    class CameraRig
    {
    public:
        // Construction.  The camera rig is designed for moving the camera
        // around in a world coordinate system.  This object is used in the
        // sample 3D applications.
        virtual ~CameraRig() = default;
        CameraRig();
        CameraRig(std::shared_ptr<Camera> const& camera,
            float translationSpeed, float rotationSpeed);

        // Member access.  The functions are for deferred construction after
        // a default construction of a camera rig.
        void Set(std::shared_ptr<Camera> const& camera,
            float translationSpeed, float rotationSpeed);

        // TODO:  The camera vectors are the world axes?  (INVARIANT)
        void ComputeWorldAxes();

        inline std::shared_ptr<Camera> const& GetCamera() const
        {
            return mCamera;
        }

        inline void SetTranslationSpeed(float translationSpeed)
        {
            mTranslationSpeed = translationSpeed;
        }

        inline float GetTranslationSpeed() const
        {
            return mTranslationSpeed;
        }

        inline void SetRotationSpeed(float rotationSpeed)
        {
            mRotationSpeed = rotationSpeed;
        }

        inline float GetRotationSpeed() const
        {
            return mRotationSpeed;
        }

        // Control of camera motion.  If the camera moves, subscribers to the
        // pvw-matrix update will have the system memory and GPU memory of the
        // constant buffers updated.  Only one motion may be active at a
        // single time.  When a motion is active, a call to Move() will
        // execute that motion.

        // The motion is controlled directly by calling SetDirect*().
        inline void SetDirectMoveForward()
        {
            mMotion = &CameraRig::MoveForward;
        }

        inline void SetDirectMoveBackward()
        {
            mMotion = &CameraRig::MoveBackward;
        }

        inline void SetDirectMoveUp()
        {
            mMotion = &CameraRig::MoveUp;
        }

        inline void SetDirectMoveDown()
        {
            mMotion = &CameraRig::MoveDown;
        }

        inline void SetDirectMoveRight()
        {
            mMotion = &CameraRig::MoveRight;
        }

        inline void SetDirectMoveLeft()
        {
            mMotion = &CameraRig::MoveLeft;
        }

        inline void SetDirectTurnRight()
        {
            mMotion = &CameraRig::TurnRight;
        }

        inline void SetDirectTurnLeft()
        {
            mMotion = &CameraRig::TurnLeft;
        }

        inline void SetDirectLookUp()
        {
            mMotion = &CameraRig::LookUp;
        }

        inline void SetDirectLookDown()
        {
            mMotion = &CameraRig::LookDown;
        }

        inline void SetDirectRollClockwise()
        {
            mMotion = &CameraRig::RollClockwise;
        }

        inline void SetDirectRollCounterclockwise()
        {
            mMotion = &CameraRig::RollCounterclockwise;
        }

        // The motion is controlled indirectly.  TheRegister* calls map the
        // 'trigger' to the function specified by the *-suffix.  If
        // trigger >= 0, the function is added to a map.  If trigger < 0 in
        // the Register* function, the corresponding function is removed from
        // the map.  A call to PushMotion(trigger) will set the active motion
        // if the trigger is currently mapped; the Boolean return is 'true'
        // iff the trigger is mapped.  A call to PopMotion(trigger) will
        // disable the motion if the trigger is currently mapped; the Boolean
        // return it 'true' iff the trigger is currently mapped.
        inline void RegisterMoveForward(int trigger)
        {
            Register(trigger, &CameraRig::MoveForward);
        }

        inline void RegisterMoveBackward(int trigger)
        {
            Register(trigger, &CameraRig::MoveBackward);
        }

        inline void RegisterMoveUp(int trigger)
        {
            Register(trigger, &CameraRig::MoveUp);
        }

        inline void RegisterMoveDown(int trigger)
        {
            Register(trigger, &CameraRig::MoveDown);
        }

        inline void RegisterMoveRight(int trigger)
        {
            Register(trigger, &CameraRig::MoveRight);
        }

        inline void RegisterMoveLeft(int trigger)
        {
            Register(trigger, &CameraRig::MoveLeft);
        }

        inline void RegisterTurnRight(int trigger)
        {
            Register(trigger, &CameraRig::TurnRight);
        }

        inline void RegisterTurnLeft(int trigger)
        {
            Register(trigger, &CameraRig::TurnLeft);
        }

        inline void RegisterLookUp(int trigger)
        {
            Register(trigger, &CameraRig::LookUp);
        }

        inline void RegisterLookDown(int trigger)
        {
            Register(trigger, &CameraRig::LookDown);
        }

        inline void RegisterRollClockwise(int trigger)
        {
            Register(trigger, &CameraRig::RollClockwise);
        }

        inline void RegisterRollCounterclockwise(int trigger)
        {
            Register(trigger, &CameraRig::RollCounterclockwise);
        }

        bool PushMotion(int trigger);
        bool PopMotion(int trigger);

        bool Move();
        void ClearMotions();

    protected:
        // Camera motion.  These are called based on the state set in the
        // public interface.
        enum { MAX_ACTIVE_MOTIONS = 12 };
        virtual void MoveForward();
        virtual void MoveBackward();
        virtual void MoveUp();
        virtual void MoveDown();
        virtual void MoveRight();
        virtual void MoveLeft();
        virtual void TurnRight();
        virtual void TurnLeft();
        virtual void LookUp();
        virtual void LookDown();
        virtual void RollClockwise();
        virtual void RollCounterclockwise();

        typedef void(CameraRig::* MoveFunction)(void);
        void Register(int trigger, MoveFunction function);

        bool SetActive(MoveFunction function);
        bool SetInactive(MoveFunction function);

        std::shared_ptr<Camera> mCamera;
        float mTranslationSpeed, mRotationSpeed;

        // Maintain the world coordinate system.
        Vector4<float> mWorldAxis[3];

        // Move via direct or indirect triggers.
        MoveFunction mMotion;
        std::map<int, MoveFunction> mIndirectMap;
        int mNumActiveMotions;
        std::array<MoveFunction, MAX_ACTIVE_MOTIONS> mActiveMotions;
    };
}
