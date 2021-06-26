// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/VisualEffect.h>
#include <Graphics/LightCameraGeometry.h>
#include <Graphics/Lighting.h>
#include <Graphics/Material.h>

namespace gte
{
    class LightEffect : public VisualEffect
    {
    protected:
        // Construction (abstract base class).  The shader source code string
        // arrays must contain strings for any supported graphics API.
        LightEffect(std::shared_ptr<ProgramFactory> const& factory,
            BufferUpdater const& updater,
            ProgramSources const& vsSource,
            ProgramSources const& psSource,
            std::shared_ptr<Material> const& material,
            std::shared_ptr<Lighting> const& lighting,
            std::shared_ptr<LightCameraGeometry> const& geometry);

    public:
        // Member access.
        virtual void SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer) override;

        inline void SetMaterial(std::shared_ptr<Material> const& material)
        {
            mMaterial = material;
        }

        inline void SetLighting(std::shared_ptr<Lighting> const& lighting)
        {
            mLighting = lighting;
        }

        inline void SetGeometry(std::shared_ptr<LightCameraGeometry> const& geometry)
        {
            mGeometry = geometry;
        }

        inline std::shared_ptr<Material> const& GetMaterial() const
        {
            return mMaterial;
        }

        inline std::shared_ptr<Lighting> const& GetLighting() const
        {
            return mLighting;
        }

        inline std::shared_ptr<LightCameraGeometry> const& GetGeometry() const
        {
            return mGeometry;
        }

        inline std::shared_ptr<ConstantBuffer> const& GetMaterialConstant() const
        {
            return mMaterialConstant;
        }

        inline std::shared_ptr<ConstantBuffer> const& GetLightingConstant() const
        {
            return mLightingConstant;
        }

        inline std::shared_ptr<ConstantBuffer> const& GetGeometryConstant() const
        {
            return mGeometryConstant;
        }

        // After you set or modify 'material', 'light' or 'geometry', call the
        // update to inform any listener that the corresponding constant
        // buffer has changed.  The derived classes construct the constant
        // buffers to store the minimal information from Material, Light or
        // Camera.  The pvw-matrix constant update requires knowledge of the
        // world transform of the object to which the effect is attached, so
        // its update must occur outside of this class.  Derived classes
        // update the system memory of the constant buffers and the base class
        // updates video memory.
        virtual void UpdateMaterialConstant();
        virtual void UpdateLightingConstant();
        virtual void UpdateGeometryConstant();

    protected:
        std::shared_ptr<Material> mMaterial;
        std::shared_ptr<Lighting> mLighting;
        std::shared_ptr<LightCameraGeometry> mGeometry;

        // The derived-class constructors are responsible for creating these
        // according to their needs.
        std::shared_ptr<ConstantBuffer> mMaterialConstant;
        std::shared_ptr<ConstantBuffer> mLightingConstant;
        std::shared_ptr<ConstantBuffer> mGeometryConstant;

        // HLSL has a shader intrinsic lit() function but GLSL does not.  The
        // string returned by this function is an implementation for GLSL.
        // Various shader strings in GTEngine are class-static and need to
        // prepend the lit-string.  Because the order of initialization of
        // such global data can vary with compiler and linker, we need to
        // return the string as data within the code segment rather than the
        // data segment.
        static std::string GetGLSLLitFunction();
    };
}
