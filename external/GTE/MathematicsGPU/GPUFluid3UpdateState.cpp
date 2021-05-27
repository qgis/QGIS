// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.28

#include <MathematicsGPU/GTMathematicsGPUPCH.h>
#include <MathematicsGPU/GPUFluid3UpdateState.h>
using namespace gte;

GPUFluid3UpdateState::GPUFluid3UpdateState(
    std::shared_ptr<ProgramFactory> const& factory,
    int xSize, int ySize, int zSize, int numXThreads, int numYThreads, int numZThreads,
    std::shared_ptr<ConstantBuffer> const& parameters)
    :
    mNumXGroups(xSize / numXThreads),
    mNumYGroups(ySize / numYThreads),
    mNumZGroups(zSize / numZThreads)
{
    mUpdateState = std::make_shared<Texture3>(DF_R32G32B32A32_FLOAT, xSize, ySize, zSize);
    mUpdateState->SetUsage(Resource::SHADER_OUTPUT);

    mAdvectionSampler = std::make_shared<SamplerState>();
    mAdvectionSampler->filter = SamplerState::MIN_L_MAG_L_MIP_P;
    mAdvectionSampler->mode[0] = SamplerState::CLAMP;
    mAdvectionSampler->mode[1] = SamplerState::CLAMP;
    mAdvectionSampler->mode[2] = SamplerState::CLAMP;

    // Create the shader for generating velocity from vortices.
    int api = factory->GetAPI();
    factory->PushDefines();
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);
    factory->defines.Set("NUM_Z_THREADS", numZThreads);

    mComputeUpdateState = factory->CreateFromSource(*msSource[api]);
    if (mComputeUpdateState)
    {
        auto cshader = mComputeUpdateState->GetComputeShader();
        cshader->Set("Parameters", parameters);
        cshader->Set("updateState", mUpdateState);
    }

    factory->PopDefines();
}

void GPUFluid3UpdateState::Execute(
    std::shared_ptr<GraphicsEngine> const& engine,
    std::shared_ptr<Texture3> const& source,
    std::shared_ptr<Texture3> const& stateTm1,
    std::shared_ptr<Texture3> const& stateT)
{
    auto cshader = mComputeUpdateState->GetComputeShader();
    cshader->Set("source", source);
    cshader->Set("stateTm1", stateTm1, "advectionSampler", mAdvectionSampler);
    cshader->Set("stateT", stateT);
    engine->Execute(mComputeUpdateState, mNumXGroups, mNumYGroups, mNumZGroups);
}


std::string const GPUFluid3UpdateState::msGLSLSource =
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

    layout(rgba32f) uniform readonly image3D source;
    layout(rgba32f) uniform readonly image3D stateT;
    uniform sampler3D advectionSampler;
    layout(rgba32f) uniform writeonly image3D updateState;

    layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = NUM_Z_THREADS) in;
    void main()
    {
        ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
        ivec3 dim = imageSize(stateT);

        int x = int(c.x);
        int y = int(c.y);
        int z = int(c.z);
        int xm = max(x - 1, 0);
        int xp = min(x + 1, dim.x - 1);
        int ym = max(y - 1, 0);
        int yp = min(y + 1, dim.y - 1);
        int zm = max(z - 1, 0);
        int zp = min(z + 1, dim.z - 1);

        // Sample states at (x,y,z) and immediate neighbors.
        vec4 stateZZZ = imageLoad(stateT, c);
        vec4 statePZZ = imageLoad(stateT, ivec3(xp, y, z));
        vec4 stateMZZ = imageLoad(stateT, ivec3(xm, y, z));
        vec4 stateZPZ = imageLoad(stateT, ivec3(x, yp, z));
        vec4 stateZMZ = imageLoad(stateT, ivec3(x, ym, z));
        vec4 stateZZP = imageLoad(stateT, ivec3(x, y, zp));
        vec4 stateZZM = imageLoad(stateT, ivec3(x, y, zm));

        // Sample the source state at (x,y,z).
        vec4 src = imageLoad(source, c);

        // Estimate second-order derivatives of state at (x,y,z).
        vec4 stateDXX = statePZZ - 2.0f * stateZZZ + stateMZZ;
        vec4 stateDYY = stateZPZ - 2.0f * stateZZZ + stateZMZ;
        vec4 stateDZZ = stateZZP - 2.0f * stateZZZ + stateZZM;

        // Compute advection.
        vec3 tcd = spaceDelta.xyz * (c - timeDelta.xyz * stateZZZ.xyz + 0.5f);
        vec4 advection = textureLod(advectionSampler, tcd, 0.0f);

        // Update the state.
        imageStore(updateState, c, advection +
            (viscosityX * stateDXX + viscosityY * stateDYY + viscosityZ * stateDZZ +
            timeDelta.w*src));
    }
)";

std::string const GPUFluid3UpdateState::msHLSLSource =
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

    Texture3D<float4> source;
    Texture3D<float4> stateTm1;
    Texture3D<float4> stateT;
    SamplerState advectionSampler;  // trilinear, clamp
    RWTexture3D<float4> updateState;

    [numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
    void CSMain(uint3 c : SV_DispatchThreadID)
    {
        uint3 dim;
        stateT.GetDimensions(dim.x, dim.y, dim.z);

        int x = int(c.x);
        int y = int(c.y);
        int z = int(c.z);
        int xm = max(x - 1, 0);
        int xp = min(x + 1, dim.x - 1);
        int ym = max(y - 1, 0);
        int yp = min(y + 1, dim.y - 1);
        int zm = max(z - 1, 0);
        int zp = min(z + 1, dim.z - 1);

        // Sample states at (x,y,z) and immediate neighbors.
        float4 stateZZZ = stateT[c];
        float4 statePZZ = stateT[int3(xp, y, z)];
        float4 stateMZZ = stateT[int3(xm, y, z)];
        float4 stateZPZ = stateT[int3(x, yp, z)];
        float4 stateZMZ = stateT[int3(x, ym, z)];
        float4 stateZZP = stateT[int3(x, y, zp)];
        float4 stateZZM = stateT[int3(x, y, zm)];

        // Sample the source state at (x,y,z).
        float4 src = source[c];

        // Estimate second-order derivatives of state at (x,y,z).
        float4 stateDXX = statePZZ - 2.0f * stateZZZ + stateMZZ;
        float4 stateDYY = stateZPZ - 2.0f * stateZZZ + stateZMZ;
        float4 stateDZZ = stateZZP - 2.0f * stateZZZ + stateZZM;

        // Compute advection.
        float3 tcd = spaceDelta.xyz * (c - timeDelta.xyz*stateZZZ.xyz + 0.5f);
        float4 advection = stateTm1.SampleLevel(advectionSampler, tcd, 0.0f);

        // Update the state.
        updateState[c] = advection +
            (viscosityX * stateDXX + viscosityY * stateDYY + +viscosityZ * stateDZZ +
            timeDelta.w*src);
    }
)";

ProgramSources const GPUFluid3UpdateState::msSource =
{
    &msGLSLSource,
    &msHLSLSource
};
