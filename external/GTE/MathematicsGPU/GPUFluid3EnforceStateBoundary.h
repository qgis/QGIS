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
#include <Graphics/Texture3.h>

namespace gte
{
    class GPUFluid3EnforceStateBoundary
    {
    public:
        // Construction.  Clamp the velocity vectors at the image boundary to
        // ensure they do not point outside the domain.  For example, let
        // image(x,y,z) have velocity (vx,vy,vz).  At the boundary pixel
        // (x,y,0), the velocity is clamped to (vx,vy,0).  The density is
        // set to zero on the image boundary.
        GPUFluid3EnforceStateBoundary(std::shared_ptr<ProgramFactory> const& factory,
            int xSize, int ySize, int zSize, int numXThreads, int numYThreads, int numZThreads);

        // Set the density and velocity values at the image boundary as
        // described in the comments for the constructor.  The state texture
        // has texels (velocity.x, velocity.y, velocity.z, density).
        void Execute(std::shared_ptr<GraphicsEngine> const& engine,
            std::shared_ptr<Texture3> const& state);

    private:
        int mNumXGroups, mNumYGroups, mNumZGroups;
        std::shared_ptr<ComputeProgram> mCopyXFace;
        std::shared_ptr<ComputeProgram> mWriteXFace;
        std::shared_ptr<ComputeProgram> mCopyYFace;
        std::shared_ptr<ComputeProgram> mWriteYFace;
        std::shared_ptr<ComputeProgram> mCopyZFace;
        std::shared_ptr<ComputeProgram> mWriteZFace;
        std::shared_ptr<Texture2> mXMin;
        std::shared_ptr<Texture2> mXMax;
        std::shared_ptr<Texture2> mYMin;
        std::shared_ptr<Texture2> mYMax;
        std::shared_ptr<Texture2> mZMin;
        std::shared_ptr<Texture2> mZMax;

        // Shader source code as strings.
        static std::string const msGLSLSource;
        static std::string const msHLSLSource;
        static ProgramSources const msSource;
    };
}
