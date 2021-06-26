// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.28

#include <MathematicsGPU/GTMathematicsGPUPCH.h>
#include <MathematicsGPU/GPUFluid3.h>
using namespace gte;

GPUFluid3::GPUFluid3(std::shared_ptr<GraphicsEngine> const& engine,
    std::shared_ptr<ProgramFactory> const& factory,
    int xSize, int ySize, int zSize, float dt)
    :
    mEngine(engine),
    mXSize(xSize),
    mYSize(ySize),
    mZSize(zSize),
    mDt(dt),
    mTime(0.0f)
{
    // Create the shared parameters for many of the simulation shaders.
    float dx = 1.0f / static_cast<float>(mXSize);
    float dy = 1.0f / static_cast<float>(mYSize);
    float dz = 1.0f / static_cast<float>(mZSize);
    float dtDivDxDx = (dt / dx) / dx;
    float dtDivDyDy = (dt / dy) / dy;
    float dtDivDzDz = (dt / dz) / dz;
    float ratio0 = dx / dy;
    float ratio1 = dx / dz;
    float ratio0Sqr = ratio0 * ratio0;
    float ratio1Sqr = ratio1 * ratio1;
    float factor = 0.5f / (1.0f + ratio0Sqr + ratio1Sqr);
    float epsilonX = factor;
    float epsilonY = ratio0Sqr * factor;
    float epsilonZ = ratio1Sqr * factor;
    float epsilon0 = dx * dx * factor;
    float const denViscosity = 0.0001f;
    float const velViscosity = 0.0001f;
    float denVX = denViscosity * dtDivDxDx;
    float denVY = denViscosity * dtDivDyDy;
    float denVZ = denViscosity * dtDivDzDz;
    float velVX = velViscosity * dtDivDxDx;
    float velVY = velViscosity * dtDivDyDy;
    float velVZ = velViscosity * dtDivDzDz;

    mParameters = std::make_shared<ConstantBuffer>(sizeof(GPUFluid3Parameters), false);
    GPUFluid3Parameters& p = *mParameters->Get<GPUFluid3Parameters>();
    p.spaceDelta = { dx, dy, dz, 0.0f };
    p.halfDivDelta = { 0.5f / dx, 0.5f / dy, 0.5f / dz, 0.0f };
    p.timeDelta = { dt / dx, dt / dy, dt / dz, dt };
    p.viscosityX = { velVX, velVX, velVX, denVX };
    p.viscosityY = { velVY, velVY, velVY, denVY };
    p.viscosityZ = { velVZ, velVZ, velVZ, denVZ };
    p.epsilon = { epsilonX, epsilonY, epsilonZ, epsilon0 };

    // Create the compute shaders and textures for the simulation.
    mInitializeSource = std::make_shared<GPUFluid3InitializeSource>(factory,
        mXSize, mYSize, mZSize, 8, 8, 8, mParameters);
    mSourceTexture = mInitializeSource->GetSource();

    mInitializeState = std::make_shared<GPUFluid3InitializeState>(factory,
        mXSize, mYSize, mZSize, 8, 8, 8);
    mStateTm1Texture = mInitializeState->GetStateTm1();
    mStateTTexture = mInitializeState->GetStateT();

    mEnforceStateBoundary = std::make_shared<GPUFluid3EnforceStateBoundary>(
        factory, mXSize, mYSize, mZSize, 8, 8, 8);

    mUpdateState = std::make_shared<GPUFluid3UpdateState>(factory, mXSize,
        mYSize, mZSize, 8, 8, 8, mParameters);
    mStateTp1Texture = mUpdateState->GetUpdateState();

    mComputeDivergence = std::make_shared<GPUFluid3ComputeDivergence>(factory,
        mXSize, mYSize, mZSize, 8, 8, 8, mParameters);
    mDivergenceTexture = mComputeDivergence->GetDivergence();

    mSolvePoisson = std::make_shared<GPUFluid3SolvePoisson>(factory, mXSize,
        mYSize, mZSize, 8, 8, 8, mParameters, 32);
    mPoissonTexture = mSolvePoisson->GetPoisson();

    mAdjustVelocity = std::make_shared<GPUFluid3AdjustVelocity>(factory, mXSize,
        mYSize, mZSize, 8, 8, 8, mParameters);
}

void GPUFluid3::Initialize()
{
    mInitializeSource->Execute(mEngine);
    mInitializeState->Execute(mEngine);
    mEnforceStateBoundary->Execute(mEngine, mStateTm1Texture);
    mEnforceStateBoundary->Execute(mEngine, mStateTTexture);
}

void GPUFluid3::DoSimulationStep()
{
    mUpdateState->Execute(mEngine, mSourceTexture, mStateTm1Texture, mStateTTexture);
    mEnforceStateBoundary->Execute(mEngine, mStateTp1Texture);
    mComputeDivergence->Execute(mEngine, mStateTp1Texture);
    mSolvePoisson->Execute(mEngine, mDivergenceTexture);
    mAdjustVelocity->Execute(mEngine, mStateTp1Texture, mPoissonTexture, mStateTm1Texture);
    mEnforceStateBoundary->Execute(mEngine, mStateTm1Texture);
    std::swap(mStateTm1Texture, mStateTTexture);

    mTime += mDt;
}
