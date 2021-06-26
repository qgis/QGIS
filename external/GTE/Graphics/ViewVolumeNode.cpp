// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/ViewVolumeNode.h>
using namespace gte;

ViewVolumeNode::ViewVolumeNode(std::shared_ptr<ViewVolume> const& viewVolume)
    :
    mOnUpdate([](ViewVolumeNode*){})
{
    SetViewVolume(viewVolume);
}

void ViewVolumeNode::SetViewVolume(std::shared_ptr<ViewVolume> const& viewVolume)
{
    mViewVolume = viewVolume;

    if (mViewVolume)
    {
        Matrix4x4<float> rotate;
#if defined(GTE_USE_MAT_VEC)
        rotate.SetCol(0, mViewVolume->GetDVector());
        rotate.SetCol(1, mViewVolume->GetUVector());
        rotate.SetCol(2, mViewVolume->GetRVector());
        rotate.SetCol(3, { 0.0f, 0.0f, 0.0f, 1.0f });
#else
        rotate.SetRow(0, mViewVolume->GetDVector());
        rotate.SetRow(1, mViewVolume->GetUVector());
        rotate.SetRow(2, mViewVolume->GetRVector());
        rotate.SetRow(3, { 0.0f, 0.0f, 0.0f, 1.0f });
#endif
        localTransform.SetTranslation(mViewVolume->GetPosition());
        localTransform.SetRotation(rotate);
        Update();
    }
}

void ViewVolumeNode::UpdateWorldData(double applicationTime)
{
    Node::UpdateWorldData(applicationTime);

    if (mViewVolume)
    {
        Vector4<float> position = worldTransform.GetTranslationW1();

        Matrix4x4<float> const& rotate = worldTransform.GetHMatrix();
#if defined(GTE_USE_MAT_VEC)
        Vector4<float> dVector = rotate.GetCol(0);
        Vector4<float> uVector = rotate.GetCol(1);
        Vector4<float> rVector = rotate.GetCol(2);
#else
        Vector4<float> dVector = rotate.GetRow(0);
        Vector4<float> uVector = rotate.GetRow(1);
        Vector4<float> rVector = rotate.GetRow(2);
#endif
        mViewVolume->SetFrame(position, dVector, uVector, rVector);
        mOnUpdate(this);
    }
}
