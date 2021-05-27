// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Matrix.h>
#include <Mathematics/IndexAttribute.h>
#include <Mathematics/VertexAttribute.h>
#include <Mathematics/Vector2.h>
#include <Mathematics/Vector3.h>

// The Mesh class is designed to support triangulations of surfaces of a small
// number of topologies. See the documents
//   https://www.geometrictools.com/MeshDifferentialGeometry.pdf
//   https://www.geometrictools.com/MeshFactory.pdf
// for details.
//
// You must set the vertex attribute sources before calling Update().
//
// The semantic "position" is required and its source must be an array
// of Real with at least 3 channels so that positions are computed as
// Vector3<Real>.
//
// The positions are assumed to be parameterized by texture coordinates
// (u,v); the position is thought of as a function P(u,v).  If texture
// coordinates are provided, the semantic must be "tcoord".  If texture
// coordinates are not provided, default texture coordinates are computed
// internally as described in the mesh factory document.
//
// The frame for the tangent space is optional.  All vectors in the frame
// must have sources that are arrays of Real with at least 3 channels per
// attribute.  If normal vectors are provided, the semantic must be
// "normal".
//
// Two options are supported for tangent vectors.  The first option is
// that the tangents are surface derivatives dP/du and dP/dv, which are
// not necessarily unit length or orthogonal.  The semantics must be
// "dpdu" and "dpdv".  The second option is that the tangents are unit
// length and orthogonal, with the infrequent possibility that a vertex
// is degenerate in that dP/du and dP/dv are linearly dependent.  The
// semantics must be "tangent" and "bitangent".
//
// For each provided vertex attribute, a derived class can initialize
// that attribute by overriding one of the Initialize*() functions whose
// stubs are defined in this class.

namespace gte
{
    enum class MeshTopology
    {
        ARBITRARY,
        RECTANGLE,
        CYLINDER,
        TORUS,
        DISK,
        SPHERE
    };

    class MeshDescription
    {
    public:
        // Constructor for MeshTopology::ARBITRARY.  The members topology,
        // numVertices, and numTriangles are set in the obvious manner.  The
        // members numRows and numCols are set to zero. The remaining members
        // must be set explicitly by the client.
        MeshDescription(uint32_t inNumVertices, uint32_t inNumTriangles)
            :
            topology(MeshTopology::ARBITRARY),
            numVertices(inNumVertices),
            numTriangles(inNumTriangles),
            wantDynamicTangentSpaceUpdate(false),
            wantCCW(true),
            hasTangentSpaceVectors(false),
            allowUpdateFrame(false),
            numRows(0),
            numCols(0),
            rMax(0),
            cMax(0),
            rIncrement(0),
            constructed(false)
        {
            LogAssert(numVertices >= 3 && numTriangles >= 1, "Invalid input.");
        }

        // Constructor for topologies other than MeshTopology::ARBITRARY.
        // Compute the number of vertices and triangles for the mesh based on
        // the requested number of rows and columns.  If the number of rows or
        // columns is invalid for the specified topology, they are modified to
        // be valid, in which case inNumRows/numRows and inNumCols/numCols can
        // differ.  If the input topology is MeshTopology::ARBITRARY, then
        // inNumRows and inNumCols are assigned to numVertices and
        // numTriangles, respectively, and numRows and numCols are set to
        // zero.  The remaining members must be set explicitly by the client.
        MeshDescription(MeshTopology inTopology, uint32_t inNumRows, uint32_t inNumCols)
            :
            topology(inTopology),
            wantDynamicTangentSpaceUpdate(false),
            wantCCW(true),
            hasTangentSpaceVectors(false),
            allowUpdateFrame(false),
            constructed(false)
        {
            switch (topology)
            {
            case MeshTopology::ARBITRARY:
                numVertices = inNumRows;
                numTriangles = inNumCols;
                numRows = 0;
                numCols = 0;
                rMax = 0;
                cMax = 0;
                rIncrement = 0;
                break;

            case MeshTopology::RECTANGLE:
                numRows = std::max(inNumRows, 2u);
                numCols = std::max(inNumCols, 2u);
                rMax = numRows - 1;
                cMax = numCols - 1;
                rIncrement = numCols;
                numVertices = (rMax + 1) * (cMax + 1);
                numTriangles = 2 * rMax * cMax;
                break;

            case MeshTopology::CYLINDER:
                numRows = std::max(inNumRows, 2u);
                numCols = std::max(inNumCols, 3u);
                rMax = numRows - 1;
                cMax = numCols;
                rIncrement = numCols + 1;
                numVertices = (rMax + 1) * (cMax + 1);
                numTriangles = 2 * rMax * cMax;
                break;

            case MeshTopology::TORUS:
                numRows = std::max(inNumRows, 2u);
                numCols = std::max(inNumCols, 3u);
                rMax = numRows;
                cMax = numCols;
                rIncrement = numCols + 1;
                numVertices = (rMax + 1) * (cMax + 1);
                numTriangles = 2 * rMax * cMax;
                break;

            case MeshTopology::DISK:
                numRows = std::max(inNumRows, 1u);
                numCols = std::max(inNumCols, 3u);
                rMax = numRows - 1;
                cMax = numCols;
                rIncrement = numCols + 1;
                numVertices = (rMax + 1) * (cMax + 1) + 1;
                numTriangles = 2 * rMax * cMax + numCols;
                break;

            case MeshTopology::SPHERE:
                numRows = std::max(inNumRows, 1u);
                numCols = std::max(inNumCols, 3u);
                rMax = numRows - 1;
                cMax = numCols;
                rIncrement = numCols + 1;
                numVertices = (rMax + 1) * (cMax + 1) + 2;
                numTriangles = 2 * rMax * cMax + 2 * numCols;
                break;
            }
        }

        MeshTopology topology;
        uint32_t numVertices;
        uint32_t numTriangles;
        std::vector<VertexAttribute> vertexAttributes;
        IndexAttribute indexAttribute;
        bool wantDynamicTangentSpaceUpdate;  // default: false
        bool wantCCW;  // default: true

        // For internal use only.
        bool hasTangentSpaceVectors;
        bool allowUpdateFrame;
        uint32_t numRows, numCols;
        uint32_t rMax, cMax, rIncrement;

        // After an attempt to construct a Mesh or Mesh-derived object,
        // examine this value to determine whether the construction was
        // successful.
        bool constructed;
    };

    template <typename Real>
    class Mesh
    {
    public:
        // Construction and destruction.  This constructor is for ARBITRARY
        // topology.  The vertices and indices must already be assigned by the
        // client.  Derived classes use the protected constructor, but
        // assignment of vertices and indices occurs in the derived-class
        // constructors.
        Mesh(MeshDescription const& description, std::vector<MeshTopology> const& validTopologies)
            :
            mDescription(description),
            mPositions(nullptr),
            mNormals(nullptr),
            mTangents(nullptr),
            mBitangents(nullptr),
            mDPDUs(nullptr),
            mDPDVs(nullptr),
            mTCoords(nullptr),
            mPositionStride(0),
            mNormalStride(0),
            mTangentStride(0),
            mBitangentStride(0),
            mDPDUStride(0),
            mDPDVStride(0),
            mTCoordStride(0)
        {
            mDescription.constructed = false;
            for (auto const& topology : validTopologies)
            {
                if (mDescription.topology == topology)
                {
                    mDescription.constructed = true;
                    break;
                }
            }

            LogAssert(mDescription.indexAttribute.source != nullptr,
                "The mesh needs triangles/indices in Mesh constructor.");

            // Set sources for the requested vertex attributes.
            mDescription.hasTangentSpaceVectors = false;
            mDescription.allowUpdateFrame = mDescription.wantDynamicTangentSpaceUpdate;
            for (auto const& attribute : mDescription.vertexAttributes)
            {
                if (attribute.source != nullptr && attribute.stride > 0)
                {
                    if (attribute.semantic == "position")
                    {
                        mPositions = reinterpret_cast<Vector3<Real>*>(attribute.source);
                        mPositionStride = attribute.stride;
                        continue;
                    }

                    if (attribute.semantic == "normal")
                    {
                        mNormals = reinterpret_cast<Vector3<Real>*>(attribute.source);
                        mNormalStride = attribute.stride;
                        continue;
                    }

                    if (attribute.semantic == "tangent")
                    {
                        mTangents = reinterpret_cast<Vector3<Real>*>(attribute.source);
                        mTangentStride = attribute.stride;
                        mDescription.hasTangentSpaceVectors = true;
                        continue;
                    }

                    if (attribute.semantic == "bitangent")
                    {
                        mBitangents = reinterpret_cast<Vector3<Real>*>(attribute.source);
                        mBitangentStride = attribute.stride;
                        mDescription.hasTangentSpaceVectors = true;
                        continue;
                    }

                    if (attribute.semantic == "dpdu")
                    {
                        mDPDUs = reinterpret_cast<Vector3<Real>*>(attribute.source);
                        mDPDUStride = attribute.stride;
                        mDescription.hasTangentSpaceVectors = true;
                        continue;
                    }

                    if (attribute.semantic == "dpdv")
                    {
                        mDPDVs = reinterpret_cast<Vector3<Real>*>(attribute.source);
                        mDPDVStride = attribute.stride;
                        mDescription.hasTangentSpaceVectors = true;
                        continue;
                    }

                    if (attribute.semantic == "tcoord")
                    {
                        mTCoords = reinterpret_cast<Vector2<Real>*>(attribute.source);
                        mTCoordStride = attribute.stride;
                        continue;
                    }
                }
            }

            LogAssert(mPositions != nullptr, "The mesh needs positions in Mesh constructor.");

            // The initial value of allowUpdateFrame is the client request
            // about wanting dynamic tangent-space updates.  If the vertex
            // attributes do not include tangent-space vectors, then dynamic
            // updates are not necessary.  If tangent-space vectors are
            // present, the update algorithm requires texture coordinates
            // (mTCoords must be nonnull) or must compute local coordinates
            // (mNormals must be nonnull).
            if (mDescription.allowUpdateFrame)
            {
                if (!mDescription.hasTangentSpaceVectors)
                {
                    mDescription.allowUpdateFrame = false;
                }

                if (!mTCoords && !mNormals)
                {
                    mDescription.allowUpdateFrame = false;
                }
            }

            if (mDescription.allowUpdateFrame)
            {
                mUTU.resize(mDescription.numVertices);
                mDTU.resize(mDescription.numVertices);
            }
        }

        virtual ~Mesh()
        {
        }

        // No copying or assignment is allowed.
        Mesh(Mesh const&) = delete;
        Mesh& operator=(Mesh const&) = delete;

        // Member accessors.
        inline MeshDescription const& GetDescription() const
        {
            return mDescription;
        }

        // If the underlying geometric data varies dynamically, call this
        // function to update whatever vertex attributes are specified by
        // the vertex pool.
        void Update()
        {
            LogAssert(mDescription.constructed, "The Mesh object failed the construction.");

            UpdatePositions();

            if (mDescription.allowUpdateFrame)
            {
                UpdateFrame();
            }
            else if (mNormals)
            {
                UpdateNormals();
            }
            // else: The mesh has no frame data, so there is nothing to do.
        }

    protected:
        // Access the vertex attributes.
        inline Vector3<Real>& Position(uint32_t i)
        {
            char* positions = reinterpret_cast<char*>(mPositions);
            return *reinterpret_cast<Vector3<Real>*>(positions + i * mPositionStride);
        }

        inline Vector3<Real>& Normal(uint32_t i)
        {
            char* normals = reinterpret_cast<char*>(mNormals);
            return *reinterpret_cast<Vector3<Real>*>(normals + i * mNormalStride);
        }

        inline Vector3<Real>& Tangent(uint32_t i)
        {
            char* tangents = reinterpret_cast<char*>(mTangents);
            return *reinterpret_cast<Vector3<Real>*>(tangents + i * mTangentStride);
        }

        inline Vector3<Real>& Bitangent(uint32_t i)
        {
            char* bitangents = reinterpret_cast<char*>(mBitangents);
            return *reinterpret_cast<Vector3<Real>*>(bitangents + i * mBitangentStride);
        }

        inline Vector3<Real>& DPDU(uint32_t i)
        {
            char* dpdus = reinterpret_cast<char*>(mDPDUs);
            return *reinterpret_cast<Vector3<Real>*>(dpdus + i * mDPDUStride);
        }

        inline Vector3<Real>& DPDV(uint32_t i)
        {
            char* dpdvs = reinterpret_cast<char*>(mDPDVs);
            return *reinterpret_cast<Vector3<Real>*>(dpdvs + i * mDPDVStride);
        }

        inline Vector2<Real>& TCoord(uint32_t i)
        {
            char* tcoords = reinterpret_cast<char*>(mTCoords);
            return *reinterpret_cast<Vector2<Real>*>(tcoords + i * mTCoordStride);
        }

        // Compute the indices for non-arbitrary topologies.  This function is
        // called by derived classes.
        void ComputeIndices()
        {
            uint32_t t = 0;
            for (uint32_t r = 0, i = 0; r < mDescription.rMax; ++r)
            {
                uint32_t v0 = i, v1 = v0 + 1;
                i += mDescription.rIncrement;
                uint32_t v2 = i, v3 = v2 + 1;
                for (uint32_t c = 0; c < mDescription.cMax; ++c, ++v0, ++v1, ++v2, ++v3)
                {
                    if (mDescription.wantCCW)
                    {
                        mDescription.indexAttribute.SetTriangle(t++, v0, v1, v2);
                        mDescription.indexAttribute.SetTriangle(t++, v1, v3, v2);
                    }
                    else
                    {
                        mDescription.indexAttribute.SetTriangle(t++, v0, v2, v1);
                        mDescription.indexAttribute.SetTriangle(t++, v1, v2, v3);
                    }
                }
            }

            if (mDescription.topology == MeshTopology::DISK)
            {
                uint32_t v0 = 0, v1 = 1, v2 = mDescription.numVertices - 1;
                for (unsigned int c = 0; c < mDescription.numCols; ++c, ++v0, ++v1)
                {
                    if (mDescription.wantCCW)
                    {
                        mDescription.indexAttribute.SetTriangle(t++, v0, v2, v1);
                    }
                    else
                    {
                        mDescription.indexAttribute.SetTriangle(t++, v0, v1, v2);
                    }
                }
            }
            else if (mDescription.topology == MeshTopology::SPHERE)
            {
                uint32_t v0 = 0, v1 = 1, v2 = mDescription.numVertices - 2;
                for (uint32_t c = 0; c < mDescription.numCols; ++c, ++v0, ++v1)
                {
                    if (mDescription.wantCCW)
                    {
                        mDescription.indexAttribute.SetTriangle(t++, v0, v2, v1);
                    }
                    else
                    {
                        mDescription.indexAttribute.SetTriangle(t++, v0, v1, v2);
                    }
                }

                v0 = (mDescription.numRows - 1) * mDescription.numCols;
                v1 = v0 + 1;
                v2 = mDescription.numVertices - 1;
                for (uint32_t c = 0; c < mDescription.numCols; ++c, ++v0, ++v1)
                {
                    if (mDescription.wantCCW)
                    {
                        mDescription.indexAttribute.SetTriangle(t++, v0, v2, v1);
                    }
                    else
                    {
                        mDescription.indexAttribute.SetTriangle(t++, v0, v1, v2);
                    }
                }
            }
        }

        // The Update() function allows derived classes to use algorithms
        // different from least-squares fitting to compute the normals (when
        // no tangent-space information is requested) or to compute the frame
        // (normals and tangent space).  The UpdatePositions() is a stub; the
        // base-class has no knowledge about how positions should be modified.
        // A derived class, however, might choose to use dynamic updating
        // and override UpdatePositions().  The base-class UpdateNormals()
        // computes vertex normals as averages of area-weighted triangle
        // normals (nonparametric approach).  The base-class UpdateFrame()
        // uses a least-squares algorithm for estimating the tangent space
        // (parametric approach).
        virtual void UpdatePositions()
        {
        }

        virtual void UpdateNormals()
        {
            // Compute normal vector as normalized weighted averages of triangle
            // normal vectors.

            // Set the normals to zero to allow accumulation of triangle normals.
            Vector3<Real> zero{ (Real)0, (Real)0, (Real)0 };
            for (uint32_t i = 0; i < mDescription.numVertices; ++i)
            {
                Normal(i) = zero;
            }

            // Accumulate the triangle normals.
            for (uint32_t t = 0; t < mDescription.numTriangles; ++t)
            {
                // Get the positions for the triangle.
                uint32_t v0, v1, v2;
                mDescription.indexAttribute.GetTriangle(t, v0, v1, v2);
                Vector3<Real> P0 = Position(v0);
                Vector3<Real> P1 = Position(v1);
                Vector3<Real> P2 = Position(v2);

                // Get the edge vectors.
                Vector3<Real> E1 = P1 - P0;
                Vector3<Real> E2 = P2 - P0;

                // Compute a triangle normal show length is twice the area of the
                // triangle.
                Vector3<Real> triangleNormal = Cross(E1, E2);

                // Accumulate the triangle normals.
                Normal(v0) += triangleNormal;
                Normal(v1) += triangleNormal;
                Normal(v2) += triangleNormal;
            }

            // Normalize the normals.
            for (uint32_t i = 0; i < mDescription.numVertices; ++i)
            {
                Normalize(Normal(i), true);
            }
        }

        virtual void UpdateFrame()
        {
            if (!mTCoords)
            {
                // We need to compute vertex normals first in order to compute
                // local texture coordinates.  The vertex normals are recomputed
                // later based on estimated tangent vectors.
                UpdateNormals();
            }

            // Use the least-squares algorithm to estimate the tangent-space vectors
            // and, if requested, normal vectors.
            Matrix<2, 2, Real> zero2x2;  // initialized to zero
            Matrix<3, 2, Real> zero3x2;  // initialized to zero
            std::fill(mUTU.begin(), mUTU.end(), zero2x2);
            std::fill(mDTU.begin(), mDTU.end(), zero3x2);
            for (uint32_t t = 0; t < mDescription.numTriangles; ++t)
            {
                // Get the positions and differences for the triangle.
                uint32_t v0, v1, v2;
                mDescription.indexAttribute.GetTriangle(t, v0, v1, v2);
                Vector3<Real> P0 = Position(v0);
                Vector3<Real> P1 = Position(v1);
                Vector3<Real> P2 = Position(v2);
                Vector3<Real> D10 = P1 - P0;
                Vector3<Real> D20 = P2 - P0;
                Vector3<Real> D21 = P2 - P1;

                if (mTCoords)
                {
                    // Get the texture coordinates and differences for the triangle.
                    Vector2<Real> C0 = TCoord(v0);
                    Vector2<Real> C1 = TCoord(v1);
                    Vector2<Real> C2 = TCoord(v2);
                    Vector2<Real> U10 = C1 - C0;
                    Vector2<Real> U20 = C2 - C0;
                    Vector2<Real> U21 = C2 - C1;

                    // Compute the outer products.
                    Matrix<2, 2, Real> outerU10 = OuterProduct(U10, U10);
                    Matrix<2, 2, Real> outerU20 = OuterProduct(U20, U20);
                    Matrix<2, 2, Real> outerU21 = OuterProduct(U21, U21);
                    Matrix<3, 2, Real> outerD10 = OuterProduct(D10, U10);
                    Matrix<3, 2, Real> outerD20 = OuterProduct(D20, U20);
                    Matrix<3, 2, Real> outerD21 = OuterProduct(D21, U21);

                    // Keep a running sum of U^T*U and D^T*U.
                    mUTU[v0] += outerU10 + outerU20;
                    mUTU[v1] += outerU10 + outerU21;
                    mUTU[v2] += outerU20 + outerU21;
                    mDTU[v0] += outerD10 + outerD20;
                    mDTU[v1] += outerD10 + outerD21;
                    mDTU[v2] += outerD20 + outerD21;
                }
                else
                {
                    // Compute local coordinates and differences for the triangle.
                    Vector3<Real> basis[3];

                    basis[0] = Normal(v0);
                    ComputeOrthogonalComplement(1, basis, true);
                    Vector2<Real> U10{ Dot(basis[1], D10), Dot(basis[2], D10) };
                    Vector2<Real> U20{ Dot(basis[1], D20), Dot(basis[2], D20) };
                    mUTU[v0] += OuterProduct(U10, U10) + OuterProduct(U20, U20);
                    mDTU[v0] += OuterProduct(D10, U10) + OuterProduct(D20, U20);

                    basis[0] = Normal(v1);
                    ComputeOrthogonalComplement(1, basis, true);
                    Vector2<Real> U01{ Dot(basis[1], D10), Dot(basis[2], D10) };
                    Vector2<Real> U21{ Dot(basis[1], D21), Dot(basis[2], D21) };
                    mUTU[v1] += OuterProduct(U01, U01) + OuterProduct(U21, U21);
                    mDTU[v1] += OuterProduct(D10, U01) + OuterProduct(D21, U21);

                    basis[0] = Normal(v2);
                    ComputeOrthogonalComplement(1, basis, true);
                    Vector2<Real> U02{ Dot(basis[1], D20), Dot(basis[2], D20) };
                    Vector2<Real> U12{ Dot(basis[1], D21), Dot(basis[2], D21) };
                    mUTU[v2] += OuterProduct(U02, U02) + OuterProduct(U12, U12);
                    mDTU[v2] += OuterProduct(D20, U02) + OuterProduct(D21, U12);
                }

            }

            for (uint32_t i = 0; i < mDescription.numVertices; ++i)
            {
                Matrix<3, 2, Real> jacobian = mDTU[i] * Inverse(mUTU[i]);

                Vector3<Real> basis[3];
                basis[0] = { jacobian(0, 0), jacobian(1, 0), jacobian(2, 0) };
                basis[1] = { jacobian(0, 1), jacobian(1, 1), jacobian(2, 1) };

                if (mDPDUs)
                {
                    DPDU(i) = basis[0];
                }
                if (mDPDVs)
                {
                    DPDV(i) = basis[1];
                }

                ComputeOrthogonalComplement(2, basis, true);

                if (mNormals)
                {
                    Normal(i) = basis[2];
                }
                if (mTangents)
                {
                    Tangent(i) = basis[0];
                }
                if (mBitangents)
                {
                    Bitangent(i) = basis[1];
                }
            }
        }

        // Constructor inputs.
        // The client requests this via the constructor; however, if it is
        // requested and the vertex attributes do not contain entries for
        // "tangent", "bitangent", "dpdu", or "dpdv", then this member is
        // set to false.
        MeshDescription mDescription;

        // Copied from mVertexAttributes when available.
        Vector3<Real>* mPositions;
        Vector3<Real>* mNormals;
        Vector3<Real>* mTangents;
        Vector3<Real>* mBitangents;
        Vector3<Real>* mDPDUs;
        Vector3<Real>* mDPDVs;
        Vector2<Real>* mTCoords;
        size_t mPositionStride;
        size_t mNormalStride;
        size_t mTangentStride;
        size_t mBitangentStride;
        size_t mDPDUStride;
        size_t mDPDVStride;
        size_t mTCoordStride;

        // When dynamic tangent-space updates are requested, the update algorithm
        // requires texture coordinates (user-specified or non-local).  It is
        // possible to create a vertex-adjacent set (with indices into the
        // vertex array) for each mesh vertex; however, instead we rely on a
        // triangle iteration and incrementally store the information needed for
        // the estimation of the tangent space.  Each vertex has associated
        // matrices D and U, but we need to store only U^T*U and D^T*U.  See the
        // PDF for details.
        std::vector<Matrix<2, 2, Real>> mUTU;
        std::vector<Matrix<3, 2, Real>> mDTU;
    };
}
