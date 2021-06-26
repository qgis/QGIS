// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Texture2.h>
#include <Graphics/Visual.h>
#include <Graphics/VisualEffect.h>
#include <Mathematics/Vector2.h>

namespace gte
{
    class BumpMapEffect : public VisualEffect
    {
    public:
        // Construction.  The texture inputs must have mipmaps.
        BumpMapEffect(std::shared_ptr<ProgramFactory> const& factory,
            std::shared_ptr<Texture2> const& baseTexture,
            std::shared_ptr<Texture2> const& normalTexture,
            SamplerState::Filter filter, SamplerState::Mode mode0,
            SamplerState::Mode mode1);

        // Member access.
        virtual void SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer);

        inline std::shared_ptr<Texture2> const& GetBaseTexture() const
        {
            return mBaseTexture;
        }

        inline std::shared_ptr<Texture2> const& GetNormalTexture() const
        {
            return mNormalTexture;
        }

        inline std::shared_ptr<SamplerState> const& GetCommonSampler() const
        {
            return mCommonSampler;
        }

        // The 'mesh' is one to which an instance of this effect is attached.
        // TODO: Move this into a compute shader to improve performance.
        static void ComputeLightVectors(std::shared_ptr<Visual> const& mesh,
            Vector4<float> const& worldLightDirection);

    private:
        // Compute a tangent at the vertex P0.  The triangle is
        // counterclockwise ordered, <P0,P1,P2>.
        static bool ComputeTangent(
            Vector3<float> const& position0, Vector2<float> const& tcoord0,
            Vector3<float> const& position1, Vector2<float> const& tcoord1,
            Vector3<float> const& position2, Vector2<float> const& tcoord2,
            Vector3<float>& tangent);

        // Pixel shader parameters.
        std::shared_ptr<Texture2> mBaseTexture;
        std::shared_ptr<Texture2> mNormalTexture;
        std::shared_ptr<SamplerState> mCommonSampler;

        // Shader source code as strings.
        static std::string const msGLSLVSSource;
        static std::string const msGLSLPSSource;
        static std::string const msHLSLVSSource;
        static std::string const msHLSLPSSource;
        static ProgramSources const msVSSource;
        static ProgramSources const msPSSource;
    };
}
