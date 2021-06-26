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
    class GPUFluid3SolvePoisson
    {
    public:
        // Construction.  Solve the Poisson equation where numIterations is the
        // number of Gauss-Seidel steps to use in Execute.
        GPUFluid3SolvePoisson(std::shared_ptr<ProgramFactory> const& factory,
            int xSize, int ySize, int zSize, int numXThreads, int numYThreads, int numZThreads,
            std::shared_ptr<ConstantBuffer> const& parameters, int numIterations);

        // Member access.  The texels are (velocity.xyz, density).
        inline std::shared_ptr<gte::Texture3> const& GetPoisson() const
        {
            return mPoisson0;
        }

        // Update the state for the fluid simulation.
        void Execute(std::shared_ptr<GraphicsEngine> const& engine,
            std::shared_ptr<Texture3> const& divergence);

    private:
        int mNumXGroups, mNumYGroups, mNumZGroups;
        std::shared_ptr<ComputeProgram> mZeroPoisson;
        std::shared_ptr<ComputeProgram> mSolvePoisson;
        std::shared_ptr<ComputeProgram> mWriteXFace;
        std::shared_ptr<ComputeProgram> mWriteYFace;
        std::shared_ptr<ComputeProgram> mWriteZFace;
        std::shared_ptr<Texture3> mPoisson0;
        std::shared_ptr<Texture3> mPoisson1;
        int mNumIterations;

        // Shader source code as strings.
        static std::string const msGLSLPoissonZeroSource;
        static std::string const msGLSLPoissonSolveSource;
        static std::string const msGLSLPoissonEnforceBoundarySource;
        static std::string const msHLSLPoissonZeroSource;
        static std::string const msHLSLPoissonSolveSource;
        static std::string const msHLSLPoissonEnforceBoundarySource;
        static ProgramSources const msPoissonZeroSource;
        static ProgramSources const msPoissonSolveSource;
        static ProgramSources const msPoissonEnforceBoundarySource;
    };
}

