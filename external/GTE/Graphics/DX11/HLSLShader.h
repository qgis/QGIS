// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Shader.h>
#include <Graphics/DX11/HLSLReflection.h>

namespace gte
{
    class HLSLShader : public Shader
    {
    public:
        HLSLShader(HLSLReflection const& reflector, GraphicsObjectType type);

        virtual void Set(std::string const& textureName,
            std::shared_ptr<TextureSingle> const& texture,
            std::string const& samplerName,
            std::shared_ptr<SamplerState> const& state) override;

        virtual void Set(std::string const& textureName,
            std::shared_ptr<TextureArray> const& texture,
            std::string const& samplerName,
            std::shared_ptr<SamplerState> const& state) override;

        virtual bool IsValid(Data const& goal, ConstantBuffer* resource) const override;
        virtual bool IsValid(Data const& goal, TextureBuffer* resource) const override;
        virtual bool IsValid(Data const& goal, StructuredBuffer* resource) const override;
        virtual bool IsValid(Data const& goal, RawBuffer* resource) const override;
        virtual bool IsValid(Data const& goal, TextureSingle* resource) const override;
        virtual bool IsValid(Data const& goal, TextureArray* resource) const override;
        virtual bool IsValid(Data const& goal, SamplerState* state) const override;
    };
}
