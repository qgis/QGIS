// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/PVWUpdater.h>
using namespace gte;

PVWUpdater::PVWUpdater()
{
    Set(nullptr, [](std::shared_ptr<Buffer> const&) {});
}

PVWUpdater::PVWUpdater(std::shared_ptr<Camera> const& camera, BufferUpdater const& updater)
{
    Set(camera, updater);
}

void PVWUpdater::Set(std::shared_ptr<Camera> const& camera, BufferUpdater const& updater)
{
    mCamera = camera;
    mUpdater = updater;
}

bool PVWUpdater::Subscribe(Matrix4x4<float> const& worldMatrix,
    std::shared_ptr<ConstantBuffer> const& cbuffer,
    std::string const& pvwMatrixName)
{
    if (cbuffer && cbuffer->HasMember(pvwMatrixName))
    {
        if (mSubscribers.find(&worldMatrix) == mSubscribers.end())
        {
            mSubscribers.insert(std::make_pair(&worldMatrix,
                std::make_pair(cbuffer, pvwMatrixName)));
            return true;
        }
    }
    return false;
}

bool PVWUpdater::Subscribe(std::shared_ptr<Visual> const& visual,
    std::string const& pvwMatrixName)
{
    if (visual)
    {
        auto const& effect = visual->GetEffect();
        if (effect)
        {
            auto const& worldMatrix = visual->worldTransform.GetHMatrix();
            auto const& cbuffer = effect->GetPVWMatrixConstant();
            return Subscribe(worldMatrix, cbuffer, pvwMatrixName);
        }
    }
    return false;
}

bool PVWUpdater::Unsubscribe(Matrix4x4<float> const& worldMatrix)
{
    return mSubscribers.erase(&worldMatrix) > 0;
}

bool PVWUpdater::Unsubscribe(std::shared_ptr<Visual> const& visual)
{
    if (visual)
    {
        auto const& worldMatrix = visual->worldTransform.GetHMatrix();
        return Unsubscribe(worldMatrix);
    }
    return false;
}

void PVWUpdater::UnsubscribeAll()
{
    mSubscribers.clear();
}

void PVWUpdater::Update()
{
    if (mCamera)
    {
        Matrix4x4<float> pvMatrix = mCamera->GetProjectionViewMatrix();
        for (auto& element : mSubscribers)
        {
            // Compute the new projection-view-world matrix.  The matrix
            // *element.first is the model-to-world matrix for the associated
            // object.
            auto const& wMatrix = *element.first;
#if defined(GTE_USE_MAT_VEC)
            Matrix4x4<float> pvwMatrix = pvMatrix * wMatrix;
#else
            Matrix4x4<float> pvwMatrix = wMatrix * pvMatrix;
#endif
            // Copy the source matrix into the CPU memory of the constant
            // buffer.
            auto const& cbuffer = element.second.first;
            auto const& name = element.second.second;
            cbuffer->SetMember(name, pvwMatrix);

            // Allow the caller to update GPU memory as desired.
            mUpdater(cbuffer);
        }
    }
}

void PVWUpdater::Update(std::vector<Visual*> const& updateSet)
{
    if (mCamera)
    {
        Matrix4x4<float> pvMatrix = mCamera->GetProjectionViewMatrix();
        for (auto& visual : updateSet)
        {
            if (visual)
            {
                auto const& effect = visual->GetEffect();
                if (effect)
                {
                    auto const& wMatrix = visual->worldTransform.GetHMatrix();
                    auto const& cbuffer = effect->GetPVWMatrixConstant();

                    // Compute the new projection-view-world matrix.
#if defined(GTE_USE_MAT_VEC)
                    Matrix4x4<float> pvwMatrix = pvMatrix * wMatrix;
#else
                    Matrix4x4<float> pvwMatrix = wMatrix * pvMatrix;
#endif
                    // Copy the source matrix into the CPU memory of the
                    // constant buffer.
                    effect->SetPVWMatrix(pvwMatrix);

                    // Allow the caller to update GPU memory as desired.
                    mUpdater(cbuffer);
                }
            }
        }
    }
}
