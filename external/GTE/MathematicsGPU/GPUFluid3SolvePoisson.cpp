// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.28

#include <MathematicsGPU/GTMathematicsGPUPCH.h>
#include <MathematicsGPU/GPUFluid3SolvePoisson.h>
using namespace gte;

GPUFluid3SolvePoisson::GPUFluid3SolvePoisson(
    std::shared_ptr<ProgramFactory> const& factory,
    int xSize, int ySize, int zSize, int numXThreads, int numYThreads, int numZThreads,
    std::shared_ptr<ConstantBuffer> const& parameters, int numIterations)
    :
    mNumXGroups(xSize / numXThreads),
    mNumYGroups(ySize / numYThreads),
    mNumZGroups(zSize / numZThreads),
    mNumIterations(numIterations)
{
    mPoisson0 = std::make_shared<Texture3>(DF_R32_FLOAT, xSize, ySize, zSize);
    mPoisson0->SetUsage(Resource::SHADER_OUTPUT);
    mPoisson1 = std::make_shared<Texture3>(DF_R32_FLOAT, xSize, ySize, zSize);
    mPoisson1->SetUsage(Resource::SHADER_OUTPUT);

    int api = factory->GetAPI();
    factory->PushDefines();
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);
    factory->defines.Set("NUM_Z_THREADS", numZThreads);

    // For zeroing mPoisson0 on the GPU.
    mZeroPoisson = factory->CreateFromSource(*msPoissonZeroSource[api]);
    if (mZeroPoisson)
    {
        mZeroPoisson->GetComputeShader()->Set("poisson", mPoisson0);
    }

    // Create the shader for generating velocity from vortices.
    mSolvePoisson = factory->CreateFromSource(*msPoissonSolveSource[api]);
    if (mSolvePoisson)
    {
        mSolvePoisson->GetComputeShader()->Set("Parameters", parameters);
    }

    factory->defines.Clear();
    factory->defines.Set("USE_ZERO_X_FACE", 1);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);
    factory->defines.Set("NUM_Z_THREADS", numZThreads);
    mWriteXFace = factory->CreateFromSource(*msPoissonEnforceBoundarySource[api]);

    factory->defines.Clear();
    factory->defines.Set("USE_ZERO_Y_FACE", 1);
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    factory->defines.Set("NUM_Z_THREADS", numZThreads);
    mWriteYFace = factory->CreateFromSource(*msPoissonEnforceBoundarySource[api]);

    factory->defines.Clear();
    factory->defines.Set("USE_ZERO_Z_FACE", 1);
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);
    mWriteZFace = factory->CreateFromSource(*msPoissonEnforceBoundarySource[api]);

    factory->PopDefines();
}

void GPUFluid3SolvePoisson::Execute(
    std::shared_ptr<GraphicsEngine> const& engine,
    std::shared_ptr<Texture3> const& divergence)
{
    auto solve = mSolvePoisson->GetComputeShader();
    auto xwrite = mWriteXFace->GetComputeShader();
    auto ywrite = mWriteYFace->GetComputeShader();
    auto zwrite = mWriteZFace->GetComputeShader();

    solve->Set("divergence", divergence);
    engine->Execute(mZeroPoisson, mNumXGroups, mNumYGroups, mNumZGroups);
    for (int i = 0; i < mNumIterations; ++i)
    {
        // Take one step of the Poisson solver.
        solve->Set("poisson", mPoisson0);
        solve->Set("outPoisson", mPoisson1);
        engine->Execute(mSolvePoisson, mNumXGroups, mNumYGroups, mNumZGroups);

        // Set the boundary to zero.
        xwrite->Set("image", mPoisson1);
        engine->Execute(mWriteXFace, 1, mNumYGroups, mNumZGroups);
        ywrite->Set("image", mPoisson1);
        engine->Execute(mWriteYFace, mNumXGroups, 1, mNumZGroups);
        zwrite->Set("image", mPoisson1);
        engine->Execute(mWriteZFace, mNumXGroups, mNumYGroups, 1);

        std::swap(mPoisson0, mPoisson1);
    }
}


std::string const GPUFluid3SolvePoisson::msGLSLPoissonZeroSource =
R"(
    layout(r32f) uniform writeonly image3D poisson;

    layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = NUM_Z_THREADS) in;
    void main()
    {
        ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
        imageStore(poisson, c, vec4(0.0f, 0.0f, 0.0f, 0.0f));
    }
)";

std::string const GPUFluid3SolvePoisson::msGLSLPoissonSolveSource =
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

    layout(r32f) uniform readonly image3D divergence;
    layout(r32f) uniform readonly image3D poisson;
    layout(r32f) uniform writeonly image3D outPoisson;

    layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = NUM_Z_THREADS) in;
    void main()
    {
        ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
        ivec3 dim = imageSize(divergence);

        int x = int(c.x);
        int y = int(c.y);
        int z = int(c.z);
        int xm = max(x - 1, 0);
        int xp = min(x + 1, dim.x - 1);
        int ym = max(y - 1, 0);
        int yp = min(y + 1, dim.y - 1);
        int zm = max(z - 1, 0);
        int zp = min(z + 1, dim.z - 1);

        // Sample the divergence at (x,y,z).
        float div = imageLoad(divergence, c).x;

        // Sample Poisson values at immediate neighbors of (x,y,z).
        float poisPZZ = imageLoad(poisson, ivec3(xp, y, z)).x;
        float poisMZZ = imageLoad(poisson, ivec3(xm, y, z)).x;
        float poisZPZ = imageLoad(poisson, ivec3(x, yp, z)).x;
        float poisZMZ = imageLoad(poisson, ivec3(x, ym, z)).x;
        float poisZZP = imageLoad(poisson, ivec3(x, y, zp)).x;
        float poisZZM = imageLoad(poisson, ivec3(x, y, zm)).x;

        vec4 temp = vec4(poisPZZ + poisMZZ, poisZPZ + poisZMZ, poisZZP + poisZZM, div);
        float outPoissonValue = dot(epsilon, temp);
        imageStore(outPoisson, c, vec4(outPoissonValue, 0.0f, 0.0f, 0.0f));
    }
)";

std::string const GPUFluid3SolvePoisson::msGLSLPoissonEnforceBoundarySource =
R"(
    #if USE_ZERO_X_FACE
    layout(r32f) uniform writeonly image3D image;

    layout (local_size_x = 1, local_size_y = NUM_Y_THREADS, local_size_z = NUM_Z_THREADS) in;
    void main()
    {
        ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
        ivec3 dim = imageSize(image);
        imageStore(image, ivec3(0, c.y, c.z), vec4(0.0f));
        imageStore(image, ivec3(dim.x - 1, c.y, c.z), vec4(0.0f));
    }
    #endif

    #if USE_ZERO_Y_FACE
    layout(r32f) uniform writeonly image3D image;

    layout (local_size_x = NUM_X_THREADS, local_size_y = 1, local_size_z = NUM_Z_THREADS) in;
    void main()
    {
        ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
        ivec3 dim = imageSize(image);
        imageStore(image, ivec3(c.x, 0, c.z), vec4(0.0f));
        imageStore(image, ivec3(c.x, dim.y - 1, c.z), vec4(0.0f));
    }
    #endif

    #if USE_ZERO_Z_FACE
    layout(r32f) uniform writeonly image3D image;

    layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
    void main()
    {
        ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
        ivec3 dim = imageSize(image);
        imageStore(image, ivec3(c.x, c.y, 0), vec4(0.0f));
        imageStore(image, ivec3(c.x, c.y, dim.z - 1), vec4(0.0f));
    }
    #endif
)";

std::string const GPUFluid3SolvePoisson::msHLSLPoissonZeroSource =
R"(
    RWTexture3D<float> poisson;

    [numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
    void CSMain(uint3 c : SV_DispatchThreadID)
    {
        poisson[c.xyz] = 0.0f;
    }
)";

std::string const GPUFluid3SolvePoisson::msHLSLPoissonSolveSource =
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

    Texture3D<float> divergence;
    Texture3D<float> poisson;
    RWTexture3D<float> outPoisson;

    [numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
    void CSMain(uint3 c : SV_DispatchThreadID)
    {
        uint3 dim;
        divergence.GetDimensions(dim.x, dim.y, dim.z);

        int x = int(c.x);
        int y = int(c.y);
        int z = int(c.z);
        int xm = max(x - 1, 0);
        int xp = min(x + 1, dim.x - 1);
        int ym = max(y - 1, 0);
        int yp = min(y + 1, dim.y - 1);
        int zm = max(z - 1, 0);
        int zp = min(z + 1, dim.z - 1);

        // Sample the divergence at (x,y,z).
        float div = divergence[c];

        // Sample Poisson values at immediate neighbors of (x,y,z).
        float poisPZZ = poisson[int3(xp, y, z)];
        float poisMZZ = poisson[int3(xm, y, z)];
        float poisZPZ = poisson[int3(x, yp, z)];
        float poisZMZ = poisson[int3(x, ym, z)];
        float poisZZP = poisson[int3(x, y, zp)];
        float poisZZM = poisson[int3(x, y, zm)];

        float4 temp = float4(poisPZZ + poisMZZ, poisZPZ + poisZMZ, poisZZP + poisZZM, div);
        outPoisson[c] = dot(epsilon, temp);
    }
)";

std::string const GPUFluid3SolvePoisson::msHLSLPoissonEnforceBoundarySource =
R"(
    #if USE_ZERO_X_FACE
    RWTexture3D<float> image;

    [numthreads(1, NUM_Y_THREADS, NUM_Z_THREADS)]
    void CSMain(uint3 c : SV_DispatchThreadID)
    {
        uint3 dim;
        image.GetDimensions(dim.x, dim.y, dim.z);
        image[uint3(0, c.y, c.z)] = 0.0f;
        image[uint3(dim.x - 1, c.y, c.z)] = 0.0f;
    }
    #endif

    #if USE_ZERO_Y_FACE
    RWTexture3D<float> image;

    [numthreads(NUM_X_THREADS, 1, NUM_Z_THREADS)]
    void CSMain(uint3 c : SV_DispatchThreadID)
    {
        uint3 dim;
        image.GetDimensions(dim.x, dim.y, dim.z);
        image[uint3(c.x, 0, c.z)] = 0.0f;
        image[uint3(c.x, dim.y - 1, c.z)] = 0.0f;
    }
    #endif

    #if USE_ZERO_Z_FACE
    RWTexture3D<float> image;

    [numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
    void CSMain(uint3 c : SV_DispatchThreadID)
    {
        uint3 dim;
        image.GetDimensions(dim.x, dim.y, dim.z);
        image[uint3(c.x, c.y, 0)] = 0.0f;
        image[uint3(c.x, c.y, dim.z - 1)] = 0.0f;
    }
    #endif
)";

ProgramSources const GPUFluid3SolvePoisson::msPoissonZeroSource =
{
    &msGLSLPoissonZeroSource,
    &msHLSLPoissonZeroSource
};

ProgramSources const GPUFluid3SolvePoisson::msPoissonSolveSource =
{
    &msGLSLPoissonSolveSource,
    &msHLSLPoissonSolveSource
};

ProgramSources const GPUFluid3SolvePoisson::msPoissonEnforceBoundarySource =
{
    &msGLSLPoissonEnforceBoundarySource,
    &msHLSLPoissonEnforceBoundarySource
};
