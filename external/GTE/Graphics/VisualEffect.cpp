// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/VisualEffect.h>
using namespace gte;

VisualEffect::VisualEffect()
{
    mPVWMatrixConstant = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    SetPVWMatrix(Matrix4x4<float>::Identity());
}

VisualEffect::VisualEffect(std::shared_ptr<VisualProgram> const& program)
    :
    mProgram(program)
{
    mPVWMatrixConstant = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    SetPVWMatrix(Matrix4x4<float>::Identity());
}

void VisualEffect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    mPVWMatrixConstant = buffer;
    SetPVWMatrix(Matrix4x4<float>::Identity());
}
