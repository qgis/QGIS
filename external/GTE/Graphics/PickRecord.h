// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/IndexFormat.h>
#include <Mathematics/Vector4.h>
#include <memory>

namespace gte
{
    class Visual;

    class PickRecord
    {
    public:
        // Construction and destruction.  The default constructor does not
        // initialize any members.  It exists to support the construction
        // of the const static PickRecord in the Picker class.
        ~PickRecord() = default;
        PickRecord();

        // The object for which some primitive satisfies the picking criteria.
        // The vertexIndex[] are relative to the vertex buffer array of the
        // 'visual'.  If the primitive is a triangle, then all three indices
        // are valid.  If the primitive is a segment, then the first two
        // indices are valid.  If the primitive is a point, then the first
        // index is valid.  The invalid indices are set to -1.
        std::shared_ptr<Visual> visual;
        IPType primitiveType;
        int primitiveIndex;
        int vertexIndex[3];

        // The linear component is parameterized by P + t*D.  The t-value
        // corresponds to the point of intersection when the primitive is a
        // triangle.  For point and segment primitives, the t-value
        // corresponds to the closest point to the primitive.  NOTE:  Picking
        // occurs in the model space of the objects.  If the model-to-world
        // transform is not rigid (i.e. the world transform has non-unit
        // scaling), the t-values cannot be compared between objects.
        // Therefore, the sorting of PickRecords must use the linePoint,
        // which is computed in world coordinates.
        float t;
        Vector4<float> linePoint;

        // The barycentric coordinates of the point of intersection when the
        // primitive is a triangle or of the closest primitive point when the
        // primitive is a segment or a point.  The coordinates have the
        // properties 0 <= bary[i] <= 1 and bary[0] + bary[1] + bary[2] = 1.
        // For a triangle, all three bary[] values are potentially positive.
        // For a segment, bary[2] = 0.  For a point, bary[0] = 1 and bary[1]
        // = bary[2] = 0.  The order of the bary[] values is consistent with
        // the ordering of the vertices of the primitive.  The primitivePoint
        // is the same as the linePoint when the primitive is a triangle.
        // However, it is the closest primitive point to the line when the
        // primitive is a segment or a point.
        float bary[3];
        Vector4<float> primitivePoint;

        // The distances are all measured in world coordinates.  The
        // distanceTo* values are measured from the world origin of the pick
        // line when the primitives are triangles.  The
        // distanceBetweenLinePrimitive is the distance between the primitive
        // and the pick object that is a line, ray or segment.
        float distanceToLinePoint;
        float distanceToPrimitivePoint;
        float distanceBetweenLinePrimitive;
    };
}
