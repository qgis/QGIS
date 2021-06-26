// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Mesh.h>
#include <Mathematics/FrenetFrame.h>
#include <functional>
#include <memory>

namespace gte
{
    template <typename Real>
    class TubeMesh : public Mesh<Real>
    {
    public:
        // Create a mesh (x(u,v),y(u,v),z(u,v)) defined by the specified
        // medial curve and radial function.  The mesh has torus topology
        // when 'closed' is true and has cylinder topology when 'closed' is
        // false.  The client is responsible for setting the topology
        // correctly in the 'description' input.  The rows correspond to
        // medial samples and the columns correspond to radial samples.  The
        // medial curve is sampled according to its natural t-parameter when
        // 'sampleByArcLength' is false; otherwise, it is sampled uniformly
        // in arclength.  TODO: Allow TORUS and remove the 'closed' input
        TubeMesh(MeshDescription const& description,
            std::shared_ptr<ParametricCurve<3, Real>> const& medial,
            std::function<Real(Real)> const& radial, bool closed,
            bool sampleByArcLength, Vector3<Real> upVector)
            :
            Mesh<Real>(description, { MeshTopology::CYLINDER }),
            mMedial(medial),
            mRadial(radial),
            mClosed(closed),
            mSampleByArcLength(sampleByArcLength),
            mUpVector(upVector)
        {
            if (!this->mDescription.constructed)
            {
                // The logger system will report these errors in the Mesh
                // constructor.
                mMedial = nullptr;
                return;
            }

            LogAssert(mMedial != nullptr, "A nonnull medial curve is required.");

            mCosAngle.resize(this->mDescription.numCols);
            mSinAngle.resize(this->mDescription.numCols);
            Real invRadialSamples = (Real)1 / (Real)(this->mDescription.numCols - 1);
            for (unsigned int i = 0; i < this->mDescription.numCols - 1; ++i)
            {
                Real angle = i * invRadialSamples * (Real)GTE_C_TWO_PI;
                mCosAngle[i] = std::cos(angle);
                mSinAngle[i] = std::sin(angle);
            }
            mCosAngle[this->mDescription.numCols - 1] = mCosAngle[0];
            mSinAngle[this->mDescription.numCols - 1] = mSinAngle[0];

            Real invDenom;
            if (mClosed)
            {
                invDenom = (Real)1 / (Real)this->mDescription.numRows;
            }
            else
            {
                invDenom = (Real)1 / (Real)(this->mDescription.numRows - 1);
            }

            Real factor;
            if (mSampleByArcLength)
            {
                factor = mMedial->GetTotalLength() * invDenom;
                mTSampler = [this, factor](unsigned int row)
                {
                    return mMedial->GetTime(row * factor);
                };
            }
            else
            {
                factor = (mMedial->GetTMax() - mMedial->GetTMin()) * invDenom;
                mTSampler = [this, factor](unsigned int row)
                {
                    return mMedial->GetTMin() + row * factor;
                };
            }

            if (mUpVector != Vector3<Real>::Zero())
            {
                mFSampler = [this](Real t)
                {
                    std::array<Vector3<Real>, 4> frame;
                    frame[0] = mMedial->GetPosition(t);
                    frame[1] = mMedial->GetTangent(t);
                    frame[3] = UnitCross(frame[1], mUpVector);
                    frame[2] = UnitCross(frame[3], frame[1]);
                    return frame;
                };
            }
            else
            {
                mFrenet = std::make_unique<FrenetFrame3<Real>>(mMedial);
                mFSampler = [this](Real t)
                {
                    std::array<Vector3<Real>, 4> frame;
                    (*mFrenet)(t, frame[0], frame[1], frame[2], frame[3]);
                    return frame;
                };
            }

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
            UpdatePositions();
            if (this->mDescription.allowUpdateFrame)
            {
                this->UpdateFrame();
            }
            else if (this->mNormals)
            {
                this->UpdateNormals();
            }
        }

        // Member access.
        inline std::shared_ptr<ParametricCurve<3, Real>> const& GetMedial() const
        {
            return mMedial;
        }

        inline std::function<Real(Real)> const& GetRadial() const
        {
            return mRadial;
        }

        inline bool IsClosed() const
        {
            return mClosed;
        }

        inline bool IsSampleByArcLength() const
        {
            return mSampleByArcLength;
        }

        inline Vector3<Real> const& GetUpVector() const
        {
            return mUpVector;
        }

    private:
        void InitializeTCoords()
        {
            Vector2<Real>tcoord;
            for (unsigned int r = 0, i = 0; r < this->mDescription.numRows; ++r)
            {
                tcoord[1] = (Real)r / (Real)this->mDescription.rMax;
                for (unsigned int c = 0; c <= this->mDescription.numCols; ++c, ++i)
                {
                    tcoord[0] = (Real)c / (Real)this->mDescription.numCols;
                    this->TCoord(i) = tcoord;
                }
            }
        }

        virtual void UpdatePositions() override
        {
            uint32_t row, col, v, save;
            for (row = 0, v = 0; row < this->mDescription.numRows; ++row, ++v)
            {
                Real t = mTSampler(row);
                Real radius = mRadial(t);
                // frame = (position, tangent, normal, binormal)
                std::array<Vector3<Real>, 4> frame = mFSampler(t);
                for (col = 0, save = v; col < this->mDescription.numCols; ++col, ++v)
                {
                    this->Position(v) = frame[0] + radius * (mCosAngle[col] * frame[2] +
                        mSinAngle[col] * frame[3]);
                }
                this->Position(v) = this->Position(save);
            }

            if (mClosed)
            {
                for (col = 0; col < this->mDescription.numCols; ++col)
                {
                    uint32_t i0 = col;
                    uint32_t i1 = col + this->mDescription.numCols * (this->mDescription.numRows - 1);
                    this->Position(i1) = this->Position(i0);
                }
            }
        }

        std::shared_ptr<ParametricCurve<3, Real>> mMedial;
        std::function<Real(Real)> mRadial;
        bool mClosed, mSampleByArcLength;
        Vector3<Real> mUpVector;
        std::vector<Real> mCosAngle, mSinAngle;
        std::function<Real(unsigned int)> mTSampler;
        std::function<std::array<Vector3<Real>, 4>(Real)> mFSampler;
        std::unique_ptr<FrenetFrame3<Real>> mFrenet;

        // If the client does not request texture coordinates, they will be
        // computed internally for use in evaluation of the surface geometry.
        std::vector<Vector2<Real>> mDefaultTCoords;
    };
}
