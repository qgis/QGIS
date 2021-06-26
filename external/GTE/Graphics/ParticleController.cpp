// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/ParticleController.h>
#include <Mathematics/Logger.h>
using namespace gte;

ParticleController::ParticleController(std::shared_ptr<Camera> const& camera,
    BufferUpdater const& postUpdate)
    :
    systemLinearSpeed(0.0f),
    systemAngularSpeed(0.0f),
    systemLinearAxis(Vector3<float>::Unit(2)),
    systemAngularAxis(Vector3<float>::Unit(2)),
    systemSizeChange(0.0f),
    mCamera(camera),
    mPostUpdate(postUpdate)
{
}

bool ParticleController::Update(double applicationTime)
{
    if (!Controller::Update(applicationTime))
    {
        return false;
    }

    float ctrlTime = static_cast<float>(GetControlTime(applicationTime));

    UpdateSystemMotion(ctrlTime);
    UpdatePointMotion(ctrlTime);
    return true;
}

void ParticleController::SetObject(ControlledObject* object)
{
    mParticleLinearSpeed.clear();
    mParticleLinearAxis.clear();
    mParticleSizeChange.clear();

    // We need only validate that the object is of type Particles.  The
    // Particles constructor already does its own validation of the vertex
    // format.
    auto visual = dynamic_cast<Particles*>(object);
    LogAssert(visual != nullptr, "Object is not of type Particles.");

    auto vbuffer = visual->GetVertexBuffer();
    size_t numParticles = static_cast<size_t>(vbuffer->GetNumElements() / 4);
    mParticleLinearSpeed.resize(numParticles);
    mParticleLinearAxis.resize(numParticles);
    mParticleSizeChange.resize(numParticles);
    for (size_t i = 0; i < numParticles; ++i)
    {
        mParticleLinearSpeed[i] = 0.0f;
        mParticleLinearAxis[i] = Vector3<float>::Unit(2);
        mParticleSizeChange[i] = 0.0f;
    }

    Controller::SetObject(object);
}

void ParticleController::UpdateSystemMotion(float ctrlTime)
{
    auto particles = static_cast<Particles*>(mObject);

    float dSize = ctrlTime * systemSizeChange;
    particles->SetSizeAdjust(particles->GetSizeAdjust() + dSize);
    if (particles->GetSizeAdjust() < 0.0f)
    {
        particles->SetSizeAdjust(0.0f);
    }

    float distance = ctrlTime * systemLinearSpeed;
    Vector3<float> currentTrn = particles->localTransform.GetTranslation();
    Vector3<float> deltaTrn = distance * systemLinearAxis;
    particles->localTransform.SetTranslation(currentTrn + deltaTrn);

    float angle = ctrlTime * systemAngularSpeed;
    Matrix3x3<float> currentRot;
    particles->localTransform.GetRotation(currentRot);
    Matrix3x3<float> deltaRot =
        Rotation<3, float>(AxisAngle<3, float>(systemAngularAxis, angle));
    particles->localTransform.SetRotation(deltaRot * currentRot);
}

void ParticleController::UpdatePointMotion(float ctrlTime)
{
    auto particles = static_cast<Particles*>(mObject);
    auto& posSize = particles->GetPositionSize();
    unsigned int numActive = particles->GetNumActive();
    for (unsigned int i = 0; i < numActive; ++i)
    {
        float distance = ctrlTime * mParticleLinearSpeed[i];
        Vector3<float> deltaTrn = distance * mParticleLinearAxis[i];
        posSize[i][0] += deltaTrn[0];
        posSize[i][1] += deltaTrn[1];
        posSize[i][2] += deltaTrn[2];
        float dSize = ctrlTime * mParticleSizeChange[i];
        posSize[i][3] += dSize;
    }

    particles->GenerateParticles(mCamera);
    mPostUpdate(particles->GetVertexBuffer());
}
