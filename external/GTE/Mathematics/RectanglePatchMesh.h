// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Mesh.h>
#include <Mathematics/ParametricSurface.h>
#include <memory>

namespace gte
{
    template <typename Real>
    class RectanglePatchMesh : public Mesh<Real>
    {
    public:
        // Create a mesh (x(u,v),y(u,v),z(u,v)) defined by the specified
        // surface.  It is required that surface->IsRectangular() return
        // 'true'.
        RectanglePatchMesh(MeshDescription const& description,
            std::shared_ptr<ParametricSurface<3, Real>> const& surface)
            :
            Mesh<Real>(description, { MeshTopology::RECTANGLE }),
            mSurface(surface)
        {
            if (!this->mDescription.constructed)
            {
                // The logger system will report these errors in the Mesh
                // constructor.
                mSurface = nullptr;
                return;
            }

            LogAssert(mSurface != nullptr && mSurface->IsRectangular(),
                "A nonnull rectangular surface is required.");

            if (!this->mTCoords)
            {
                mDefaultTCoords.resize(this->mDescription.numVertices);
                this->mTCoords = mDefaultTCoords.data();
                this->mTCoordStride = sizeof(Vector2<Real>);

                this->mDescription.allowUpdateFrame = this->mDescription.wantDynamicTangentSpaceUpdate;
                if (this->mDescription.allowUpdateFrame)
                {
                    if (!this->mDescription.hasTangentSpaceVectors)
                    {
                        this->mDescription.allowUpdateFrame = false;
                    }

                    if (!this->mNormals)
                    {
                        this->mDescription.allowUpdateFrame = false;
                    }
                }
            }

            this->ComputeIndices();
            InitializeTCoords();
            InitializePositions();
            if (this->mDescription.allowUpdateFrame)
            {
                InitializeFrame();
            }
            else if (this->mNormals)
            {
                InitializeNormals();
            }
        }

        // Member access.
        inline std::shared_ptr<ParametricSurface<3, Real>> const& GetSurface() const
        {
            return mSurface;
        }

    protected:
        void InitializeTCoords()
        {
            Real uMin = mSurface->GetUMin();
            Real uDelta = (mSurface->GetUMax() - uMin) / static_cast<Real>(this->mDescription.numCols - 1);
            Real vMin = mSurface->GetVMin();
            Real vDelta = (mSurface->GetVMax() - vMin) / static_cast<Real>(this->mDescription.numRows - 1);
            Vector2<Real> tcoord;
            for (uint32_t r = 0, i = 0; r < this->mDescription.numRows; ++r)
            {
                tcoord[1] = vMin + vDelta * (Real)r;
                for (uint32_t c = 0; c < this->mDescription.numCols; ++c, ++i)
                {
                    tcoord[0] = uMin + uDelta * (Real)c;
                    this->TCoord(i) = tcoord;
                }
            }
        }

        void InitializePositions()
        {
            for (uint32_t r = 0, i = 0; r < this->mDescription.numRows; ++r)
            {
                for (uint32_t c = 0; c < this->mDescription.numCols; ++c, ++i)
                {
                    Vector2<Real> tcoord = this->TCoord(i);
                    this->Position(i) = mSurface->GetPosition(tcoord[0], tcoord[1]);
                }
            }
        }

        void InitializeNormals()
        {
            for (uint32_t r = 0, i = 0; r < this->mDescription.numRows; ++r)
            {
                for (uint32_t c = 0; c < this->mDescription.numCols; ++c, ++i)
                {
                    Vector2<Real> tcoord = this->TCoord(i);
                    Vector3<Real> values[6];
                    mSurface->Evaluate(tcoord[0], tcoord[1], 1, values);
                    Normalize(values[1], true);
                    Normalize(values[2], true);
                    this->Normal(i) = UnitCross(values[1], values[2], true);
                }
            }
        }

        void InitializeFrame()
        {
            for (unsigned int r = 0, i = 0; r < this->mDescription.numRows; ++r)
            {
                for (unsigned int c = 0; c < this->mDescription.numCols; ++c, ++i)
                {
                    Vector2<Real> tcoord = this->TCoord(i);
                    Vector3<Real> values[6];
                    mSurface->Evaluate(tcoord[0], tcoord[1], 1, values);
                    Normalize(values[1], true);
                    Normalize(values[2], true);

                    if (this->mDPDUs)
                    {
                        this->DPDU(i) = values[1];
                    }
                    if (this->mDPDVs)
                    {
                        this->DPDV(i) = values[2];
                    }

                    ComputeOrthogonalComplement(2, &values[1], true);

                    if (this->mNormals)
                    {
                        this->Normal(i) = values[3];
                    }
                    if (this->mTangents)
                    {
                        this->Tangent(i) = values[1];
                    }
                    if (this->mBitangents)
                    {
                        this->Bitangent(i) = values[2];
                    }
                }
            }
        }

        virtual void UpdatePositions() override
        {
            if (mSurface)
            {
                InitializePositions();
            }
        }

        virtual void UpdateNormals() override
        {
            if (mSurface)
            {
                InitializeNormals();
            }
        }

        virtual void UpdateFrame() override
        {
            if (mSurface)
            {
                InitializeFrame();
            }
        }

        std::shared_ptr<ParametricSurface<3, Real>> mSurface;

        // If the client does not request texture coordinates, they will be
        // computed internally for use in evaluation of the surface geometry.
        std::vector<Vector2<Real>> mDefaultTCoords;
    };
}
