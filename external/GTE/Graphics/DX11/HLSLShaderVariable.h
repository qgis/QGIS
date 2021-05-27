// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/DX11/HLSLResource.h>
#include <vector>

namespace gte
{
    class HLSLShaderVariable
    {
    public:
        struct Description
        {
            std::string name;
            unsigned int offset;
            unsigned int numBytes;
            unsigned int flags;
            unsigned int textureStart;
            unsigned int textureNumSlots;
            unsigned int samplerStart;
            unsigned int samplerNumSlots;
            std::vector<unsigned char> defaultValue;
        };

        // Construction.  Shader variables are reported for constant buffers,
        // texture buffers, and structs defined in the shaders (resource
        // binding information).
        HLSLShaderVariable() = default;

        // Deferred construction for shader reflection.  This function is
        // intended to be write-once.
        void SetDescription(D3D_SHADER_VARIABLE_DESC const& desc);

        // Member access.
        inline std::string const& GetName() const
        {
            return mDesc.name;
        }

        inline unsigned int GetOffset() const
        {
            return mDesc.offset;
        }

        inline unsigned int GetNumBytes() const
        {
            return mDesc.numBytes;
        }

        inline unsigned int GetFlags() const
        {
            return mDesc.flags;
        }

        inline unsigned int GetTextureStart() const
        {
            return mDesc.textureStart;
        }

        inline unsigned int GetTextureNumSlots() const
        {
            return mDesc.textureNumSlots;
        }

        inline unsigned int GetSamplerStart() const
        {
            return mDesc.samplerStart;
        }

        inline unsigned int GetSamplerNumSlots() const
        {
            return mDesc.samplerNumSlots;
        }

        inline std::vector<unsigned char> const& GetDefaultValue() const
        {
            return mDesc.defaultValue;
        }

        // Print to a text file for human readability.
        void Print(std::ofstream& output) const;

    private:
        Description mDesc;
    };
}
