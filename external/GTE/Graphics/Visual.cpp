// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/Visual.h>
#include <Mathematics/Logger.h>
using namespace gte;

Visual::Visual(
    std::shared_ptr<VertexBuffer> const& vbuffer,
    std::shared_ptr<IndexBuffer> const& ibuffer,
    std::shared_ptr<VisualEffect> const& effect)
    :
    mVBuffer(vbuffer),
    mIBuffer(ibuffer),
    mEffect(effect)
{
}

bool Visual::UpdateModelBound()
{
    LogAssert(mVBuffer != nullptr, "Buffer not attached.");

    std::set<DFType> required;
    required.insert(DF_R32G32B32_FLOAT);
    required.insert(DF_R32G32B32A32_FLOAT);
    char const* positions = mVBuffer->GetChannel(VA_POSITION, 0, required);
    if (positions)
    {
        int const numElements = mVBuffer->GetNumElements();
        int const vertexSize = (int)mVBuffer->GetElementSize();
        modelBound.ComputeFromData(numElements, vertexSize, positions);
        return true;
    }

    return false;
}

bool Visual::UpdateModelNormals()
{
    LogAssert(mVBuffer != nullptr && mIBuffer != nullptr, "Buffer not attached.");

    // Get vertex positions.
    std::set<DFType> required;
    required.insert(DF_R32G32B32_FLOAT);
    required.insert(DF_R32G32B32A32_FLOAT);
    char const* positions = mVBuffer->GetChannel(VA_POSITION, 0, required);
    if (!positions)
    {
        return false;
    }

    // Get vertex normals.
    char* normals = mVBuffer->GetChannel(VA_NORMAL, 0, required);
    if (!normals)
    {
        return false;
    }

    // Get triangle primitives.
    IPType primitiveType = mIBuffer->GetPrimitiveType();
    if ((primitiveType & IP_HAS_TRIANGLES) == 0)
    {
        // Normal vectors are not defined for point or segment primitives.
        return false;
    }

    unsigned int const numVertices = mVBuffer->GetNumElements();
    unsigned int const stride = (int)mVBuffer->GetElementSize();
    unsigned int i;
    for (i = 0; i < numVertices; ++i)
    {
        Vector3<float>& normal = *(Vector3<float>*)(normals + i * stride);
        normal = { 0.0f, 0.0f, 0.0f };
    }

    unsigned int const numTriangles = mIBuffer->GetNumPrimitives();
    bool isIndexed = mIBuffer->IsIndexed();
    for (i = 0; i < numTriangles; ++i)
    {
        // Get the vertex indices for the triangle.
        unsigned int v0, v1, v2;
        if (isIndexed)
        {
            mIBuffer->GetTriangle(i, v0, v1, v2);
        }
        else if (primitiveType == IP_TRIMESH)
        {
            v0 = 3 * i;
            v1 = v0 + 1;
            v2 = v0 + 2;
        }
        else  // primitiveType == IP_TRISTRIP
        {
            int offset = (i & 1);
            v0 = i + offset;
            v1 = i + 1 + offset;
            v2 = i + 2 - offset;
        }

        // Get the vertex positions.
        Vector3<float> pos0 = *(Vector3<float>*)(positions + v0 * stride);
        Vector3<float> pos1 = *(Vector3<float>*)(positions + v1 * stride);
        Vector3<float> pos2 = *(Vector3<float>*)(positions + v2 * stride);

        // Compute the triangle normal.  The length of this normal is used
        // in the weighted sum of normals.
        Vector3<float> edge1 = pos1 - pos0;
        Vector3<float> edge2 = pos2 - pos0;
        Vector3<float> normal = Cross(edge1, edge2);

        // Add the triangle normal to the vertices' normal sums.
        Vector3<float>& nor0 = *(Vector3<float>*)(normals + v0 * stride);
        Vector3<float>& nor1 = *(Vector3<float>*)(normals + v1 * stride);
        Vector3<float>& nor2 = *(Vector3<float>*)(normals + v2 * stride);
        nor0 += normal;
        nor1 += normal;
        nor2 += normal;
    }

    // The vertex normals must be unit-length vectors.
    for (i = 0; i < numVertices; ++i)
    {
        Vector3<float>& normal = *(Vector3<float>*)(normals + i * stride);
        if (normal != Vector3<float>::Zero())
        {
            Normalize(normal);
        }
    }

    return true;
}
