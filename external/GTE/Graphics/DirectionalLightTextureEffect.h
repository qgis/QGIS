// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/LightEffect.h>
#include <Graphics/Texture2.h>

namespace gte
{
    class DirectionalLightTextureEffect : public LightEffect
    {
    public:
        // Construction.
        DirectionalLightTextureEffect(std::shared_ptr<ProgramFactory> const& factory,
            BufferUpdater const& updater, std::shared_ptr<Material> const& material,
            std::shared_ptr<Lighting> const& lighting, std::shared_ptr<LightCameraGeometry> const& geometry,
            std::shared_ptr<Texture2> const& texture, SamplerState::Filter filter,
            SamplerState::Mode mode0, SamplerState::Mode mode1);

        // Member access.
        inline std::shared_ptr<Texture2> const& GetTexture() const
        {
            return mTexture;
        }

        inline std::shared_ptr<SamplerState> const& GetSampler() const
        {
            return mSampler;
        }

        // After you set or modify 'material', 'light', or 'geometry', call
        // the update to inform any listener that the corresponding constant
        // buffer has changed.
        virtual void UpdateMaterialConstant();
        virtual void UpdateLightingConstant();
        virtual void UpdateGeometryConstant();

    private:
        struct InternalMaterial
        {
            Vector4<float> emissive;
            Vector4<float> ambient;
            Vector4<float> diffuse;
            Vector4<float> specular;
        };

        struct InternalLighting
        {
            Vector4<float> ambient;
            Vector4<float> diffuse;
            Vector4<float> specular;
            Vector4<float> attenuation;
        };

        struct InternalGeometry
        {
            Vector4<float> lightModelDirection;
            Vector4<float> cameraModelPosition;
        };

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
