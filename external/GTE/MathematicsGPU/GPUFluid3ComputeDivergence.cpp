// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.28

#include <MathematicsGPU/GTMathematicsGPUPCH.h>
#include <MathematicsGPU/GPUFluid3ComputeDivergence.h>
using namespace gte;

GPUFluid3ComputeDivergence::GPUFluid3ComputeDivergence(
    std::shared_ptr<ProgramFactory> const& factory,
    int xSize, int ySize, int zSize, int numXThreads, int numYThreads,
    int numZThreads, std::shared_ptr<ConstantBuffer> const& parameters)
    :
    mNumXGroups(xSize / numXThreads),
    mNumYGroups(ySize / numYThreads),
    mNumZGroups(zSize / numZThreads)
{
    mDivergence = std::make_shared<Texture3>(DF_R32_FLOAT, xSize, ySize, zSize);
    mDivergence->SetUsage(Resource::SHADER_OUTPUT);

    int api = factory->GetAPI();
    factory->PushDefines();
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);
    factory->defines.Set("NUM_Z_THREADS", numZThreads);
    mComputeDivergence = factory->CreateFromSource(*msSource[api]);
    if (mComputeDivergence)
    {
        auto cshader = mComputeDivergence->GetComputeShader();
        cshader->Set("Parameters", parameters);
        cshader->Set("divergence", mDivergence);
    }

    factory->PopDefines();
}

void GPUFluid3ComputeDivergence::Execute(
    std::shared_ptr<GraphicsEngine> const& engine,
    std::shared_ptr<Texture3> const& state)
{
    mComputeDivergence->GetComputeShader()->Set("state", state);
    engine->Execute(mComputeDivergence, mNumXGroups, mNumYGroups, mNumZGroups);
}


std::string const GPUFluid3ComputeDivergence::msGLSLSource =
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

    layout(rgba32f) uniform readonly image3D state;
    layout(r32f) uniform writeonly image3D divergence;

    layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = NUM_Z_THREADS) in;
    void main()
    {
        ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
        ivec3 dim = imageSize(state);

        int x = int(c.x);
        int y = int(c.y);
        int z = int(c.z);
        int xm = max(x - 1, 0);
        int xp = min(x + 1, dim.x - 1);
        int ym = max(y - 1, 0);
        int yp = min(y + 1, dim.y - 1);
        int zm = max(z - 1, 0);
        int zp = min(z + 1, dim.z - 1);

        vec3 velocityGradient = vec3
        (
            imageLoad(state, ivec3(xp, y, z)).x - imageLoad(state, ivec3(xm, y, z)).x,
            imageLoad(state, ivec3(x, yp, z)).y - imageLoad(state, ivec3(x, ym, z)).y,
            imageLoad(state, ivec3(x, y, zp)).z - imageLoad(state, ivec3(x, y, zm)).z
        );

        float divergenceValue = dot(halfDivDelta.xyz, velocityGradient);
        imageStore(divergence, c, vec4(divergenceValue, 0.0f, 0.0f, 0.0f));
    }
)";

std::string const GPUFluid3ComputeDivergence::msHLSLSource =
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

    Texture3D<float4> state;
    RWTexture3D<float> divergence;

    [numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
    void CSMain(uint3 c : SV_DispatchThreadID)
    {
        uint3 dim;
        state.GetDimensions(dim.x, dim.y, dim.z);

        int x = int(c.x);
        int y = int(c.y);
        int z = int(c.z);
        int xm = max(x - 1, 0);
        int xp = min(x + 1, dim.x - 1);
        int ym = max(y - 1, 0);
        int yp = min(y + 1, dim.y - 1);
        int zm = max(z - 1, 0);
        int zp = min(z + 1, dim.z - 1);

        float3 velocityGradient = float3
        (
            state[int3(xp, y, z)].x - state[int3(xm, y, z)].x,
            state[int3(x, yp, z)].y - state[int3(x, ym, z)].y,
            state[int3(x, y, zp)].z - state[int3(x, y, zm)].z
        );

        divergence[c] = dot(halfDivDelta.xyz, velocityGradient);
    }
)";

ProgramSources const GPUFluid3ComputeDivergence::msSource =
{
    &msGLSLSource,
    &msHLSLSource
};
