// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Spatial.h>

namespace gte
{
    class IKController : public Controller
    {
    public:
        // Construction and destruction.  The joint and goal information must
        // be initialized after construction by the member functions in the
        // section "Deferred construction."
        virtual ~IKController() = default;
        IKController(size_t numJoints, size_t numGoals, size_t numIterations, bool orderEndToRoot);

        // Deferred construction.
        void InitializeGoal(size_t g, std::shared_ptr<Spatial> const& target,
            std::shared_ptr<Spatial> const& effector, float weight);

        void InitializeJoint(size_t j, std::shared_ptr<Spatial> const& object,
            std::vector<size_t> const& goalIndices);

        void SetJointAllowTranslation(size_t j, int axis, bool allow);
        void SetJointMinTranslation(size_t j, int axis, float minTranslation);
        void SetJointMaxTranslation(size_t j, int axis, float maxTranslation);
        bool GetJointAllowTranslation(size_t j, int axis) const;
        float GetJointMinTranslation(size_t j, int axis) const;
        float GetJointMaxTranslation(size_t j, int axis) const;

        void SetJointAllowRotation(size_t j, int axis, bool allow);
        void SetJointMinRotation(size_t j, int axis, float minRotation);
        void SetJointMaxRotation(size_t j, int axis, float maxRotation);
        bool GetJointAllowRotation(size_t j, int axis) const;
        float GetJointMinRotation(size_t j, int axis) const;
        float GetJointMaxRotation(size_t j, int axis) const;

        // The animation update.  The application time is in milliseconds.
        virtual bool Update(double applicationTime) override;

    protected:
        struct Goal
        {
            Goal();

            inline Vector3<float> GetTargetPosition() const
            {
                return target->worldTransform.GetTranslation();
            }

            inline Vector3<float> GetEffectorPosition() const
            {
                return effector->worldTransform.GetTranslation();
            }

            Spatial* target;
            Spatial* effector;
            float weight;
        };

        struct Joint
        {
            Joint();

            // No range checking is performed in the functions with input
            // 'int axis' because the IKController wrapper ensures the
            // correct input.
            void UpdateWorldSRT();
            void UpdateWorldRT();
            Vector3<float> GetAxis(int axis);
            bool UpdateLocalT(int axis, std::vector<Goal> const& goals);
            bool UpdateLocalR(int axis, std::vector<Goal> const& goals);

            Spatial* object;
            std::vector<size_t> goalIndices;

            // Index i is for the joint's parent's world axis[i].
            std::array<bool, 3> allowTranslation;  // default = false
            std::array<float, 3> minTranslation;   // default = -infinity
            std::array<float, 3> maxTranslation;   // default = +infinity
            std::array<bool, 3> allowRotation;     // default = false
            std::array<float, 3> minRotation;      // default = -infinity
            std::array<float, 3> maxRotation;      // default = +infinity
        };

        std::vector<Joint> mJoints;
        std::vector<Goal> mGoals;
        size_t mNumIterations;
        bool mOrderEndToRoot;
    };
}
