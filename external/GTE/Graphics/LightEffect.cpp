// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/LightEffect.h>
using namespace gte;

LightEffect::LightEffect(std::shared_ptr<ProgramFactory> const& factory,
    BufferUpdater const& updater,
    ProgramSources const& vsSource,
    ProgramSources const& psSource,
    std::shared_ptr<Material> const& material,
    std::shared_ptr<Lighting> const& lighting,
    std::shared_ptr<LightCameraGeometry> const& geometry)
    :
    mMaterial(material),
    mLighting(lighting),
    mGeometry(geometry)
{
    int api = factory->GetAPI();
    mProgram = factory->CreateFromSources(*vsSource[api], *psSource[api], "");
    if (mProgram)
    {
        mBufferUpdater = updater;
        mProgram->GetVertexShader()->Set("PVWMatrix", mPVWMatrixConstant);
    }
    else
    {
        LogError("Failed to compile shader programs.");
    }
}

void LightEffect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    VisualEffect::SetPVWMatrixConstant(buffer);
    mProgram->GetVertexShader()->Set("PVWMatrix", mPVWMatrixConstant);
}

void LightEffect::UpdateMaterialConstant()
{
    if (mMaterialConstant)
    {
        mBufferUpdater(mMaterialConstant);
    }
}

void LightEffect::UpdateLightingConstant()
{
    if (mLightingConstant)
    {
        mBufferUpdater(mLightingConstant);
    }
}

void LightEffect::UpdateGeometryConstant()
{
    if (mGeometryConstant)
    {
        mBufferUpdater(mGeometryConstant);
    }
}


// HLSL and Cg have a 'lit' function for computing coefficients of the
// ambient, diffuse and specular lighting contributions.  GLSL does not.
// This string is prepended to any GLSL shader that needs the 'lit' function:
//    ambient = 1;
//    diffuse = ((n dot l) < 0) ? 0 : n dot l;
//    specular = ((n dot l) < 0) || ((n dot h) < 0) ? 0 : (pow(n dot h, m));
// where the vector N is the normal vector, L is the direction to light and H
// is the half vector.  All three vectors are unit length.  The inputs are
// NdotL = Dot(N,L), NdotH = Dot(N,H), and m is the specular exponent that is
// stored in Material:diffuse[3] in GTEngine.
std::string LightEffect::GetGLSLLitFunction()
{
    return
    R"(
        vec4 lit(float NdotL, float NdotH, float m)
        {
          float ambient = 1.0;
          float diffuse = max(NdotL, 0.0);
          float specular = step(0.0, NdotL) * pow(max(NdotH, 0.0), m);
          return vec4(ambient, diffuse, specular, 1.0);
        }
    )";
}
