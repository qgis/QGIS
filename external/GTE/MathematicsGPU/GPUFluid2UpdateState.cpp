// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.28

#include <MathematicsGPU/GTMathematicsGPUPCH.h>
#include <MathematicsGPU/GPUFluid2UpdateState.h>
using namespace gte;

GPUFluid2UpdateState::GPUFluid2UpdateState(
    std::shared_ptr<ProgramFactory> const& factory,
    int xSize, int ySize, int numXThreads, int numYThreads,
    std::shared_ptr<ConstantBuffer> const& parameters)
    :
    mNumXGroups(xSize / numXThreads),
    mNumYGroups(ySize / numYThreads)
{
    mUpdateState = std::make_shared<Texture2>(DF_R32G32B32A32_FLOAT, xSize, ySize);
    mUpdateState->SetUsage(Resource::SHADER_OUTPUT);

    mAdvectionSampler = std::make_shared<SamplerState>();
    mAdvectionSampler->filter = SamplerState::MIN_L_MAG_L_MIP_P;
    mAdvectionSampler->mode[0] = SamplerState::CLAMP;
    mAdvectionSampler->mode[1] = SamplerState::CLAMP;

    // Create the shader for generating velocity from vortices.
    int api = factory->GetAPI();
    factory->PushDefines();
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);

    mComputeUpdateState = factory->CreateFromSource(*msSource[api]);
    if (mComputeUpdateState)
    {
        auto cshader =  mComputeUpdateState->GetComputeShader();
        cshader->Set("Parameters", parameters);
        cshader->Set("updateState", mUpdateState);
    }

    factory->PopDefines();
}

void GPUFluid2UpdateState::Execute(
    std::shared_ptr<GraphicsEngine> const& engine,
    std::shared_ptr<Texture2> const& source,
    std::shared_ptr<Texture2> const& stateTm1,
    std::shared_ptr<Texture2> const& stateT)
{
    auto cshader = mComputeUpdateState->GetComputeShader();
    cshader->Set("source", source);
    cshader->Set("stateTm1", stateTm1, "advectionSampler", mAdvectionSampler);
    cshader->Set("stateT", stateT);
    engine->Execute(mComputeUpdateState, mNumXGroups, mNumYGroups, 1);
}


std::string const GPUFluid2UpdateState::msGLSLSource =
R"(
    uniform Parameters
    {
        vec4 spaceDelta;    // (dx, dy, 0, 0)
        vec4 halfDivDelta;  // (0.5/dx, 0.5/dy, 0, 0)
        vec4 timeDelta;     // (dt/dx, dt/dy, 0, dt)
        vec4 viscosityX;    // (velVX, velVX, 0, denVX)
        vec4 viscosityY;    // (velVX, velVY, 0, denVY)
        vec4 epsilon;       // (epsilonX, epsilonY, 0, epsilon0)
    };

    layout(rgba32f) uniform readonly image2D source;
    layout(rgba32f) uniform readonly image2D stateT;
    uniform sampler2D advectionSampler;
    layout(rgba32f) uniform writeonly image2D updateState;

    layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
    void main()
    {
        ivec2 c = ivec2(gl_GlobalInvocationID.xy);
        ivec2 dim = imageSize(stateT);

        int x = int(c.x);
        int y = int(c.y);
        int xm = max(x - 1, 0);
        int xp = min(x + 1, dim.x - 1);
        int ym = max(y - 1, 0);
        int yp = min(y + 1, dim.y - 1);

        // Sample states at (x,y), (x+dx,y), (x-dx,y), (x,y+dy), (x,y-dy).
        vec4 stateZZ = imageLoad(stateT, c);
        vec4 statePZ = imageLoad(stateT, ivec2(xp, y));
        vec4 stateMZ = imageLoad(stateT, ivec2(xm, y));
        vec4 stateZP = imageLoad(stateT, ivec2(x, yp));
        vec4 stateZM = imageLoad(stateT, ivec2(x, ym));

        // Sample the source state at (x,y).
        vec4 src = imageLoad(source, c);

        // Estimate second-order derivatives of state at (x,y).
        vec4 stateDXX = statePZ - 2.0f * stateZZ + stateMZ;
        vec4 stateDYY = stateZP - 2.0f * stateZZ + stateZM;

        // Compute advection.
        vec2 tcd = spaceDelta.xy * (c.xy - timeDelta.xy * stateZZ.xy + 0.5f);
        vec4 advection = textureLod(advectionSampler, tcd, 0.0f);

        // Update the state.
        imageStore(updateState, c, advection +
            (viscosityX * stateDXX + viscosityY * stateDYY + timeDelta.w * src));
    }
)";

std::string const GPUFluid2UpdateState::msHLSLSource =
R"(
    cbuffer Parameters
    {
        float4 spaceDelta;    // (dx, dy, 0, 0)
        float4 halfDivDelta;  // (0.5/dx, 0.5/dy, 0, 0)
        float4 timeDelta;     // (dt/dx, dt/dy, 0, dt)
        float4 viscosityX;    // (velVX, velVX, 0, denVX)
        float4 viscosityY;    // (velVX, velVY, 0, denVY)
        float4 epsilon;       // (epsilonX, epsilonY, 0, epsilon0)
    };

    Texture2D<float4> source;
    Texture2D<float4> stateTm1;
    Texture2D<float4> stateT;
    SamplerState advectionSampler;  // bilinear, clamp
    RWTexture2D<float4> updateState;

    [numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
    void CSMain(uint2 c : SV_DispatchThreadID)
    {
        uint2 dim;
        stateT.GetDimensions(dim.x, dim.y);

        int x = int(c.x);
        int y = int(c.y);
        int xm = max(x - 1, 0);
        int xp = min(x + 1, dim.x - 1);
        int ym = max(y - 1, 0);
        int yp = min(y + 1, dim.y - 1);

        // Sample states at (x,y), (x+dx,y), (x-dx,y), (x,y+dy), (x,y-dy).
        float4 stateZZ = stateT[int2(x, y)];
        float4 statePZ = stateT[int2(xp, y)];
        float4 stateMZ = stateT[int2(xm, y)];
        float4 stateZP = stateT[int2(x, yp)];
        float4 stateZM = stateT[int2(x, ym)];

        // Sample the source state at (x,y).
        float4 src = source[int2(x, y)];

        // Estimate second-order derivatives of state at (x,y).
        float4 stateDXX = statePZ - 2.0f * stateZZ + stateMZ;
        float4 stateDYY = stateZP - 2.0f * stateZZ + stateZM;

        // Compute advection.
        float2 tcd = spaceDelta.xy * (c - timeDelta.xy * stateZZ.xy + 0.5f);
        float4 advection = stateTm1.SampleLevel(advectionSampler, tcd, 0.0f);

        // Update the state.
        updateState[c] = advection +
            (viscosityX * stateDXX + viscosityY * stateDYY + timeDelta.w * src);
    }
)";

ProgramSources const GPUFluid2UpdateState::msSource =
{
    &msGLSLSource,
    &msHLSLSource
};
