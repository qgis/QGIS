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
    class GPUFluid3InitializeSource
    {
    public:
        // Construction.  The source density consists of a density producer
        // (increase density) and a density consumer (decrease density).  Each
        // has a location, a variance from that location, and an amplitude.
        // The source velocity (impulse) is generated from gravity, a single
        // wind source, and randomly generated vortices.  Each vortex is
        // selected with a random location, a variance from that location and
        // an amplitude for the impulse.
        GPUFluid3InitializeSource(std::shared_ptr<ProgramFactory> const& factory,
            int xSize, int ySize, int zSize, int numXThreads, int numYThreads, int numZThreads,
            std::shared_ptr<ConstantBuffer> const& parameters);

        // Member access.  The texels are (velocity.xyz, density).
        inline std::shared_ptr<Texture3> const& GetSource() const
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
            Vector4<float> position;
            Vector4<float> normal;
            Vector4<float> data;
        };

        struct External
        {
            Vector4<float> densityProducer;  // (x, y, z, *)
            Vector4<float> densityPData;     // (variance, amplitude, *, *)
            Vector4<float> densityConsumer;  // (x, y, z, *)
            Vector4<float> densityCData;     // (variance, amplitude, *, *)
            Vector4<float> gravity;
            Vector4<float> windData;
        };

        int mNumXGroups, mNumYGroups, mNumZGroups;
        std::shared_ptr<ComputeProgram> mGenerateVortex;
        std::shared_ptr<ComputeProgram> mInitializeSource;
        std::shared_ptr<ConstantBuffer> mVortex;
        std::shared_ptr<ConstantBuffer> mExternal;
        std::shared_ptr<Texture3> mVelocity0;
        std::shared_ptr<Texture3> mVelocity1;
        std::shared_ptr<Texture3> mSource;

        // Shader source code as strings.
        static std::string const msGLSLGenerateVortexSource;
        static std::string const msGLSLInitializeSourceSource;
        static std::string const msHLSLGenerateVortexSource;
        static std::string const msHLSLInitializeSourceSource;
        static ProgramSources const msGenerateVortexSource;
        static ProgramSources const msInitializeSourceSource;
    };
}
