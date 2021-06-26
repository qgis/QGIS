// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Vector3.h>
#include <Graphics/Camera.h>
#include <Graphics/Controller.h>
#include <Graphics/Particles.h>

namespace gte
{
    class ParticleController : public Controller
    {
    public:
        // Abstract base class. The object to which this is attached must be
        // from class Particles.
        virtual ~ParticleController() = default;
    protected:
        ParticleController(std::shared_ptr<Camera> const& camera,
            BufferUpdater const& postUpdate);

    public:
        // The system motion, in local coordinates.  The velocity vectors
        // must be unit length.
        float systemLinearSpeed;
        float systemAngularSpeed;
        Vector3<float> systemLinearAxis;
        Vector3<float> systemAngularAxis;
        float systemSizeChange;

        // Particle motion, in the model space of the system.  The velocity
        // vectors should be unit length.
        inline size_t GetNumParticles() const
        {
            return mParticleLinearSpeed.size();
        }

        inline std::vector<float> const& GetPointLinearSpeed() const
        {
            return mParticleLinearSpeed;
        }

        inline std::vector<Vector3<float>> const& GetPointLinearAxis() const
        {
            return mParticleLinearAxis;
        }

        inline std::vector<float> const& GetParticleSizeChange() const
        {
            return mParticleSizeChange;
        }

        inline std::vector<float>& GetParticleSizeChange()
        {
            return mParticleSizeChange;
        }

        inline void SetCamera(std::shared_ptr<Camera> const& camera)
        {
            mCamera = camera;
        }

        inline std::shared_ptr<Camera> const& GetCamera() const
        {
            return mCamera;
        }

        // The animation update.  The application time is in milliseconds.
        virtual bool Update(double applicationTime) override;

    protected:
        // Override the base-class member function in order to verify that
        // the object is Visual with a vertex format and vertex buffer that
        // satisfy the preconditions of the ParticleController.
        virtual void SetObject(ControlledObject* object) override;

        // This class computes the new positions and orientations from the
        // motion parameters.  Derived classes should update the motion
        // parameters and then either call the base class update methods or
        // provide its own update methods for position and orientation.
        virtual void UpdateSystemMotion(float ctrlTime);
        virtual void UpdatePointMotion(float ctrlTime);

        std::vector<float> mParticleLinearSpeed;
        std::vector<Vector3<float>> mParticleLinearAxis;
        std::vector<float> mParticleSizeChange;

        std::shared_ptr<Camera> mCamera;
        BufferUpdater mPostUpdate;
    };
}
