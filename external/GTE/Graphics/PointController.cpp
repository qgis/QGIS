// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/PointController.h>
#include <Graphics/IndexBuffer.h>
#include <Graphics/Visual.h>
#include <Mathematics/Logger.h>
#include <Mathematics/Rotation.h>
using namespace gte;

PointController::PointController(BufferUpdater const& postUpdate)
    :
    systemLinearSpeed(0.0f),
    systemAngularSpeed(0.0f),
    systemLinearAxis(Vector3<float>::Unit(2)),
    systemAngularAxis(Vector3<float>::Unit(2)),
    mPostUpdate(postUpdate)
{
}

bool PointController::Update(double applicationTime)
{
    if (!Controller::Update(applicationTime))
    {
        return false;
    }

    float ctrlTime = static_cast<float>(GetControlTime(applicationTime));

    UpdateSystemMotion(ctrlTime);
    UpdatePointMotion(ctrlTime);
    return true;
}

void PointController::SetObject(ControlledObject* object)
{
    mPointLinearSpeed.clear();
    mPointAngularSpeed.clear();
    mPointLinearAxis.clear();
    mPointAngularAxis.clear();

    auto visual = dynamic_cast<Visual*>(object);
    LogAssert(visual != nullptr, "Object is not of type Visual.");

    auto ibuffer = visual->GetIndexBuffer();
    IPType primitiveType = ibuffer->GetPrimitiveType();
    LogAssert(primitiveType == IP_POLYPOINT, "Geometric primitive must be points.");

    // The vertex buffer for a Visual controlled by a PointController must
    // have 3-tuple or 4-tuple float-valued position that occurs at the
    // beginning (offset 0) of the vertex structure.
    auto vbuffer = visual->GetVertexBuffer();
    VertexFormat vformat = vbuffer->GetFormat();
    int index = vformat.GetIndex(VA_POSITION, 0);
    LogAssert(index >= 0, "Vertex format does not have VA_POSITION.");

    DFType type = vformat.GetType(index);
    LogAssert(type == DF_R32G32B32_FLOAT || type == DF_R32G32B32A32_FLOAT, "Invalid position type.");

    unsigned int offset = vformat.GetOffset(index);
    LogAssert(offset == 0, "Position offset must be 0.");

    // If the vertex buffer for a Visual controlled by a PointController has
    // normal vectors, they must be 3-tuple or 4-tuple float-valued that
    // occurs at an offset >= sizeof(Vector3<float>>).
    index = vformat.GetIndex(VA_NORMAL, 0);
    if (index >= 0)
    {
        type = vformat.GetType(index);
        LogAssert(type == DF_R32G32B32_FLOAT || type == DF_R32G32B32A32_FLOAT, "Invalid normal type.");
    }

    size_t numPoints = static_cast<size_t>(vbuffer->GetNumElements());
    mPointLinearSpeed.resize(numPoints);
    mPointAngularSpeed.resize(numPoints);
    mPointLinearAxis.resize(numPoints);
    mPointAngularAxis.resize(numPoints);
    for (size_t i = 0; i < numPoints; ++i)
    {
        mPointLinearSpeed[i] = 0.0f;
        mPointAngularSpeed[i] = 0.0f;
        mPointLinearAxis[i] = Vector3<float>::Unit(2);
        mPointAngularAxis[i] = Vector3<float>::Unit(2);
    }

    Controller::SetObject(object);
}

void PointController::UpdateSystemMotion(float ctrlTime)
{
    auto visual = static_cast<Visual*>(mObject);

    float distance = ctrlTime * systemLinearSpeed;
    Vector3<float> currentTrn = visual->localTransform.GetTranslation();
    Vector3<float> deltaTrn = distance * systemLinearAxis;
    visual->localTransform.SetTranslation(currentTrn + deltaTrn);

    float angle = ctrlTime * systemAngularSpeed;
    Matrix3x3<float> currentRot;
    visual->localTransform.GetRotation(currentRot);
    Matrix3x3<float> deltaRot =
        Rotation<3, float>(AxisAngle<3, float>(systemAngularAxis, angle));
    visual->localTransform.SetRotation(deltaRot * currentRot);
}

void PointController::UpdatePointMotion(float ctrlTime)
{
    auto visual = static_cast<Visual*>(mObject);
    auto vbuffer = visual->GetVertexBuffer();
    VertexFormat vformat = vbuffer->GetFormat();
    unsigned int numVertices = vbuffer->GetNumElements();
    char* vertices = vbuffer->GetData();
    size_t vertexSize = static_cast<size_t>(vformat.GetVertexSize());

    for (unsigned int i = 0; i < numVertices; ++i)
    {
        Vector3<float>& position = *reinterpret_cast<Vector3<float>*>(vertices);
        float distance = ctrlTime * mPointLinearSpeed[i];
        Vector3<float> deltaTrn = distance * mPointLinearAxis[i];
        position += deltaTrn;
        vertices += vertexSize;
    }

    int index = vformat.GetIndex(VA_NORMAL, 0);
    if (index >= 0)
    {
        unsigned int offset = vformat.GetOffset(index);
        vertices = vbuffer->GetData() + offset;
        for (unsigned int i = 0; i < numVertices; ++i)
        {
            Vector3<float>& normal = *reinterpret_cast<Vector3<float>*>(vertices);
            Normalize(normal);
            float angle = ctrlTime * mPointAngularSpeed[i];
            Matrix3x3<float> deltaRot =
                Rotation<3, float>(AxisAngle<3, float>(mPointAngularAxis[i], angle));
            normal = deltaRot * normal;
            vertices += vertexSize;
        }
    }

    visual->UpdateModelBound();
    visual->UpdateModelNormals();
    mPostUpdate(vbuffer);
}
