// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.28

#include <MathematicsGPU/GTMathematicsGPUPCH.h>
#include <MathematicsGPU/GPUFluid3AdjustVelocity.h>
using namespace gte;

GPUFluid3AdjustVelocity::GPUFluid3AdjustVelocity(std::shared_ptr<ProgramFactory> const& factory,
    int xSize, int ySize, int zSize, int numXThreads, int numYThreads, int numZThreads,
    std::shared_ptr<ConstantBuffer> const& parameters)
    :
    mNumXGroups(xSize / numXThreads),
    mNumYGroups(ySize / numYThreads),
    mNumZGroups(zSize / numZThreads)
{
    int api = factory->GetAPI();
    factory->PushDefines();
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);
    factory->defines.Set("NUM_Z_THREADS", numZThreads);

    mAdjustVelocity = factory->CreateFromSource(*msSource[api]);
    if (mAdjustVelocity)
    {
        mAdjustVelocity->GetComputeShader()->Set("Parameters", parameters);
    }

    factory->PopDefines();
}

void GPUFluid3AdjustVelocity::Execute(std::shared_ptr<GraphicsEngine> const& engine,
    std::shared_ptr<Texture3> const& inState,
    std::shared_ptr<Texture3> const& poisson,
    std::shared_ptr<Texture3> const& outState)
{
    auto cshader = mAdjustVelocity->GetComputeShader();
    cshader->Set("inState", inState);
    cshader->Set("poisson", poisson);
    cshader->Set("outState", outState);
    engine->Execute(mAdjustVelocity, mNumXGroups, mNumYGroups, mNumZGroups);
}


std::string const GPUFluid3AdjustVelocity::msGLSLSource =
R"(
    uniform Parameters
    {
        vec4 spaceDelta;    // (dx, dy, dz, 0)
        vec4 halfDivDelta;  // (0.5/dx, 0.5/dy, 0.5/dz, 0)
        vec4 timeDelta;     // (dt/dx, dt/dy, dt/dz, dt)
        vec4 viscosityX;    // (velVX, velVX, velVX, denVX)
        vec4 viscosityY;    // (velVX, velVY, velVY, denVY)
        vec4 viscosityZ;    // (velVZ, velVZ, velVZ, denVZ)
        vec4 epsilon;       // (epsilonX, epsilonY, epsilonZ, epsilon0)
    };

    layout(rgba32f) uniform readonly image3D inState;
    layout(r32f) uniform readonly image3D poisson;
    layout(rgba32f) uniform writeonly image3D outState;

    layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = NUM_Z_THREADS) in;
    void main()
    {
        ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
        ivec3 dim = imageSize(inState);

        int x = int(c.x);
        int y = int(c.y);
        int z = int(c.z);
        int xm = max(x - 1, 0);
        int xp = min(x + 1, dim.x - 1);
        int ym = max(y - 1, 0);
        int yp = min(y + 1, dim.y - 1);
        int zm = max(z - 1, 0);
        int zp = min(z + 1, dim.z - 1);

        // Sample the state at (x,y,z).
        vec4 state = imageLoad(inState, c);

        // Sample Poisson values at immediate neighbors of (x,y,z).
        float poisPZZ = imageLoad(poisson, ivec3(xp, y, z)).x;
        float poisMZZ = imageLoad(poisson, ivec3(xm, y, z)).x;
        float poisZPZ = imageLoad(poisson, ivec3(x, yp, z)).x;
        float poisZMZ = imageLoad(poisson, ivec3(x, ym, z)).x;
        float poisZZP = imageLoad(poisson, ivec3(x, y, zp)).x;
        float poisZZM = imageLoad(poisson, ivec3(x, y, zm)).x;

        vec4 diff = vec4(poisPZZ - poisMZZ, poisZPZ - poisZMZ, poisZZP - poisZZM, 0.0f);
        imageStore(outState, c, state + halfDivDelta * diff);
    }
)";

std::string const GPUFluid3AdjustVelocity::msHLSLSource =
R"(
    cbuffer Parameters
    {
        float4 spaceDelta;    // (dx, dy, dz, 0)
        float4 halfDivDelta;  // (0.5/dx, 0.5/dy, 0.5/dz, 0)
        float4 timeDelta;     // (dt/dx, dt/dy, dt/dz, dt)
        float4 viscosityX;    // (velVX, velVX, velVX, denVX)
        float4 viscosityY;    // (velVX, velVY, velVY, denVY)
        float4 viscosityZ;    // (velVZ, velVZ, velVZ, denVZ)
        float4 epsilon;       // (epsilonX, epsilonY, epsilonZ, epsilon0)
    };

    Texture3D<float4> inState;
    Texture3D<float> poisson;
    RWTexture3D<float4> outState;

    [numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
    void CSMain(uint3 c : SV_DispatchThreadID)
    {
        uint3 dim;
        inState.GetDimensions(dim.x, dim.y, dim.z);

        int x = int(c.x);
        int y = int(c.y);
        int z = int(c.z);
        int xm = max(x - 1, 0);
        int xp = min(x + 1, dim.x - 1);
        int ym = max(y - 1, 0);
        int yp = min(y + 1, dim.y - 1);
        int zm = max(z - 1, 0);
        int zp = min(z + 1, dim.z - 1);

        // Sample the state at (x,y,z).
        float4 state = inState[c];

        // Sample Poisson values at immediate neighbors of (x,y,z).
        float poisPZZ = poisson[int3(xp, y, z)];
        float poisMZZ = poisson[int3(xm, y, z)];
        float poisZPZ = poisson[int3(x, yp, z)];
        float poisZMZ = poisson[int3(x, ym, z)];
        float poisZZP = poisson[int3(x, y, zp)];
        float poisZZM = poisson[int3(x, y, zm)];

        float4 diff = float4(poisPZZ - poisMZZ, poisZPZ - poisZMZ, poisZZP - poisZZM, 0.0f);
        outState[c] = state + halfDivDelta * diff;
    }
)";

ProgramSources const GPUFluid3AdjustVelocity::msSource =
{
    &msGLSLSource,
    &msHLSLSource
};
