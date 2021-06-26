// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.28

#include <MathematicsGPU/GTMathematicsGPUPCH.h>
#include <MathematicsGPU/GPUFluid2SolvePoisson.h>
using namespace gte;

GPUFluid2SolvePoisson::GPUFluid2SolvePoisson(
    std::shared_ptr<ProgramFactory> const& factory,
    int xSize, int ySize, int numXThreads, int numYThreads,
    std::shared_ptr<ConstantBuffer> const& parameters, int numIterations)
    :
    mNumXGroups(xSize / numXThreads),
    mNumYGroups(ySize / numYThreads),
    mNumIterations(numIterations)
{
    mPoisson0 = std::make_shared<Texture2>(DF_R32_FLOAT, xSize, ySize);
    mPoisson0->SetUsage(Resource::SHADER_OUTPUT);
    mPoisson1 = std::make_shared<Texture2>(DF_R32_FLOAT, xSize, ySize);
    mPoisson1->SetUsage(Resource::SHADER_OUTPUT);

    int api = factory->GetAPI();
    factory->PushDefines();
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);

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
    factory->defines.Set("USE_ZERO_X_EDGE", 1);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);
    mWriteXEdge = factory->CreateFromSource(*msPoissonEnforceBoundarySource[api]);

    factory->defines.Clear();
    factory->defines.Set("USE_ZERO_Y_EDGE", 1);
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    mWriteYEdge = factory->CreateFromSource(*msPoissonEnforceBoundarySource[api]);

    factory->PopDefines();
}

void GPUFluid2SolvePoisson::Execute(
    std::shared_ptr<GraphicsEngine> const& engine,
    std::shared_ptr<Texture2> const& divergence)
{
    auto solve = mSolvePoisson->GetComputeShader();
    auto xwrite = mWriteXEdge->GetComputeShader();
    auto ywrite = mWriteYEdge->GetComputeShader();

    solve->Set("divergence", divergence);
    engine->Execute(mZeroPoisson, mNumXGroups, mNumYGroups, 1);
    for (int i = 0; i < mNumIterations; ++i)
    {
        // Take one step of the Poisson solver.
        solve->Set("poisson", mPoisson0);
        solve->Set("outPoisson", mPoisson1);
        engine->Execute(mSolvePoisson, mNumXGroups, mNumYGroups, 1);

        // Set the boundary to zero.
        xwrite->Set("image", mPoisson1);
        engine->Execute(mWriteXEdge, 1, mNumYGroups, 1);
        ywrite->Set("image", mPoisson1);
        engine->Execute(mWriteYEdge, mNumXGroups, 1, 1);

        std::swap(mPoisson0, mPoisson1);
    }
}


std::string const GPUFluid2SolvePoisson::msGLSLPoissonZeroSource =
R"(
    layout(r32f) uniform writeonly image2D poisson;

    layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
    void main()
    {
        ivec2 c = ivec2(gl_GlobalInvocationID.xy);
        imageStore(poisson, c, vec4(0.0f, 0.0f, 0.0f, 0.0f));
    }
)";

std::string const GPUFluid2SolvePoisson::msGLSLPoissonSolveSource =
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

    layout(r32f) uniform readonly image2D divergence;
    layout(r32f) uniform readonly image2D poisson;
    layout(r32f) uniform writeonly image2D outPoisson;

    layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
    void main()
    {
        ivec2 c = ivec2(gl_GlobalInvocationID.xy);
        ivec2 dim = imageSize(divergence);

        int x = int(c.x);
        int y = int(c.y);
        int xm = max(x - 1, 0);
        int xp = min(x + 1, dim.x - 1);
        int ym = max(y - 1, 0);
        int yp = min(y + 1, dim.y - 1);

        // Sample the divergence at (x,y).
        float div = imageLoad(divergence, c).x;

        // Sample Poisson values at (x+dx,y), (x-dx,y), (x,y+dy), (x,y-dy).
        float poisPZ = imageLoad(poisson, ivec2(xp, y)).x;
        float poisMZ = imageLoad(poisson, ivec2(xm, y)).x;
        float poisZP = imageLoad(poisson, ivec2(x, yp)).x;
        float poisZM = imageLoad(poisson, ivec2(x, ym)).x;

        vec4 temp = vec4(poisPZ + poisMZ, poisZP + poisZM, 0.0f, div);
        float outPoissonValue = dot(epsilon, temp);
        imageStore(outPoisson, c, vec4(outPoissonValue, 0.0f, 0.0f, 0.0f));
    }
)";

std::string const GPUFluid2SolvePoisson::msGLSLPoissonEnforceBoundarySource =
R"(
    #if USE_ZERO_X_EDGE
    layout(r32f) uniform writeonly image2D image;

    layout (local_size_x = 1, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
    void main()
    {
        ivec2 c = ivec2(gl_GlobalInvocationID.xy);
        ivec2 dim = imageSize(image);
        imageStore(image, ivec2(0, c.y), vec4(0.0f, 0.0f, 0.0f, 0.0f));
        imageStore(image, ivec2(dim.x - 1, c.y), vec4(0.0f, 0.0f, 0.0f, 0.0f));
    }
    #endif

    #if USE_ZERO_Y_EDGE
    layout(r32f) uniform writeonly image2D image;

    layout (local_size_x = NUM_X_THREADS, local_size_y = 1, local_size_z = 1) in;
    void main()
    {
        ivec2 c = ivec2(gl_GlobalInvocationID.xy);
        ivec2 dim = imageSize(image);
        imageStore(image, ivec2(c.x, 0), vec4(0.0f, 0.0f, 0.0f, 0.0f));
        imageStore(image, ivec2(c.x, dim.y - 1), vec4(0.0f, 0.0f, 0.0f, 0.0f));
    }
    #endif
)";

std::string const GPUFluid2SolvePoisson::msHLSLPoissonZeroSource =
R"(
    RWTexture2D<float> poisson;

    [numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
    void CSMain(uint2 c : SV_DispatchThreadID)
    {
        poisson[c] = 0.0f;
    }
)";

std::string const GPUFluid2SolvePoisson::msHLSLPoissonSolveSource =
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

    Texture2D<float> divergence;
    Texture2D<float> poisson;
    RWTexture2D<float> outPoisson;

    [numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
    void CSMain(uint2 c : SV_DispatchThreadID)
    {
        uint2 dim;
        divergence.GetDimensions(dim.x, dim.y);

        int x = int(c.x);
        int y = int(c.y);
        int xm = max(x - 1, 0);
        int xp = min(x + 1, dim.x - 1);
        int ym = max(y - 1, 0);
        int yp = min(y + 1, dim.y - 1);

        // Sample the divergence at (x,y).
        float div = divergence[int2(x, y)];

        // Sample Poisson values at (x+dx,y), (x-dx,y), (x,y+dy), (x,y-dy).
        float poisPZ = poisson[int2(xp, y)];
        float poisMZ = poisson[int2(xm, y)];
        float poisZP = poisson[int2(x, yp)];
        float poisZM = poisson[int2(x, ym)];

        float4 temp = float4(poisPZ + poisMZ, poisZP + poisZM, 0.0f, div);
        outPoisson[c] = dot(epsilon, temp);
    }
)";

std::string const GPUFluid2SolvePoisson::msHLSLPoissonEnforceBoundarySource =
R"(
    #if USE_ZERO_X_EDGE
    RWTexture2D<float> image;

    [numthreads(1, NUM_Y_THREADS, 1)]
    void CSMain(uint2 c : SV_DispatchThreadID)
    {
        uint2 dim;
        image.GetDimensions(dim.x, dim.y);
        image[uint2(0, c.y)] = 0.0f;
        image[uint2(dim.x - 1, c.y)] = 0.0f;
    }
    #endif

    #if USE_ZERO_Y_EDGE
    RWTexture2D<float> image;

    [numthreads(NUM_X_THREADS, 1, 1)]
    void CSMain(uint2 c : SV_DispatchThreadID)
    {
        uint2 dim;
        image.GetDimensions(dim.x, dim.y);
        image[uint2(c.x, 0)] = 0.0f;
        image[uint2(c.x, dim.y - 1)] = 0.0f;
    }
    #endif
)";

ProgramSources const GPUFluid2SolvePoisson::msPoissonZeroSource =
{
    &msGLSLPoissonZeroSource,
    &msHLSLPoissonZeroSource
};

ProgramSources const GPUFluid2SolvePoisson::msPoissonSolveSource =
{
    &msGLSLPoissonSolveSource,
    &msHLSLPoissonSolveSource
};

ProgramSources const GPUFluid2SolvePoisson::msPoissonEnforceBoundarySource =
{
    &msGLSLPoissonEnforceBoundarySource,
    &msHLSLPoissonEnforceBoundarySource
};
