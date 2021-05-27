// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/Picker.h>
#include <Mathematics/Logger.h>
#include <Mathematics/IntrLine3Triangle3.h>
#include <Mathematics/DistLineSegment.h>
#include <Mathematics/DistPointLine.h>
#include <thread>
using namespace gte;

Picker::Picker(unsigned int numThreads)
    :
    mNumThreads(numThreads > 1 ? numThreads : 1),
    mMaxDistance(0.0f),
    mOrigin{ 0.0f, 0.0f, 0.0f, 1.0f },
    mDirection{ 0.0f, 0.0f, 0.0f, 0.0f },
    mTMin(0.0f),
    mTMax(0.0f)
{
}

void Picker::SetMaxDistance(float maxDistance)
{
    mMaxDistance = std::max(maxDistance, 0.0f);
}

float Picker::GetMaxDistance() const
{
    return mMaxDistance;
}

void Picker::operator()(std::shared_ptr<Spatial> const& scene,
    Vector4<float> const& origin, Vector4<float> const& direction, float tmin, float tmax)
{
    if (tmin == -std::numeric_limits<float>::max())
    {
        LogAssert(tmax == std::numeric_limits<float>::max(),
            "The line must have t in (-infinity,infinity)");
    }
    else
    {
        LogAssert(tmin == 0.0f && tmax > 0.0f, "The rat must have t in [0,infinity)");
    }

    mOrigin = origin;
    mDirection = direction;
    mTMin = tmin;
    mTMax = tmax;

    records.clear();
    ExecuteRecursive(scene);
}

PickRecord const& Picker::GetClosestToZero() const
{
    if (records.size() > 0)
    {
        auto iter = records.begin(), end = records.end(), candidate = iter;
        float closest = iter->distanceToLinePoint;
        for (/**/; iter != end; ++iter)
        {
            float tmp = iter->distanceToLinePoint;
            if (tmp < closest)
            {
                closest = tmp;
                candidate = iter;
            }
        }
        return *candidate;
    }
    else
    {
        return msInvalid;
    }
}

PickRecord const& Picker::GetClosestNonnegative() const
{
    if (records.size() > 0)
    {
        // Get first nonnegative value.
        auto iter = records.begin(), end = records.end(), candidate = iter;
        float closest = std::numeric_limits<float>::max();
        for (/**/; iter != end; ++iter)
        {
            if (iter->t >= 0.0f)
            {
                closest = iter->distanceToLinePoint;
                candidate = iter;
                break;
            }
        }

        if (iter != end)
        {
            for (++iter; iter != end; ++iter)
            {
                if (iter->t >= 0.0f && iter->distanceToLinePoint < closest)
                {
                    closest = iter->distanceToLinePoint;
                    candidate = iter;
                }
            }
            return *candidate;
        }
        else
        {
            // All values are negative.
            return msInvalid;
        }
    }
    else
    {
        return msInvalid;
    }
}

PickRecord const& Picker::GetClosestNonpositive() const
{
    if (records.size() > 0)
    {
        // Get first nonpositive value.
        auto iter = records.begin(), end = records.end(), candidate = iter;
        float closest = std::numeric_limits<float>::max();
        for (/**/; iter != end; ++iter)
        {
            if (iter->t <= 0.0f)
            {
                closest = iter->distanceToLinePoint;
                candidate = iter;
                break;
            }
        }

        if (iter != end)
        {
            for (++iter; iter != end; ++iter)
            {
                if (iter->t <= 0.0f && closest < iter->distanceToLinePoint)
                {
                    closest = iter->distanceToLinePoint;
                    candidate = iter;
                }
            }
            return *candidate;
        }
        else
        {
            // All values are positive.
            return msInvalid;
        }
    }
    else
    {
        return msInvalid;
    }
}

void Picker::ExecuteRecursive(std::shared_ptr<Spatial> const& object)
{
    auto visual = std::dynamic_pointer_cast<Visual>(object);
    if (visual)
    {
        if (visual->worldBound.TestIntersection(HProject(mOrigin), HProject(mDirection), mTMin, mTMax))
        {
            // Convert the linear component to model-space coordinates.
            Matrix4x4<float> const& invWorldMatrix = visual->worldTransform.GetHInverse();
            Line3<float> line;
            Vector4<float> temp;
#if defined (GTE_USE_MAT_VEC)
            temp = invWorldMatrix * mOrigin;
            line.origin = { temp[0], temp[1], temp[2] };
            temp = invWorldMatrix * mDirection;
            line.direction = { temp[0], temp[1], temp[2] };
#else
            temp = mOrigin * invWorldMatrix;
            line.origin = { temp[0], temp[1], temp[2] };
            temp = mDirection * invWorldMatrix;
            line.direction = { temp[0], temp[1], temp[2] };
#endif
            // The world transformation might have non-unit scales, in which case the
            // model-space line direction is not unit length.
            Normalize(line.direction);

            // Get the position data.
            VertexBuffer* vbuffer = visual->GetVertexBuffer().get();
            std::set<DFType> required;
            required.insert(DF_R32G32B32_FLOAT);
            required.insert(DF_R32G32B32A32_FLOAT);
            char const* positions = vbuffer->GetChannel(VA_POSITION, 0, required);
            LogAssert(positions != nullptr, "Expecting 3D positions.");

            // The picking algorithm depends on the primitive type.
            unsigned int vstride = vbuffer->GetElementSize();
            IndexBuffer* ibuffer = visual->GetIndexBuffer().get();
            IPType primitiveType = ibuffer->GetPrimitiveType();
            if (primitiveType & IP_HAS_TRIANGLES)
            {
                PickTriangles(visual, positions, vstride, ibuffer, line);
            }
            else if (primitiveType & IP_HAS_SEGMENTS)
            {
                PickSegments(visual, positions, vstride, ibuffer, line);
            }
            else if (primitiveType & IP_HAS_POINTS)
            {
                PickPoints(visual, positions, vstride, ibuffer, line);
            }
        }
        return;
    }

    auto node = std::dynamic_pointer_cast<Node>(object);
    if (node)
    {
        if (node->worldBound.TestIntersection(HProject(mOrigin), HProject(mDirection), mTMin, mTMax))
        {
            int const numChildren = node->GetNumChildren();
            for (int i = 0; i < numChildren; ++i)
            {
                std::shared_ptr<Spatial> child = node->GetChild(i);
                if (child)
                {
                    ExecuteRecursive(child);
                }
            }
        }
        return;
    }

    // We should not get here when the scene graph has only Spatial, Node,
    // and Visual.  However, in case someone adds a new Spatial-derived
    // type later, let's trap it.
    LogError("Invalid object type.");
}


void Picker::PickTriangles(std::shared_ptr<Visual> const& visual, char const* positions,
    unsigned int vstride, IndexBuffer* ibuffer, Line3<float> const& line)
{
    // Partition the items for multiple threads.
    auto const firstTriangle = ibuffer->GetFirstPrimitive();
    auto const numTriangles = ibuffer->GetNumActivePrimitives();
    auto const numThreads = std::min(numTriangles, mNumThreads);

    if (numThreads > 1)
    {
        auto const numPerThread = numTriangles / numThreads;
        std::vector<int> imin(numThreads), imax(numThreads);
        for (unsigned int t = 0; t < numThreads; ++t)
        {
            imin[t] = firstTriangle + t * numPerThread;
            imax[t] = imin[t] + numPerThread - 1;
        }
        imax[numThreads - 1] = firstTriangle + numTriangles - 1;

        // Process blocks of items in multiple threads.
        std::vector<std::thread> process(numThreads);
        std::vector<std::vector<PickRecord>> threadOutputs(numThreads);
        for (unsigned int t = 0; t < numThreads; ++t)
        {
            auto const i0 = imin[t];
            auto const i1 = imax[t];
            process[t] = std::thread(
                [this, t, visual, positions, vstride, ibuffer, line, i0, i1, &threadOutputs]()
                {
                    PickTriangles(visual, positions, vstride, ibuffer, line,
                        i0, i1, threadOutputs[t]);
                });
        }

        // Wait for all threads to finish.
        for (unsigned int t = 0; t < numThreads; ++t)
        {
            process[t].join();
            std::copy(threadOutputs[t].begin(), threadOutputs[t].end(), std::back_inserter(records));
        }
    }
    else
    {
        PickTriangles(visual, positions, vstride, ibuffer, line,
            firstTriangle, firstTriangle + numTriangles - 1, records);
    }
}

void Picker::PickTriangles(std::shared_ptr<Visual> const& visual, char const* positions,
    unsigned int vstride, IndexBuffer* ibuffer, Line3<float> const& line,
    unsigned int i0, unsigned int i1, std::vector<PickRecord>& output) const
{
    // Compute intersections with the model-space triangles.
    bool isIndexed = ibuffer->IsIndexed();
    IPType primitiveType = ibuffer->GetPrimitiveType();
    for (unsigned int i = i0; i <= i1; ++i)
    {
        // Get the vertex indices for the triangle.
        unsigned int v0, v1, v2;
        if (isIndexed)
        {
            ibuffer->GetTriangle(i, v0, v1, v2);
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
        Vector3<float> const& p0 = *(Vector3<float> const*)(positions + v0 * vstride);
        Vector3<float> const& p1 = *(Vector3<float> const*)(positions + v1 * vstride);
        Vector3<float> const& p2 = *(Vector3<float> const*)(positions + v2 * vstride);

        // Create the query triangle in model space.
        Triangle3<float> triangle(p0, p1, p2);

        // Compute line-triangle intersection.
        FIQuery<float, Line3<float>, Triangle3<float>> query;
        auto result = query(line, triangle);
        if (result.intersect
            && mTMin <= result.parameter
            && result.parameter <= mTMax)
        {
            PickRecord record;
            record.visual = visual;
            record.primitiveType = primitiveType;
            record.primitiveIndex = i;
            record.vertexIndex[0] = static_cast<int>(v0);
            record.vertexIndex[1] = static_cast<int>(v1);
            record.vertexIndex[2] = static_cast<int>(v2);
            record.t = result.parameter;
            record.bary[0] = result.triangleBary[0];
            record.bary[1] = result.triangleBary[1];
            record.bary[2] = result.triangleBary[2];
            record.linePoint = HLift(result.point, 1.0f);

#if defined (GTE_USE_MAT_VEC)
            record.linePoint = visual->worldTransform * record.linePoint;
#else
            record.linePoint = record.linePoint * visual->worldTransform;
#endif
            record.primitivePoint = record.linePoint;

            record.distanceToLinePoint =
                Length(record.linePoint - mOrigin);
            record.distanceToPrimitivePoint =
                Length(record.primitivePoint - mOrigin);
            record.distanceBetweenLinePrimitive =
                Length(record.linePoint - record.primitivePoint);

            output.push_back(record);
        }
    }
}

void Picker::PickSegments(std::shared_ptr<Visual> const& visual, char const* positions,
    unsigned int vstride, IndexBuffer* ibuffer, Line3<float> const& line)
{
    // Compute distances from the model-space segments to the line.
    unsigned int const firstSegment = ibuffer->GetFirstPrimitive();
    unsigned int const numSegments = ibuffer->GetNumActivePrimitives();
    bool isIndexed = ibuffer->IsIndexed();
    IPType primitiveType = ibuffer->GetPrimitiveType();
    for (unsigned int i = firstSegment; i < firstSegment + numSegments; ++i)
    {
        // Get the vertex indices for the segment.
        unsigned int v0, v1;
        if (isIndexed)
        {
            ibuffer->GetSegment(i, v0, v1);
        }
        else if (primitiveType == IP_POLYSEGMENT_DISJOINT)
        {
            v0 = 2 * i;
            v1 = v0 + 1;
        }
        else  // primitiveType == IP_POLYSEGMENT_CONTIGUOUS
        {
            v0 = i;
            v1 = v0 + 1;
        }

        // Get the vertex positions.
        auto p0 = *reinterpret_cast<Vector3<float> const*>(positions + v0 * vstride);
        auto p1 = *reinterpret_cast<Vector3<float> const*>(positions + v1 * vstride);

        // Create the query segment in model space.
        Segment3<float> segment(p0, p1);

        // Compute segment-line distance.
        DCPQuery<float, Line3<float>, Segment3<float>> query;
        auto result = query(line, segment);
        if (result.distance <= mMaxDistance && mTMin <= result.parameter[0] && result.parameter[0] <= mTMax)
        {
            PickRecord record;
            record.visual = visual;
            record.primitiveType = primitiveType;
            record.primitiveIndex = i;
            record.vertexIndex[0] = static_cast<int>(v0);
            record.vertexIndex[1] = static_cast<int>(v1);
            record.vertexIndex[2] = -1;
            record.t = result.parameter[0];
            record.bary[0] = 1.0f - result.parameter[1];
            record.bary[1] = result.parameter[1];
            record.bary[2] = 0.0f;
            record.linePoint = HLift(result.closestPoint[0], 1.0f);
            record.primitivePoint = HLift(result.closestPoint[1], 1.0f);

#if defined (GTE_USE_MAT_VEC)
            record.linePoint = visual->worldTransform * record.linePoint;
            record.primitivePoint = visual->worldTransform * record.primitivePoint;
#else
            record.linePoint = record.linePoint * visual->worldTransform;
            record.primitivePoint = record.primitivePoint * visual->worldTransform;
#endif
            record.distanceToLinePoint = Length(record.linePoint - mOrigin);
            record.distanceToPrimitivePoint = Length(record.primitivePoint - mOrigin);
            record.distanceBetweenLinePrimitive = Length(record.linePoint - record.primitivePoint);

            records.push_back(record);
        }
    }
}

void Picker::PickPoints(std::shared_ptr<Visual> const& visual, char const* positions,
    unsigned int vstride, IndexBuffer* ibuffer, Line3<float> const& line)
{
    // Compute distances from the model-space points to the line.
    unsigned int const firstPoint = ibuffer->GetFirstPrimitive();
    unsigned int const numPoints = ibuffer->GetNumActivePrimitives();
    bool isIndexed = ibuffer->IsIndexed();
    for (unsigned int i = firstPoint; i < firstPoint + numPoints; ++i)
    {
        // Get the vertex index for the point.
        unsigned int v;
        if (isIndexed)
        {
            ibuffer->GetPoint(i, v);
        }
        else
        {
            v = i;
        }

        // Get the vertex position.
        auto p = *reinterpret_cast<Vector3<float> const*>(positions + v * vstride);

        // Compute point-line distance.
        DCPQuery<float, Vector3<float>, Line3<float>> query;
        auto result = query(p, line);
        if (result.distance <= mMaxDistance && mTMin <= result.lineParameter && result.lineParameter <= mTMax)
        {
            PickRecord record;
            record.visual = visual;
            record.primitiveType = IP_POLYPOINT;
            record.primitiveIndex = i;
            record.vertexIndex[0] = static_cast<int>(v);
            record.vertexIndex[1] = -1;
            record.vertexIndex[2] = -1;
            record.t = result.lineParameter;
            record.bary[0] = 1.0f;
            record.bary[1] = 0.0f;
            record.bary[2] = 0.0f;
            record.linePoint = HLift(result.lineClosest, 1.0f);
            record.primitivePoint = HLift(p, 1.0f);

#if defined (GTE_USE_MAT_VEC)
            record.linePoint = visual->worldTransform * record.linePoint;
            record.primitivePoint = visual->worldTransform * record.primitivePoint;
#else
            record.linePoint = record.linePoint * visual->worldTransform;
            record.primitivePoint = record.primitivePoint * visual->worldTransform;
#endif
            record.distanceToLinePoint = Length(record.linePoint - mOrigin);
            record.distanceToPrimitivePoint = Length(record.primitivePoint - mOrigin);
            record.distanceBetweenLinePrimitive = Length(record.linePoint - record.primitivePoint);

            records.push_back(record);
        }
    }
}


PickRecord const Picker::msInvalid;
