// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/AreaLightEffect.h>
#include <Graphics/Material.h>
using namespace gte;

AreaLightEffect::AreaLightEffect(std::shared_ptr<ProgramFactory> const& factory,
    std::shared_ptr<Texture2> const& baseTexture,
    std::shared_ptr<Texture2> const& normalTexture, SamplerState::Filter filter,
    SamplerState::Mode mode0, SamplerState::Mode mode1)
    :
    mBaseTexture(baseTexture),
    mNormalTexture(normalTexture)
{
    int api = factory->GetAPI();
    mProgram = factory->CreateFromSources(*msVSSource[api], *msPSSource[api], "");
    if (mProgram)
    {
        // Create the shader constants.  These must be initialized by the
        // application before the first use of the effect.
        mMaterialConstant = std::make_shared<ConstantBuffer>(sizeof(Material), true);
        mCameraConstant = std::make_shared<ConstantBuffer>(sizeof(Vector4<float>), true);
        mAreaLightConstant = std::make_shared<ConstantBuffer>(sizeof(Parameters), true);

        mCommonSampler = std::make_shared<SamplerState>();
        mCommonSampler->filter = filter;
        mCommonSampler->mode[0] = mode0;
        mCommonSampler->mode[1] = mode1;

        std::shared_ptr<Shader> vshader = mProgram->GetVertexShader();
        std::shared_ptr<Shader> pshader = mProgram->GetPixelShader();
        vshader->Set("PVWMatrix", mPVWMatrixConstant);
        pshader->Set("Material", mMaterialConstant);
        pshader->Set("Camera", mCameraConstant);
        pshader->Set("AreaLight", mAreaLightConstant);
        pshader->Set("baseTexture", mBaseTexture, "baseSampler", mCommonSampler);
        pshader->Set("normalTexture", mNormalTexture, "normalSampler", mCommonSampler);
    }
    else
    {
        LogError("Failed to compile shader programs.");
    }

}

void AreaLightEffect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    VisualEffect::SetPVWMatrixConstant(buffer);
    mProgram->GetVertexShader()->Set("PVWMatrix", mPVWMatrixConstant);
}


std::string const AreaLightEffect::msGLSLVSSource =
R"(
    uniform PVWMatrix
    {
        mat4 pvwMatrix;
    };

    layout(location = 0) in vec3 modelPosition;
    layout(location = 1) in vec2 modelTCoord;
    layout(location = 0) out vec4 vertexPosition;
    layout(location = 1) out vec2 vertexTCoord;

    void main()
    {
        vertexPosition = vec4(modelPosition, 1.0f);
        vertexTCoord = modelTCoord;
    #if GTE_USE_MAT_VEC
        gl_Position = pvwMatrix * vertexPosition;
    #else
        gl_Position = vertexPosition * pvwMatrix;
    #endif
    }
)";

std::string const AreaLightEffect::msGLSLPSSource =
R"(
    uniform Material
    {
        // Properties of the surface.
        vec4 materialEmissive;
        vec4 materialAmbient;
        vec4 materialDiffuse;
        vec4 materialSpecular;
    };

    uniform Camera
    {
        vec4 cameraModelPosition;
    };

    uniform AreaLight
    {
        vec4 lightingAmbient;
        vec4 lightingDiffuse;
        vec4 lightingSpecular;
        vec4 lightingAttenuation;

        // The light is rectangular in shape, represented by an oriented bounding
        // rectangle in 3D.  The application is responsible for setting the
        // rectangle members in the model space of the surface to be illuminated.
        vec4 rectPosition;  // (x,y,z,1)
        vec4 rectNormal, rectAxis0, rectAxis1;  // (x,y,z,0)
        vec4 rectExtent;  // (extent0, extent1, *, *)
    };

    uniform sampler2D baseSampler;
    uniform sampler2D normalSampler;

    layout(location = 0) in vec4 vertexPosition;
    layout(location = 1) in vec2 vertexTCoord;
    layout(location = 0) out vec4 pixelColor;

    vec4 ComputeLightModelPosition(in vec4 vertexPosition)
    {
        vec4 diff = vertexPosition - rectPosition;
        vec2 crd = vec2(dot(rectAxis0, diff), dot(rectAxis1, diff));
        crd = max(min(crd, rectExtent.xy), -rectExtent.xy);
        vec4 closest = rectPosition + crd.x * rectAxis0 + crd.y * rectAxis1;
        return closest;
    }

    vec4 lit(float NdotL, float NdotH, float m)
    {
        float ambient = 1.0f;
        float diffuse = max(NdotL, 0.0f);
        float specular = step(0.0f, NdotL) * pow(max(NdotH, 0.0f), m);
        return vec4(ambient, diffuse, specular, 1.0f);
    }

    void main()
    {
        vec4 lightModelPosition = ComputeLightModelPosition(vertexPosition);
        vec3 modelLightDiff = vertexPosition.xyz - lightModelPosition.xyz;
        float distance = length(modelLightDiff);
        float attenuation = lightingAttenuation.w / (lightingAttenuation.x + distance *
            (lightingAttenuation.y + distance * lightingAttenuation.z));

        // This code is specific to the brick texture used by the application.  The
        // brick geometry is in a plane whose normal is consistent with the normal
        // map generated for the brick texture.  Thus, the normal may be looked up
        // directly without computing tangent-space quantities.
        vec3 vertexNormal = texture(normalSampler, vertexTCoord).rgb;
        vertexNormal = normalize(2.0f * vertexNormal - 1.0f);

        vec3 vertexDirection = normalize(modelLightDiff);
        float NDotL = -dot(vertexNormal, vertexDirection);
        vec3 viewVector = normalize(cameraModelPosition.xyz - vertexPosition.xyz);
        vec3 halfVector = normalize(viewVector - vertexDirection);
        float NDotH = dot(vertexNormal, halfVector);
        vec4 lighting = lit(NDotL, NDotH, materialSpecular.a);

        vec3 emissive = materialEmissive.rgb;
        vec3 ambient = materialAmbient.rgb * lightingAmbient.rgb;
        vec4 textureDiffuse = texture(baseSampler, vertexTCoord);
        vec3 diffuse = materialDiffuse.rgb * textureDiffuse.rgb * lightingDiffuse.rgb;
        vec3 specular = materialSpecular.rgb * lightingSpecular.rgb;

        vec3 colorRGB = emissive +
            attenuation * (ambient + lighting.y * diffuse + lighting.z * specular);
        float colorA = materialDiffuse.a * textureDiffuse.a;
        pixelColor = vec4(colorRGB, colorA);
    }
)";

std::string const AreaLightEffect::msHLSLVSSource =
R"(
    cbuffer PVWMatrix
    {
        float4x4 pvwMatrix;
    };

    struct VS_INPUT
    {
        float3 modelPosition : POSITION;
        float2 modelTCoord : TEXCOORD0;
    };

    struct VS_OUTPUT
    {
        float4 vertexPosition : TEXCOORD0;
        float2 vertexTCoord : TEXCOORD1;
        float4 clipPosition : SV_POSITION;
    };

    VS_OUTPUT VSMain(VS_INPUT input)
    {
        VS_OUTPUT output;
        output.vertexPosition = float4(input.modelPosition, 1.0f);
        output.vertexTCoord = input.modelTCoord;
    #if GTE_USE_MAT_VEC
        output.clipPosition = mul(pvwMatrix, output.vertexPosition);
    #else
        output.clipPosition = mul(output.vertexPosition, pvwMatrix);
    #endif
        return output;
    }
)";

std::string const AreaLightEffect::msHLSLPSSource =
R"(
    cbuffer Material
    {
        // Properties of the surface.
        float4 materialEmissive;
        float4 materialAmbient;
        float4 materialDiffuse;
        float4 materialSpecular;
    };

    cbuffer Camera
    {
        float4 cameraModelPosition;
    };

    cbuffer AreaLight
    {
        float4 lightingAmbient;
        float4 lightingDiffuse;
        float4 lightingSpecular;
        float4 lightingAttenuation;

        // The light is rectangular in shape, represented by an oriented bounding
        // rectangle in 3D.  The application is responsible for setting the
        // rectangle members in the model space of the surface to be illuminated.
        float4 rectPosition;  // (x,y,z,1)
        float4 rectNormal, rectAxis0, rectAxis1;  // (x,y,z,0)
        float4 rectExtent;  // (extent0, extent1, *, *)
    };

    Texture2D<float4> baseTexture;
    SamplerState baseSampler;

    Texture2D<float4> normalTexture;
    SamplerState normalSampler;

    struct PS_INPUT
    {
        float4 vertexPosition : TEXCOORD0;
        float2 vertexTCoord : TEXCOORD1;
    };

    struct PS_OUTPUT
    {
        float4 pixelColor : SV_TARGET0;
    };

    float4 ComputeLightModelPosition(in float4 vertexPosition)
    {
        float4 diff = vertexPosition - rectPosition;
        float2 crd = float2(dot(rectAxis0, diff), dot(rectAxis1, diff));
        crd = clamp(crd, -rectExtent.xy, rectExtent.xy);
        float4 closest = rectPosition + crd.x * rectAxis0 + crd.y * rectAxis1;
        return closest;
    }

    PS_OUTPUT PSMain(PS_INPUT input)
    {
        PS_OUTPUT output;

        float4 lightModelPosition = ComputeLightModelPosition(input.vertexPosition);
        float3 modelLightDiff = input.vertexPosition.xyz - lightModelPosition.xyz;
        float distance = length(modelLightDiff);
        float attenuation = lightingAttenuation.w / (lightingAttenuation.x + distance *
            (lightingAttenuation.y + distance * lightingAttenuation.z));

        // This code is specific to the brick texture used by the application.  The
        // brick geometry is in a plane whose normal is consistent with the normal
        // map generated for the brick texture.  Thus, the normal may be looked up
        // directly without computing tangent-space quantities.
        float3 vertexNormal = normalTexture.Sample(normalSampler, input.vertexTCoord).rgb;
        vertexNormal = normalize(2.0f * vertexNormal - 1.0f);

        float3 vertexDirection = normalize(modelLightDiff);
        float NDotL = -dot(vertexNormal, vertexDirection);
        float3 viewVector = normalize(cameraModelPosition.xyz - input.vertexPosition.xyz);
        float3 halfVector = normalize(viewVector - vertexDirection);
        float NDotH = dot(vertexNormal, halfVector);
        float4 lighting = lit(NDotL, NDotH, materialSpecular.a);

        float3 emissive = materialEmissive.rgb;
        float3 ambient = materialAmbient.rgb * lightingAmbient.rgb;
        float4 textureDiffuse = baseTexture.Sample(baseSampler, input.vertexTCoord);
        float3 diffuse = materialDiffuse.rgb * textureDiffuse.rgb * lightingDiffuse.rgb;
        float3 specular = materialSpecular.rgb * lightingSpecular.rgb;

        float3 colorRGB = emissive +
            attenuation * (ambient + lighting.y * diffuse + lighting.z * specular);
        float colorA = materialDiffuse.a * textureDiffuse.a;
        output.pixelColor = float4(colorRGB, colorA);

        return output;
    }
)";

ProgramSources const AreaLightEffect::msVSSource =
{
    &msGLSLVSSource,
    &msHLSLVSSource
};

ProgramSources const AreaLightEffect::msPSSource =
{
    &msGLSLPSSource,
    &msHLSLPSSource
};
