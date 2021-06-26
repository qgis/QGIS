// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/BumpMapEffect.h>
using namespace gte;

BumpMapEffect::BumpMapEffect(std::shared_ptr<ProgramFactory> const& factory,
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
        mCommonSampler = std::make_shared<SamplerState>();
        mCommonSampler->filter = filter;
        mCommonSampler->mode[0] = mode0;
        mCommonSampler->mode[1] = mode1;

        auto vshader = mProgram->GetVertexShader();
        auto pshader = mProgram->GetPixelShader();
        vshader->Set("PVWMatrix", mPVWMatrixConstant);
        pshader->Set("baseTexture", mBaseTexture, "baseSampler", mCommonSampler);
        pshader->Set("normalTexture", mNormalTexture, "normalSampler", mCommonSampler);
    }
    else
    {
        LogError("Failed to compile shader programs.");
    }
}

void BumpMapEffect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    VisualEffect::SetPVWMatrixConstant(buffer);
    mProgram->GetVertexShader()->Set("PVWMatrix", mPVWMatrixConstant);
}

void BumpMapEffect::ComputeLightVectors(std::shared_ptr<Visual> const& mesh,
    Vector4<float> const& worldLightDirection)
{
    // The tangent-space coordinates for the light direction vector at each
    // vertex is stored in the color0 channel.  The computations use the
    // vertex normals and the texture coordinates for the base mesh, which
    // are stored in the tcoord0 channel.  Thus, the mesh must have positions,
    // normals, colors (unit 0), and texture coordinates (unit 0).  The struct
    // shown next is consistent with mesh->GetVertexFormat().
    struct Vertex
    {
        Vector3<float> position;
        Vector3<float> normal;
        Vector3<float> lightDirection;
        Vector2<float> baseTCoord;
        Vector2<float> normalTCoord;
    };

    // The light direction D is in world-space coordinates.  Negate it,
    // transform it to model-space coordinates, and then normalize it.  The
    // world-space direction is unit-length, but the geometric primitive
    // might have non-unit scaling in its model-to-world transformation, in
    // which case the normalization is necessary.
    Matrix4x4<float> invWMatrix = mesh->worldTransform.GetHInverse();
    Vector4<float> tempDirection = -DoTransform(invWMatrix, worldLightDirection);
    Vector3<float> modelLightDirection = HProject(tempDirection);

    // Set the light vectors to (0,0,0) as a flag that the quantity has not
    // yet been computed.  The probability that a light vector is actually
    // (0,0,0) should be small, so the flag system should save computation
    // time overall.
    auto vbuffer = mesh->GetVertexBuffer();
    unsigned int const numVertices = vbuffer->GetNumElements();
    auto* vertices = vbuffer->Get<Vertex>();
    Vector3<float> const zero{ 0.0f, 0.0f, 0.0f };
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        vertices[i].lightDirection = zero;
    }


    auto ibuffer = mesh->GetIndexBuffer();
    unsigned int numTriangles = ibuffer->GetNumPrimitives();
    auto* indices = ibuffer->Get<unsigned int>();
    for (unsigned int t = 0; t < numTriangles; ++t)
    {
        // Get the triangle vertices and attributes.
        unsigned int v[3];
        v[0] = *indices++;
        v[1] = *indices++;
        v[2] = *indices++;

        for (int i = 0; i < 3; ++i)
        {
            int v0 = v[i];
            if (vertices[v0].lightDirection != zero)
            {
                continue;
            }

            int iP = (i == 0) ? 2 : i - 1;
            int iN = (i + 1) % 3;
            int v1 = v[iN], v2 = v[iP];

            Vector3<float> const& pos0 = vertices[v0].position;
            Vector2<float> const& tcd0 = vertices[v0].baseTCoord;
            Vector3<float> const& pos1 = vertices[v1].position;
            Vector2<float> const& tcd1 = vertices[v1].baseTCoord;
            Vector3<float> const& pos2 = vertices[v2].position;
            Vector2<float> const& tcd2 = vertices[v2].baseTCoord;
            Vector3<float> const& normal = vertices[v0].normal;

            Vector3<float> tangent;
            if (!ComputeTangent(pos0, tcd0, pos1, tcd1, pos2, tcd2, tangent))
            {
                // The texture coordinate mapping is not properly defined for
                // this.  Just say that the tangent space light vector points
                // in the same direction as the surface normal.
                vertices[v0].lightDirection = normal;
                continue;
            }

            // Project T into the tangent plane by projecting out the surface
            // normal N, and then make it unit length.
            tangent -= Dot(normal, tangent) * normal;
            Normalize(tangent);

            // Compute the bitangent B, another tangent perpendicular to T.
            Vector3<float> bitangent = UnitCross(normal, tangent);

            // The set {T,B,N} is a right-handed orthonormal set.  The
            // negated light direction U = -D is represented in this
            // coordinate system as
            //   U = Dot(U,T)*T + Dot(U,B)*B + Dot(U,N)*N
            float dotUT = Dot(modelLightDirection, tangent);
            float dotUB = Dot(modelLightDirection, bitangent);
            float dotUN = Dot(modelLightDirection, normal);

            // Transform the light vector into [0,1]^3 to make it a valid
            // Vector3<float> object.
            vertices[v0].lightDirection =
            {
                0.5f * (dotUT + 1.0f),
                0.5f * (dotUB + 1.0f),
                0.5f * (dotUN + 1.0f)
            };
        }
    }
}

bool BumpMapEffect::ComputeTangent(
    Vector3<float> const& position0, Vector2<float> const& tcoord0,
    Vector3<float> const& position1, Vector2<float> const& tcoord1,
    Vector3<float> const& position2, Vector2<float> const& tcoord2,
    Vector3<float>& tangent)
{
    // Compute the change in positions at the vertex P0.
    Vector3<float> deltaPos1 = position1 - position0;
    Vector3<float> deltaPos2 = position2 - position0;

    float const epsilon = 1e-08f;
    if (Length(deltaPos1) <= epsilon || Length(deltaPos1) <= epsilon )
    {
        // The triangle is degenerate.
        return false;
    }

    // Compute the change in texture coordinates at the vertex P0 in the
    // direction of edge P1-P0.
    float du1 = tcoord1[0] - tcoord0[0];
    float dv1 = tcoord1[1] - tcoord0[1];
    if (std::fabs(dv1) <= epsilon)
    {
        // The triangle effectively has no variation in the v texture
        // coordinate.
        if (std::fabs(du1) <= epsilon)
        {
            // The triangle effectively has no variation in the u coordinate.
            // Since the texture coordinates do not vary on this triangle,
            // treat it as a degenerate parametric surface.
            return false;
        }

        // The variation is effectively all in u, so set the tangent vector
        // to be T = dP/du.
        tangent = deltaPos1 / du1;
        return true;
    }

    // Compute the change in texture coordinates at the vertex P0 in the
    // direction of edge P2-P0.
    float du2 = tcoord2[0] - tcoord0[0];
    float dv2 = tcoord2[1] - tcoord0[1];
    float det = dv1 * du2 - dv2 * du1;
    if (std::fabs(det) <= epsilon)
    {
        // The triangle vertices are collinear in parameter space, so treat
        // this as a degenerate parametric surface.
        return false;
    }

    // The triangle vertices are not collinear in parameter space, so choose
    // the tangent to be dP/du = (dv1*dP2-dv2*dP1)/(dv1*du2-dv2*du1)
    tangent = (dv1 * deltaPos2 - dv2 * deltaPos1) / det;
    return true;
}


std::string const BumpMapEffect::msGLSLVSSource =
R"(
    uniform PVWMatrix
    {
        mat4 pvwMatrix;
    };

    // The BumpMaps sample vertex buffer has a vec3 normal channel, but
    // the shader does not use it.  The location for that channel turns
    // out to be 1, so modelLightDirection must have location 2,
    // modelBaseTCoord must have location 3, and modelNormalTCoord must
    // have location 4.
    layout(location = 0) in vec3 modelPosition;
    layout(location = 1) in vec3 modelNormal;  // unused
    layout(location = 2) in vec3 modelLightDirection;
    layout(location = 3) in vec2 modelBaseTCoord;
    layout(location = 4) in vec2 modelNormalTCoord;
    layout(location = 0) out vec3 vertexLightDirection;
    layout(location = 1) out vec2 vertexBaseTCoord;
    layout(location = 2) out vec2 vertexNormalTCoord;

    void main()
    {
        vertexLightDirection = modelLightDirection;
        vertexBaseTCoord = modelBaseTCoord;
        vertexNormalTCoord = modelNormalTCoord;
    #if GTE_USE_MAT_VEC
        gl_Position = pvwMatrix * vec4(modelPosition, 1.0f);
    #else
        gl_Position = vec4(modelPosition, 1.0f) * pvwMatrix;
    #endif
    }
)";

std::string const BumpMapEffect::msGLSLPSSource =
R"(
    uniform sampler2D baseSampler;
    uniform sampler2D normalSampler;

    layout(location = 0) in vec3 vertexLightDirection;
    layout(location = 1) in vec2 vertexBaseTCoord;
    layout(location = 2) in vec2 vertexNormalTCoord;
    layout(location = 0) out vec4 pixelColor;

    void main()
    {
        vec3 baseColor = texture(baseSampler, vertexBaseTCoord).rgb;
        vec3 normalColor = texture(normalSampler, vertexNormalTCoord).rgb;
        vec3 lightDirection = 2.0f * vertexLightDirection - 1.0f;
        vec3 normalDirection = 2.0f * normalColor - 1.0f;
        lightDirection = normalize(lightDirection);
        normalDirection = normalize(normalDirection);
        float LdN = dot(lightDirection, normalDirection);
        LdN = clamp(LdN, 0.0f, 1.0f);
        pixelColor = vec4(LdN * baseColor, 1.0f);
    }
)";

std::string const BumpMapEffect::msHLSLVSSource =
R"(
    cbuffer PVWMatrix
    {
        float4x4 pvwMatrix;
    };

    struct VS_INPUT
    {
        float3 modelPosition : POSITION;
        float3 modelLightDirection : COLOR;
        float2 modelBaseTCoord : TEXCOORD0;
        float2 modelNormalTCoord : TEXCOORD1;
    };

    struct VS_OUTPUT
    {
        float3 vertexLightDirection : COLOR;
        float2 vertexBaseTCoord : TEXCOORD0;
        float2 vertexNormalTCoord : TEXCORD1;
        float4 clipPosition : SV_POSITION;
    };

    VS_OUTPUT VSMain(VS_INPUT input)
    {
        VS_OUTPUT output;
    #if GTE_USE_MAT_VEC
        output.clipPosition = mul(pvwMatrix, float4(input.modelPosition, 1.0f));
    #else
        output.clipPosition = mul(float4(input.modelPosition, 1.0f), pvwMatrix);
    #endif

        output.vertexLightDirection = input.modelLightDirection;
        output.vertexBaseTCoord = input.modelBaseTCoord;
        output.vertexNormalTCoord = input.modelNormalTCoord;
        return output;
    }
)";

std::string const BumpMapEffect::msHLSLPSSource =
R"(
    Texture2D<float4> baseTexture;
    SamplerState baseSampler;

    Texture2D<float4> normalTexture;
    SamplerState normalSampler;

    struct PS_INPUT
    {
        float3 vertexLightDirection : COLOR;
        float2 vertexBaseTCoord : TEXCOORD0;
        float2 vertexNormalTCoord : TEXCOORD1;
    };

    struct PS_OUTPUT
    {
        float4 pixelColor : SV_TARGET;
    };

    PS_OUTPUT PSMain(PS_INPUT input)
    {
        PS_OUTPUT output;
        float3 baseColor = baseTexture.Sample(baseSampler, input.vertexBaseTCoord).rgb;
        float3 normalColor = normalTexture.Sample(normalSampler, input.vertexNormalTCoord).rgb;
        float3 lightDirection = 2.0f * input.vertexLightDirection - 1.0f;
        float3 normalDirection = 2.0f * normalColor - 1.0f;
        lightDirection = normalize(lightDirection);
        normalDirection = normalize(normalDirection);
        float LdN = dot(lightDirection, normalDirection);
        LdN = saturate(LdN);
        output.pixelColor = float4(LdN * baseColor, 1.0f);
        return output;
    }
)";

ProgramSources const BumpMapEffect::msVSSource =
{
    &msGLSLVSSource,
    &msHLSLVSSource
};

ProgramSources const BumpMapEffect::msPSSource =
{
    &msGLSLPSSource,
    &msHLSLPSSource
};
