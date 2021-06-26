// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.12.05

#pragma once

#include <Graphics/VisualEffect.h>

namespace gte
{
    class AreaLightEffect : public VisualEffect
    {
    public:
        struct Parameters
        {
            Vector4<float> ambient;
            Vector4<float> diffuse;
            Vector4<float> specular;
            Vector4<float> attenuation;
            Vector4<float> position;    // (x,y,z,1)
            Vector4<float> normal;      // (x,y,z,0)
            Vector4<float> axis0;       // (x,y,z,0)
            Vector4<float> axis1;       // (x,y,z,0)
            Vector4<float> extent;      // (extent0, extent1, *, *)
        };

        // Construction.  The shader constants are not initialized by the
        // constructor.  They must be initialized by the application before
        // the first use of the effect.
        AreaLightEffect(std::shared_ptr<ProgramFactory> const& factory,
            std::shared_ptr<Texture2> const& baseTexture,
            std::shared_ptr<Texture2> const& normalTexture, SamplerState::Filter filter,
            SamplerState::Mode mode0, SamplerState::Mode mode1);

        // Required to bind and update resources.
        virtual void SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer);

        inline std::shared_ptr<ConstantBuffer> const& GetMaterialConstant() const
        {
            return mMaterialConstant;
        }

        inline std::shared_ptr<ConstantBuffer> const& GetCameraConstant() const
        {
            return mCameraConstant;
        }

        inline std::shared_ptr<ConstantBuffer> const& GetAreaLightConstant() const
        {
            return mAreaLightConstant;
        }

    private:
        // Pixel shader parameters.
        std::shared_ptr<ConstantBuffer> mMaterialConstant;
        std::shared_ptr<ConstantBuffer> mCameraConstant;
        std::shared_ptr<ConstantBuffer> mAreaLightConstant;
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
