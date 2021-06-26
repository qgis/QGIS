// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.06.06

#include <Applications/GTApplicationsPCH.h>
#include <Applications/MSW/WICFileIO.h>
using namespace gte;

std::shared_ptr<Texture2> WICFileIO::Load(std::string const& filename, bool wantMipmaps)
{
    std::shared_ptr<Texture2> texture;

    auto creator = [&texture, &wantMipmaps](uint32_t format, size_t width, size_t height)
    {
        DFType type = msFormatToDFType[format];
        texture = std::make_shared<Texture2>(type, static_cast<uint32_t>(width),
            static_cast<uint32_t>(height), wantMipmaps);
        return texture->Get<uint8_t>();
    };

    WICFileIONative::Load(filename, creator);

    return texture;
}

std::shared_ptr<Texture2> WICFileIO::Load(void* module, std::string const& rtype,
    int resource, bool wantMipmaps)
{
    std::shared_ptr<Texture2> texture;

    auto creator = [&texture, &wantMipmaps](uint32_t format, size_t width, size_t height)
    {
        DFType type = msFormatToDFType[format];
        texture = std::make_shared<Texture2>(type, static_cast<uint32_t>(width),
            static_cast<uint32_t>(height), wantMipmaps);
        return texture->Get<uint8_t>();
    };

    WICFileIONative::Load(module, rtype, resource, creator);

    return texture;
}

void WICFileIO::SaveToPNG(std::string const& filename,
    std::shared_ptr<Texture2> const& texture)
{
    uint32_t format = DFTypeToFormat(texture->GetFormat());
    WICFileIONative::SaveToPNG(filename, format, texture->GetWidth(),
        texture->GetHeight(), texture->Get<uint8_t>());
}

void WICFileIO::SaveToJPEG(std::string const& filename,
    std::shared_ptr<Texture2> const& texture, float imageQuality)
{
    uint32_t format = DFTypeToFormat(texture->GetFormat());
    WICFileIONative::SaveToJPEG(filename, format, texture->GetWidth(),
        texture->GetHeight(), texture->Get<uint8_t>(), imageQuality);
}

uint32_t WICFileIO::DFTypeToFormat(DFType type)
{
    switch (type)
    {
    case DF_R10G10B10A2_UNORM:
        return WICFileIONative::R10G10B10A2;
    case DF_R10G10B10_XR_BIAS_A2_UNORM:
        return WICFileIONative::R10G10B10_XR_BIAS_A2;
    case DF_R32_FLOAT:
        return WICFileIONative::R32_FLOAT;
    case DF_R16G16B16A16_UNORM:
        return WICFileIONative::R16G16B16A16;
    case DF_B5G6R5_UNORM:
        return WICFileIONative::B5G6R5;
    case DF_B5G5R5A1_UNORM:
        return WICFileIONative::B5G5R5A1;
    case DF_R1_UNORM:
        return WICFileIONative::R1;
    case DF_R8_UNORM:
        return WICFileIONative::R8;
    case DF_R16_UNORM:
        return WICFileIONative::R16;
    case DF_R8G8B8A8_UNORM:
        return WICFileIONative::R8G8B8A8;
    }

    LogError("Invalid DFType");
}


std::array<DFType, WICFileIO::NUM_SUPPORTED_FORMATS> const
WICFileIO::msFormatToDFType =
{
    DF_R10G10B10A2_UNORM,
    DF_R10G10B10_XR_BIAS_A2_UNORM,
    DF_R32_FLOAT,
    DF_R16G16B16A16_UNORM,
    DF_B5G6R5_UNORM,
    DF_B5G5R5A1_UNORM,
    DF_R1_UNORM,
    DF_R8_UNORM,
    DF_R16_UNORM,
    DF_R8G8B8A8_UNORM
};
