// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/ProjectedTextureEffect.h>
using namespace gte;

ProjectedTextureEffect::ProjectedTextureEffect(
    std::shared_ptr<ProgramFactory> const& factory, BufferUpdater const& updater,
    std::shared_ptr<Material> const& material, std::shared_ptr<Lighting> const& lighting,
    std::shared_ptr<LightCameraGeometry> const& geometry,
    std::shared_ptr<Texture2> const& texture, SamplerState::Filter filter,
    SamplerState::Mode mode0, SamplerState::Mode mode1)
    :
    LightEffect(factory, updater, msVSSource, msPSSource, material, lighting, geometry),
    mTexture(texture)
{
    mSampler = std::make_shared<SamplerState>();
    mSampler->filter = filter;
    mSampler->mode[0] = mode0;
    mSampler->mode[1] = mode1;

    mMaterialConstant = std::make_shared<ConstantBuffer>(sizeof(InternalMaterial), true);
    UpdateMaterialConstant();

    mLightingConstant = std::make_shared<ConstantBuffer>(sizeof(InternalLighting), true);
    UpdateLightingConstant();

    mGeometryConstant = std::make_shared<ConstantBuffer>(sizeof(InternalGeometry), true);
    UpdateGeometryConstant();

    mProjectorMatrixConstant = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);

    if (mProgram)
    {
        auto vshader = mProgram->GetVertexShader();
        vshader->Set("ProjectorMatrix", mProjectorMatrixConstant);
        vshader->Set("Material", mMaterialConstant);
        vshader->Set("Lighting", mLightingConstant);
        vshader->Set("LightCameraGeometry", mGeometryConstant);
        mProgram->GetPixelShader()->Set("baseTexture", mTexture, "baseSampler", mSampler);
    }
    else
    {
        LogError("Failed to compile shader programs.");
    }
}

void ProjectedTextureEffect::SetProjectorMatrix(Matrix4x4<float> const& projectorMatrix)
{
    *mProjectorMatrixConstant->Get<Matrix4x4<float>>() = projectorMatrix;
}

void ProjectedTextureEffect::UpdateMaterialConstant()
{
    InternalMaterial* internalMaterial = mMaterialConstant->Get<InternalMaterial>();
    internalMaterial->emissive = mMaterial->emissive;
    internalMaterial->ambient = mMaterial->ambient;
    internalMaterial->diffuse = mMaterial->diffuse;
    internalMaterial->specular = mMaterial->specular;
    LightEffect::UpdateMaterialConstant();
}

void ProjectedTextureEffect::UpdateLightingConstant()
{
    InternalLighting* internalLighting = mLightingConstant->Get<InternalLighting>();
    internalLighting->ambient = mLighting->ambient;
    internalLighting->diffuse = mLighting->diffuse;
    internalLighting->specular = mLighting->specular;
    internalLighting->attenuation = mLighting->attenuation;
    LightEffect::UpdateLightingConstant();
}

void ProjectedTextureEffect::UpdateGeometryConstant()
{
    InternalGeometry* internalGeometry = mGeometryConstant->Get<InternalGeometry>();
    internalGeometry->lightModelDirection = mGeometry->lightModelDirection;
    internalGeometry->cameraModelPosition = mGeometry->cameraModelPosition;
    LightEffect::UpdateGeometryConstant();
}

void ProjectedTextureEffect::UpdateProjectorMatrixConstant()
{
    mBufferUpdater(mProjectorMatrixConstant);
}


std::string const ProjectedTextureEffect::msGLSLVSSource =
LightEffect::GetGLSLLitFunction() +
R"(
    uniform PVWMatrix
    {
        mat4 pvwMatrix;
    };

    uniform ProjectorMatrix
    {
        mat4 projectorMatrix;
    };

    uniform Material
    {
        vec4 materialEmissive;
        vec4 materialAmbient;
        vec4 materialDiffuse;
        vec4 materialSpecular;
    };

    uniform Lighting
    {
        vec4 lightingAmbient;
        vec4 lightingDiffuse;
        vec4 lightingSpecular;
        vec4 lightingAttenuation;
    };

    uniform LightCameraGeometry
    {
        vec4 lightModelDirection;
        vec4 cameraModelPosition;
    };

    layout(location = 0) in vec3 inModelPosition;
    layout(location = 1) in vec3 inModelNormal;
    layout(location = 0) out vec4 vertexColor;
    layout(location = 1) out vec4 projectorTCoord;

    void main()
    {
        float NDotL = -dot(inModelNormal, lightModelDirection.xyz);
        vec3 viewVector = normalize(cameraModelPosition.xyz - inModelPosition);
        vec3 halfVector = normalize(viewVector - lightModelDirection.xyz);
        float NDotH = dot(inModelNormal, halfVector);
        vec4 lighting = lit(NDotL, NDotH, materialSpecular.a);

        vertexColor.rgb = materialEmissive.rgb +
            materialAmbient.rgb * lightingAmbient.rgb +
            lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +
            lighting.z * materialSpecular.rgb * lightingSpecular.rgb;
        vertexColor.a = materialDiffuse.a;

        vec4 modelPosition = vec4(inModelPosition, 1.0f);
    #if GTE_USE_MAT_VEC
        gl_Position = pvwMatrix * modelPosition;
        projectorTCoord = projectorMatrix * modelPosition;
    #else
        gl_Position = modelPosition * pvwMatrix;
        projectorTCoord = modelPosition * projectorMatrix;
    #endif
    }
)";

std::string const ProjectedTextureEffect::msGLSLPSSource =
R"(
    layout(location = 0) in vec4 vertexColor;
    layout(location = 1) in vec4 projectorTCoord;
    layout(location = 0) out vec4 pixelColor;

    uniform sampler2D baseSampler;

    void main()
    {
        vec2 tcoord = projectorTCoord.xy / projectorTCoord.w;
        vec4 baseColor = texture(baseSampler, tcoord);
        pixelColor = baseColor * vertexColor;
    }
)";

std::string const ProjectedTextureEffect::msHLSLVSSource =
R"(
    cbuffer PVWMatrix
    {
        float4x4 pvwMatrix;
    };

    cbuffer ProjectorMatrix
    {
        float4x4 projectorMatrix;
    };

    cbuffer Material
    {
        float4 materialEmissive;
        float4 materialAmbient;
        float4 materialDiffuse;
        float4 materialSpecular;
    };

    cbuffer Lighting
    {
        float4 lightingAmbient;
        float4 lightingDiffuse;
        float4 lightingSpecular;
        float4 lightingAttenuation;
    };

    cbuffer LightCameraGeometry
    {
        float4 lightModelDirection;
        float4 cameraModelPosition;
    };

    struct VS_INPUT
    {
        float3 modelPosition : POSITION;
        float3 modelNormal : NORMAL;
    };

    struct VS_OUTPUT
    {
        float4 vertexColor : COLOR;
        float4 projectorTCoord : TEXCOORD0;
        float4 clipPosition : SV_POSITION;
    };

    VS_OUTPUT VSMain(VS_INPUT input)
    {
        VS_OUTPUT output;

        float NDotL = -dot(input.modelNormal, lightModelDirection.xyz);
        float3 viewVector = normalize(cameraModelPosition.xyz - input.modelPosition);
        float3 halfVector = normalize(viewVector - lightModelDirection.xyz);
        float NDotH = dot(input.modelNormal, halfVector);
        float4 lighting = lit(NDotL, NDotH, materialSpecular.a);

        output.vertexColor.rgb = materialEmissive.rgb +
            materialAmbient.rgb * lightingAmbient.rgb +
            lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +
            lighting.z * materialSpecular.rgb * lightingSpecular.rgb;
        output.vertexColor.a = materialDiffuse.a;

        float4 modelPosition = float4(input.modelPosition, 1.0f);
    #if GTE_USE_MAT_VEC
        output.clipPosition = mul(pvwMatrix, float4(input.modelPosition, 1.0f));
        output.projectorTCoord = mul(projectorMatrix, modelPosition);
    #else
        output.clipPosition = mul(float4(input.modelPosition, 1.0f), pvwMatrix);
        output.projectorTCoord = mul(modelPosition, projectorMatrix);
    #endif
        return output;
    }
)";

std::string const ProjectedTextureEffect::msHLSLPSSource =
R"(
    struct PS_INPUT
    {
        float4 vertexColor : COLOR;
        float4 projectorTCoord : TEXCOORD0;
    };

    struct PS_OUTPUT
    {
        float4 pixelColor : SV_TARGET0;
    };

    Texture2D<float4> baseTexture;
    SamplerState baseSampler;

    PS_OUTPUT PSMain(PS_INPUT input)
    {
        PS_OUTPUT output;
        float2 tcoord = input.projectorTCoord.xy / input.projectorTCoord.w;
        float4 baseColor = baseTexture.Sample(baseSampler, tcoord);
        output.pixelColor = baseColor * input.vertexColor;
        return output;
    }
)";

ProgramSources const ProjectedTextureEffect::msVSSource =
{
    &msGLSLVSSource,
    &msHLSLVSSource
};

ProgramSources const ProjectedTextureEffect::msPSSource =
{
    &msGLSLPSSource,
    &msHLSLPSSource
};
