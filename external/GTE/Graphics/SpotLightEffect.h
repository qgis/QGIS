// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/LightEffect.h>

namespace gte
{
    class SpotLightEffect : public LightEffect
    {
    public:
        // Construction.  Set 'select' to 0 for per-vertex effects or to 1 for
        // per-pixel effects.
        SpotLightEffect(std::shared_ptr<ProgramFactory> const& factory,
            BufferUpdater const& updater, int select,
            std::shared_ptr<Material> const& material,
            std::shared_ptr<Lighting> const& lighting,
            std::shared_ptr<LightCameraGeometry> const& geometry);

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
            Vector4<float> spotCutoff;
            Vector4<float> attenuation;
        };

        struct InternalGeometry
        {
            Vector4<float> lightModelPosition;
            Vector4<float> lightModelDirection;
            Vector4<float> cameraModelPosition;
        };

        // Shader source code as strings.
        static std::string const msGLSLVSSource[2];
        static std::string const msGLSLPSSource[2];
        static std::string const msHLSLVSSource[2];
        static std::string const msHLSLPSSource[2];
        static ProgramSources const msVSSource[2];
        static ProgramSources const msPSSource[2];
    };
}
