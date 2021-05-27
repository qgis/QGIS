// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.28

#include <MathematicsGPU/GTMathematicsGPUPCH.h>
#include <MathematicsGPU/GPUFluid3InitializeSource.h>
#include <random>
using namespace gte;

GPUFluid3InitializeSource::GPUFluid3InitializeSource(
    std::shared_ptr<ProgramFactory> const& factory,
    int xSize, int ySize, int zSize, int numXThreads, int numYThreads,
    int numZThreads, std::shared_ptr<ConstantBuffer> const& parameters)
    :
    mNumXGroups(xSize / numXThreads),
    mNumYGroups(ySize / numYThreads),
    mNumZGroups(zSize / numZThreads)
{
    // Create the resources for generating velocity from vortices.
    mVortex = std::make_shared<ConstantBuffer>(sizeof(Vortex), true);
    mVelocity0 = std::make_shared<Texture3>(DF_R32G32B32A32_FLOAT, xSize, ySize, zSize);
    mVelocity0->SetUsage(Resource::SHADER_OUTPUT);
    mVelocity1 = std::make_shared<Texture3>(DF_R32G32B32A32_FLOAT, xSize, ySize, zSize);
    mVelocity1->SetUsage(Resource::SHADER_OUTPUT);

    // Create the resources for generating velocity from wind and gravity.
    mExternal = std::make_shared<ConstantBuffer>(sizeof(External), false);
    External& e = *mExternal->Get<External>();
    e.densityProducer = { 0.5f, 0.5f, 0.5f, 0.0f };
    e.densityPData = { 0.01f, 16.0f, 0.0f, 0.0f };
    e.densityConsumer = { 0.75f, 0.75f, 0.75f, 0.0f };
    e.densityCData = { 0.01f, 0.0f, 0.0f, 0.0f };
    e.gravity = { 0.0f, 0.0f, 0.0f, 0.0f };
    e.windData = { 0.001f, 0.0f, 0.0f, 0.0f };
    mSource = std::make_shared<Texture3>(DF_R32G32B32A32_FLOAT, xSize, ySize, zSize);
    mSource->SetUsage(Resource::SHADER_OUTPUT);

    // Create the shader for generating velocity from vortices.
    int api = factory->GetAPI();
    factory->PushDefines();
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);
    factory->defines.Set("NUM_Z_THREADS", numZThreads);

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

void GPUFluid3InitializeSource::Execute(
    std::shared_ptr<GraphicsEngine> const& engine)
{
    // Use a Mersenne twister engine for random numbers.
    std::mt19937 mte;
    std::uniform_real_distribution<float> unirnd(0.0f, 1.0f);
    std::uniform_real_distribution<float> symrnd(-1.0f, 1.0f);
    std::uniform_real_distribution<float> posrnd0(0.001f, 0.01f);
    std::uniform_real_distribution<float> posrnd1(64.0f, 128.0f);

    // Compute the velocity one vortex at a time.  After the loop terminates,
    // the final velocity is stored in mVelocity0.
    auto cshader = mGenerateVortex->GetComputeShader();
    std::memset(mVelocity0->GetData(), 0, mVelocity0->GetNumBytes());
    Vortex& v = *mVortex->Get<Vortex>();
    for (int i = 0; i < NUM_VORTICES; ++i)
    {
        v.position[0] = unirnd(mte);
        v.position[1] = unirnd(mte);
        v.position[2] = unirnd(mte);
        v.position[3] = 0.0f;
        v.normal[0] = symrnd(mte);
        v.normal[1] = symrnd(mte);
        v.normal[2] = symrnd(mte);
        v.normal[3] = 0.0f;
        Normalize(v.normal);
        v.data[0] = posrnd0(mte);
        v.data[1] = posrnd1(mte);
        v.data[2] = 0.0f;
        v.data[3] = 0.0f;
        engine->Update(mVortex);

        engine->Execute(mGenerateVortex, mNumXGroups, mNumYGroups, mNumZGroups);

        std::swap(mVelocity0, mVelocity1);
        cshader->Set("inVelocity", mVelocity0);
        cshader->Set("outVelocity", mVelocity1);
    }

    // Compute the sources for the fluid simulation.
    cshader = mInitializeSource->GetComputeShader();
    cshader->Set("vortexVelocity", mVelocity0);
    engine->Execute(mInitializeSource, mNumXGroups, mNumYGroups, mNumZGroups);
}


std::string const GPUFluid3InitializeSource::msGLSLGenerateVortexSource =
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

    uniform Vortex
    {
        vec4 position;  // (px, py, pz, *)
        vec4 normal;    // (nx, ny, nz, *)
        vec4 data;      // (variance, amplitude, *, *)
    };

    layout(rgba32f) uniform readonly image3D inVelocity;
    layout(rgba32f) uniform writeonly image3D outVelocity;

    layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = NUM_Z_THREADS) in;
    void main()
    {
        ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
        vec3 location = spaceDelta.xyz * (c + 0.5f);
        vec3 diff = location - position.xyz;
        float arg = -dot(diff, diff) / data.x;
        float magnitude = data.y * exp(arg);
        vec4 vortexVelocity = vec4(magnitude * cross(normal.xyz, diff), 0.0f);
        imageStore(outVelocity, c, imageLoad(inVelocity, c) + vortexVelocity);
    }
)";

std::string const GPUFluid3InitializeSource::msGLSLInitializeSourceSource =
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

    uniform External
    {
        vec4 densityProducer;  // (x, y, z, *)
        vec4 densityPData;     // (variance, amplitude, *, *)
        vec4 densityConsumer;  // (x, y, z, *)
        vec4 densityCData;     // (variance, amplitude, *, *)
        vec4 gravity;          // (x, y, z, *)
        vec4 windData;         // (variance, amplitude, *, *)
    };

    layout(rgba32f) uniform readonly image3D vortexVelocity;
    layout(rgba32f) uniform writeonly image3D source;

    layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = NUM_Z_THREADS) in;
    void main()
    {
        ivec3 c = ivec3(gl_GlobalInvocationID.xyz);

        // Compute the location of the voxel (x,y,z) in normalized [0,1]^3.
        vec3 location = spaceDelta.xyz * (c + 0.5f);

        // Compute an input to the fluid simulation consisting of a producer of
        // density and a consumer of density.
        vec3 diff = location - densityProducer.xyz;
        float arg = -dot(diff, diff) / densityPData.x;
        float density = densityPData.y * exp(arg);
        diff = location - densityConsumer.xyz;
        arg = -dot(diff, diff) / densityCData.x;
        density -= densityCData.y * exp(arg);

        // Compute an input to the fluid simulation consisting of gravity,
        // a single wind source, and vortex impulses.
        float windArg = -dot(location.xz, location.xz) / windData.x;
        vec3 windVelocity = vec3(0.0f, windData.y * exp(windArg), 0.0f);
        vec3 velocity = gravity.xyz + windVelocity + imageLoad(vortexVelocity, c).xyz;

        imageStore(source, c, vec4(velocity.xyz, density));
    }
)";

std::string const GPUFluid3InitializeSource::msHLSLGenerateVortexSource =
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

    cbuffer Vortex
    {
        float4 position;  // (px, py, pz, *)
        float4 normal;    // (nx, ny, nz, *)
        float4 data;      // (variance, amplitude, *, *)
    };

    Texture3D<float4> inVelocity;
    RWTexture3D<float4> outVelocity;

    [numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
    void CSMain(uint3 c : SV_DispatchThreadID)
    {
        float3 location = spaceDelta.xyz * (c + 0.5f);
        float3 diff = location - position.xyz;
        float arg = -dot(diff, diff) / data.x;
        float magnitude = data.y * exp(arg);
        float4 vortexVelocity = float4(magnitude * cross(normal.xyz, diff), 0.0f);
        outVelocity[c] = inVelocity[c] + vortexVelocity;
    }
)";

std::string const GPUFluid3InitializeSource::msHLSLInitializeSourceSource =
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

    cbuffer External
    {
        float4 densityProducer;  // (x, y, z, *)
        float4 densityPData;     // (variance, amplitude, *, *)
        float4 densityConsumer;  // (x, y, z, *)
        float4 densityCData;     // (variance, amplitude, *, *)
        float4 gravity;          // (x, y, z, *)
        float4 windData;         // (variance, amplitude, *, *)
    };

    Texture3D<float4> vortexVelocity;
    RWTexture3D<float4> source;

    [numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
    void CSMain(uint3 c : SV_DispatchThreadID)
    {
        // Compute the location of the voxel (x,y,z) in normalized [0,1]^3.
        float3 location = spaceDelta.xyz*(c + 0.5f);

        // Compute an input to the fluid simulation consisting of a producer of
        // density and a consumer of density.
        float3 diff = location - densityProducer.xyz;
        float arg = -dot(diff, diff) / densityPData.x;
        float density = densityPData.y * exp(arg);
        diff = location - densityConsumer.xyz;
        arg = -dot(diff, diff) / densityCData.x;
        density -= densityCData.y * exp(arg);

        // Compute an input to the fluid simulation consisting of gravity,
        // a single wind source, and vortex impulses.
        float windArg = -dot(location.xz, location.xz) / windData.x;
        float3 windVelocity = float3(0.0f, windData.y * exp(windArg), 0.0f);
        float3 velocity = gravity.xyz + windVelocity + vortexVelocity[c].xyz;

        source[c] = float4(velocity.xyz, density);
    }
)";

ProgramSources const GPUFluid3InitializeSource::msGenerateVortexSource =
{
    &msGLSLGenerateVortexSource,
    &msHLSLGenerateVortexSource
};

ProgramSources const GPUFluid3InitializeSource::msInitializeSourceSource =
{
    &msGLSLInitializeSourceSource,
    &msHLSLInitializeSourceSource
};
