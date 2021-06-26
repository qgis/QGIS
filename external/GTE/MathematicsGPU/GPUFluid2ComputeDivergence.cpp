// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.28

#include <MathematicsGPU/GTMathematicsGPUPCH.h>
#include <MathematicsGPU/GPUFluid2ComputeDivergence.h>
using namespace gte;

GPUFluid2ComputeDivergence::GPUFluid2ComputeDivergence(
    std::shared_ptr<ProgramFactory> const& factory,
    int xSize, int ySize, int numXThreads, int numYThreads,
    std::shared_ptr<ConstantBuffer> const& parameters)
    :
    mNumXGroups(xSize / numXThreads),
    mNumYGroups(ySize / numYThreads)
{
    mDivergence = std::make_shared<Texture2>(DF_R32_FLOAT, xSize, ySize);
    mDivergence->SetUsage(Resource::SHADER_OUTPUT);

    int api = factory->GetAPI();
    factory->PushDefines();
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);
    mComputeDivergence = factory->CreateFromSource(*msSource[api]);
    if (mComputeDivergence)
    {
        auto cshader = mComputeDivergence->GetComputeShader();
        cshader->Set("Parameters", parameters);
        cshader->Set("divergence", mDivergence);
    }

    factory->PopDefines();
}

void GPUFluid2ComputeDivergence::Execute(
    std::shared_ptr<GraphicsEngine> const& engine,
    std::shared_ptr<Texture2> const& state)
{
    mComputeDivergence->GetComputeShader()->Set("state", state);
    engine->Execute(mComputeDivergence, mNumXGroups, mNumYGroups, 1);
}


std::string const GPUFluid2ComputeDivergence::msGLSLSource =
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

    layout(rgba32f) uniform readonly image2D state;
    layout(r32f) uniform writeonly image2D divergence;

    layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
    void main()
    {
        ivec2 c = ivec2(gl_GlobalInvocationID.xy);
        ivec2 dim = imageSize(state);

        int x = int(c.x);
        int y = int(c.y);
        int xm = max(x - 1, 0);
        int xp = min(x + 1, dim.x - 1);
        int ym = max(y - 1, 0);
        int yp = min(y + 1, dim.y - 1);

        vec2 velocityGradient = vec2
        (
            imageLoad(state, ivec2(xp, y)).x - imageLoad(state, ivec2(xm, y)).x,
            imageLoad(state, ivec2(x, yp)).y - imageLoad(state, ivec2(x, ym)).y
        );

        float divergenceValue = dot(halfDivDelta.xy, velocityGradient);
        imageStore(divergence, c, vec4(divergenceValue, 0.0f, 0.0f, 0.0f));
    }
)";

std::string const GPUFluid2ComputeDivergence::msHLSLSource =
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

    Texture2D<float4> state;
    RWTexture2D<float> divergence;

    [numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
    void CSMain(uint2 c : SV_DispatchThreadID)
    {
        uint2 dim;
        state.GetDimensions(dim.x, dim.y);

        int x = int(c.x);
        int y = int(c.y);
        int xm = max(x - 1, 0);
        int xp = min(x + 1, dim.x - 1);
        int ym = max(y - 1, 0);
        int yp = min(y + 1, dim.y - 1);

        float2 velocityGradient = float2(
            state[int2(xp, y)].x - state[int2(xm, y)].x,
            state[int2(x, yp)].y - state[int2(x, ym)].y
        );

        divergence[c] = dot(halfDivDelta.xy, velocityGradient);
    }
)";

ProgramSources const GPUFluid2ComputeDivergence::msSource =
{
    &msGLSLSource,
    &msHLSLSource
};
