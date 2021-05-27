// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Visual.h>
#include <Mathematics/Mesh.h>

// This class is a factory for Visual objects corresponding to common
// geometric primitives.  Triangle mesh primitives are generated.  Each mesh
// is centered at (0,0,0) and has an up-axis of (0,0,1).  The other axes
// forming the coordinate system are (1,0,0) and (0,1,0).
//
// The factory always generates 3-tuple positions.  If normals, tangents, or
// binormals are requested, they are also generated as 3-tuples.  They are
// stored in the vertex buffer as 3-tuples or 4-tuples as requested (w = 1 for
// positions, w = 0 for the others).  The factory also generates 2-tuple
// texture coordinates.  These are stored in the vertex buffer for 2-tuple
// units.  All other attribute types are unassigned by the factory.

namespace gte
{
    class MeshFactory
    {
    public:
        // Construction and destruction.
        ~MeshFactory() = default;
        MeshFactory();

        // Specify the vertex format.
        inline void SetVertexFormat(VertexFormat const& format)
        {
            mVFormat = format;
        }

        // Specify the usage for the vertex buffer data.  The default is
        // Resource::IMMUTABLE.
        inline void SetVertexBufferUsage(Resource::Usage usage)
        {
            mVBUsage = usage;
        }

        // Specify the type of indices and where the index buffer data should
        // be stored.  For 'unsigned int' indices, set 'use32Bit' to 'true';
        // for 'unsigned short' indices, set 'use32Bit' to false.  The default
        // is 'unsigned int'.
        inline void SetIndexFormat(bool use32Bit)
        {
            mIndexSize = (use32Bit ? sizeof(unsigned int) : sizeof(unsigned short));
        }

        // Specify the usage for the index buffer data.  The default is
        // Resource::IMMUTABLE.
        inline void SetIndexBufferUsage(Resource::Usage usage)
        {
            mIBUsage = usage;
        }

        // For the geometric primitives that have an inside and an outside,
        // you may specify where the observer is expected to see the object.
        // If the observer must see the primitive from the outside, pass
        // 'true' to this function.  If the observer must see the primitive
        // from the inside, pass 'false'.  This Boolean flag simply controls
        // the triangle face order for face culling.  The default is 'true'
        // (observer view object from the outside).
        inline void SetOutside(bool outside)
        {
            mOutside = outside;
        }

        // The rectangle is in the plane z = 0 and is visible to an observer
        // who is on the side of the plane to which the normal (0,0,1) points.
        // It has corners (-xExtent, -yExtent, 0), (+xExtent, -yExtent, 0),
        // (-xExtent, +yExtent, 0), and (+xExtent, +yExtent, 0).  The mesh has
        // numXSamples vertices in the x-direction and numYSamples vertices in
        // the y-direction for a total of numXSamples*numYSamples vertices.
        std::shared_ptr<Visual> CreateRectangle(unsigned int numXSamples,
            unsigned int numYSamples, float xExtent, float yExtent);

        // The triangle is in the plane z = 0 and is visible to an observer
        // who is on the side of the plane to which the normal (0,0,1) points.
        // It has vertices (0, 0, 0), (xExtent, 0, 0), and (0, yExtent, 0).
        // The mesh has numSamples vertices along each of the x- and y-axes
        // for a total of numSamples*(numSamples+1)/2 vertices.
        std::shared_ptr<Visual> CreateTriangle(unsigned int numSamples,
            float xExtent, float yExtent);

        // The circular disk is in the plane z = 0 and is visible to an
        // observer who is on the side of the plane to which the normal
        // (0,0,1) points.  It has center (0,0,0) and the specified radius.
        // The mesh has its first vertex at the center.  Samples are placed
        // along rays whose common origin is the center.  There are
        // numRadialSamples rays.  Along each ray the mesh has
        // numShellSamples vertices.
        std::shared_ptr<Visual> CreateDisk(unsigned int numShellSamples,
            unsigned int numRadialSamples, float radius);

        // The box has center (0,0,0); unit-length axes (1,0,0), (0,1,0), and
        // (0,0,1); and extents (half-lengths) xExtent, yExtent, and zExtent.
        // The mesh has 8 vertices and 12 triangles.  For example, the box
        // corner in the first octant is (xExtent, yExtent, zExtent).
        std::shared_ptr<Visual> CreateBox(float xExtent, float yExtent,
            float zExtent);

        // The cylinder has center (0,0,0), the specified radius, and the
        // specified height.  The cylinder axis is a line segment of the form
        // (0,0,0) + t*(0,0,1) for |t| <= height/2.  The cylinder wall is
        // implicitly defined by x^2+y^2 = radius^2.  CreateCylinderOpen leads
        // to a cylinder whose end-disks are omitted; you have an open tube.
        // CreateCylinderClosed leads to a cylinder with end-disks.  Each
        // end-disk is a regular polygon that is tessellated by including a
        // vertex at the center of the polygon and decomposing the polygon
        // into triangles that all share the center vertex and each triangle
        // containing an edge of the polygon.
        std::shared_ptr<Visual> CreateCylinderOpen(unsigned int numAxisSamples,
            unsigned int numRadialSamples, float radius, float height);

        std::shared_ptr<Visual> CreateCylinderClosed(unsigned int numAxisSamples,
            unsigned int numRadialSamples, float radius, float height);

        // The sphere has center (0,0,0) and the specified radius.  The north
        // pole is at (0,0,radius) and the south pole is at (0,0,-radius).
        // The mesh has the topology of an open cylinder (which is also the
        // topology of a rectangle with wrap-around for one pair of parallel
        // edges) and is then stitched to the north and south poles.  The
        // triangles are unevenly distributed.  If you want a more even
        // distribution, create an icosahedron and subdivide it.
        std::shared_ptr<Visual> CreateSphere(unsigned int numZSamples,
            unsigned int numRadialSamples, float radius);

        // The torus has center (0,0,0).  If you observe the torus along the
        // line with direction (0,0,1), you will see an annulus.  The circle
        // that is the center of the annulus has radius 'outerRadius'.  The
        // distance from this circle to the boundaries of the annulus is the
        // 'inner radius'.
        std::shared_ptr<Visual> CreateTorus(unsigned int numCircleSamples,
            unsigned int numRadialSamples, float outerRadius, float innerRadius);

        // Platonic solids, all inscribed in a unit sphere centered at
        // (0,0,0).
        std::shared_ptr<Visual> CreateTetrahedron();
        std::shared_ptr<Visual> CreateHexahedron();
        std::shared_ptr<Visual> CreateOctahedron();
        std::shared_ptr<Visual> CreateDodecahedron();
        std::shared_ptr<Visual> CreateIcosahedron();

    private:
        // Support for creating vertex and index buffers.
        std::shared_ptr<VertexBuffer> CreateVBuffer(unsigned int numVertices);
        std::shared_ptr<IndexBuffer> CreateIBuffer(unsigned int numTriangles);

        // Support for vertex buffers.
        char* GetGeometricChannel(std::shared_ptr<VertexBuffer> const& vbuffer,
            VASemantic semantic, float w);

        inline Vector3<float>& Position(unsigned int i)
        {
            return *reinterpret_cast<Vector3<float>*>(mPositions + i * mVFormat.GetVertexSize());
        }

        inline Vector3<float>& Normal(unsigned int i)
        {
            return *reinterpret_cast<Vector3<float>*>(mNormals + i * mVFormat.GetVertexSize());
        }

        inline Vector3<float>& Tangent(unsigned int i)
        {
            return *reinterpret_cast<Vector3<float>*>(mTangents + i * mVFormat.GetVertexSize());
        }

        inline Vector3<float>& Bitangent(unsigned int i)
        {
            return *reinterpret_cast<Vector3<float>*>(mBitangents + i * mVFormat.GetVertexSize());
        }

        inline Vector2<float>& TCoord(unsigned int unit, unsigned int i)
        {
            return *reinterpret_cast<Vector2<float>*>(mTCoords[unit] + i * mVFormat.GetVertexSize());
        }

        inline void SetPosition(unsigned int i, Vector3<float> const& pos)
        {
            Position(i) = pos;
        }

        void SetNormal(unsigned int i, Vector3<float> const& nor)
        {
            if (mNormals)
            {
                Normal(i) = nor;
            }
        }

        void SetTangent(unsigned int i, Vector3<float> const& tan)
        {
            if (mTangents)
            {
                Tangent(i) = tan;
            }
        }

        void SetBitangent(unsigned int i, Vector3<float> const& bin)
        {
            if (mBitangents)
            {
                Bitangent(i) = bin;
            }
        }

        void SetTCoord(unsigned int i, Vector2<float> const& tcd)
        {
            for (unsigned int unit = 0; unit < VA_MAX_TCOORD_UNITS; ++unit)
            {
                if (mAssignTCoords[unit])
                {
                    TCoord(unit, i) = tcd;
                }
            }
        }

        void SetPlatonicTCoord(unsigned int i, Vector3<float> const& pos);

        // Support for index buffers.
        void ReverseTriangleOrder(IndexBuffer* ibuffer);

        VertexFormat mVFormat;
        size_t mIndexSize;
        Resource::Usage mVBUsage, mIBUsage;
        bool mOutside;
        bool mAssignTCoords[VA_MAX_TCOORD_UNITS];

        char* mPositions;
        char* mNormals;
        char* mTangents;
        char* mBitangents;
        char* mTCoords[VA_MAX_TCOORD_UNITS];
    };
}
