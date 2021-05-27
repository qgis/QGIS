// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Visual.h>

// Expose this preprocessor symbol if you want Particles::IsValid to throw
// exceptions when the format is invalid.  The default for an invalid format
// is to return std::numeric_limits<unsigned int>::max().
//
//#define GTE_THROW_ON_PARTICLES_INVALID

namespace gte
{
    class Particles : public Visual
    {
    public:
        // Construction and destruction.  The vertex format must have 3-tuple
        // float-valued position in unit 0 and that has offset 0 (occurs first
        // in a vertex), 2-tuple float-valued texture coordinates in unit 0
        // and occurs immediately after position.  The number of particles is
        // the number of elements in positionSize.  Each particle is drawn as
        // a square billboard.  The number of vertices of the vertex buffer is
        // 4 times the number of particles and the index buffer is IP_TRIMESH
        // with 6 indices per particle, forming two triangles per particle.
        // The positionSize elements are (x,y,z,size), where the first three
        // components are the particle center and the last component
        // determines the particle billboard size.
        virtual ~Particles() = default;
        Particles(std::vector<Vector4<float>> const& positionSize, float sizeAdjust,
            VertexFormat const& vformat);

        // Member access.
        inline size_t GetNumParticles() const
        {
            return mPositionSize.size();
        }

        inline std::vector<Vector4<float>> const& GetPositionSize() const
        {
            return mPositionSize;
        }

        inline std::vector<Vector4<float>>& GetPositionSize()
        {
            return mPositionSize;
        }

        void SetSizeAdjust(float sizeAdjust);

        inline float GetSizeAdjust() const
        {
            return mSizeAdjust;
        }

        // Allow the application to specify fewer than the maximum number of
        // vertices to draw.
        void SetNumActive(unsigned int numActive);

        inline unsigned int GetNumActive() const
        {
            return mNumActive;
        }

        // The particles are billboards that always face the camera.  The
        // ParticleController class, when attached to Particles, will call
        // this function.  Afterwards, it will update the GPU copy of the
        // particles vertex buffer from the CPU copy.  If you call this
        // function explicitly, you are responsible for updating the GPU
        // copy from the CPU copy.
        void GenerateParticles(std::shared_ptr<Camera> const& camera);

    protected:
        unsigned int IsValid(VertexFormat const& vformat) const;

        std::vector<Vector4<float>> mPositionSize;
        float mSizeAdjust;
        unsigned int mNumActive;
    };
}
