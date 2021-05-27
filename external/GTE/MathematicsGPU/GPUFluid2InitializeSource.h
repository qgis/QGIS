// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.28

#pragma once

#include <MathematicsGPU/GPUFluid2Parameters.h>
#include <Graphics/GraphicsEngine.h>
#include <Graphics/ProgramFactory.h>
#include <Graphics/ConstantBuffer.h>
#include <Graphics/Texture2.h>

namespace gte
{
    class GPUFluid2InitializeSource
    {
    public:
        // Construction.  The source density consists of a density producer
        // (increase density) and a density consumer (decrease density).  Each
        // has a location, a variance from that location, and an amplitude.
        // The source velocity (impulse) is generated from gravity, a single
        // wind source, and randomly generated vortices.  Each vortex is
        // selected with a random location, a variance from that location and
        // an amplitude for the impulse.
        GPUFluid2InitializeSource(std::shared_ptr<ProgramFactory> const& factory,
            int xSize, int ySize, int numXThreads, int numYThreads,
            std::shared_ptr<ConstantBuffer> const& parameters);

        // Member access. The texels are (velocity.x, velocity.y, 0, density).
        // The third component is unused in the simulation; a 3D simulation
        // will store velocity.z in this component.
        inline std::shared_ptr<Texture2> const& GetSource() const
        {
            return mSource;
        }

        // Compute the source density and source velocity for the fluid
        // simulation.
        void Execute(std::shared_ptr<GraphicsEngine> const& engine);

    private:
        enum { NUM_VORTICES = 1024 };

        struct Vortex
        {
            Vector4<float> data;
        };

        struct External
        {
            Vector4<float> densityProducer;
            Vector4<float> densityConsumer;
            Vector4<float> gravity;
            Vector4<float> wind;
        };

        int mNumXGroups, mNumYGroups;
        std::shared_ptr<ComputeProgram> mGenerateVortex;
        std::shared_ptr<ComputeProgram> mInitializeSource;
        std::shared_ptr<ConstantBuffer> mVortex;
        std::shared_ptr<ConstantBuffer> mExternal;
        std::shared_ptr<Texture2> mVelocity0;
        std::shared_ptr<Texture2> mVelocity1;
        std::shared_ptr<Texture2> mSource;

        // Shader source code as strings.
        static std::string const msGLSLGenerateVortexSource;
        static std::string const msGLSLInitializeSourceSource;
        static std::string const msHLSLGenerateVortexSource;
        static std::string const msHLSLInitializeSourceSource;
        static ProgramSources const msGenerateVortexSource;
        static ProgramSources const msInitializeSourceSource;
    };
}
