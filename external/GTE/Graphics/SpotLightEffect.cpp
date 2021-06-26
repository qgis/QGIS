// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/SpotLightEffect.h>
using namespace gte;

SpotLightEffect::SpotLightEffect(std::shared_ptr<ProgramFactory> const& factory,
    BufferUpdater const& updater, int select, std::shared_ptr<Material> const& material,
    std::shared_ptr<Lighting> const& lighting, std::shared_ptr<LightCameraGeometry> const& geometry)
    :
    LightEffect(factory, updater, msVSSource[select & 1], msPSSource[select & 1],
        material, lighting, geometry)
{
    mMaterialConstant = std::make_shared<ConstantBuffer>(sizeof(InternalMaterial), true);
    UpdateMaterialConstant();

    mLightingConstant = std::make_shared<ConstantBuffer>(sizeof(InternalLighting), true);
    UpdateLightingConstant();

    mGeometryConstant = std::make_shared<ConstantBuffer>(sizeof(InternalGeometry), true);
    UpdateGeometryConstant();

    if ((select & 1) == 0)
    {
        auto vshader = mProgram->GetVertexShader();
        vshader->Set("Material", mMaterialConstant);
        vshader->Set("Lighting", mLightingConstant);
        vshader->Set("LightCameraGeometry", mGeometryConstant);
    }
    else
    {
        auto pshader = mProgram->GetPixelShader();
        pshader->Set("Material", mMaterialConstant);
        pshader->Set("Lighting", mLightingConstant);
        pshader->Set("LightCameraGeometry", mGeometryConstant);
    }
}

void SpotLightEffect::UpdateMaterialConstant()
{
    InternalMaterial* internalMaterial = mMaterialConstant->Get<InternalMaterial>();
    internalMaterial->emissive = mMaterial->emissive;
    internalMaterial->ambient = mMaterial->ambient;
    internalMaterial->diffuse = mMaterial->diffuse;
    internalMaterial->specular = mMaterial->specular;
    LightEffect::UpdateMaterialConstant();
}

void SpotLightEffect::UpdateLightingConstant()
{
    InternalLighting* internalLighting = mLightingConstant->Get<InternalLighting>();
    internalLighting->ambient = mLighting->ambient;
    internalLighting->diffuse = mLighting->diffuse;
    internalLighting->specular = mLighting->specular;
    internalLighting->spotCutoff = mLighting->spotCutoff;
    internalLighting->attenuation = mLighting->attenuation;
    LightEffect::UpdateLightingConstant();
}

void SpotLightEffect::UpdateGeometryConstant()
{
    InternalGeometry* internalGeometry = mGeometryConstant->Get<InternalGeometry>();
    internalGeometry->lightModelPosition = mGeometry->lightModelPosition;
    internalGeometry->lightModelDirection = mGeometry->lightModelDirection;
    internalGeometry->cameraModelPosition = mGeometry->cameraModelPosition;
    LightEffect::UpdateGeometryConstant();
}


std::string const SpotLightEffect::msGLSLVSSource[2] =
{
    LightEffect::GetGLSLLitFunction() +
    R"(
        uniform PVWMatrix
        {
            mat4 pvwMatrix;
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
            vec4 lightingSpotCutoff;
            vec4 lightingAttenuation;
        };
    
        uniform LightCameraGeometry
        {
            vec4 lightModelPosition;
            vec4 lightModelDirection;
            vec4 cameraModelPosition;
        };
    
        layout(location = 0) in vec3 modelPosition;
        layout(location = 1) in vec3 modelNormal;
        layout(location = 0) out vec4 vertexColor;
    
        void main()
        {
            vec4 lighting;
            vec3 modelLightDiff = modelPosition - lightModelPosition.xyz;
            vec3 vertexDirection = normalize(modelLightDiff);
            float vertexCosAngle = dot(lightModelDirection.xyz, vertexDirection);
            if (vertexCosAngle >= lightingSpotCutoff.y)
            {
                float NDotL = -dot(modelNormal, vertexDirection);
                vec3 viewVector = normalize(cameraModelPosition.xyz - modelPosition);
                vec3 halfVector = normalize(viewVector - vertexDirection);
                float NDotH = dot(modelNormal, halfVector);
                lighting = lit(NDotL, NDotH, materialSpecular.a);
                lighting.w = pow(abs(vertexCosAngle), lightingSpotCutoff.w);
            }
            else
            {
                lighting = vec4(1.0f, 0.0f, 0.0f, 0.0f);
            }
    
            float distance = length(modelLightDiff);
            float attenuation = lightingAttenuation.w / (lightingAttenuation.x + distance *
                (lightingAttenuation.y + distance * lightingAttenuation.z));
    
            vec3 color = materialAmbient.rgb * lightingAmbient.rgb + lighting.w * (
                lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +
                lighting.z * materialSpecular.rgb * lightingSpecular.rgb);
    
            vertexColor.rgb = materialEmissive.rgb + attenuation * color;
            vertexColor.a = materialDiffuse.a;
        #if GTE_USE_MAT_VEC
            gl_Position = pvwMatrix * vec4(modelPosition, 1.0f);
        #else
            gl_Position = vec4(modelPosition, 1.0f) * pvwMatrix;
        #endif
        }
    )"
    ,
    R"(
        uniform PVWMatrix
        {
            mat4 pvwMatrix;
        };
    
        layout(location = 0) in vec3 modelPosition;
        layout(location = 1) in vec3 modelNormal;
        layout(location = 0) out vec3 vertexPosition;
        layout(location = 1) out vec3 vertexNormal;
    
        void main()
        {
            vertexPosition = modelPosition;
            vertexNormal = modelNormal;
        #if GTE_USE_MAT_VEC
            gl_Position = pvwMatrix * vec4(modelPosition, 1.0f);
        #else
            gl_Position = vec4(modelPosition, 1.0f) * pvwMatrix;
        #endif
        }
    )"
};

std::string const SpotLightEffect::msGLSLPSSource[2] =
{
    R"(
        layout(location = 0) in vec4 vertexColor;
        layout(location = 0) out vec4 pixelColor;
    
        void main()
        {
            pixelColor = vertexColor;
        }
    )"
    ,
    LightEffect::GetGLSLLitFunction() +
    R"(
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
            vec4 lightingSpotCutoff;
            vec4 lightingAttenuation;
        };
    
        uniform LightCameraGeometry
        {
            vec4 lightModelPosition;
            vec4 lightModelDirection;
            vec4 cameraModelPosition;
        };
    
        layout(location = 0) in vec3 vertexPosition;
        layout(location = 1) in vec3 vertexNormal;
        layout(location = 0) out vec4 pixelColor;
    
        void main()
        {
            vec4 lighting;
            vec3 normal = normalize(vertexNormal);
            vec3 modelLightDiff = vertexPosition - lightModelPosition.xyz;
            vec3 vertexDirection = normalize(modelLightDiff);
            float vertexCosAngle = dot(lightModelDirection.xyz, vertexDirection);
            if (vertexCosAngle >= lightingSpotCutoff.y)
            {
                float NDotL = -dot(normal, vertexDirection);
                vec3 viewVector = normalize(cameraModelPosition.xyz - vertexPosition);
                vec3 halfVector = normalize(viewVector - vertexDirection);
                float NDotH = dot(normal, halfVector);
                lighting = lit(NDotL, NDotH, materialSpecular.a);
                lighting.w = pow(abs(vertexCosAngle), lightingSpotCutoff.w);
            }
            else
            {
                lighting = vec4(1.0f, 0.0f, 0.0f, 0.0f);
            }
    
            // Compute the distance-based attenuation.
            float distance = length(modelLightDiff);
            float attenuation = lightingAttenuation.w / (lightingAttenuation.x + distance *
                (lightingAttenuation.y + distance * lightingAttenuation.z));
    
            // Compute the lighting color.
            vec3 color = materialAmbient.rgb * lightingAmbient.rgb + lighting.w * (
                lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +
                lighting.z * materialSpecular.rgb * lightingSpecular.rgb);
    
            // Compute the pixel color.
            pixelColor.rgb = materialEmissive.rgb + attenuation*color;
            pixelColor.a = materialDiffuse.a;
        }
    )"
};

std::string const SpotLightEffect::msHLSLVSSource[2] =
{
    R"(
        cbuffer PVWMatrix
        {
            float4x4 pvwMatrix;
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
            float4 lightingSpotCutoff;
            float4 lightingAttenuation;
        };
    
        cbuffer LightCameraGeometry
        {
            float4 lightModelPosition;
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
            float4 vertexColor : COLOR0;
            float4 clipPosition : SV_POSITION;
        };
    
        VS_OUTPUT VSMain(VS_INPUT input)
        {
            VS_OUTPUT output;
    
            float4 lighting;
            float3 modelLightDiff = input.modelPosition - lightModelPosition.xyz;
            float3 vertexDirection = normalize(modelLightDiff);
            float vertexCosAngle = dot(lightModelDirection.xyz, vertexDirection);
            if (vertexCosAngle >= lightingSpotCutoff.y)
            {
                float NDotL = -dot(input.modelNormal, vertexDirection);
                float3 viewVector = normalize(cameraModelPosition.xyz - input.modelPosition);
                float3 halfVector = normalize(viewVector - vertexDirection);
                float NDotH = dot(input.modelNormal, halfVector);
                lighting = lit(NDotL, NDotH, materialSpecular.a);
                lighting.w = pow(abs(vertexCosAngle), lightingSpotCutoff.w);
            }
            else
            {
                lighting = float4(1.0f, 0.0f, 0.0f, 0.0f);
            }
    
            float distance = length(modelLightDiff);
            float attenuation = lightingAttenuation.w / (lightingAttenuation.x + distance *
                (lightingAttenuation.y + distance * lightingAttenuation.z));
    
            float3 color = materialAmbient.rgb * lightingAmbient.rgb + lighting.w * (
                lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +
                lighting.z * materialSpecular.rgb * lightingSpecular.rgb);
    
            output.vertexColor.rgb = materialEmissive.rgb + attenuation * color;
            output.vertexColor.a = materialDiffuse.a;
        #if GTE_USE_MAT_VEC
            output.clipPosition = mul(pvwMatrix, float4(input.modelPosition, 1.0f));
        #else
            output.clipPosition = mul(float4(input.modelPosition, 1.0f), pvwMatrix);
        #endif
            return output;
        }
    )"
    ,
    R"(
        cbuffer PVWMatrix
        {
            float4x4 pvwMatrix;
        };
    
        struct VS_INPUT
        {
            float3 modelPosition : POSITION;
            float3 modelNormal : NORMAL;
        };
    
        struct VS_OUTPUT
        {
            float3 vertexPosition : TEXCOORD0;
            float3 vertexNormal : TEXCOORD1;
            float4 clipPosition : SV_POSITION;
        };
    
        VS_OUTPUT VSMain(VS_INPUT input)
        {
            VS_OUTPUT output;
    
            output.vertexPosition = input.modelPosition;
            output.vertexNormal = input.modelNormal;
        #if GTE_USE_MAT_VEC
            output.clipPosition = mul(pvwMatrix, float4(input.modelPosition, 1.0f));
        #else
            output.clipPosition = mul(float4(input.modelPosition, 1.0f), pvwMatrix);
        #endif
            return output;
        }
    )"
};

std::string const SpotLightEffect::msHLSLPSSource[2] =
{
    R"(
        struct PS_INPUT
        {
            float4 vertexColor : COLOR0;
        };
    
        struct PS_OUTPUT
        {
            float4 pixelColor : SV_TARGET0;
        };
    
        PS_OUTPUT PSMain(PS_INPUT input)
        {
            PS_OUTPUT output;
            output.pixelColor = input.vertexColor;
            return output;
        }
    )"
    ,
    R"(
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
            float4 lightingSpotCutoff;
            float4 lightingAttenuation;
        };
    
        cbuffer LightCameraGeometry
        {
            float4 lightModelPosition;
            float4 lightModelDirection;
            float4 cameraModelPosition;
        };
    
        struct PS_INPUT
        {
            float3 vertexPosition : TEXCOORD0;
            float3 vertexNormal : TEXCOORD1;
        };
    
        struct PS_OUTPUT
        {
            float4 pixelColor : SV_TARGET0;
        };
    
        PS_OUTPUT PSMain(PS_INPUT input)
        {
            PS_OUTPUT output;
    
            float4 lighting;
            float3 normal = normalize(input.vertexNormal);
            float3 modelLightDiff = input.vertexPosition - lightModelPosition.xyz;
            float3 vertexDirection = normalize(modelLightDiff);
            float vertexCosAngle = dot(lightModelDirection.xyz, vertexDirection);
            if (vertexCosAngle >= lightingSpotCutoff.y)
            {
                float NDotL = -dot(normal, vertexDirection);
                float3 viewVector = normalize(cameraModelPosition.xyz - input.vertexPosition);
                float3 halfVector = normalize(viewVector - vertexDirection);
                float NDotH = dot(normal, halfVector);
                lighting = lit(NDotL, NDotH, materialSpecular.a);
                lighting.w = pow(abs(vertexCosAngle), lightingSpotCutoff.w);
            }
            else
            {
                lighting = float4(1.0f, 0.0f, 0.0f, 0.0f);
            }
    
            // Compute the distance-based attenuation.
            float distance = length(modelLightDiff);
            float attenuation = lightingAttenuation.w / (lightingAttenuation.x + distance *
                (lightingAttenuation.y + distance * lightingAttenuation.z));
    
            // Compute the lighting color.
            float3 color = materialAmbient.rgb * lightingAmbient.rgb + lighting.w * (
                lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +
                lighting.z * materialSpecular.rgb * lightingSpecular.rgb);
    
            // Compute the pixel color.
            output.pixelColor.rgb = materialEmissive.rgb + attenuation*color;
            output.pixelColor.a = materialDiffuse.a;
            return output;
        }
    )"
};

ProgramSources const SpotLightEffect::msVSSource[2] =
{
    {
        &msGLSLVSSource[0],
        &msHLSLVSSource[0]
    },
    {
        &msGLSLVSSource[1],
        &msHLSLVSSource[1]
    }
};

ProgramSources const SpotLightEffect::msPSSource[2] =
{
    {
        &msGLSLPSSource[0],
        &msHLSLPSSource[0]
    },
    {
        &msGLSLPSSource[1],
        &msHLSLPSSource[1]
    }
};
