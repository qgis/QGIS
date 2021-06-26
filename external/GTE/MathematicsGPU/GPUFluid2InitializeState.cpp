// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.28

#include <MathematicsGPU/GTMathematicsGPUPCH.h>
#include <MathematicsGPU/GPUFluid2InitializeState.h>
#include <random>
using namespace gte;

GPUFluid2InitializeState::GPUFluid2InitializeState(
    std::shared_ptr<ProgramFactory> const& factory,
    int xSize, int ySize, int numXThreads, int numYThreads)
    :
    mNumXGroups(xSize / numXThreads),
    mNumYGroups(ySize / numYThreads)
{
    // Use a Mersenne twister engine for random numbers.
    std::mt19937 mte;
    std::uniform_real_distribution<float> unirnd(0.0f, 1.0f);

    // Initial density values are randomly generated.
    mDensity = std::make_shared<Texture2>(DF_R32_FLOAT, xSize, ySize);
    float* data = mDensity->Get<float>();
    for (unsigned int i = 0; i < mDensity->GetNumElements(); ++i, ++data)
    {
        *data = unirnd(mte);
    }

    // Initial velocity values are zero.
    mVelocity = std::make_shared<Texture2>(DF_R32G32_FLOAT, xSize, ySize);
    std::memset(mVelocity->GetData(), 0, mVelocity->GetNumBytes());

    // The states at time 0 and time -dt are initialized by a compute shader.
    mStateTm1 = std::make_shared<Texture2>(DF_R32G32B32A32_FLOAT, xSize, ySize);
    mStateTm1->SetUsage(Resource::SHADER_OUTPUT);

    mStateT = std::make_shared<Texture2>(DF_R32G32B32A32_FLOAT, xSize, ySize);
    mStateT->SetUsage(Resource::SHADER_OUTPUT);

    // Create the shader for initializing velocity and density.
    int api = factory->GetAPI();
    factory->PushDefines();
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);
    mInitializeState = factory->CreateFromSource(*msSource[api]);
    if (mInitializeState)
    {
        auto cshader =  mInitializeState->GetComputeShader();
        cshader->Set("density", mDensity);
        cshader->Set("velocity", mVelocity);
        cshader->Set("stateTm1", mStateTm1);
        cshader->Set("stateT", mStateT);
    }
    factory->PopDefines();
}

void GPUFluid2InitializeState::Execute(
    std::shared_ptr<GraphicsEngine> const& engine)
{
    engine->Execute(mInitializeState, mNumXGroups, mNumYGroups, 1);
}


std::string const GPUFluid2InitializeState::msGLSLSource =
R"(
    layout(r32f) uniform readonly image2D density;
    layout(rg32f) uniform readonly image2D velocity;
    layout(rgba32f) uniform writeonly image2D stateTm1;
    layout(rgba32f) uniform writeonly image2D stateT;

    layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
    void main()
    {
        ivec2 c = ivec2(gl_GlobalInvocationID.xy);
        vec4 initial = vec4(imageLoad(velocity, c).xy, 0.0f, imageLoad(density, c).x);
        imageStore(stateTm1, c, initial);
        imageStore(stateT, c, initial);
    }
)";

std::string const GPUFluid2InitializeState::msHLSLSource =
R"(
    Texture2D<float> density;
    Texture2D<float2> velocity;
    RWTexture2D<float4> stateTm1;
    RWTexture2D<float4> stateT;

    [numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
    void CSMain(uint2 c : SV_DispatchThreadID)
    {
        float4 initial = float4(velocity[c], 0.0f, density[c]);
        stateTm1[c] = initial;
        stateT[c] = initial;
    }
)";

ProgramSources const GPUFluid2InitializeState::msSource =
{
    &msGLSLSource,
    &msHLSLSource
};
