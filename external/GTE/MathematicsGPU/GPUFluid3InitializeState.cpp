// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.28

#include <MathematicsGPU/GTMathematicsGPUPCH.h>
#include <MathematicsGPU/GPUFluid3InitializeState.h>
#include <random>
using namespace gte;

GPUFluid3InitializeState::GPUFluid3InitializeState(
    std::shared_ptr<ProgramFactory> const& factory,
    int xSize, int ySize, int zSize, int numXThreads, int numYThreads, int numZThreads)
    :
    mNumXGroups(xSize / numXThreads),
    mNumYGroups(ySize / numYThreads),
    mNumZGroups(zSize / numZThreads)
{
    // Use a Mersenne twister engine for random numbers.
    std::mt19937 mte;
    std::uniform_real_distribution<float> unirnd(0.0f, 1.0f);

    // Initial density values are randomly generated.
    mDensity = std::make_shared<Texture3>(DF_R32_FLOAT, xSize, ySize, zSize);
    float* data = mDensity->Get<float>();
    for (unsigned int i = 0; i < mDensity->GetNumElements(); ++i, ++data)
    {
        *data = unirnd(mte);
    }

    // Initial velocity values are zero.
    mVelocity = std::make_shared<Texture3>(DF_R32G32B32A32_FLOAT, xSize, ySize, zSize);
    mVelocity->SetUsage(Resource::SHADER_OUTPUT);
    std::memset(mVelocity->GetData(), 0, mVelocity->GetNumBytes());

    mStateTm1 = std::make_shared<Texture3>(DF_R32G32B32A32_FLOAT, xSize, ySize, zSize);
    mStateTm1->SetUsage(Resource::SHADER_OUTPUT);

    mStateT = std::make_shared<Texture3>(DF_R32G32B32A32_FLOAT, xSize, ySize, zSize);
    mStateT->SetUsage(Resource::SHADER_OUTPUT);

    // Create the shader for generating velocity from vortices.
    int api = factory->GetAPI();
    factory->PushDefines();
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);
    factory->defines.Set("NUM_Z_THREADS", numZThreads);
    mInitializeState = factory->CreateFromSource(*msSource[api]);
    if (mInitializeState)
    {
        auto cshader = mInitializeState->GetComputeShader();
        cshader->Set("density", mDensity);
        cshader->Set("velocity", mVelocity);
        cshader->Set("stateTm1", mStateTm1);
        cshader->Set("stateT", mStateT);
    }
    factory->PopDefines();
}

void GPUFluid3InitializeState::Execute(
    std::shared_ptr<GraphicsEngine> const& engine)
{
    engine->Execute(mInitializeState, mNumXGroups, mNumYGroups, mNumZGroups);
}


std::string const GPUFluid3InitializeState::msGLSLSource =
R"(
    layout(r32f) uniform readonly image3D density;
    layout(rgba32f) uniform readonly image3D velocity;
    layout(rgba32f) uniform writeonly image3D stateTm1;
    layout(rgba32f) uniform writeonly image3D stateT;

    layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = NUM_Z_THREADS) in;
    void main()
    {
        ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
        vec4 initial = vec4(imageLoad(velocity, c).xyz, imageLoad(density, c).x);
        imageStore(stateTm1, c, initial);
        imageStore(stateT, c, initial);
    }
)";

std::string const GPUFluid3InitializeState::msHLSLSource =
R"(
    Texture3D<float> density;
    Texture3D<float4> velocity;
    RWTexture3D<float4> stateTm1;
    RWTexture3D<float4> stateT;

    [numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
    void CSMain(uint3 c : SV_DispatchThreadID)
    {
        float4 initial = float4(velocity[c].xyz, density[c]);
        stateTm1[c] = initial;
        stateT[c] = initial;
    }
)";

ProgramSources const GPUFluid3InitializeState::msSource =
{
    &msGLSLSource,
    &msHLSLSource
};
