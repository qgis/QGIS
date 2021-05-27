// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/BlendTransformController.h>
#include <Graphics/Spatial.h>
#include <Mathematics/Rotation.h>
using namespace gte;

BlendTransformController::BlendTransformController(
    std::shared_ptr<TransformController> const& controller0,
    std::shared_ptr<TransformController> const& controller1,
    bool geometricRotation, bool geometricScale)
    :
    TransformController(Transform<float>()),  // the identity transform
    mController0(controller0),
    mController1(controller1),
    mWeight(0.0f),
    mGeometricRotation(geometricRotation),
    mGeometricScale(geometricScale)
{
}

bool BlendTransformController::Update(double applicationTime)
{
    if (!Controller::Update(applicationTime))
    {
        return false;
    }

    mController0->Update(applicationTime);
    mController1->Update(applicationTime);

    Transform<float> const& xfrm0 = mController0->GetTransform();
    Transform<float> const& xfrm1 = mController1->GetTransform();
    float oneMinusWeight = 1.0f - mWeight;

    // Compute the blended translation.
    Vector4<float> trn0 = xfrm0.GetTranslationW1();
    Vector4<float> trn1 = xfrm1.GetTranslationW1();
    Vector4<float> blendTrn = oneMinusWeight * trn0 + mWeight * trn1;
    mLocalTransform.SetTranslation(blendTrn);

    // Compute the blended rotation.
    Matrix4x4<float> const& rot0 = xfrm0.GetRotation();
    Matrix4x4<float> const& rot1 = xfrm1.GetRotation();

    Quaternion<float> quat0 = Rotation<4, float>(rot0);
    Quaternion<float> quat1 = Rotation<4, float>(rot1);
    if (Dot(quat0, quat1) < 0.0f)
    {
        quat1 = -quat1;
    }

    Quaternion<float> blendQuat;
    if (mGeometricRotation)
    {
        blendQuat = Slerp(mWeight, quat0, quat1);
    }
    else
    {
        blendQuat = oneMinusWeight * quat0 + mWeight * quat1;
        Normalize(blendQuat);
    }

    Matrix4x4<float> blendRot = Rotation<4, float>(blendQuat);
    mLocalTransform.SetRotation(blendRot);

    // Compute the blended scale.
    Vector3<float> sca0 = xfrm0.GetScale();
    Vector3<float> sca1 = xfrm1.GetScale();
    Vector3<float> blendSca;
    if (mGeometricScale)
    {
        for (int i = 0; i < 3; ++i)
        {
            float s0 = sca0[i], s1 = sca1[i];
            if (s0 != 0.0f && s1 != 0.0f)
            {
                float sign0 = (s0 > 0.0f ? 1.0f : -1.0f);
                float sign1 = (s1 > 0.0f ? 1.0f : -1.0f);
                s0 = std::fabs(s0);
                s1 = std::fabs(s1);
                float pow0 = std::pow(s0, oneMinusWeight);
                float pow1 = std::pow(s1, mWeight);
                blendSca[i] = sign0 * sign1 * pow0 * pow1;
            }
            else
            {
                blendSca[i] = 0.0f;
            }
        }
    }
    else
    {
        blendSca = oneMinusWeight * sca0 + mWeight * sca1;
    }
    mLocalTransform.SetScale(blendSca);

    auto spatial = static_cast<Spatial*>(mObject);
    spatial->localTransform = mLocalTransform;
    return true;
}

void BlendTransformController::SetObject(ControlledObject* object)
{
    TransformController::SetObject(object);
    mController0->SetObject(object);
    mController1->SetObject(object);
}
