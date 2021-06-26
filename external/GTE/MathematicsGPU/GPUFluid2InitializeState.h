// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.28

#pragma once

#include <Graphics/GraphicsEngine.h>
#include <Graphics/ProgramFactory.h>
#include <Graphics/ConstantBuffer.h>
#include <Graphics/Texture2.h>

namespace gte
{
    class GPUFluid2InitializeState
    {
    public:
        // Construction.  The initial velocity is zero and the initial density
        // is randomly generated with values in [0,1].
        GPUFluid2InitializeState(std::shared_ptr<ProgramFactory> const& factory,
            int xSize, int ySize, int numXThreads, int numYThreads);

        // Member access. The texels are (velocity.x, velocity.y, 0, density).
        // The third component is unused in the simulation; a 3D simulation
        // will store velocity.z in this component.
        inline std::shared_ptr<Texture2> const& GetStateTm1() const
        {
            return mStateTm1;
        }

        inline std::shared_ptr<Texture2> const& GetStateT() const
        {
            return mStateT;
        }

        // Compute the initial density and initial velocity for the fluid
        // simulation.
        void Execute(std::shared_ptr<GraphicsEngine> const& engine);

    private:
        int mNumXGroups, mNumYGroups;
        std::shared_ptr<ComputeProgram> mInitializeState;
        std::shared_ptr<Texture2> mDensity;
        std::shared_ptr<Texture2> mVelocity;
        std::shared_ptr<Texture2> mStateTm1;
        std::shared_ptr<Texture2> mStateT;

        // Shader source code as strings.
        static std::string const msGLSLSource;
        static std::string const msHLSLSource;
        static ProgramSources const msSource;
    };
}
