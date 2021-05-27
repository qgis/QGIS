// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/CubeMapEffect.h>
#include <Graphics/Texture2Effect.h>
using namespace gte;

CubeMapEffect::CubeMapEffect(std::shared_ptr<ProgramFactory> const& factory,
    std::shared_ptr<TextureCube> const& texture, SamplerState::Filter filter,
    SamplerState::Mode mode0, SamplerState::Mode mode1, float reflectivity)
    :
    mCubeTexture(texture),
    mDepthRangeIs01(factory->GetAPI() == ProgramFactory::PF_HLSL),
    mDynamicUpdates(false)
{
    int api = factory->GetAPI();
    mProgram = factory->CreateFromSources(*msVSSource[api], *msPSSource[api], "");
    if (mProgram)
    {

        mWMatrixConstant = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
        SetWMatrix(Matrix4x4<float>::Identity());

        mCameraWorldPositionConstant = std::make_shared<ConstantBuffer>(sizeof(Vector4<float>), true);
        SetCameraWorldPosition(Vector4<float>::Unit(3));

        mReflectivityConstant = std::make_shared<ConstantBuffer>(sizeof(float), true);
        SetReflectivity(reflectivity);

        mCubeSampler = std::make_shared<SamplerState>();
        mCubeSampler->filter = filter;
        mCubeSampler->mode[0] = mode0;
        mCubeSampler->mode[1] = mode1;

        auto vshader = mProgram->GetVertexShader();
        auto pshader = mProgram->GetPixelShader();
        vshader->Set("PVWMatrix", mPVWMatrixConstant);
        vshader->Set("WMatrix", mWMatrixConstant);
        vshader->Set("CameraWorldPosition", mCameraWorldPositionConstant);
        pshader->Set("Reflectivity", mReflectivityConstant);
        pshader->Set("cubeTexture", mCubeTexture, "cubeSampler", mCubeSampler);
    }
    else
    {
        LogError("Failed to compile shader programs.");
    }
}

void CubeMapEffect::UseDynamicUpdates(float dmin, float dmax)
{
    // Create the camera used to draw each of the 6 faces of the cube.
    mCamera = std::make_shared<Camera>(true, mDepthRangeIs01);
    mCamera->SetFrustum(90.0f, 1.0f, dmin, dmax);

    // Create a draw target for the faces.
    mTarget = std::make_shared<DrawTarget>(1, mCubeTexture->GetFormat(),
        mCubeTexture->GetLength(), mCubeTexture->GetLength(), true);
    mTarget->AutogenerateRTMipmaps();
    mTarget->GetRTTexture(0)->SetCopyType(Resource::COPY_STAGING_TO_CPU);

    mDynamicUpdates = true;
}

void CubeMapEffect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    VisualEffect::SetPVWMatrixConstant(buffer);
    mProgram->GetVertexShader()->Set("PVWMatrix", mPVWMatrixConstant);
}

void CubeMapEffect::UpdateFaces(std::shared_ptr<GraphicsEngine> const& engine,
    std::shared_ptr<Spatial> const& scene, Culler& culler,
    Vector4<float> const& envOrigin, Vector4<float> const& envDVector,
    Vector4<float> const& envUVector, Vector4<float> const& envRVector)
{
    std::array<Vector4<float>, 6> dVector =
    {
        -envRVector,
        envRVector,
        envUVector,
        -envUVector,
        envDVector,
        -envDVector
    };

    std::array<Vector4<float>, 6> uVector =
    {
        envUVector,
        envUVector,
        -envDVector,
        envDVector,
        envUVector,
        envUVector
    };

    std::array<Vector4<float>, 6> rVector =
    {
        envDVector,
        -envDVector,
        envRVector,
        envRVector,
        envRVector,
        -envRVector
    };

    // The camera is oriented six times along the coordinate axes and using
    // a frustum with a 90-degree field of view and an aspect ratio of 1 (the
    // cube faces are squares).
    for (unsigned int face = 0; face < 6; ++face)
    {
        mCamera->SetFrame(envOrigin, dVector[face], uVector[face], rVector[face]);
        culler.ComputeVisibleSet(mCamera, scene);

        // We need to update the constant buffers that store pvwMatrices.
        // TODO: For the sample application, we know the effects are Texture2Effect.
        // Generally, do we need the ability to query a VisualEffect object for any
        // constant buffers that store pvwMatrices?
        Matrix4x4<float> pvMatrix = mCamera->GetProjectionViewMatrix();
        for (auto visual : culler.GetVisibleSet())
        {
            auto effect = std::dynamic_pointer_cast<Texture2Effect>(visual->GetEffect());
            if (effect)
            {
                // Compute the new projection-view-world matrix.  The matrix
                // *element.first is the model-to-world matrix for the associated
                // object.
                Matrix4x4<float> wMatrix = visual->worldTransform;
                Matrix4x4<float> pvwMatrix = DoTransform(pvMatrix, wMatrix);
                effect->SetPVWMatrix(pvwMatrix);
                engine->Update(effect->GetPVWMatrixConstant());
            }
        }

        // Draw the scene from the center of the cube to the specified face.
        // The resulting image is stored in the draw target texture.
        engine->Enable(mTarget);
        engine->ClearBuffers();
        for (auto visual : culler.GetVisibleSet())
        {
            engine->Draw(visual);
        }
        engine->Disable(mTarget);

        // Copy the draw target texture to the cube map.  TODO: Implement
        // cube-map render targets to avoid the expensive copies of render
        // target textures to the cube texture.
        auto texture = mTarget->GetRTTexture(0);
        engine->CopyGpuToCpu(texture);

        if (mDepthRangeIs01)
        {
            // DirectX requires this block of code.  The face textures need to
            // be reflected in the u-coordinate to be consistent with the input
            // "*Face.png" cube-map images of this sample.  TODO: Hide this
            // somehow in the graphics engine code to avoid exposing dependency
            // on the graphics API.
            unsigned int const numLevels = texture->GetNumLevels();
            for (unsigned int level = 0; level < numLevels; ++level)
            {
                unsigned int uSize = texture->GetDimensionFor(level, 0);
                unsigned int vSize = texture->GetDimensionFor(level, 1);
                auto input = reinterpret_cast<unsigned int const*>(texture->GetDataFor(level));
                auto output = reinterpret_cast<unsigned int*>(mCubeTexture->GetDataFor(face, level));
                for (unsigned int u = 0, uReflect = uSize - 1; u < uSize; ++u, --uReflect)
                {
                    for (unsigned int v = 0; v < vSize; ++v)
                    {
                        unsigned int src = u + uSize * v;
                        unsigned int trg = uReflect + uSize * v;
                        output[trg] = input[src];
                    }
                }
            }
        }
    }

    engine->CopyCpuToGpu(mCubeTexture);
}


std::string const CubeMapEffect::msGLSLVSSource =
R"(
    uniform PVWMatrix
    {
        mat4 pvwMatrix;
    };

    uniform WMatrix
    {
        mat4 wMatrix;
    };

    uniform CameraWorldPosition
    {
        vec4 cameraWorldPosition;
    };

    layout(location = 0) in vec3 modelPosition;
    layout(location = 1) in vec3 modelNormal;
    layout(location = 2) in vec4 modelColor;

    layout(location = 0) out vec4 vertexColor;
    layout(location = 1) out vec3 cubeTCoord;

    void main()
    {
        vec4 hModelPosition = vec4(modelPosition, 1.0f);
        vec3 worldPosition, worldNormal;
    #if GTE_USE_MAT_VEC
        gl_Position = pvwMatrix * hModelPosition;
        worldPosition = (wMatrix * hModelPosition).xyz;
        worldNormal = normalize(wMatrix * vec4(modelNormal, 0.0f)).xyz;
    #else
        gl_Position = hModelPosition * pvwMatrix;
        worldPosition = (hModelPosition * wMatrix).xyz;
        worldNormal = normalize(vec4(modelNormal, 0.0f) * wMatrix).xyz;
    #endif

        // Calculate the eye direction.  The direction does not have to be
        // normalized, because the texture coordinates for the cube map are
        // invariant to scaling: directions V and s*V for s > 0 generate the
        // same texture coordinates.
        vec3 eyeDirection = worldPosition - cameraWorldPosition.xyz;

        // Calculate the reflected vector.
        cubeTCoord = reflect(eyeDirection, worldNormal);

        // Pass through the model color.
        vertexColor = modelColor;
    }
)";

std::string const CubeMapEffect::msGLSLPSSource =
R"(
    uniform Reflectivity
    {
        float reflectivity;
    };

    uniform samplerCube cubeSampler;

    layout(location = 0) in vec4 vertexColor;
    layout(location = 1) in vec3 cubeTCoord;

    layout(location = 0) out vec4 pixelColor;

    void main()
    {
        vec4 reflectedColor = texture(cubeSampler, cubeTCoord);

        // In HLSL lerp(x,y,s) -> x + s*(y-x)
        // pixelColor = lerp(vertexColor, reflectedColor, reflectivity)
        float s = clamp(reflectivity, 0.0f, 1.0f);
        pixelColor = vertexColor + s * (reflectedColor - vertexColor);
    }
)";

std::string const CubeMapEffect::msHLSLVSSource =
R"(
    cbuffer PVWMatrix
    {
        float4x4 pvwMatrix;
    };

    cbuffer WMatrix
    {
        float4x4 wMatrix;
    };

    cbuffer CameraWorldPosition
    {
        float4 cameraWorldPosition;
    };

    struct VS_INPUT
    {
        float3 modelPosition : POSITION;
        float3 modelNormal : NORMAL;
        float4 modelColor : COLOR;
    };

    struct VS_OUTPUT
    {
        float4 vertexColor : COLOR;
        float3 cubeTCoord : TEXCOORD0;
        float4 clipPosition : SV_POSITION;
    };

    VS_OUTPUT VSMain(VS_INPUT input)
    {
        VS_OUTPUT output;

        float4 hModelPosition = float4(input.modelPosition, 1.0f);
        float3 worldPosition, worldNormal;
    #if GTE_USE_MAT_VEC
        output.clipPosition = mul(pvwMatrix, hModelPosition);
        worldPosition = mul(wMatrix, hModelPosition).xyz;
        worldNormal = normalize(mul(wMatrix, float4(input.modelNormal, 0.0f))).xyz;
    #else
        output.clipPosition = mul(hModelPosition, pvwMatrix);
        worldPosition = mul(hModelPosition, wMatrix).xyz;
        worldNormal = normalize(mul(float4(input.modelNormal, 0.0f), wMatrix)).xyz;
    #endif

        // Calculate the eye direction.  The direction does not have to be
        // normalized, because the texture coordinates for the cube map are
        // invariant to scaling: directions V and s*V for s > 0 generate the
        // same texture coordinates.
        float3 eyeDirection = worldPosition - cameraWorldPosition.xyz;

        // Calculate the reflected vector.
        output.cubeTCoord = reflect(eyeDirection, worldNormal);

        // Pass through the model color.
        output.vertexColor = input.modelColor;

        return output;
    }
)";

std::string const CubeMapEffect::msHLSLPSSource =
R"(
    cbuffer Reflectivity
    {
        float reflectivity;
    };

    TextureCube<float4> cubeTexture;
    SamplerState cubeSampler;

    struct PS_INPUT
    {
        float4 vertexColor : COLOR;
        float3 cubeTCoord : TEXCOORD0;
    };

    struct PS_OUTPUT
    {
        float4 pixelColor : SV_TARGET;
    };

    PS_OUTPUT PSMain(PS_INPUT input)
    {
        PS_OUTPUT output;
        float4 reflectedColor = cubeTexture.Sample(cubeSampler, input.cubeTCoord);
        output.pixelColor = lerp(input.vertexColor, reflectedColor, reflectivity);
        return output;
    }
)";

ProgramSources const CubeMapEffect::msVSSource =
{
    &msGLSLVSSource,
    &msHLSLVSSource
};

ProgramSources const CubeMapEffect::msPSSource =
{
    &msGLSLPSSource,
    &msHLSLPSSource
};
