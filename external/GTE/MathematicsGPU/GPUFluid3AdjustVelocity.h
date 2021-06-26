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
#include <Graphics/Texture3.h>

namespace gte
{
    class GPUFluid3AdjustVelocity
    {
    public:
        // Construction.  Adjust the velocities using the solution to the
        // Poisson equation.
        GPUFluid3AdjustVelocity(std::shared_ptr<ProgramFactory> const& factory,
            int xSize, int ySize, int zSize, int numXThreads, int numYThreads, int numZThreads,
            std::shared_ptr<ConstantBuffer> const& parameters);

        // Update the state for the fluid simulation.
        void Execute(std::shared_ptr<GraphicsEngine> const& engine,
            std::shared_ptr<Texture3> const& inState,
            std::shared_ptr<Texture3> const& poisson,
            std::shared_ptr<Texture3> const& outState);

    private:
        int mNumXGroups, mNumYGroups, mNumZGroups;
        std::shared_ptr<ComputeProgram> mAdjustVelocity;

        // Shader source code as strings.
        static std::string const msGLSLSource;
        static std::string const msHLSLSource;
        static ProgramSources const msSource;
    };
}
