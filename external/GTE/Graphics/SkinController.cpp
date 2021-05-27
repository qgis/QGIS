// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/SkinController.h>
#include <Graphics/Node.h>
#include <Graphics/Visual.h>
using namespace gte;

SkinController::SkinController(int numVertices, int numBones, BufferUpdater const& postUpdate)
    :
    mNumVertices(numVertices),
    mNumBones(numBones),
    mBones(numBones),
    mWeights(numVertices * numBones),
    mOffsets(numVertices * numBones),
    mPostUpdate(postUpdate),
    mPosition(nullptr),
    mStride(0),
    mFirstUpdate(true),
    mCanUpdate(false)
{
}

bool SkinController::Update(double applicationTime)
{
    if (!Controller::Update(applicationTime))
    {
        return false;
    }

    if (mFirstUpdate)
    {
        mFirstUpdate = false;
        OnFirstUpdate();
    }

    if (mCanUpdate)
    {
        // The skin vertices are calculated in the bone world coordinate system,
        // so the visual's world transform must be the identity.
        auto visual = static_cast<Visual*>(mObject);
        visual->worldTransform = Transform<float>::Identity();
        visual->worldTransformIsCurrent = true;

        // Package the bone transformations into a std::vector to avoid the
        // expensive lock() calls in the inner loop of the position updates.
        std::vector<Matrix4x4<float>> worldTransforms(mNumBones);
        for (int bone = 0; bone < mNumBones; ++bone)
        {
            worldTransforms[bone] = mBones[bone].lock()->worldTransform;
        }

        // Compute the skin vertex locations.  The typecasting to raw 'float'
        // pointers increases the frame rate dramatically, both in Debug and
        // Release builds.  Without this in Debug builds, the lack of inlining
        // of Vector4, Matrix4, std::array and std::vector operator[]
        // functions leads to a low frame rate.  Without this in Release
        // builds, the lack of a highly efficient implementation of operator[]
        // in std::array and std::vector leads to a low frame rate.  Running
        // on an Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz, the Debug frame rate
        // before the typecasting is 16 fps and after the typecasting is
        // 223 fps.  The Release frame rate before the typecasting is 2390 fps
        // and after the typecasting is 4170 fps (sync to vertical retrace is
        // turned off for these experiments).
        char* current = mPosition;
        float const* weights = mWeights.data();
        Vector4<float> const* offsets = mOffsets.data();
        for (int vertex = 0; vertex < mNumVertices; ++vertex)
        {
            Matrix4x4<float> const* worldTransform = worldTransforms.data();
            float position[3] = { 0.0f, 0.0f, 0.0f };
            for (int bone = 0; bone < mNumBones; ++bone, ++weights, ++offsets, ++worldTransform)
            {
                float weight = *weights;
                if (weight != 0.0f)
                {
                    float const* M = reinterpret_cast<float const*>(worldTransform);
                    float const* P = reinterpret_cast<float const*>(offsets);
#if defined (GTE_USE_MAT_VEC)
                    position[0] += weight * (M[0] * P[0] + M[1] * P[1] + M[2] * P[2] + M[3]);
                    position[1] += weight * (M[4] * P[0] + M[5] * P[1] + M[6] * P[2] + M[7]);
                    position[2] += weight * (M[8] * P[0] + M[9] * P[1] + M[10] * P[2] + M[11]);
#else
                    position[0] += weight * (M[0] * P[0] + M[4] * P[1] + M[8] * P[2] + M[12]);
                    position[1] += weight * (M[1] * P[0] + M[5] * P[1] + M[9] * P[2] + M[13]);
                    position[2] += weight * (M[2] * P[0] + M[6] * P[1] + M[10] * P[2] + M[14]);
#endif
                }
            }

            float* target = reinterpret_cast<float*>(current);
            target[0] = position[0];
            target[1] = position[1];
            target[2] = position[2];
            current += mStride;
        }

        visual->UpdateModelBound();
        visual->UpdateModelNormals();
        mPostUpdate(visual->GetVertexBuffer());
        return true;
    }

    return false;
}

void SkinController::OnFirstUpdate()
{
    // Get access to the vertex buffer positions to store the blended targets.
    Visual* visual = reinterpret_cast<Visual*>(mObject);
    VertexBuffer* vbuffer = visual->GetVertexBuffer().get();
    if (mNumVertices == static_cast<int>(vbuffer->GetNumElements()))
    {
        // Get the position data.
        VertexFormat vformat = vbuffer->GetFormat();
        int const numAttributes = vformat.GetNumAttributes();
        for (int i = 0; i < numAttributes; ++i)
        {
            VASemantic semantic;
            DFType type;
            unsigned int unit, offset;
            vformat.GetAttribute(i, semantic, type, unit, offset);
            if (semantic == VA_POSITION && (type == DF_R32G32B32_FLOAT || type == DF_R32G32B32A32_FLOAT))
            {
                mPosition = vbuffer->GetData() + offset;
                mStride = vformat.GetVertexSize();
                mCanUpdate = true;
                break;
            }
        }
    }

    mCanUpdate = (mPosition != nullptr);
}
