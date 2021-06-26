// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.28

#pragma once

#include <MathematicsGPU/GPUFluid3Parameters.h>
#include <Graphics/GraphicsEngine.h>
#include <Graphics/ProgramFactory.h>
#include <Graphics/ConstantBuffer.h>
#include <Graphics/SamplerState.h>
#include <Graphics/Texture3.h>

namespace gte
{
    class GPUFluid3UpdateState
    {
    public:
        // Construction.
        GPUFluid3UpdateState(std::shared_ptr<ProgramFactory> const& factory,
            int xSize, int ySize, int zSize, int numXThreads, int numYThreads, int numZThreads,
            std::shared_ptr<ConstantBuffer> const& parameters);

        // Member access.  The texels are (velocity.xyz, density).
        inline std::shared_ptr<Texture3> const& GetUpdateState() const
        {
            return mUpdateState;
        }

        // Update the state for the fluid simulation.
        void Execute(std::shared_ptr<GraphicsEngine> const& engine,
            std::shared_ptr<Texture3> const& source,
            std::shared_ptr<Texture3> const& stateTm1,
            std::shared_ptr<Texture3> const& stateT);

    private:
        int mNumXGroups, mNumYGroups, mNumZGroups;
        std::shared_ptr<ComputeProgram> mComputeUpdateState;
        std::shared_ptr<SamplerState> mAdvectionSampler;
        std::shared_ptr<Texture3> mUpdateState;

        // Shader source code as strings.
        static std::string const msGLSLSource;
        static std::string const msHLSLSource;
        static ProgramSources const msSource;
    };
}
