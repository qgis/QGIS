// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.02.25

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/MeshFactory.h>
using namespace gte;

MeshFactory::MeshFactory()
    :
    mIndexSize(sizeof(unsigned int)),
    mVBUsage(Resource::IMMUTABLE),
    mIBUsage(Resource::IMMUTABLE),
    mOutside(true),
    mPositions(nullptr),
    mNormals(nullptr),
    mTangents(nullptr),
    mBitangents(nullptr)
{
    for (int i = 0; i < VA_MAX_TCOORD_UNITS; ++i)
    {
        mAssignTCoords[i] = false;
        mTCoords[i] = nullptr;
    }
}

std::shared_ptr<Visual> MeshFactory::CreateRectangle(unsigned int numXSamples,
    unsigned int numYSamples, float xExtent, float yExtent)
{
    // Quantities derived from inputs.
    float inv0 = 1.0f / (static_cast<float>(numXSamples) - 1.0f);
    float inv1 = 1.0f / (static_cast<float>(numYSamples) - 1.0f);
    unsigned int numVertices = numXSamples * numYSamples;
    unsigned int numTriangles = 2 * (numXSamples - 1) * (numYSamples - 1);

    // Generate geometry.
    std::shared_ptr<VertexBuffer> vbuffer = CreateVBuffer(numVertices);
    if (!vbuffer)
    {
        return nullptr;
    }

    Vector3<float> pos;
    Vector3<float> nor{ 0.0f, 0.0f, 1.0f };
    Vector3<float> tan{ 1.0f, 0.0f, 0.0f };
    Vector3<float> bin{ 0.0f, 1.0f, 0.0f };  // = Cross(nor,tan)
    Vector2<float> tcd;
    pos[2] = 0.0f;
    for (unsigned int i1 = 0, i = 0; i1 < numYSamples; ++i1)
    {
        tcd[1] = i1 * inv1;
        pos[1] = (2.0f * tcd[1] - 1.0f) * yExtent;
        for (unsigned int i0 = 0; i0 < numXSamples; ++i0, ++i)
        {
            tcd[0] = i0 * inv0;
            pos[0] = (2.0f * tcd[0] - 1.0f) * xExtent;

            SetPosition(i, pos);
            SetNormal(i, nor);
            SetTangent(i, tan);
            SetBitangent(i, bin);
            SetTCoord(i, tcd);
        }
    }

    // Generate indices.
    std::shared_ptr<IndexBuffer> ibuffer = CreateIBuffer(numTriangles);
    if (!ibuffer)
    {
        return nullptr;
    }
    for (unsigned int i1 = 0, t = 0; i1 < numYSamples - 1; ++i1)
    {
        for (unsigned int i0 = 0; i0 < numXSamples - 1; ++i0)
        {
            unsigned int v0 = i0 + numXSamples * i1;
            unsigned int v1 = v0 + 1;
            unsigned int v2 = v1 + numXSamples;
            unsigned int v3 = v0 + numXSamples;

            ibuffer->SetTriangle(t++, v0, v1, v2);
            ibuffer->SetTriangle(t++, v0, v2, v3);
        }
    }

    // Create the mesh.
    std::shared_ptr<Visual> visual(new Visual(vbuffer, ibuffer));
    if (visual)
    {
        visual->UpdateModelBound();
    }
    return visual;
}

std::shared_ptr<Visual> MeshFactory::CreateTriangle(unsigned int numSamples,
    float xExtent, float yExtent)
{
    // Quantities derived from inputs.
    float inv = 1.0f / (static_cast<float>(numSamples) - 1.0f);
    unsigned int numVertices = numSamples * (numSamples + 1) / 2;
    unsigned int numTriangles = (numSamples - 1) * (numSamples - 1);

    // Generate geometry.
    auto vbuffer = CreateVBuffer(numVertices);
    if (!vbuffer)
    {
        return nullptr;
    }

    Vector3<float> pos;
    Vector3<float> nor{ 0.0f, 0.0f, 1.0f };
    Vector3<float> tan{ 1.0f, 0.0f, 0.0f };
    Vector3<float> bit{ 0.0f, 1.0f, 0.0f };  // = Cross(nor,tan)
    Vector2<float> tcd;
    pos[2] = 0.0f;
    for (unsigned int i1 = 0, i = 0; i1 < numSamples; ++i1)
    {
        tcd[1] = i1 * inv;
        pos[1] = tcd[1] * yExtent;
        for (unsigned int i0 = 0; i0 + i1 < numSamples; ++i0, ++i)
        {
            tcd[0] = i0 * inv;
            pos[0] = tcd[0] * xExtent;

            SetPosition(i, pos);
            SetNormal(i, nor);
            SetTangent(i, tan);
            SetBitangent(i, bit);
            SetTCoord(i, tcd);
        }
    }

    // Generate indices.
    auto ibuffer = CreateIBuffer(numTriangles);
    if (!ibuffer)
    {
        return nullptr;
    }

    int y0 = 0, y1 = numSamples;
    unsigned int t = 0;
    for (unsigned int i1 = 0; i1 < numSamples - 2; ++i1)
    {
        int bot0 = y0, bot1 = bot0 + 1, top0 = y1, top1 = y1 + 1;
        for (unsigned int i0 = 0; i0 + i1 < numSamples - 2; ++i0)
        {
            ibuffer->SetTriangle(t++, bot0, bot1, top0);
            ibuffer->SetTriangle(t++, bot1, top1, top0);
            bot0 = bot1++;
            top0 = top1++;
        }
        ibuffer->SetTriangle(t++, bot0, bot1, top0);
        y0 = y1;
        y1 = top0 + 1;
    }
    ibuffer->SetTriangle(t++, y0, y0 + 1, y1);

    // Create the mesh.
    auto visual = std::make_shared<Visual>(vbuffer, ibuffer);
    if (visual)
    {
        visual->UpdateModelBound();
    }
    return visual;
}

std::shared_ptr<Visual> MeshFactory::CreateDisk(unsigned int numShellSamples,
    unsigned int numRadialSamples, float radius)
{
    // Quantities derived from inputs.
    unsigned int ssm1 = numShellSamples - 1;
    unsigned int rsm1 = numRadialSamples - 1;
    float invSSm1 = 1.0f / static_cast<float>(ssm1);
    float invRS = 1.0f / static_cast<float>(numRadialSamples);
    unsigned int numVertices = 1 + numRadialSamples * ssm1;
    unsigned int numTriangles = numRadialSamples * (2 * ssm1 - 1);

    // Generate geometry.
    auto vbuffer = CreateVBuffer(numVertices);
    if (!vbuffer)
    {
        return nullptr;
    }

    Vector3<float> pos;
    Vector3<float> nor{ 0.0f, 0.0f, 1.0f };
    Vector3<float> tan{ 1.0f, 0.0f, 0.0f };
    Vector3<float> bit{ 0.0f, 1.0f, 0.0f };  // = Cross(nor,tan)
    Vector2<float> tcd;

    // Center of disk.
    pos = { 0.0f, 0.0f, 0.0f };
    tcd = { 0.5f, 0.5f };
    SetPosition(0, pos);
    SetNormal(0, nor);
    SetTangent(0, tan);
    SetBitangent(0, bit);
    SetTCoord(0, tcd);

    for (unsigned int r = 0; r < numRadialSamples; ++r)
    {
        float angle = invRS * r * static_cast<float>(GTE_C_TWO_PI);
        float cs = std::cos(angle);
        float sn = std::sin(angle);
        Vector3<float> radial{ cs, sn, 0.0f };

        for (unsigned int s = 1; s < numShellSamples; ++s)
        {
            float fraction = invSSm1 * s;  // in (0,R]
            Vector3<float> fracRadial = fraction * radial;
            unsigned int i = s + ssm1 * r;
            pos = radius * fracRadial;
            tcd[0] = 0.5f + 0.5f * fracRadial[0];
            tcd[1] = 0.5f + 0.5f * fracRadial[1];

            SetPosition(i, pos);
            SetNormal(i, nor);
            SetTangent(i, tan);
            SetBitangent(i, bit);
            SetTCoord(i, tcd);
        }
    }

    // Generate indices.
    auto ibuffer = CreateIBuffer(numTriangles);
    if (!ibuffer)
    {
        return nullptr;
    }
    for (unsigned int r0 = rsm1, r1 = 0, t = 0; r1 < numRadialSamples; r0 = r1++)
    {
        ibuffer->SetTriangle(t++, 0, 1 + ssm1 * r0, 1 + ssm1 * r1);

        for (unsigned int s = 1; s < ssm1; ++s)
        {
            unsigned int i00 = s + ssm1 * r0;
            unsigned int i01 = s + ssm1 * r1;
            unsigned int i10 = i00 + 1;
            unsigned int i11 = i01 + 1;

            ibuffer->SetTriangle(t++, i00, i10, i11);
            ibuffer->SetTriangle(t++, i00, i11, i01);
        }
    }

    // Create the mesh.
    auto visual = std::make_shared<Visual>(vbuffer, ibuffer);
    if (visual)
    {
        visual->UpdateModelBound();
    }
    return visual;
}

std::shared_ptr<Visual> MeshFactory::CreateBox(float xExtent, float yExtent, float zExtent)
{
    // Quantities derived from inputs.
    int numVertices = 8;
    int numTriangles = 12;

    // Generate geometry.
    auto vbuffer = CreateVBuffer(numVertices);
    if (!vbuffer)
    {
        return nullptr;
    }

    Vector3<float> pos, nor;
    std::array<Vector3<float>, 3> basis;
    Vector2<float> tcd;

    // Choose vertex normals in the diagonal directions.
    Vector3<float> diag{ xExtent, yExtent, zExtent };
    Normalize(diag);
    if (!mOutside)
    {
        diag = -diag;
    }

    for (unsigned int z = 0, v = 0; z < 2; ++z)
    {
        float fz = static_cast<float>(z), omfz = 1.0f - fz;
        float zSign = 2.0f * fz - 1.0f;
        pos[2] = zSign * zExtent;
        nor[2] = zSign * diag[2];
        for (unsigned int y = 0; y < 2; ++y)
        {
            float fy = static_cast<float>(y);
            float ySign = 2.0f * fy - 1.0f;
            pos[1] = ySign * yExtent;
            nor[1] = ySign * diag[1];
            tcd[1] = (1.0f - fy) * omfz + (0.75f - 0.5f * fy) * fz;
            for (unsigned int x = 0; x < 2; ++x, ++v)
            {
                float fx = static_cast<float>(x);
                float xSign = 2.0f * fx - 1.0f;
                pos[0] = xSign * xExtent;
                nor[0] = xSign * diag[0];
                tcd[0] = fx * omfz + (0.25f + 0.5f * fx) * fz;

                basis[0] = nor;
                ComputeOrthogonalComplement(1, basis.data());

                SetPosition(v, pos);
                SetNormal(v, nor);
                SetTangent(v, basis[1]);
                SetBitangent(v, basis[2]);
                SetTCoord(v, tcd);
            }
        }
    }

    // Generate indices (outside view).
    auto ibuffer = CreateIBuffer(numTriangles);
    if (!ibuffer)
    {
        return nullptr;
    }
    ibuffer->SetTriangle(0, 0, 2, 3);
    ibuffer->SetTriangle( 1, 0, 3, 1);
    ibuffer->SetTriangle( 2, 0, 1, 5);
    ibuffer->SetTriangle( 3, 0, 5, 4);
    ibuffer->SetTriangle( 4, 0, 4, 6);
    ibuffer->SetTriangle( 5, 0, 6, 2);
    ibuffer->SetTriangle( 6, 7, 6, 4);
    ibuffer->SetTriangle( 7, 7, 4, 5);
    ibuffer->SetTriangle( 8, 7, 5, 1);
    ibuffer->SetTriangle( 9, 7, 1, 3);
    ibuffer->SetTriangle(10, 7, 3, 2);
    ibuffer->SetTriangle(11, 7, 2, 6);
    if (!mOutside)
    {
        ReverseTriangleOrder(ibuffer.get());
    }

    // Create the mesh.
    auto visual = std::make_shared<Visual>(vbuffer, ibuffer);
    if (visual)
    {
        visual->UpdateModelBound();
    }
    return visual;
}

std::shared_ptr<Visual> MeshFactory::CreateCylinderOpen(unsigned int numAxisSamples,
    unsigned int numRadialSamples, float radius, float height)
{
    // Quantities derived from inputs.
    unsigned int numVertices = numAxisSamples * (numRadialSamples + 1);
    unsigned int numTriangles = 2 * (numAxisSamples - 1) * numRadialSamples;
    float invRS = 1.0f / static_cast<float>(numRadialSamples);
    float invASm1 = 1.0f / static_cast<float>(numAxisSamples - 1);
    float halfHeight = 0.5f * height;

    // Generate geometry.
    auto vbuffer = CreateVBuffer(numVertices);
    if (!vbuffer)
    {
        return nullptr;
    }

    Vector3<float> pos, nor;
    std::array<Vector3<float>, 3> basis;
    Vector2<float> tcd;

    // Generate points on the unit circle to be used in computing the mesh
    // points on a cylinder slice.
    std::vector<float> cs(numRadialSamples + 1);
    std::vector<float> sn(numRadialSamples + 1);
    for (unsigned int r = 0; r < numRadialSamples; ++r)
    {
        float angle = invRS * r * static_cast<float>(GTE_C_TWO_PI);
        cs[r] = std::cos(angle);
        sn[r] = std::sin(angle);
    }
    cs[numRadialSamples] = cs[0];
    sn[numRadialSamples] = sn[0];

    // Generate the cylinder itself.
    for (unsigned int a = 0, i = 0; a < numAxisSamples; ++a)
    {
        float axisFraction = a * invASm1;  // in [0,1]
        float z = -halfHeight + height * axisFraction;

        // Compute center of slice.
        Vector3<float> sliceCenter{ 0.0f, 0.0f, z };

        // Compute slice vertices with duplication at endpoint.
        for (unsigned int r = 0; r <= numRadialSamples; ++r, ++i)
        {
            float radialFraction = r * invRS;  // in [0,1)
            nor = { cs[r], sn[r], 0.0f };
            pos = sliceCenter + radius * nor;
            if (!mOutside)
            {
                nor = -nor;
            }

            basis[0] = nor;
            ComputeOrthogonalComplement(1, basis.data());
            tcd = { radialFraction, axisFraction };

            SetPosition(i, pos);
            SetNormal(i, nor);
            SetTangent(i, basis[1]);
            SetBitangent(i, basis[2]);
            SetTCoord(i, tcd);
        }
    }

    // Generate indices (outside view).
    auto ibuffer = CreateIBuffer(numTriangles);
    if (!ibuffer)
    {
        return nullptr;
    }
    for (unsigned int a = 0, aStart = 0, t = 0; a < numAxisSamples - 1; ++a)
    {
        unsigned int i0 = aStart;
        unsigned int i1 = i0 + 1;
        aStart += numRadialSamples + 1;
        unsigned int i2 = aStart;
        unsigned int i3 = i2 + 1;
        for (unsigned int i = 0; i < numRadialSamples; ++i, ++i0, ++i1, ++i2, ++i3)
        {
            ibuffer->SetTriangle(t++, i0, i1, i2);
            ibuffer->SetTriangle(t++, i1, i3, i2);
        }
    }
    if (!mOutside)
    {
        ReverseTriangleOrder(ibuffer.get());
    }

    // Create the mesh.
    auto visual = std::make_shared<Visual>(vbuffer, ibuffer);
    if (visual)
    {
        visual->UpdateModelBound();

        // The duplication of vertices at the seam causes the automatically
        // generated bounding volume to be slightly off center.  Reset the
        // bound to use the true information.
        float maxDist = std::sqrt(radius * radius + height * height);
        visual->modelBound.SetCenter({ 0.0f, 0.0f, 0.0f });
        visual->modelBound.SetRadius(maxDist);
    }

    return visual;
}

std::shared_ptr<Visual> MeshFactory::CreateCylinderClosed(unsigned int numAxisSamples,
    unsigned int numRadialSamples, float radius, float height)
{
    // Create a sphere and then deform it into a closed cylinder.
    auto visual = CreateSphere(numAxisSamples, numRadialSamples, radius);
    if (!visual)
    {
        return nullptr;
    }

    auto vbuffer = visual->GetVertexBuffer();
    unsigned int numVertices = vbuffer->GetNumElements();
    Vector3<float> pos;
    std::array<Vector3<float>, 3> basis;
    unsigned int i;

    // Flatten sphere at poles.
    float hDiv2 = 0.5f * height;
    i = numVertices - 2;
    pos = Position(i);
    pos[2] = -hDiv2;
    SetPosition(i, pos);  // south pole
    i = numVertices - 1;
    pos = Position(i);
    pos[2] = +hDiv2;
    SetPosition(i, pos);  // north pole

    // Remap z-values to [-h/2,h/2].
    float zFactor = 2.0f / static_cast<float>(numAxisSamples - 1);
    float tmp0 = radius * (-1.0f + zFactor);
    float tmp1 = 1.0f / (radius * (+1.0f - zFactor));
    for (i = 0; i < numVertices - 2; ++i)
    {
        pos = Position(i);
        pos[2] = hDiv2*(-1.0f + tmp1 * (pos[2] - tmp0));
        float adjust = radius / std::sqrt(pos[0] * pos[0] + pos[1] * pos[1]);
        pos[0] *= adjust;
        pos[1] *= adjust;
        SetPosition(i, pos);
    }

    // Let the Visual update normals using its algorithm.
    if (visual->UpdateModelNormals())
    {
        // Update tangent space, if relevant.
        for (i = 0; i < numVertices; ++i)
        {
            basis[0] = Normal(i);
            ComputeOrthogonalComplement(1, basis.data());
            SetTangent(i, basis[1]);
            SetBitangent(i, basis[2]);
        }
    }

    // The duplication of vertices at the seam causes the automatically
    // generated bounding volume to be slightly off center.  Reset the bound
    // to use the true information.
    float maxDist = std::sqrt(radius * radius + height * height);
    visual->modelBound.SetCenter({ 0.0f, 0.0f, 0.0f });
    visual->modelBound.SetRadius(maxDist);
    return visual;
}

std::shared_ptr<Visual> MeshFactory::CreateSphere(unsigned int numZSamples,
    unsigned int numRadialSamples, float radius)
{
    // Quantities derived from inputs.
    unsigned int zsm1 = numZSamples - 1;
    unsigned int zsm2 = numZSamples - 2;
    unsigned int zsm3 = numZSamples - 3;
    unsigned int rsp1 = numRadialSamples + 1;
    float invRS = 1.0f / static_cast<float>(numRadialSamples);
    float zFactor = 2.0f / static_cast<float>(zsm1);
    unsigned int numVertices = zsm2 * rsp1 + 2;
    unsigned int numTriangles = 2 * zsm2 * numRadialSamples;

    // Generate geometry.
    auto vbuffer = CreateVBuffer(numVertices);
    if (!vbuffer)
    {
        return nullptr;
    }

    Vector3<float> pos, nor;
    std::array<Vector3<float>, 3> basis;
    Vector2<float> tcd;

    // Generate points on the unit circle to be used in computing the mesh
    // points on a sphere slice.
    std::vector<float> cs(rsp1), sn(rsp1);
    for (unsigned int r = 0; r < numRadialSamples; ++r)
    {
        float angle = invRS * r * static_cast<float>(GTE_C_TWO_PI);
        cs[r] = std::cos(angle);
        sn[r] = std::sin(angle);
    }
    cs[numRadialSamples] = cs[0];
    sn[numRadialSamples] = sn[0];

    // Generate the sphere itself.
    unsigned int i = 0;
    for (unsigned int z = 1; z < zsm1; ++z)
    {
        float zFraction = -1.0f + zFactor*static_cast<float>(z);  // in (-1,1)
        float zValue = radius*zFraction;

        // Compute center of slice.
        Vector3<float> sliceCenter{ 0.0f, 0.0f, zValue };

        // Compute radius of slice.
        float sliceRadius = std::sqrt(std::max(radius * radius - zValue * zValue, 0.0f));

        // Compute slice vertices with duplication at endpoint.
        for (unsigned int r = 0; r <= numRadialSamples; ++r, ++i)
        {
            float radialFraction = r * invRS;  // in [0,1)
            Vector3<float> radial{ cs[r], sn[r], 0.0f };
            pos = sliceCenter + sliceRadius * radial;
            nor = pos;
            Normalize(nor);
            if (!mOutside)
            {
                nor = -nor;
            }

            basis[0] = nor;
            ComputeOrthogonalComplement(1, basis.data());
            tcd = { radialFraction, 0.5f * (zFraction + 1.0f) };

            SetPosition(i, pos);
            SetNormal(i, nor);
            SetTangent(i, basis[1]);
            SetBitangent(i, basis[2]);
            SetTCoord(i, tcd);
        }
    }

    // The point at the south pole.
    pos = { 0.0f, 0.0f, -radius };
    if (mOutside)
    {
        nor = { 0.0f, 0.0f, -1.0f };
    }
    else
    {
        nor = { 0.0f, 0.0f, 1.0f };
    }
    basis[0] = nor;
    ComputeOrthogonalComplement(1, basis.data());
    tcd = { 0.5f, 0.0f };
    SetPosition(i, pos);
    SetNormal(i, nor);
    SetTangent(i, basis[1]);
    SetBitangent(i, basis[2]);
    SetTCoord(i, tcd);
    ++i;

    // The point at the north pole.
    pos = { 0.0f, 0.0f, radius };
    if (mOutside)
    {
        nor = { 0.0f, 0.0f, 1.0f };
    }
    else
    {
        nor = { 0.0f, 0.0f, -1.0f };
    }
    basis[0] = nor;
    ComputeOrthogonalComplement(1, basis.data());
    tcd = { 0.5f, 1.0f };
    SetPosition(i, pos);
    SetNormal(i, nor);
    SetTangent(i, basis[1]);
    SetBitangent(i, basis[2]);
    SetTCoord(i, tcd);

    // Generate indices (outside view).
    auto ibuffer = CreateIBuffer(numTriangles);
    if (!ibuffer)
    {
        return nullptr;
    }
    unsigned int t = 0;
    for (unsigned int z = 0, zStart = 0; z < zsm3; ++z)
    {
        unsigned int i0 = zStart;
        unsigned int i1 = i0 + 1;
        zStart += rsp1;
        unsigned int i2 = zStart;
        unsigned int i3 = i2 + 1;
        for (i = 0; i < numRadialSamples; ++i, ++i0, ++i1, ++i2, ++i3)
        {
            ibuffer->SetTriangle(t++, i0, i1, i2);
            ibuffer->SetTriangle(t++, i1, i3, i2);
        }
    }

    // The south pole triangles (outside view).
    unsigned int numVerticesM2 = numVertices - 2;
    for (i = 0; i < numRadialSamples; ++i, ++t)
    {
        ibuffer->SetTriangle(t, i, numVerticesM2, i+1);
    }

    // The north pole triangles (outside view).
    unsigned int numVerticesM1 = numVertices - 1, offset = zsm3 * rsp1;
    for (i = 0; i < numRadialSamples; ++i, ++t)
    {
        ibuffer->SetTriangle(t, i + offset, i + 1 + offset, numVerticesM1);
    }

    if (!mOutside)
    {
        ReverseTriangleOrder(ibuffer.get());
    }

    // Create the mesh.
    auto visual = std::make_shared<Visual>(vbuffer, ibuffer);
    if (visual)
    {
        visual->UpdateModelBound();

        // The duplication of vertices at the seam cause the automatically
        // generated bounding volume to be slightly off center.  Reset the
        // bound to use the true information.
        visual->modelBound.SetCenter({ 0.0f, 0.0f, 0.0f });
        visual->modelBound.SetRadius(radius);
    }
    return visual;
}

std::shared_ptr<Visual> MeshFactory::CreateTorus(
    unsigned int numCircleSamples, unsigned int numRadialSamples,
    float outerRadius, float innerRadius)
{
    // Quantities derived from inputs.
    float invCS = 1.0f / static_cast<float>(numCircleSamples);
    float invRS = 1.0f / static_cast<float>(numRadialSamples);
    unsigned int numVertices = (numCircleSamples + 1) * (numRadialSamples + 1);
    unsigned int numTriangles = 2 * numCircleSamples * numRadialSamples;

    // Generate geometry.
    auto vbuffer = CreateVBuffer(numVertices);
    if (!vbuffer)
    {
        return nullptr;
    }

    Vector3<float> pos, nor;
    std::array<Vector3<float>, 3> basis;
    Vector2<float> tcd;

    // Generate an open cylinder that is warped into a torus.
    unsigned int i = 0;
    for (unsigned int c = 0; c < numCircleSamples; ++c)
    {
        // Compute center point on torus circle at specified angle.
        float circleFraction = static_cast<float>(c) * invCS;  // in [0,1)
        float theta = circleFraction * static_cast<float>(GTE_C_TWO_PI);
        float cosTheta = std::cos(theta);
        float sinTheta = std::sin(theta);
        Vector3<float> radial{ cosTheta, sinTheta, 0.0f };
        Vector3<float> torusMiddle = outerRadius*radial;

        // Compute slice vertices with duplication at endpoint.
        for (unsigned int r = 0; r <= numRadialSamples; ++r, ++i)
        {
            float radialFraction = static_cast<float>(r) * invRS;  // in [0,1)
            float phi = radialFraction * static_cast<float>(GTE_C_TWO_PI);
            float cosPhi = std::cos(phi);
            float sinPhi = std::sin(phi);
            nor = cosPhi * radial + sinPhi * Vector3<float>::Unit(2);
            pos = torusMiddle + innerRadius * nor;
            if (!mOutside)
            {
                nor = -nor;
            }

            basis[0] = nor;
            ComputeOrthogonalComplement(1, basis.data());
            tcd = { radialFraction, circleFraction };

            SetPosition(i, pos);
            SetNormal(i, nor);
            SetTangent(i, basis[1]);
            SetBitangent(i, basis[2]);
            SetTCoord(i, tcd);
        }
    }

    // Duplicate the cylinder ends to form a torus.
    for (unsigned int r = 0; r <= numRadialSamples; ++r, ++i)
    {
        Position(i) = Position(r);
        if (mNormals)
        {
            Normal(i) = Normal(r);
        }
        if (mTangents)
        {
            Tangent(i) = Tangent(r);
        }
        if (mBitangents)
        {
            Bitangent(i) = Bitangent(r);
        }
        for (unsigned int unit = 0; unit < VA_MAX_TCOORD_UNITS; ++unit)
        {
            if (mAssignTCoords[unit])
            {
                TCoord(unit, i) = Vector2<float>{ TCoord(unit, r)[0], 1.0f };
            }
        }
    }

    // Generate indices (outside view).
    auto ibuffer = CreateIBuffer(numTriangles);
    if (!ibuffer)
    {
        return nullptr;
    }
    unsigned int t = 0;
    for (unsigned int c = 0, cStart = 0; c < numCircleSamples; ++c)
    {
        unsigned int i0 = cStart;
        unsigned int i1 = i0 + 1;
        cStart += numRadialSamples + 1;
        unsigned int i2 = cStart;
        unsigned int i3 = i2 + 1;
        for (i = 0; i < numRadialSamples; ++i, ++i0, ++i1, ++i2, ++i3)
        {
            ibuffer->SetTriangle(t++, i0, i2, i1);
            ibuffer->SetTriangle(t++, i1, i2, i3);
        }
    }

    if (!mOutside)
    {
        ReverseTriangleOrder(ibuffer.get());
    }

    // Create the mesh.
    auto visual = std::make_shared<Visual>(vbuffer, ibuffer);
    if (visual)
    {
        visual->UpdateModelBound();

        // The duplication of vertices at the seam cause the automatically
        // generated bounding volume to be slightly off center.  Reset the
        // bound to use the true information.
        visual->modelBound.SetCenter({ 0.0f, 0.0f, 0.0f });
        visual->modelBound.SetRadius(outerRadius);
    }
    return visual;
}

std::shared_ptr<Visual> MeshFactory::CreateTetrahedron()
{
    float const sqrt2Div3 = std::sqrt(2.0f) / 3.0f;
    float const sqrt6Div3 = std::sqrt(6.0f) / 3.0f;
    float const oneThird = 1.0f / 3.0f;
    unsigned int const numVertices = 4;
    unsigned int const numTriangles = 4;

    // Generate geometry.
    auto vbuffer = CreateVBuffer(numVertices);
    if (!vbuffer)
    {
        return nullptr;
    }

    Vector3<float> pos, nor;
    std::array<Vector3<float>, 3> basis;

    Position(0) = { 0.0f, 0.0f, 1.0f };
    Position(1) = { 2.0f * sqrt2Div3, 0.0f, -oneThird };
    Position(2) = { -sqrt2Div3, sqrt6Div3, -oneThird };
    Position(3) = { -sqrt2Div3, -sqrt6Div3, -oneThird };
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        pos = Position(i);
        nor = (mOutside ? pos : -pos);
        SetNormal(i, nor);
        basis[0] = nor;
        ComputeOrthogonalComplement(1, basis.data());
        SetTangent(i, basis[1]);
        SetBitangent(i, basis[2]);
        SetPlatonicTCoord(i, pos);
    }

    // Generate indices (outside view).
    auto ibuffer = CreateIBuffer(numTriangles);
    if (!ibuffer)
    {
        return nullptr;
    }
    ibuffer->SetTriangle(0, 0, 1, 2);
    ibuffer->SetTriangle(1, 0, 2, 3);
    ibuffer->SetTriangle(2, 0, 3, 1);
    ibuffer->SetTriangle(3, 1, 3, 2);

    if (!mOutside)
    {
        ReverseTriangleOrder(ibuffer.get());
    }

    // Create the mesh.
    auto visual = std::make_shared<Visual>(vbuffer, ibuffer);
    if (visual)
    {
        // The bound is the unit sphere.
        visual->modelBound.SetCenter({ 0.0f, 0.0f, 0.0f });
        visual->modelBound.SetRadius(1.0f);
    }
    return visual;
}

std::shared_ptr<Visual> MeshFactory::CreateHexahedron()
{
    float const sqrtThird = std::sqrt(1.0f / 3.0f);
    unsigned int const numVertices = 8;
    unsigned int const numTriangles = 12;

    // Generate geometry.
    auto vbuffer = CreateVBuffer(numVertices);
    if (!vbuffer)
    {
        return nullptr;
    }

    Vector3<float> pos, nor;
    std::array<Vector3<float>, 3> basis;

    Position(0) = { -sqrtThird, -sqrtThird, -sqrtThird };
    Position(1) = {  sqrtThird, -sqrtThird, -sqrtThird };
    Position(2) = {  sqrtThird,  sqrtThird, -sqrtThird };
    Position(3) = { -sqrtThird,  sqrtThird, -sqrtThird };
    Position(4) = { -sqrtThird, -sqrtThird,  sqrtThird };
    Position(5) = {  sqrtThird, -sqrtThird,  sqrtThird };
    Position(6) = {  sqrtThird,  sqrtThird,  sqrtThird };
    Position(7) = { -sqrtThird,  sqrtThird,  sqrtThird };
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        pos = Position(i);
        nor = (mOutside ? pos : -pos);
        SetNormal(i, nor);
        basis[0] = nor;
        ComputeOrthogonalComplement(1, basis.data());
        SetTangent(i, basis[1]);
        SetBitangent(i, basis[2]);
        SetPlatonicTCoord(i, pos);
    }

    // Generate indices (outside view).
    auto ibuffer = CreateIBuffer(numTriangles);
    if (!ibuffer)
    {
        return nullptr;
    }
    ibuffer->SetTriangle(0, 0, 3, 2);
    ibuffer->SetTriangle( 1, 0, 2, 1);
    ibuffer->SetTriangle( 2, 0, 1, 5);
    ibuffer->SetTriangle( 3, 0, 5, 4);
    ibuffer->SetTriangle( 4, 0, 4, 7);
    ibuffer->SetTriangle( 5, 0, 7, 3);
    ibuffer->SetTriangle( 6, 6, 5, 1);
    ibuffer->SetTriangle( 7, 6, 1, 2);
    ibuffer->SetTriangle( 8, 6, 2, 3);
    ibuffer->SetTriangle( 9, 6, 3, 7);
    ibuffer->SetTriangle(10, 6, 7, 4);
    ibuffer->SetTriangle(11, 6, 4, 5);

    if (!mOutside)
    {
        ReverseTriangleOrder(ibuffer.get());
    }

    // Create the mesh.
    auto visual = std::make_shared<Visual>(vbuffer, ibuffer);
    if (visual)
    {
        // The bound is the unit sphere.
        visual->modelBound.SetCenter({ 0.0f, 0.0f, 0.0f });
        visual->modelBound.SetRadius(1.0f);
    }
    return visual;
}

std::shared_ptr<Visual> MeshFactory::CreateOctahedron()
{
    unsigned int const numVertices = 6;
    unsigned int const numTriangles = 8;

    // Generate geometry.
    auto vbuffer = CreateVBuffer(numVertices);
    if (!vbuffer)
    {
        return nullptr;
    }

    Vector3<float> pos, nor;
    std::array<Vector3<float>, 3> basis;

    Position(0) = { 1.0f, 0.0f, 0.0f };
    Position(1) = { -1.0f, 0.0f, 0.0f };
    Position(2) = { 0.0f, 1.0f, 0.0f };
    Position(3) = { 0.0f, -1.0f, 0.0f };
    Position(4) = { 0.0f, 0.0f, 1.0f };
    Position(5) = { 0.0f, 0.0f, -1.0f };
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        pos = Position(i);
        nor = (mOutside ? pos : -pos);
        SetNormal(i, nor);
        basis[0] = nor;
        ComputeOrthogonalComplement(1, basis.data());
        SetTangent(i, basis[1]);
        SetBitangent(i, basis[2]);
        SetPlatonicTCoord(i, pos);
    }

    // Generate indices (outside view).
    auto ibuffer = CreateIBuffer(numTriangles);
    if (!ibuffer)
    {
        return nullptr;
    }
    ibuffer->SetTriangle(0, 4, 0, 2);
    ibuffer->SetTriangle(1, 4, 2, 1);
    ibuffer->SetTriangle(2, 4, 1, 3);
    ibuffer->SetTriangle(3, 4, 3, 0);
    ibuffer->SetTriangle(4, 5, 2, 0);
    ibuffer->SetTriangle(5, 5, 1, 2);
    ibuffer->SetTriangle(6, 5, 3, 1);
    ibuffer->SetTriangle(7, 5, 0, 3);

    if (!mOutside)
    {
        ReverseTriangleOrder(ibuffer.get());
    }

    // Create the mesh.
    auto visual = std::make_shared<Visual>(vbuffer, ibuffer);
    if (visual)
    {
        // The bound is the unit sphere.
        visual->modelBound.SetCenter({ 0.0f, 0.0f, 0.0f });
        visual->modelBound.SetRadius(1.0f);
    }
    return visual;
}

std::shared_ptr<Visual> MeshFactory::CreateDodecahedron()
{
    float const a = 1.0f / std::sqrt(3.0f);
    float const b = std::sqrt((3.0f - std::sqrt(5.0f)) / 6.0f);
    float const c = std::sqrt((3.0f + std::sqrt(5.0f)) / 6.0f);
    unsigned int const numVertices = 20;
    unsigned int const numTriangles = 36;

    // Generate geometry.
    auto vbuffer = CreateVBuffer(numVertices);
    if (!vbuffer)
    {
        return nullptr;
    }

    Vector3<float> pos, nor;
    std::array<Vector3<float>, 3> basis;

    Position(0) = { a, a, a };
    Position(1) = { a, a, -a };
    Position(2) = { a, -a, a };
    Position(3) = { a, -a, -a };
    Position(4) = { -a, a, a };
    Position(5) = { -a, a, -a };
    Position(6) = { -a, -a, a };
    Position(7) = { -a, -a, -a };
    Position(8) = { b, c, 0.0f };
    Position(9) = { -b, c, 0.0f };
    Position(10) = { b, -c, 0.0f };
    Position(11) = { -b, -c, 0.0f };
    Position(12) = { c, 0.0f, b };
    Position(13) = { c, 0.0f, -b };
    Position(14) = { -c, 0.0f, b };
    Position(15) = { -c, 0.0f, -b };
    Position(16) = { 0.0f, b, c };
    Position(17) = { 0.0f, -b, c };
    Position(18) = { 0.0f, b, -c };
    Position(19) = { 0.0f, -b, -c };
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        pos = Position(i);
        nor = (mOutside ? pos : -pos);
        SetNormal(i, nor);
        basis[0] = nor;
        ComputeOrthogonalComplement(1, basis.data());
        SetTangent(i, basis[1]);
        SetBitangent(i, basis[2]);
        SetPlatonicTCoord(i, pos);
    }

    // Generate indices.
    auto ibuffer = CreateIBuffer(numTriangles);
    if (!ibuffer)
    {
        return nullptr;
    }
    ibuffer->SetTriangle(0, 0, 8, 9);
    ibuffer->SetTriangle( 1,  0,  9,  4);
    ibuffer->SetTriangle( 2,  0,  4, 16);
    ibuffer->SetTriangle( 3,  0, 12, 13);
    ibuffer->SetTriangle( 4,  0, 13,  1);
    ibuffer->SetTriangle( 5,  0,  1,  8);
    ibuffer->SetTriangle( 6,  0, 16, 17);
    ibuffer->SetTriangle( 7,  0, 17,  2);
    ibuffer->SetTriangle( 8,  0,  2, 12);
    ibuffer->SetTriangle( 9,  8,  1, 18);
    ibuffer->SetTriangle(10,  8, 18,  5);
    ibuffer->SetTriangle(11,  8,  5,  9);
    ibuffer->SetTriangle(12, 12,  2, 10);
    ibuffer->SetTriangle(13, 12, 10,  3);
    ibuffer->SetTriangle(14, 12,  3, 13);
    ibuffer->SetTriangle(15, 16,  4, 14);
    ibuffer->SetTriangle(16, 16, 14,  6);
    ibuffer->SetTriangle(17, 16,  6, 17);
    ibuffer->SetTriangle(18,  9,  5, 15);
    ibuffer->SetTriangle(19,  9, 15, 14);
    ibuffer->SetTriangle(20,  9, 14,  4);
    ibuffer->SetTriangle(21,  6, 11, 10);
    ibuffer->SetTriangle(22,  6, 10,  2);
    ibuffer->SetTriangle(23,  6,  2, 17);
    ibuffer->SetTriangle(24,  3, 19, 18);
    ibuffer->SetTriangle(25,  3, 18,  1);
    ibuffer->SetTriangle(26,  3,  1, 13);
    ibuffer->SetTriangle(27,  7, 15,  5);
    ibuffer->SetTriangle(28,  7,  5, 18);
    ibuffer->SetTriangle(29,  7, 18, 19);
    ibuffer->SetTriangle(30,  7, 11,  6);
    ibuffer->SetTriangle(31,  7,  6, 14);
    ibuffer->SetTriangle(32,  7, 14, 15);
    ibuffer->SetTriangle(33,  7, 19,  3);
    ibuffer->SetTriangle(34,  7,  3, 10);
    ibuffer->SetTriangle(35,  7, 10, 11);

    if (!mOutside)
    {
        ReverseTriangleOrder(ibuffer.get());
    }

    // Create the mesh.
    auto visual = std::make_shared<Visual>(vbuffer, ibuffer);
    if (visual)
    {
        // The bound is the unit sphere.
        visual->modelBound.SetCenter({ 0.0f, 0.0f, 0.0f });
        visual->modelBound.SetRadius(1.0f);
    }
    return visual;
}

std::shared_ptr<Visual> MeshFactory::CreateIcosahedron()
{
    float const goldenRatio = 0.5f * (1.0f + std::sqrt(5.0f));
    float const invRoot = 1.0f / std::sqrt(1.0f + goldenRatio * goldenRatio);
    float const u = goldenRatio * invRoot;
    float const v = invRoot;
    unsigned int const numVertices = 12;
    unsigned int const numTriangles = 20;

    // Generate geometry.
    auto vbuffer = CreateVBuffer(numVertices);
    if (!vbuffer)
    {
        return nullptr;
    }

    Vector3<float> pos, nor;
    std::array<Vector3<float>, 3> basis;

    Position(0) = { u, v, 0.0f };
    Position(1) = { -u, v, 0.0f };
    Position(2) = { u, -v, 0.0f };
    Position(3) = { -u, -v, 0.0f };
    Position(4) = { v, 0.0f, u };
    Position(5) = { v, 0.0f, -u };
    Position(6) = { -v, 0.0f, u };
    Position(7) = { -v, 0.0f, -u };
    Position(8) = { 0.0f, u, v };
    Position(9) = { 0.0f, -u, v };
    Position(10) = { 0.0f, u, -v };
    Position(11) = { 0.0f, -u, -v };
    for (unsigned int i = 0; i < numVertices; ++i)
    {
        pos = Position(i);
        nor = (mOutside ? pos : -pos);
        SetNormal(i, nor);
        basis[0] = nor;
        ComputeOrthogonalComplement(1, basis.data());
        SetTangent(i, basis[1]);
        SetBitangent(i, basis[2]);
        SetPlatonicTCoord(i, pos);
    }

    // Generate indices.
    auto ibuffer = CreateIBuffer(numTriangles);
    if (!ibuffer)
    {
        return nullptr;
    }
    ibuffer->SetTriangle(0, 0, 8, 4);
    ibuffer->SetTriangle( 1,  0,  5, 10);
    ibuffer->SetTriangle( 2,  2,  4,  9);
    ibuffer->SetTriangle( 3,  2, 11,  5);
    ibuffer->SetTriangle( 4,  1,  6,  8);
    ibuffer->SetTriangle( 5,  1, 10,  7);
    ibuffer->SetTriangle( 6,  3,  9,  6);
    ibuffer->SetTriangle( 7,  3,  7, 11);
    ibuffer->SetTriangle( 8,  0, 10,  8);
    ibuffer->SetTriangle( 9,  1,  8, 10);
    ibuffer->SetTriangle(10,  2,  9, 11);
    ibuffer->SetTriangle(11,  3, 11,  9);
    ibuffer->SetTriangle(12,  4,  2,  0);
    ibuffer->SetTriangle(13,  5,  0,  2);
    ibuffer->SetTriangle(14,  6,  1,  3);
    ibuffer->SetTriangle(15,  7,  3,  1);
    ibuffer->SetTriangle(16,  8,  6,  4);
    ibuffer->SetTriangle(17,  9,  4,  6);
    ibuffer->SetTriangle(18, 10,  5,  7);
    ibuffer->SetTriangle(19, 11,  7,  5);

    if (!mOutside)
    {
        ReverseTriangleOrder(ibuffer.get());
    }

    // Create the mesh.
    auto visual = std::make_shared<Visual>(vbuffer, ibuffer);
    if (visual)
    {
        // The bound is the unit sphere.
        visual->modelBound.SetCenter({ 0.0f, 0.0f, 0.0f });
        visual->modelBound.SetRadius(1.0f);
    }
    return visual;
}

std::shared_ptr<VertexBuffer> MeshFactory::CreateVBuffer(unsigned int numVertices)
{
    auto vbuffer = std::make_shared<VertexBuffer>(mVFormat, numVertices);
    if (vbuffer)
    {
        // Get the position channel.
        mPositions = GetGeometricChannel(vbuffer, VA_POSITION, 1.0f);
        LogAssert(mPositions != nullptr, "Positions are required.");

        // Get the optional geometric channels.
        mNormals = GetGeometricChannel(vbuffer, VA_NORMAL, 0.0f);
        mTangents = GetGeometricChannel(vbuffer, VA_TANGENT, 0.0f);
        mBitangents = GetGeometricChannel(vbuffer, VA_BINORMAL, 0.0f);

        // Get texture coordinate channels that are to be assigned values.
        // Clear the mAssignTCoords element in case any elements were set by a
        // previous mesh factory creation call.
        std::set<DFType> required;
        required.insert(DF_R32G32_FLOAT);
        for (unsigned int unit = 0; unit < VA_MAX_TCOORD_UNITS; ++unit)
        {
            mTCoords[unit] = vbuffer->GetChannel(VA_TEXCOORD, unit, required);
            if (mTCoords[unit])
            {
                mAssignTCoords[unit] = true;
            }
            else
            {
                mAssignTCoords[unit] = false;
            }
        }

        vbuffer->SetUsage(mVBUsage);
    }
    return vbuffer;
}

std::shared_ptr<IndexBuffer> MeshFactory::CreateIBuffer(unsigned int numTriangles)
{
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, mIndexSize);
    if (ibuffer)
    {
        ibuffer->SetUsage(mIBUsage);
    }
    return ibuffer;
}

char* MeshFactory::GetGeometricChannel(
    std::shared_ptr<VertexBuffer> const& vbuffer, VASemantic semantic,
    float w)
{
    char* channel = nullptr;
    int index = mVFormat.GetIndex(semantic, 0);
    if (index >= 0)
    {
        channel = vbuffer->GetChannel(semantic, 0, std::set<DFType>());
        LogAssert(channel != nullptr, "Unexpected condition.");
        if (mVFormat.GetType(index) == DF_R32G32B32A32_FLOAT)
        {
            // Fill in the w-components.
            int const numVertices = vbuffer->GetNumElements();
            for (int i = 0; i < numVertices; ++i)
            {
                auto tuple4 = reinterpret_cast<float*>(channel + i * mVFormat.GetVertexSize());
                tuple4[3] = w;
            }
        }
    }
    return channel;
}

void MeshFactory::SetPlatonicTCoord(unsigned int i, Vector3<float> const& pos)
{
    Vector2<float> tcd;
    if (std::fabs(pos[2]) < 1.0f)
    {
        tcd[0] = 0.5f * (1.0f + std::atan2(pos[1], pos[0]) * static_cast<float>(GTE_C_INV_PI));
    }
    else
    {
        tcd[0] = 0.5f;
    }

    tcd[1] = std::acos(pos[2]) * static_cast<float>(GTE_C_INV_PI);

    for (int unit = 0; unit < VA_MAX_TCOORD_UNITS; ++unit)
    {
        if (mAssignTCoords[unit])
        {
            TCoord(unit, i) = tcd;
        }
    }
}

void MeshFactory::ReverseTriangleOrder(IndexBuffer* ibuffer)
{
    unsigned int const numTriangles = ibuffer->GetNumPrimitives();
    for (unsigned int t = 0; t < numTriangles; ++t)
    { 
        unsigned int v0, v1, v2;
        ibuffer->GetTriangle(t, v0, v1, v2);
        ibuffer->SetTriangle(t, v0, v2, v1);
    }
}
