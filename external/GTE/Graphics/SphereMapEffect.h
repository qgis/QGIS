// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/VisualEffect.h>

namespace gte
{
    class SphereMapEffect : public VisualEffect
    {
    public:
        // Construction.
        SphereMapEffect(std::shared_ptr<ProgramFactory> const& factory,
            std::shared_ptr<Texture2> const& texture, SamplerState::Filter filter,
            SamplerState::Mode mode0, SamplerState::Mode mode1);

        // Member access.
        virtual void SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer);

        inline void SetVWMatrix(Matrix4x4<float> const& vwMatrix)
        {
            *mVWMatrixConstant->Get<Matrix4x4<float>>() = vwMatrix;
        }

        inline Matrix4x4<float> const& GetVWMatrix() const
        {
            return *mVWMatrixConstant->Get<Matrix4x4<float>>();
        }

        inline std::shared_ptr<ConstantBuffer> const& GetVWMatrixConstant() const
        {
            return mVWMatrixConstant;
        }

        inline std::shared_ptr<Texture2> const& GetTexture() const
        {
            return mTexture;
        }

        inline std::shared_ptr<SamplerState> const& GetSampler() const
        {
            return mSampler;
        }

    private:
        // Vertex shader parameter.
        std::shared_ptr<ConstantBuffer> mVWMatrixConstant;

        // Pixel shader parameters.
        std::shared_ptr<Texture2> mTexture;
        std::shared_ptr<SamplerState> mSampler;

        // Shader source code as strings.
        static std::string const msGLSLVSSource;
        static std::string const msGLSLPSSource;
        static std::string const msHLSLVSSource;
        static std::string const msHLSLPSSource;
        static ProgramSources const msVSSource;
        static ProgramSources const msPSSource;
    };
}
