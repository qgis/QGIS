// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/MorphController.h>
#include <Graphics/Visual.h>
#include <Mathematics/Logger.h>
using namespace gte;

MorphController::MorphController(size_t numTargets, size_t numVertices, size_t numTimes,
    BufferUpdater const& postUpdate)
    :
    mNumTargets(numTargets),
    mNumVertices(numVertices),
    mNumTimes(numTimes),
    mVertices(numTargets * numVertices),
    mTimes(numTimes),
    mWeights(numTimes * mNumTargets),
    mLastIndex(0),
    mPostUpdate(postUpdate)
{
    LogAssert(numTargets > 0 && numVertices > 0 && numTimes > 0,
        "Invalid input to MorphController constructor.");
}

void MorphController::SetVertices(size_t target, std::vector<Vector3<float>> const& vertices)
{
    LogAssert(target < mNumTargets && vertices.size() >= mNumVertices,
        "Invalid target or input vertices array is too small.");
    std::copy(vertices.begin(), vertices.end(), mVertices.begin() + target * mNumVertices);
}

void MorphController::SetTimes(std::vector<float> const& times)
{
    LogAssert(times.size() >= mNumTimes, "Input times array is too small.");
    std::copy(times.begin(), times.end(), mTimes.begin());
}

void MorphController::SetWeights(size_t key, std::vector<float> const& weights)
{
    LogAssert(key < mNumTimes && weights.size() >= mNumTargets,
        "Invalid key or input weights array is too small.");
    std::copy(weights.begin(), weights.end(), mWeights.begin() + key * mNumTargets);
}

void MorphController::GetVertices(size_t target, std::vector<Vector3<float>>& vertices)
{
    LogAssert(target < mNumTargets, "Invalid target.");
    vertices.resize(mNumVertices);
    auto begin = mVertices.begin() + target * mNumVertices;
    auto end = begin + mNumVertices;
    std::copy(begin, end, vertices.begin());
}

void MorphController::GetTimes(std::vector<float>& times)
{
    times.resize(mNumTimes);
    std::copy(mTimes.begin(), mTimes.end(), times.begin());
}

void MorphController::GetWeights(size_t key, std::vector<float>& weights)
{
    LogAssert(key < mNumTimes, "Invalid key.");
    weights.resize(mNumTargets);
    auto begin = mWeights.begin() + key * mNumTargets;
    auto end = begin + mNumTargets;
    std::copy(begin, end, weights.begin());
}

bool MorphController::Update(double applicationTime)
{
    // The key interpolation uses linear interpolation.  To get higher-order
    // interpolation, you need to provide a more sophisticated key (Bezier
    // cubic or TCB spline, for example).

    if (!Controller::Update(applicationTime))
    {
        return false;
    }

    // Get access to the vertex buffer to store the blended targets.
    auto visual = static_cast<Visual*>(mObject);
    auto vbuffer = visual->GetVertexBuffer();
    VertexFormat vformat = vbuffer->GetFormat();

    // Initialize the 3-tuple positions (x,y,z) to zero for accumulation.
    unsigned int numVertices = vbuffer->GetNumElements();
    char* combination = vbuffer->GetData();
    size_t vertexSize = static_cast<size_t>(vformat.GetVertexSize());
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        Vector3<float>& vertex = *reinterpret_cast<Vector3<float>*>(combination);
        vertex = { 0.0f, 0.0f, 0.0f };
        combination += vertexSize;
    }

    // Look up the bounding keys.
    float ctrlTime = static_cast<float>(GetControlTime(applicationTime));
    float normTime;
    size_t key0, key1;
    GetKeyInfo(ctrlTime, normTime, key0, key1);
    float oneMinusNormTime = 1.0f - normTime;

    // Compute the weighted combination.
    float const* weights0 = &mWeights[key0 * mNumTargets];
    float const* weights1 = &mWeights[key1 * mNumTargets];
    Vector3<float> const* vertices = mVertices.data();
    float wsum0 = 0.0f, wsum1 = 0.0f;
    for (size_t n = 0; n < mNumTargets; ++n)
    {
        float w = oneMinusNormTime * weights0[n] + normTime * weights1[n];
        wsum0 += weights0[n];
        wsum1 += weights1[n];
        combination = vbuffer->GetData();
        for (size_t m = 0; m < mNumVertices; ++m)
        {
            Vector3<float>& position = *reinterpret_cast<Vector3<float>*>(combination);
            position += w * (*vertices++);
            // += w * mVertices[m + mNumTargets * n];
            combination += vertexSize;
        }
    }

    visual->UpdateModelBound();
    visual->UpdateModelNormals();
    mPostUpdate(vbuffer);
    return true;
}

void MorphController::SetObject(ControlledObject* object)
{
    // Verify that the object satisfies the preconditions that allow a
    // MorphController to be attached to it.
    auto visual = dynamic_cast<Visual*>(object);
    LogAssert(visual != nullptr, "Object is not of type Visual.");

    auto vbuffer = visual->GetVertexBuffer();
    LogAssert(vbuffer->GetNumElements() == mNumVertices, "Mismatch in number of vertices.");

    // The vertex buffer for a Visual controlled by a MorphController must
    // have 3-tuple or 4-tuple float-valued position that occurs at the
    // beginning (offset 0) of the vertex structure.
    VertexFormat vformat = vbuffer->GetFormat();
    int index = vformat.GetIndex(VA_POSITION, 0);
    LogAssert(index >= 0, "Vertex format does not have VA_POSITION.");

    DFType type = vformat.GetType(index);
    LogAssert(type == DF_R32G32B32_FLOAT || type == DF_R32G32B32A32_FLOAT, "Invalid position type.");

    unsigned int offset = vformat.GetOffset(index);
    LogAssert(offset == 0, "Position offset must be 0.");

    Controller::SetObject(object);
}

void MorphController::GetKeyInfo(float ctrlTime, float& normTime, size_t& key0, size_t& key1)
{
    if (ctrlTime <= mTimes[0])
    {
        normTime = 0.0f;
        mLastIndex = 0;
        key0 = 0;
        key1 = 0;
        return;
    }

    if (ctrlTime >= mTimes[mNumTimes - 1])
    {
        normTime = 0.0f;
        mLastIndex = mNumTimes - 1;
        key0 = mLastIndex;
        key1 = mLastIndex;
        return;
    }

    size_t nextIndex;
    if (ctrlTime > mTimes[mLastIndex])
    {
        nextIndex = mLastIndex + 1;
        while (ctrlTime >= mTimes[nextIndex])
        {
            mLastIndex = nextIndex;
            ++nextIndex;
        }

        key0 = mLastIndex;
        key1 = nextIndex;
        normTime = (ctrlTime - mTimes[key0]) / (mTimes[key1] - mTimes[key0]);
    }
    else if (ctrlTime < mTimes[mLastIndex])
    {
        nextIndex = mLastIndex - 1;
        while (ctrlTime <= mTimes[nextIndex])
        {
            mLastIndex = nextIndex;
            --nextIndex;
        }

        key0 = nextIndex;
        key1 = mLastIndex;
        normTime = (ctrlTime - mTimes[key0]) / (mTimes[key1] - mTimes[key0]);
    }
    else
    {
        normTime = 0.0f;
        key0 = mLastIndex;
        key1 = mLastIndex;
    }
}
