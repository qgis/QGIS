// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.28

#pragma once

#include <MathematicsGPU/GPUFluid3AdjustVelocity.h>
#include <MathematicsGPU/GPUFluid3ComputeDivergence.h>
#include <MathematicsGPU/GPUFluid3EnforceStateBoundary.h>
#include <MathematicsGPU/GPUFluid3InitializeSource.h>
#include <MathematicsGPU/GPUFluid3InitializeState.h>
#include <MathematicsGPU/GPUFluid3SolvePoisson.h>
#include <MathematicsGPU/GPUFluid3UpdateState.h>

namespace gte
{
    class GPUFluid3
    {
    public:
        // Construction.  The (x,y,z) grid covers [0,1]^3.
        GPUFluid3(std::shared_ptr<GraphicsEngine> const& engine,
            std::shared_ptr<ProgramFactory> const& factory,
            int xSize, int ySize, int zSize, float dt);

        void Initialize();
        void DoSimulationStep();

        inline std::shared_ptr<Texture3> const& GetState() const
        {
            return mStateTTexture;
        }

    private:
        // Constructor inputs.
        std::shared_ptr<GraphicsEngine> mEngine;
        int mXSize, mYSize, mZSize;
        float mDt;

        // Current simulation time.
        float mTime;

        std::shared_ptr<ConstantBuffer> mParameters;
        std::shared_ptr<GPUFluid3InitializeSource> mInitializeSource;
        std::shared_ptr<GPUFluid3InitializeState> mInitializeState;
        std::shared_ptr<GPUFluid3EnforceStateBoundary> mEnforceStateBoundary;
        std::shared_ptr<GPUFluid3UpdateState> mUpdateState;
        std::shared_ptr<GPUFluid3ComputeDivergence> mComputeDivergence;
        std::shared_ptr<GPUFluid3SolvePoisson> mSolvePoisson;
        std::shared_ptr<GPUFluid3AdjustVelocity> mAdjustVelocity;

        std::shared_ptr<Texture3> mSourceTexture;
        std::shared_ptr<Texture3> mStateTm1Texture;
        std::shared_ptr<Texture3> mStateTTexture;
        std::shared_ptr<Texture3> mStateTp1Texture;
        std::shared_ptr<Texture3> mDivergenceTexture;
        std::shared_ptr<Texture3> mPoissonTexture;
    };
}
