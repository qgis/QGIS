// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.28

#include <MathematicsGPU/GTMathematicsGPUPCH.h>
#include <MathematicsGPU/GPUFluid2InitializeSource.h>
#include <random>
using namespace gte;

GPUFluid2InitializeSource::GPUFluid2InitializeSource(
    std::shared_ptr<ProgramFactory> const& factory,
    int xSize, int ySize, int numXThreads, int numYThreads,
    std::shared_ptr<ConstantBuffer> const& parameters)
    :
    mNumXGroups(xSize / numXThreads),
    mNumYGroups(ySize / numYThreads)
{
    // Create the resources for generating velocity from vortices.
    mVortex = std::make_shared<ConstantBuffer>(sizeof(Vortex), true);
    mVelocity0 = std::make_shared<Texture2>(DF_R32G32_FLOAT, xSize, ySize);
    mVelocity0->SetUsage(Resource::SHADER_OUTPUT);
    mVelocity1 = std::make_shared<Texture2>(DF_R32G32_FLOAT, xSize, ySize);
    mVelocity1->SetUsage(Resource::SHADER_OUTPUT);

    // Create the resources for generating velocity from wind and gravity.
    mExternal = std::make_shared<ConstantBuffer>(sizeof(External), false);
    External& e = *mExternal->Get<External>();
    e.densityProducer = { 0.25f, 0.75f, 0.01f, 2.0f };
    e.densityConsumer = { 0.75f, 0.25f, 0.01f, 2.0f };
    e.gravity = { 0.0f, 0.0f, 0.0f, 0.0f };
    e.wind = { 0.0f, 0.5f, 0.001f, 32.0f };
    mSource = std::make_shared<Texture2>(DF_R32G32B32A32_FLOAT, xSize, ySize);
    mSource->SetUsage(Resource::SHADER_OUTPUT);

    // Create the shader for generating velocity from vortices.
    int api = factory->GetAPI();
    factory->PushDefines();
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);

    mGenerateVortex = factory->CreateFromSource(*msGenerateVortexSource[api]);
    if (mGenerateVortex)
    {
        auto cshader = mGenerateVortex->GetComputeShader();
        cshader->Set("Parameters", parameters);
        cshader->Set("Vortex", mVortex);
        cshader->Set("inVelocity", mVelocity0);
        cshader->Set("outVelocity", mVelocity1);
    }

    // Create the shader for generating the sources to the fluid simulation.
    mInitializeSource = factory->CreateFromSource(*msInitializeSourceSource[api]);
    if (mInitializeSource)
    {
        auto cshader = mInitializeSource->GetComputeShader();
        cshader->Set("Parameters", parameters);
        cshader->Set("External", mExternal);
        cshader->Set("source", mSource);
    }

    factory->PopDefines();
}

void GPUFluid2InitializeSource::Execute(
    std::shared_ptr<GraphicsEngine> const& engine)
{
    // Use a Mersenne twister engine for random numbers.
    std::mt19937 mte;
    std::uniform_real_distribution<float> unirnd(0.0f, 1.0f);
    std::uniform_real_distribution<float> symrnd(-1.0f, 1.0f);
    std::uniform_real_distribution<float> posrnd0(0.001f, 0.01f);
    std::uniform_real_distribution<float> posrnd1(128.0f, 256.0f);

    // Compute the velocity one vortex at a time.  After the loop terminates,
    // the final velocity is stored in mVelocity0.
    auto cshader = mGenerateVortex->GetComputeShader();
    std::memset(mVelocity0->GetData(), 0, mVelocity0->GetNumBytes());
    Vortex& v = *mVortex->Get<Vortex>();
    for (int i = 0; i < NUM_VORTICES; ++i)
    {
        v.data[0] = unirnd(mte);
        v.data[1] = unirnd(mte);
        v.data[2] = posrnd0(mte);
        v.data[3] = posrnd1(mte);
        if (symrnd(mte) < 0.0f)
        {
            v.data[3] = -v.data[3];
        }
        engine->Update(mVortex);

        engine->Execute(mGenerateVortex, mNumXGroups, mNumYGroups, 1);

        std::swap(mVelocity0, mVelocity1);
        cshader->Set("inVelocity", mVelocity0);
        cshader->Set("outVelocity", mVelocity1);
    }

    // Compute the sources for the fluid simulation.
    cshader = mInitializeSource->GetComputeShader();
    cshader->Set("vortexVelocity", mVelocity0);
    engine->Execute(mInitializeSource, mNumXGroups, mNumYGroups, 1);
}


std::string const GPUFluid2InitializeSource::msGLSLGenerateVortexSource =
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

    uniform Vortex
    {
        vec4 data;      // (x, y, variance, amplitude)
    };

    layout(rg32f) uniform readonly image2D inVelocity;
    layout(rg32f) uniform writeonly image2D outVelocity;

    layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
    void main()
    {
        ivec2 c = ivec2(gl_GlobalInvocationID.xy);
        vec2 location = spaceDelta.xy * (c + 0.5f);
        vec2 diff = location - data.xy;
        float arg = -dot(diff, diff) / data.z;
        float magnitude = data.w * exp(arg);
        vec2 vortexVelocity = magnitude * vec2(diff.y, -diff.x);
        imageStore(outVelocity, c, vec4(imageLoad(inVelocity, c).xy + vortexVelocity, 0.0f, 0.0f));
    }
)";

std::string const GPUFluid2InitializeSource::msGLSLInitializeSourceSource =
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

    uniform External
    {
        vec4 densityProducer;  // (x, y, variance, amplitude)
        vec4 densityConsumer;  // (x, y, variance, amplitude)
        vec4 gravity;          // (x, y, *, *)
        vec4 wind;             // (x, y, variance, amplitude)
    };

    layout(rg32f) uniform readonly image2D vortexVelocity;
    layout(rgba32f) uniform writeonly image2D source;

    layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
    void main()
    {
        ivec2 c = ivec2(gl_GlobalInvocationID.xy);

        // Compute the location of the pixel (x,y) in normalized [0,1]^2.
        vec2 location = spaceDelta.xy * (c + 0.5f);

        // Compute an input to the fluid simulation consisting of a producer of
        // density and a consumer of density.
        vec2 diff = location - densityProducer.xy;
        float arg = -dot(diff, diff) / densityProducer.z;
        float density = densityProducer.w * exp(arg);
        diff = location - densityConsumer.xy;
        arg = -dot(diff, diff) / densityConsumer.z;
        density -= densityConsumer.w * exp(arg);

        // Compute an input to the fluid simulation consisting of gravity,
        // a single wind source, and vortex impulses.
        float windDiff = location.y - wind.y;
        float windArg = -windDiff * windDiff / wind.z;
        vec2 windVelocity = vec2(wind.w * exp(windArg), 0.0f);
        vec2 velocity = gravity.xy + windVelocity + imageLoad(vortexVelocity, c).xy;

        imageStore(source, c, vec4(velocity, 0.0f, density));
    }
)";

std::string const GPUFluid2InitializeSource::msHLSLGenerateVortexSource =
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

    cbuffer Vortex
    {
        float4 data;  // (x, y, variance, amplitude)
    };

    Texture2D<float2> inVelocity;
    RWTexture2D<float2> outVelocity;

    [numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
    void CSMain(uint3 c : SV_DispatchThreadID)
    {
        float2 location = spaceDelta.xy * (c.xy + 0.5f);
        float2 diff = location - data.xy;
        float arg = -dot(diff, diff) / data.z;
        float magnitude = data.w * exp(arg);
        float2 vortexVelocity = magnitude * float2(diff.y, -diff.x);
        outVelocity[c.xy] = inVelocity[c.xy] + vortexVelocity;
    }
)";

std::string const GPUFluid2InitializeSource::msHLSLInitializeSourceSource =
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

    cbuffer External
    {
        float4 densityProducer;  // (x, y, variance, amplitude)
        float4 densityConsumer;  // (x, y, variance, amplitude)
        float4 gravity;          // (x, y, *, *)
        float4 wind;             // (x, y, variance, amplitude)
    };

    Texture2D<float2> vortexVelocity;
    RWTexture2D<float4> source;

    [numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
    void CSMain(uint2 c : SV_DispatchThreadID)
    {
        // Compute the location of the pixel (x,y) in normalized [0,1]^2.
        float2 location = spaceDelta.xy * (c + 0.5f);

        // Compute an input to the fluid simulation consisting of a producer of
        // density and a consumer of density.
        float2 diff = location - densityProducer.xy;
        float arg = -dot(diff, diff) / densityProducer.z;
        float density = densityProducer.w * exp(arg);
        diff = location - densityConsumer.xy;
        arg = -dot(diff, diff) / densityConsumer.z;
        density -= densityConsumer.w * exp(arg);

        // Compute an input to the fluid simulation consisting of gravity,
        // a single wind source, and vortex impulses.
        float windDiff = location.y - wind.y;
        float windArg = -windDiff * windDiff / wind.z;
        float2 windVelocity = float2(wind.w * exp(windArg), 0.0f);
        float2 velocity = gravity.xy + windVelocity + vortexVelocity[c];

        source[c] = float4(velocity, 0.0f, density);
    }
)";

ProgramSources const GPUFluid2InitializeSource::msGenerateVortexSource =
{
    &msGLSLGenerateVortexSource,
    &msHLSLGenerateVortexSource
};

ProgramSources const GPUFluid2InitializeSource::msInitializeSourceSource =
{
    &msGLSLInitializeSourceSource,
    &msHLSLInitializeSourceSource
};
