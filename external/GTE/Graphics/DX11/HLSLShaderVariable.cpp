// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/HLSLShaderVariable.h>
#include <iomanip>
using namespace gte;

void HLSLShaderVariable::SetDescription(D3D_SHADER_VARIABLE_DESC const& desc)
{
    mDesc.name = std::string(desc.Name ? desc.Name : "");
    mDesc.offset = desc.StartOffset;
    mDesc.numBytes = desc.Size;
    mDesc.flags = desc.uFlags;
    mDesc.textureStart = desc.StartTexture;
    mDesc.textureNumSlots = desc.TextureSize;
    mDesc.samplerStart = desc.StartSampler;
    mDesc.samplerNumSlots = desc.SamplerSize;
    if (desc.DefaultValue && desc.Size > 0)
    {
        mDesc.defaultValue.resize(desc.Size);
        std::memcpy(&mDesc.defaultValue[0], desc.DefaultValue, desc.Size);
    }
}

void HLSLShaderVariable::Print(std::ofstream& output) const
{
    output << "name = " << mDesc.name << std::endl;
    output << "offset = " << mDesc.offset << std::endl;
    output << "numBytes = " << mDesc.numBytes << std::endl;
    output << "flags = " << mDesc.flags << std::endl;

    if (mDesc.textureStart == 0xFFFFFFFF)
    {
        output << "textureStart = -1" << std::endl;
    }
    else
    {
        output << "textureStart = " << mDesc.textureStart << std::endl;
    }
    output << "textureNumSlots = " << mDesc.textureNumSlots << std::endl;

    if (mDesc.samplerStart == 0xFFFFFFFF)
    {
        output << "samplerStart = -1" << std::endl;
    }
    else
    {
        output << "samplerStart = " << mDesc.samplerStart << std::endl;
    }
    output << "textureNumSlots = " << mDesc.samplerNumSlots << std::endl;

    output << "default value = ";
    size_t size = mDesc.defaultValue.size();
    if (size > 0)
    {
        output << std::hex << std::endl;
        size_t j = 0;
        for (auto c : mDesc.defaultValue)
        {
            unsigned int hc = static_cast<unsigned int>(c);
            output << "0x" << std::setw(2) << std::setfill('0') << hc;
            if ((++j % 16) == 0)
            {
                if (j != size)
                {
                    output << std::endl;
                }
            }
            else
            {
                output << ' ';
            }
        }
        output << std::dec;
    }
    else
    {
        output << "none";
    }
    output << std::endl;
}
