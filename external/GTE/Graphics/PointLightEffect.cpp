// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/PointLightEffect.h>
using namespace gte;

PointLightEffect::PointLightEffect(std::shared_ptr<ProgramFactory> const& factory,
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

void PointLightEffect::UpdateMaterialConstant()
{
    InternalMaterial* internalMaterial = mMaterialConstant->Get<InternalMaterial>();
    internalMaterial->emissive = mMaterial->emissive;
    internalMaterial->ambient = mMaterial->ambient;
    internalMaterial->diffuse = mMaterial->diffuse;
    internalMaterial->specular = mMaterial->specular;
    LightEffect::UpdateMaterialConstant();
}

void PointLightEffect::UpdateLightingConstant()
{
    InternalLighting* internalLighting = mLightingConstant->Get<InternalLighting>();
    internalLighting->ambient = mLighting->ambient;
    internalLighting->diffuse = mLighting->diffuse;
    internalLighting->specular = mLighting->specular;
    internalLighting->attenuation = mLighting->attenuation;
    LightEffect::UpdateLightingConstant();
}

void PointLightEffect::UpdateGeometryConstant()
{
    InternalGeometry* internalGeometry = mGeometryConstant->Get<InternalGeometry>();
    internalGeometry->lightModelPosition = mGeometry->lightModelPosition;
    internalGeometry->cameraModelPosition = mGeometry->cameraModelPosition;
    LightEffect::UpdateGeometryConstant();
}


std::string const PointLightEffect::msGLSLVSSource[2] =
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
            vec4 lightingAttenuation;
        };
    
        uniform LightCameraGeometry
        {
            vec4 lightModelPosition;
            vec4 cameraModelPosition;
        };
    
        layout(location = 0) in vec3 modelPosition;
        layout(location = 1) in vec3 modelNormal;
        layout(location = 0) out vec4 vertexColor;
    
        void main()
        {
            vec3 modelLightDiff = modelPosition - lightModelPosition.xyz;
            vec3 vertexDirection = normalize(modelLightDiff);
            float NDotL = -dot(modelNormal, vertexDirection);
            vec3 viewVector = normalize(cameraModelPosition.xyz - modelPosition);
            vec3 halfVector = normalize(viewVector - vertexDirection);
            float NDotH = dot(modelNormal, halfVector);
            vec4 lighting = lit(NDotL, NDotH, materialSpecular.a);
    
            float distance = length(modelLightDiff);
            float attenuation = lightingAttenuation.w / (lightingAttenuation.x + distance *
                (lightingAttenuation.y + distance * lightingAttenuation.z));
    
            vec3 color = materialAmbient.rgb * lightingAmbient.rgb +
                lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +
                lighting.z * materialSpecular.rgb * lightingSpecular.rgb;
    
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

std::string const PointLightEffect::msGLSLPSSource[2] =
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
            vec4 lightingAttenuation;
        };
    
        uniform LightCameraGeometry
        {
            vec4 lightModelPosition;
            vec4 cameraModelPosition;
        };
    
        layout(location = 0) in vec3 vertexPosition;
        layout(location = 1) in vec3 vertexNormal;
        layout(location = 0) out vec4 pixelColor;
    
        void main()
        {
            vec3 modelLightDiff = vertexPosition - lightModelPosition.xyz;
            vec3 vertexDirection = normalize(modelLightDiff);
            float NDotL = -dot(vertexNormal, vertexDirection);
            vec3 viewVector = normalize(cameraModelPosition.xyz - vertexPosition.xyz);
            vec3 halfVector = normalize(viewVector - vertexDirection);
            float NDotH = dot(vertexNormal, halfVector);
            vec4 lighting = lit(NDotL, NDotH, materialSpecular.a);
    
            float distance = length(modelLightDiff);
            float attenuation = lightingAttenuation.w / (lightingAttenuation.x + distance *
                (lightingAttenuation.y + distance * lightingAttenuation.z));
    
            vec3 color = materialAmbient.rgb * lightingAmbient.rgb +
                lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +
                lighting.z * materialSpecular.rgb * lightingSpecular.rgb;
    
            pixelColor.rgb = materialEmissive.rgb + attenuation * color;
            pixelColor.a = materialDiffuse.a;
        }
    )"
};

std::string const PointLightEffect::msHLSLVSSource[2] =
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
            float4 lightingAttenuation;
        };
    
        cbuffer LightCameraGeometry
        {
            float4 lightModelPosition;
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
    
            float3 modelLightDiff = input.modelPosition - lightModelPosition.xyz;
            float3 vertexDirection = normalize(modelLightDiff);
            float NDotL = -dot(input.modelNormal, vertexDirection);
            float3 viewVector = normalize(cameraModelPosition.xyz - input.modelPosition);
            float3 halfVector = normalize(viewVector - vertexDirection);
            float NDotH = dot(input.modelNormal, halfVector);
            float4 lighting = lit(NDotL, NDotH, materialSpecular.a);
    
            float distance = length(modelLightDiff);
            float attenuation = lightingAttenuation.w / (lightingAttenuation.x + distance *
                (lightingAttenuation.y + distance * lightingAttenuation.z));
    
            float3 color = materialAmbient.rgb * lightingAmbient.rgb +
                lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +
                lighting.z * materialSpecular.rgb * lightingSpecular.rgb;
    
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

std::string const PointLightEffect::msHLSLPSSource[2] =
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
            float4 lightingAttenuation;
        };
    
        cbuffer LightCameraGeometry
        {
            float4 lightModelPosition;
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
    
            float3 modelLightDiff = input.vertexPosition - lightModelPosition.xyz;
            float3 vertexDirection = normalize(modelLightDiff);
            float NDotL = -dot(input.vertexNormal, vertexDirection);
            float3 viewVector = normalize(cameraModelPosition.xyz - input.vertexPosition.xyz);
            float3 halfVector = normalize(viewVector - vertexDirection);
            float NDotH = dot(input.vertexNormal, halfVector);
            float4 lighting = lit(NDotL, NDotH, materialSpecular.a);
    
            float distance = length(modelLightDiff);
            float attenuation = lightingAttenuation.w / (lightingAttenuation.x + distance *
                (lightingAttenuation.y + distance * lightingAttenuation.z));
    
            float3 color = materialAmbient.rgb * lightingAmbient.rgb +
                lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +
                lighting.z * materialSpecular.rgb * lightingSpecular.rgb;
    
            output.pixelColor.rgb = materialEmissive.rgb + attenuation * color;
            output.pixelColor.a = materialDiffuse.a;
            return output;
        }
    )"
};

ProgramSources const PointLightEffect::msVSSource[2] =
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

ProgramSources const PointLightEffect::msPSSource[2] =
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
