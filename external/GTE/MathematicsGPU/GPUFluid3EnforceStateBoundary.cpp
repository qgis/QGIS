// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.28

#include <MathematicsGPU/GTMathematicsGPUPCH.h>
#include <MathematicsGPU/GPUFluid3EnforceStateBoundary.h>
using namespace gte;

GPUFluid3EnforceStateBoundary::GPUFluid3EnforceStateBoundary(
    std::shared_ptr<ProgramFactory> const& factory,
    int xSize, int ySize, int zSize, int numXThreads, int numYThreads, int numZThreads)
    :
    mNumXGroups(xSize / numXThreads),
    mNumYGroups(ySize / numYThreads),
    mNumZGroups(zSize / numZThreads)
{
    mXMin = std::make_shared<Texture2>(DF_R32G32_FLOAT, ySize, zSize);
    mXMin->SetUsage(Resource::SHADER_OUTPUT);
    mXMax = std::make_shared<Texture2>(DF_R32G32_FLOAT, ySize, zSize);
    mXMax->SetUsage(Resource::SHADER_OUTPUT);
    mYMin = std::make_shared<Texture2>(DF_R32G32_FLOAT, xSize, zSize);
    mYMin->SetUsage(Resource::SHADER_OUTPUT);
    mYMax = std::make_shared<Texture2>(DF_R32G32_FLOAT, xSize, zSize);
    mYMax->SetUsage(Resource::SHADER_OUTPUT);
    mZMin = std::make_shared<Texture2>(DF_R32G32_FLOAT, xSize, ySize);
    mZMin->SetUsage(Resource::SHADER_OUTPUT);
    mZMax = std::make_shared<Texture2>(DF_R32G32_FLOAT, xSize, ySize);
    mZMax->SetUsage(Resource::SHADER_OUTPUT);

    int api = factory->GetAPI();
    factory->PushDefines();
    factory->defines.Set("USE_COPY_X_FACE", 1);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);
    factory->defines.Set("NUM_Z_THREADS", numZThreads);
    mCopyXFace = factory->CreateFromSource(*msSource[api]);
    if (mCopyXFace)
    {
        auto cshader = mCopyXFace->GetComputeShader();
        cshader->Set("xMin", mXMin);
        cshader->Set("xMax", mXMax);
    }

    factory->defines.Clear();
    factory->defines.Set("USE_WRITE_X_FACE", 1);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);
    factory->defines.Set("NUM_Z_THREADS", numZThreads);
    mWriteXFace = factory->CreateFromSource(*msSource[api]);
    if (mWriteXFace)
    {
        auto cshader = mWriteXFace->GetComputeShader();
        cshader->Set("xMin", mXMin);
        cshader->Set("xMax", mXMax);
    }

    factory->defines.Clear();
    factory->defines.Set("USE_COPY_Y_FACE", 1);
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    factory->defines.Set("NUM_Z_THREADS", numZThreads);
    mCopyYFace = factory->CreateFromSource(*msSource[api]);
    if (mCopyYFace)
    {
        auto cshader = mCopyYFace->GetComputeShader();
        cshader->Set("yMin", mYMin);
        cshader->Set("yMax", mYMax);
    }

    factory->defines.Clear();
    factory->defines.Set("USE_WRITE_Y_FACE", 1);
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    factory->defines.Set("NUM_Z_THREADS", numZThreads);
    mWriteYFace = factory->CreateFromSource(*msSource[api]);
    if (mWriteYFace)
    {
        auto cshader = mWriteYFace->GetComputeShader();
        cshader->Set("yMin", mYMin);
        cshader->Set("yMax", mYMax);
    }

    factory->defines.Clear();
    factory->defines.Set("USE_COPY_Z_FACE", 1);
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);
    mCopyZFace = factory->CreateFromSource(*msSource[api]);
    if (mCopyZFace)
    {
        auto cshader = mCopyZFace->GetComputeShader();
        cshader->Set("zMin", mZMin);
        cshader->Set("zMax", mZMax);
    }

    factory->defines.Clear();
    factory->defines.Set("USE_WRITE_Z_FACE", 1);
    factory->defines.Set("NUM_X_THREADS", numXThreads);
    factory->defines.Set("NUM_Y_THREADS", numYThreads);
    mWriteZFace = factory->CreateFromSource(*msSource[api]);
    if (mWriteZFace)
    {
        auto cshader = mWriteZFace->GetComputeShader();
        cshader->Set("zMin", mZMin);
        cshader->Set("zMax", mZMax);
    }

    factory->PopDefines();
}

void GPUFluid3EnforceStateBoundary::Execute(
    std::shared_ptr<GraphicsEngine> const& engine,
    std::shared_ptr<Texture3> const& state)
{
    // in: state
    // out: mXMin, mXMax
    mCopyXFace->GetComputeShader()->Set("state", state);
    engine->Execute(mCopyXFace, 1, mNumYGroups, mNumZGroups);

    // in: mXMin, mXMax
    // out: state
    mWriteXFace->GetComputeShader()->Set("state", state);
    engine->Execute(mWriteXFace, 1, mNumYGroups, mNumZGroups);

    // in: state
    // out: mYMin, mYMax
    mCopyYFace->GetComputeShader()->Set("state", state);
    engine->Execute(mCopyYFace, mNumXGroups, 1, mNumZGroups);

    // in: mYMin, mYMax
    // out: state
    mWriteYFace->GetComputeShader()->Set("state", state);
    engine->Execute(mWriteYFace, mNumXGroups, 1, mNumZGroups);

    // in: state
    // out: mZMin, mZMax
    mCopyZFace->GetComputeShader()->Set("state", state);
    engine->Execute(mCopyZFace, mNumXGroups, mNumYGroups, 1);

    // in: mZMin, mZMax
    // out: state
    mWriteZFace->GetComputeShader()->Set("state", state);
    engine->Execute(mWriteZFace, mNumXGroups, mNumYGroups, 1);
}


std::string const GPUFluid3EnforceStateBoundary::msGLSLSource =
R"(
    #if USE_COPY_X_FACE
    layout(rgba32f) uniform readonly image3D state;
    layout(rg32f) uniform writeonly image2D xMin;
    layout(rg32f) uniform writeonly image2D xMax;

    layout (local_size_x = 1, local_size_y = NUM_Y_THREADS, local_size_z = NUM_Z_THREADS) in;
    void main()
    {
        ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
        ivec3 dim = imageSize(state);
        vec2 xMinValue = imageLoad(state, ivec3(1, c.y, c.z)).yz;
        vec2 xMaxValue = imageLoad(state, ivec3(dim.x - 2, c.y, c.z)).yz;
        imageStore(xMin, c.yz, vec4(xMinValue, 0.0f, 0.0f));
        imageStore(xMax, c.yz, vec4(xMaxValue, 0.0f, 0.0f));
    }
    #endif

    #if USE_WRITE_X_FACE
    layout(rg32f) uniform readonly image2D xMin;
    layout(rg32f) uniform readonly image2D xMax;
    layout(rgba32f) uniform writeonly image3D state;

    layout (local_size_x = 1, local_size_y = NUM_Y_THREADS, local_size_z = NUM_Z_THREADS) in;
    void main()
    {
        ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
        ivec3 dim = imageSize(state);
        vec2 xMinValue = imageLoad(xMin, c.yz).xy;
        vec2 xMaxValue = imageLoad(xMax, c.yz).xy;
        imageStore(state, ivec3(0, c.y, c.z), vec4(0.0f, xMinValue.x, xMinValue.y, 0.0f));
        imageStore(state, ivec3(dim.x - 1, c.y, c.z), vec4(0.0f, xMaxValue.x, xMaxValue.y, 0.0f));
    }
    #endif

    #if USE_COPY_Y_FACE
    layout(rgba32f) uniform readonly image3D state;
    layout(rg32f) uniform writeonly image2D yMin;
    layout(rg32f) uniform writeonly image2D yMax;

    layout (local_size_x = NUM_X_THREADS, local_size_y = 1, local_size_z = NUM_Z_THREADS) in;
    void main()
    {
        ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
        ivec3 dim = imageSize(state);
        vec2 yMinValue = imageLoad(state, ivec3(c.x, 1, c.z)).xz;
        vec2 yMaxValue = imageLoad(state, ivec3(c.x, dim.y - 2, c.z)).xz;
        imageStore(yMin, c.xz, vec4(yMinValue, 0.0f, 0.0f));
        imageStore(yMax, c.xz, vec4(yMaxValue, 0.0f, 0.0f));
    }
    #endif

    #if USE_WRITE_Y_FACE
    layout(rg32f) uniform readonly image2D yMin;
    layout(rg32f) uniform readonly image2D yMax;
    layout(rgba32f) uniform writeonly image3D state;

    layout (local_size_x = NUM_X_THREADS, local_size_y = 1, local_size_z = NUM_Z_THREADS) in;
    void main()
    {
        ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
        ivec3 dim = imageSize(state);
        vec2 yMinValue = imageLoad(yMin, c.xz).xy;
        vec2 yMaxValue = imageLoad(yMax, c.xz).xy;
        imageStore(state, ivec3(c.x, 0, c.z), vec4(yMinValue.x, 0.0f, yMinValue.y, 0.0f));
        imageStore(state, ivec3(c.x, dim.y - 1, c.z), vec4(yMaxValue.x, 0.0f, yMaxValue.y, 0.0f));
    }
    #endif

    #if USE_COPY_Z_FACE
    layout(rgba32f) uniform readonly image3D state;
    layout(rg32f) uniform writeonly image2D zMin;
    layout(rg32f) uniform writeonly image2D zMax;

    layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
    void main()
    {
        ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
        ivec3 dim = imageSize(state);
        vec2 zMinValue = imageLoad(state, ivec3(c.x, c.y, 1)).xy;
        vec2 zMaxValue = imageLoad(state, ivec3(c.x, c.y, dim.z - 2)).xy;
        imageStore(zMin, c.xy, vec4(zMinValue, 0.0f, 0.0f));
        imageStore(zMax, c.xy, vec4(zMaxValue, 0.0f, 0.0f));
    }
    #endif

    #if USE_WRITE_Z_FACE
    layout(rg32f) uniform readonly image2D zMin;
    layout(rg32f) uniform readonly image2D zMax;
    layout(rgba32f) uniform writeonly image3D state;

    layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
    void main()
    {
        ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
        ivec3 dim = imageSize(state);
        vec2 zMinValue = imageLoad(zMin, c.xy).xy;
        vec2 zMaxValue = imageLoad(zMax, c.xy).xy;
        imageStore(state, ivec3(c.x, c.y, 0), vec4(zMinValue.x, zMinValue.y, 0.0f, 0.0f));
        imageStore(state, ivec3(c.x, c.y, dim.z - 1), vec4(zMaxValue.x, zMaxValue.y, 0.0f, 0.0f));
    }
    #endif
)";

std::string const GPUFluid3EnforceStateBoundary::msHLSLSource =
R"(
    #if USE_COPY_X_FACE
    Texture3D<float4> state;
    RWTexture2D<float2> xMin;
    RWTexture2D<float2> xMax;

    [numthreads(1, NUM_Y_THREADS, NUM_Z_THREADS)]
    void CSMain(uint3 c : SV_DispatchThreadID)
    {
        uint3 dim;
        state.GetDimensions(dim.x, dim.y, dim.z);
        xMin[c.yz] = state[uint3(1, c.y, c.z)].yz;
        xMax[c.yz] = state[uint3(dim.x - 2, c.y, c.z)].yz;
    }
    #endif

    #if USE_WRITE_X_FACE
    Texture2D<float2> xMin;
    Texture2D<float2> xMax;
    RWTexture3D<float4> state;

    [numthreads(1, NUM_Y_THREADS, NUM_Z_THREADS)]
    void CSMain(uint3 c : SV_DispatchThreadID)
    {
        uint3 dim;
        state.GetDimensions(dim.x, dim.y, dim.z);
        state[uint3(0, c.y, c.z)] = float4(0.0f, xMin[c.yz].x, xMin[c.yz].y, 0.0f);
        state[uint3(dim.x - 1, c.y, c.z)] = float4(0.0f, xMax[c.yz].x, xMax[c.yz].y, 0.0f);
    }
    #endif

    #if USE_COPY_Y_FACE
    Texture3D<float4> state;
    RWTexture2D<float2> yMin;
    RWTexture2D<float2> yMax;

    [numthreads(NUM_X_THREADS, 1, NUM_Z_THREADS)]
    void CSMain(uint3 c : SV_DispatchThreadID)
    {
        uint3 dim;
        state.GetDimensions(dim.x, dim.y, dim.z);
        yMin[c.xz] = state[uint3(c.x, 1, c.z)].xz;
        yMax[c.xz] = state[uint3(c.x, dim.y - 2, c.z)].xz;
    }
    #endif

    #if USE_WRITE_Y_FACE
    Texture2D<float2> yMin;
    Texture2D<float2> yMax;
    RWTexture3D<float4> state;

    [numthreads(NUM_X_THREADS, 1, NUM_Z_THREADS)]
    void CSMain(uint3 c : SV_DispatchThreadID)
    {
        uint3 dim;
        state.GetDimensions(dim.x, dim.y, dim.z);
        state[uint3(c.x, 0, c.z)] = float4(yMin[c.xz].x, 0.0f, yMin[c.xz].y, 0.0f);
        state[uint3(c.x, dim.y - 1, c.z)] = float4(yMax[c.xz].x, 0.0f, yMax[c.xz].y, 0.0f);
    }
    #endif

    #if USE_COPY_Z_FACE
    Texture3D<float4> state;
    RWTexture2D<float2> zMin;
    RWTexture2D<float2> zMax;

    [numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
    void CSMain(uint3 c : SV_DispatchThreadID)
    {
        uint3 dim;
        state.GetDimensions(dim.x, dim.y, dim.z);
        zMin[c.xy] = state[uint3(c.x, c.y, 1)].xy;
        zMax[c.xy] = state[uint3(c.x, c.y, dim.z - 2)].xy;
    }
    #endif

    #if USE_WRITE_Z_FACE
    Texture2D<float2> zMin;
    Texture2D<float2> zMax;
    RWTexture3D<float4> state;

    [numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
    void CSMain(uint3 c : SV_DispatchThreadID)
    {
        uint3 dim;
        state.GetDimensions(dim.x, dim.y, dim.z);
        state[uint3(c.x, c.y, 0)] = float4(zMin[c.xy].x, zMin[c.xy].y, 0.0f, 0.0f);
        state[uint3(c.x, c.y, dim.z - 1)] = float4(zMax[c.xy].x, zMax[c.xy].y, 0.0f, 0.0f);
    }
    #endif
)";

ProgramSources const GPUFluid3EnforceStateBoundary::msSource =
{
    &msGLSLSource,
    &msHLSLSource
};
