// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.28

#include <MathematicsGPU/GTMathematicsGPUPCH.h>
#include <MathematicsGPU/GPUFluid2AdjustVelocity.h>
using namespace gte;

GPUFluid2AdjustVelocity::GPUFluid2AdjustVelocity(
    std::shared_ptr<ProgramFactory> const& factory,
    int xSize, int ySize, int numXThreads, int numYThreads,
    std::shared_ptr<ConstantBuffer> const& parameters)
    :
    mNumXGroups(xSize / numXThreads),
    mNumYGroups(ySize / numYThreads)
{
    int api = factory->GetAPI();
    factory->PushDefines();
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);

    mAdjustVelocity = factory->CreateFromSource(*msSource[api]);
    if (mAdjustVelocity)
    {
        mAdjustVelocity->GetComputeShader()->Set("Parameters", parameters);
    }

    factory->PopDefines();
}

void GPUFluid2AdjustVelocity::Execute(
    std::shared_ptr<GraphicsEngine> const& engine,
    std::shared_ptr<Texture2> const& inState,
    std::shared_ptr<Texture2> const& poisson,
    std::shared_ptr<Texture2> const& outState)

{
    auto cshader = mAdjustVelocity->GetComputeShader();
    cshader->Set("inState", inState);
    cshader->Set("poisson", poisson);
    cshader->Set("outState", outState);
    engine->Execute(mAdjustVelocity, mNumXGroups, mNumYGroups, 1);
}


std::string const GPUFluid2AdjustVelocity::msGLSLSource =
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

    layout(rgba32f) uniform readonly image2D inState;
    layout(r32f) uniform readonly image2D poisson;
    layout(rgba32f) uniform writeonly image2D outState;

    layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
    void main()
    {
        ivec2 c = ivec2(gl_GlobalInvocationID.xy);
        ivec2 dim = imageSize(inState);

        int x = int(c.x);
        int y = int(c.y);
        int xm = max(x - 1, 0);
        int xp = min(x + 1, dim.x - 1);
        int ym = max(y - 1, 0);
        int yp = min(y + 1, dim.y - 1);

        // Sample the state at (x,y).
        vec4 state = imageLoad(inState, c);

        // Sample Poisson values at immediate neighbors of (x,y).
        float poisPZ = imageLoad(poisson, ivec2(xp, y)).x;
        float poisMZ = imageLoad(poisson, ivec2(xm, y)).x;
        float poisZP = imageLoad(poisson, ivec2(x, yp)).x;
        float poisZM = imageLoad(poisson, ivec2(x, ym)).x;

        vec4 diff = vec4(poisPZ - poisMZ, poisZP - poisZM, 0.0f, 0.0f);
        imageStore(outState, c, state + halfDivDelta * diff);
    }
)";

std::string const GPUFluid2AdjustVelocity::msHLSLSource =
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

    Texture2D<float4> inState;
    Texture2D<float> poisson;
    RWTexture2D<float4> outState;

    [numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
    void CSMain(uint2 c : SV_DispatchThreadID)
    {
        uint2 dim;
        inState.GetDimensions(dim.x, dim.y);

        int x = int(c.x);
        int y = int(c.y);
        int xm = max(x - 1, 0);
        int xp = min(x + 1, dim.x - 1);
        int ym = max(y - 1, 0);
        int yp = min(y + 1, dim.y - 1);

        // Sample the state at (x,y).
        float4 state = inState[c];

        // Sample Poisson values at immediate neighbors of (x,y).
        float poisPZ = poisson[int2(xp, y)];
        float poisMZ = poisson[int2(xm, y)];
        float poisZP = poisson[int2(x, yp)];
        float poisZM = poisson[int2(x, ym)];

        float4 diff = float4(poisPZ - poisMZ, poisZP - poisZM, 0.0f, 0.0f);
        outState[c] = state + halfDivDelta * diff;
    }
)";

ProgramSources const GPUFluid2AdjustVelocity::msSource =
{
    &msGLSLSource,
    &msHLSLSource
};
