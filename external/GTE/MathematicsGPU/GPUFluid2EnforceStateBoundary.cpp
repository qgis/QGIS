// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.28

#include <MathematicsGPU/GTMathematicsGPUPCH.h>
#include <MathematicsGPU/GPUFluid2EnforceStateBoundary.h>
using namespace gte;

GPUFluid2EnforceStateBoundary::GPUFluid2EnforceStateBoundary(
    std::shared_ptr<ProgramFactory> const& factory,
    int xSize, int ySize, int numXThreads, int numYThreads)
    :
    mNumXGroups(xSize / numXThreads),
    mNumYGroups(ySize / numYThreads)
{
    mXMin = std::make_shared<Texture1>(DF_R32_FLOAT, ySize);
    mXMin->SetUsage(Resource::SHADER_OUTPUT);
    mXMax = std::make_shared<Texture1>(DF_R32_FLOAT, ySize);
    mXMax->SetUsage(Resource::SHADER_OUTPUT);
    mYMin = std::make_shared<Texture1>(DF_R32_FLOAT, xSize);
    mYMin->SetUsage(Resource::SHADER_OUTPUT);
    mYMax = std::make_shared<Texture1>(DF_R32_FLOAT, xSize);
    mYMax->SetUsage(Resource::SHADER_OUTPUT);

    int api = factory->GetAPI();
    factory->PushDefines();
    factory->defines.Set("USE_COPY_X_EDGE", 1);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);
    mCopyXEdge = factory->CreateFromSource(*msSource[api]);
    if (mCopyXEdge)
    {
        auto cshader = mCopyXEdge->GetComputeShader();
        cshader->Set("xMin", mXMin);
        cshader->Set("xMax", mXMax);
    }

    factory->defines.Clear();
    factory->defines.Set("USE_WRITE_X_EDGE", 1);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);
    mWriteXEdge = factory->CreateFromSource(*msSource[api]);
    if (mWriteXEdge)
    {
        auto cshader = mWriteXEdge->GetComputeShader();
        cshader->Set("xMin", mXMin);
        cshader->Set("xMax", mXMax);
    }

    factory->defines.Clear();
    factory->defines.Set("USE_COPY_Y_EDGE", 1);
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    mCopyYEdge = factory->CreateFromSource(*msSource[api]);
    if (mCopyYEdge)
    {
        auto cshader = mCopyYEdge->GetComputeShader();
        cshader->Set("yMin", mYMin);
        cshader->Set("yMax", mYMax);
    }

    factory->defines.Clear();
    factory->defines.Set("USE_WRITE_Y_EDGE", 1);
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    mWriteYEdge = factory->CreateFromSource(*msSource[api]);
    if (mWriteYEdge)
    {
        auto cshader = mWriteYEdge->GetComputeShader();
        cshader->Set("yMin", mYMin);
        cshader->Set("yMax", mYMax);
    }

    factory->PopDefines();
}

void GPUFluid2EnforceStateBoundary::Execute(
    std::shared_ptr<GraphicsEngine> const& engine,
    std::shared_ptr<Texture2> const& state)
{
    // in: state
    // out: mXMin, mXMax
    mCopyXEdge->GetComputeShader()->Set("state", state);
    engine->Execute(mCopyXEdge, 1, mNumYGroups, 1);

    // in: mXMin, mXMax
    // out: state
    mWriteXEdge->GetComputeShader()->Set("state", state);
    engine->Execute(mWriteXEdge, 1, mNumYGroups, 1);

    // in: state
    // out: mYMin, mYMax
    mCopyYEdge->GetComputeShader()->Set("state", state);
    engine->Execute(mCopyYEdge, mNumXGroups, 1, 1);

    // in: mYMin, mYMax
    // out: state
    mWriteYEdge->GetComputeShader()->Set("state", state);
    engine->Execute(mWriteYEdge, mNumXGroups, 1, 1);
}


std::string const GPUFluid2EnforceStateBoundary::msGLSLSource =
R"(
    #if USE_COPY_X_EDGE
    layout(rgba32f) uniform readonly image2D state;
    layout(r32f) uniform writeonly image1D xMin;
    layout(r32f) uniform writeonly image1D xMax;

    layout (local_size_x = 1, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
    void main()
    {
        ivec2 c = ivec2(gl_GlobalInvocationID.xy);
        ivec2 dim = imageSize(state);
        float xMinValue = imageLoad(state, ivec2(1, c.y)).y;
        float xMaxValue = imageLoad(state, ivec2(dim.x - 2, c.y)).y;
        imageStore(xMin, c.y, vec4(xMinValue, 0.0f, 0.0f, 0.0f));
        imageStore(xMax, c.y, vec4(xMaxValue, 0.0f, 0.0f, 0.0f));
    }
    #endif

    #if USE_WRITE_X_EDGE
    layout(r32f) uniform readonly image1D xMin;
    layout(r32f) uniform readonly image1D xMax;
    layout(rgba32f) uniform writeonly image2D state;

    layout (local_size_x = 1, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
    void main()
    {
        ivec2 c = ivec2(gl_GlobalInvocationID.xy);
        ivec2 dim = imageSize(state);
        float xMinValue = imageLoad(xMin, c.y).x;
        float xMaxValue = imageLoad(xMax, c.y).x;
        imageStore(state, ivec2(0, c.y), vec4(0.0f, xMinValue, 0.0f, 0.0f));
        imageStore(state, ivec2(dim.x - 1, c.y), vec4(0.0f, xMaxValue, 0.0f, 0.0f));
    }
    #endif

    #if USE_COPY_Y_EDGE
    layout(rgba32f) uniform readonly image2D state;
    layout(r32f) uniform writeonly image1D yMin;
    layout(r32f) uniform writeonly image1D yMax;

    layout (local_size_x = NUM_X_THREADS, local_size_y = 1, local_size_z = 1) in;
    void main()
    {
        ivec2 c = ivec2(gl_GlobalInvocationID.xy);
        ivec2 dim = imageSize(state);
        float yMinValue = imageLoad(state, ivec2(c.x, 1)).x;
        float yMaxValue = imageLoad(state, ivec2(c.x, dim.y - 2)).x;
        imageStore(yMin, c.x, vec4(yMinValue, 0.0f, 0.0f, 0.0f));
        imageStore(yMax, c.x, vec4(yMaxValue, 0.0f, 0.0f, 0.0f));
    }
    #endif

    #if USE_WRITE_Y_EDGE
    layout(r32f) uniform readonly image1D yMin;
    layout(r32f) uniform readonly image1D yMax;
    layout(rgba32f) uniform writeonly image2D state;

    layout (local_size_x = NUM_X_THREADS, local_size_y = 1, local_size_z = 1) in;
    void main()
    {
        ivec2 c = ivec2(gl_GlobalInvocationID.xy);
        ivec2 dim = imageSize(state);
        float yMinValue = imageLoad(yMin, c.x).x;
        float yMaxValue = imageLoad(yMax, c.x).x;
        imageStore(state, ivec2(c.x, 0), vec4(yMinValue, 0.0f, 0.0f, 0.0f));
        imageStore(state, ivec2(c.x, dim.y - 1), vec4(yMaxValue, 0.0f, 0.0f, 0.0f));
    }
    #endif
)";

std::string const GPUFluid2EnforceStateBoundary::msHLSLSource =
R"(
    #if USE_COPY_X_EDGE
    Texture2D<float4> state;
    RWTexture1D<float> xMin;
    RWTexture1D<float> xMax;

    [numthreads(1, NUM_Y_THREADS, 1)]
    void CSMain(uint2 c : SV_DispatchThreadID)
    {
        uint2 dim;
        state.GetDimensions(dim.x, dim.y);
        xMin[c.y] = state[uint2(1, c.y)].y;
        xMax[c.y] = state[uint2(dim.x - 2, c.y)].y;
    }
    #endif

    #if USE_WRITE_X_EDGE
    Texture1D<float> xMin;
    Texture1D<float> xMax;
    RWTexture2D<float4> state;

    [numthreads(1, NUM_Y_THREADS, 1)]
    void CSMain(uint2 c : SV_DispatchThreadID)
    {
        uint2 dim;
        state.GetDimensions(dim.x, dim.y);
        state[uint2(0, c.y)] = float4(0.0f, xMin[c.y], 0.0f, 0.0f);
        state[uint2(dim.x - 1, c.y)] = float4(0.0f, xMax[c.y], 0.0f, 0.0f);
    }
    #endif

    #if USE_COPY_Y_EDGE
    Texture2D<float4> state;
    RWTexture1D<float> yMin;
    RWTexture1D<float> yMax;

    [numthreads(NUM_X_THREADS, 1, 1)]
    void CSMain(uint2 c : SV_DispatchThreadID)
    {
        uint2 dim;
        state.GetDimensions(dim.x, dim.y);
        yMin[c.x] = state[uint2(c.x, 1)].x;
        yMax[c.x] = state[uint2(c.x, dim.y - 2)].x;
    }
    #endif

    #if USE_WRITE_Y_EDGE
    Texture1D<float> yMin;
    Texture1D<float> yMax;
    RWTexture2D<float4> state;

    [numthreads(NUM_X_THREADS, 1, 1)]
    void CSMain(uint2 c : SV_DispatchThreadID)
    {
        uint2 dim;
        state.GetDimensions(dim.x, dim.y);
        state[uint2(c.x, 0)] = float4(yMin[c.x], 0.0f, 0.0f, 0.0f);
        state[uint2(c.x, dim.y - 1)] = float4(yMax[c.x], 0.0f, 0.0f, 0.0f);
    }
    #endif
)";

ProgramSources const GPUFluid2EnforceStateBoundary::msSource =
{
    &msGLSLSource,
    &msHLSLSource
};
