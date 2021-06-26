// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/DataFormat.h>
using namespace gte;

// A string version of the DF_* enumeration.
std::string const& DataFormat::GetName(DFType type)
{
    return msName[type];
}

// The number of bytes per struct.
unsigned int DataFormat::GetNumBytesPerStruct(DFType type)
{
    return msNumBytesPerStruct[type];
}

// The number of channels per struct.
unsigned int DataFormat::GetNumChannels(DFType type)
{
    return msNumChannels[type];
}

// The type of the channel.
DFChannelType DataFormat::GetChannelType(DFType type)
{
    return msChannelType[type];
}

// The conversion semantics for the channel.  When true, signed
// integers are converted to floats in [-1,1] and unsigned integers
// are converted to floats in [0,1].  When false, integer data is
// converted directly to floats.
bool DataFormat::ConvertChannel(DFType type)
{
    return msConvertChannel[type];
}

// Not all data formats are currently supported.
bool DataFormat::IsSupported(DFType type)
{
    return msSupported[type];
}

// The struct has a depth format.
bool DataFormat::IsDepth(DFType type)
{
    return type == DF_D32_FLOAT_S8X24_UINT
        || type == DF_D32_FLOAT
        || type == DF_D24_UNORM_S8_UINT
        || type == DF_D16_UNORM;
}


std::string const DataFormat::msName[DF_NUM_FORMATS] =
{
    "UNKNOWN",
    "R32G32B32A32_TYPELESS",
    "R32G32B32A32_FLOAT",
    "R32G32B32A32_UINT",
    "R32G32B32A32_SINT",
    "R32G32B32_TYPELESS",
    "R32G32B32_FLOAT",
    "R32G32B32_UINT",
    "R32G32B32_SINT",
    "R16G16B16A16_TYPELESS",
    "R16G16B16A16_FLOAT",
    "R16G16B16A16_UNORM",
    "R16G16B16A16_UINT",
    "R16G16B16A16_SNORM",
    "R16G16B16A16_SINT",
    "R32G32_TYPELESS",
    "R32G32_FLOAT",
    "R32G32_UINT",
    "R32G32_SINT",
    "R32G8X24_TYPELESS",
    "D32_FLOAT_S8X24_UINT",
    "R32_FLOAT_X8X24_TYPELESS",
    "X32_TYPELESS_G8X24_UINT",
    "R10G10B10A2_TYPELESS",
    "R10G10B10A2_UNORM",
    "R10G10B10A2_UINT",
    "R11G11B10_FLOAT",
    "R8G8B8A8_TYPELESS",
    "R8G8B8A8_UNORM",
    "R8G8B8A8_UNORM_SRGB",
    "R8G8B8A8_UINT",
    "R8G8B8A8_SNORM",
    "R8G8B8A8_SINT",
    "R16G16_TYPELESS",
    "R16G16_FLOAT",
    "R16G16_UNORM",
    "R16G16_UINT",
    "R16G16_SNORM",
    "R16G16_SINT",
    "R32_TYPELESS",
    "D32_FLOAT",
    "R32_FLOAT",
    "R32_UINT",
    "R32_SINT",
    "R24G8_TYPELESS",
    "D24_UNORM_S8_UINT",
    "R24_UNORM_X8_TYPELESS",
    "X24_TYPELESS_G8_UINT",
    "R8G8_TYPELESS",
    "R8G8_UNORM",
    "R8G8_UINT",
    "R8G8_SNORM",
    "R8G8_SINT",
    "R16_TYPELESS",
    "R16_FLOAT",
    "D16_UNORM",
    "R16_UNORM",
    "R16_UINT",
    "R16_SNORM",
    "R16_SINT",
    "R8_TYPELESS",
    "R8_UNORM",
    "R8_UINT",
    "R8_SNORM",
    "R8_SINT",
    "A8_UNORM",
    "R1_UNORM",
    "R9G9B9E5_SHAREDEXP",
    "R8G8_B8G8_UNORM",
    "G8R8_G8B8_UNORM",
    "BC1_TYPELESS",
    "BC1_UNORM",
    "BC1_UNORM_SRGB",
    "BC2_TYPELESS",
    "BC2_UNORM",
    "BC2_UNORM_SRGB",
    "BC3_TYPELESS",
    "BC3_UNORM",
    "BC3_UNORM_SRGB",
    "BC4_TYPELESS",
    "BC4_UNORM",
    "BC4_SNORM",
    "BC5_TYPELESS",
    "BC5_UNORM",
    "BC5_SNORM",
    "B5G6R5_UNORM",
    "B5G5R5A1_UNORM",
    "B8G8R8A8_UNORM",
    "B8G8R8X8_UNORM",
    "R10G10B10_XR_BIAS_A2_UNORM",
    "B8G8R8A8_TYPELESS",
    "B8G8R8A8_UNORM_SRGB",
    "B8G8R8X8_TYPELESS",
    "B8G8R8X8_UNORM_SRGB",
    "BC6H_TYPELESS",
    "BC6H_UF16",
    "BC6H_SF16",
    "BC7_TYPELESS",
    "BC7_UNORM",
    "BC7_UNORM_SRGB",
    "AYUV",
    "Y410",
    "Y416",
    "NV12",
    "P010",
    "P016",
    "420_OPAQUE",
    "YUY2",
    "Y210",
    "Y216",
    "NV11",
    "AI44",
    "IA44",
    "P8",
    "A8P8",
    "B4G4R4A4_UNORM"
};

unsigned int const DataFormat::msNumBytesPerStruct[DF_NUM_FORMATS] =
{
    0,  // DF_UNKNOWN
    16, // DF_R32G32B32A32_TYPELESS
    16, // DF_R32G32B32A32_FLOAT
    16, // DF_R32G32B32A32_UINT
    16, // DF_R32G32B32A32_SINT
    12, // DF_R32G32B32_TYPELESS
    12, // DF_R32G32B32_FLOAT
    12, // DF_R32G32B32_UINT
    12, // DF_R32G32B32_SINT
    8,  // DF_R16G16B16A16_TYPELESS
    8,  // DF_R16G16B16A16_FLOAT
    8,  // DF_R16G16B16A16_UNORM
    8,  // DF_R16G16B16A16_UINT
    8,  // DF_R16G16B16A16_SNORM
    8,  // DF_R16G16B16A16_SINT
    8,  // DF_R32G32_TYPELESS
    8,  // DF_R32G32_FLOAT
    8,  // DF_R32G32_UINT
    8,  // DF_R32G32_SINT
    8,  // DF_R32G8X24_TYPELESS
    4,  // DF_D32_FLOAT_S8X24_UINT
    4,  // DF_R32_FLOAT_X8X24_TYPELESS
    4,  // DF_X32_TYPELESS_G8X24_UINT
    4,  // DF_R10G10B10A2_TYPELESS
    4,  // DF_R10G10B10A2_UNORM
    4,  // DF_R10G10B10A2_UINT
    4,  // DF_R11G11B10_FLOAT
    4,  // DF_R8G8B8A8_TYPELESS
    4,  // DF_R8G8B8A8_UNORM
    4,  // DF_R8G8B8A8_UNORM_SRGB
    4,  // DF_R8G8B8A8_UINT
    4,  // DF_R8G8B8A8_SNORM
    4,  // DF_R8G8B8A8_SINT
    4,  // DF_R16G16_TYPELESS
    4,  // DF_R16G16_FLOAT
    4,  // DF_R16G16_UNORM
    4,  // DF_R16G16_UINT
    4,  // DF_R16G16_SNORM
    4,  // DF_R16G16_SINT
    4,  // DF_R32_TYPELESS
    4,  // DF_D32_FLOAT
    4,  // DF_R32_FLOAT
    4,  // DF_R32_UINT
    4,  // DF_R32_SINT
    4,  // DF_R24G8_TYPELESS
    4,  // DF_D24_UNORM_S8_UINT
    4,  // DF_R24_UNORM_X8_TYPELESS
    4,  // DF_X24_TYPELESS_G8_UINT
    2,  // DF_R8G8_TYPELESS
    2,  // DF_R8G8_UNORM
    2,  // DF_R8G8_UINT
    2,  // DF_R8G8_SNORM
    2,  // DF_R8G8_SINT
    2,  // DF_R16_TYPELESS
    2,  // DF_R16_FLOAT
    2,  // DF_D16_UNORM
    2,  // DF_R16_UNORM
    2,  // DF_R16_UINT
    2,  // DF_R16_SNORM
    2,  // DF_R16_SINT
    1,  // DF_R8_TYPELESS
    1,  // DF_R8_UNORM
    1,  // DF_R8_UINT
    1,  // DF_R8_SNORM
    1,  // DF_R8_SINT
    1,  // DF_A8_UNORM
    0,  // DF_R1_UNORM
    2,  // DF_R9G9B9E5_SHAREDEXP
    2,  // DF_R8G8_B8G8_UNORM
    2,  // DF_G8R8_G8B8_UNORM
    0,  // DF_BC1_TYPELESS
    0,  // DF_BC1_UNORM
    0,  // DF_BC1_UNORM_SRGB
    0,  // DF_BC2_TYPELESS
    0,  // DF_BC2_UNORM
    0,  // DF_BC2_UNORM_SRGB
    0,  // DF_BC3_TYPELESS
    0,  // DF_BC3_UNORM
    0,  // DF_BC3_UNORM_SRGB
    0,  // DF_BC4_TYPELESS
    0,  // DF_BC4_UNORM
    0,  // DF_BC4_SNORM
    0,  // DF_BC5_TYPELESS
    0,  // DF_BC5_UNORM
    0,  // DF_BC5_SNORM
    2,  // DF_B5G6R5_UNORM
    2,  // DF_B5G5R5A1_UNORM
    4,  // DF_B8G8R8A8_UNORM
    4,  // DF_B8G8R8X8_UNORM
    4,  // DF_R10G10B10_XR_BIAS_A2_UNORM
    4,  // DF_B8G8R8A8_TYPELESS
    4,  // DF_B8G8R8A8_UNORM_SRGB
    4,  // DF_B8G8R8X8_TYPELESS
    4,  // DF_B8G8R8X8_UNORM_SRGB
    0,  // DF_BC6H_TYPELESS
    0,  // DF_BC6H_UF16
    0,  // DF_BC6H_SF16
    0,  // DF_BC7_TYPELESS
    0,  // DF_BC7_UNORM
    0,  // DF_BC7_UNORM_SRGB
    // DX11.1 formats (TODO: Determine bytes per channel)
    0,  // DF_AYUV
    0,  // DF_Y410
    0,  // DF_Y416
    0,  // DF_NV12
    0,  // DF_P010
    0,  // DF_P016
    0,  // DF_420_OPAQUE
    0,  // DF_YUY2
    0,  // DF_Y210
    0,  // DF_Y216
    0,  // DF_NV11
    0,  // DF_AI44
    0,  // DF_IA44
    0,  // DF_P8
    0,  // DF_A8P8
    0   // DF_B4G4R4A4_UNORM
};

unsigned int const DataFormat::msNumChannels[DF_NUM_FORMATS] =
{
    0,  // DF_UNKNOWN
    4,  // DF_R32G32B32A32_TYPELESS
    4,  // DF_R32G32B32A32_FLOAT
    4,  // DF_R32G32B32A32_UINT
    4,  // DF_R32G32B32A32_SINT
    3,  // DF_R32G32B32_TYPELESS
    3,  // DF_R32G32B32_FLOAT
    3,  // DF_R32G32B32_UINT
    3,  // DF_R32G32B32_SINT
    4,  // DF_R16G16B16A16_TYPELESS
    4,  // DF_R16G16B16A16_FLOAT
    4,  // DF_R16G16B16A16_UNORM
    4,  // DF_R16G16B16A16_UINT
    4,  // DF_R16G16B16A16_SNORM
    4,  // DF_R16G16B16A16_SINT
    2,  // DF_R32G32_TYPELESS
    2,  // DF_R32G32_FLOAT
    2,  // DF_R32G32_UINT
    2,  // DF_R32G32_SINT
    2,  // DF_R32G8X24_TYPELESS
    2,  // DF_D32_FLOAT_S8X24_UINT
    2,  // DF_R32_FLOAT_X8X24_TYPELESS
    2,  // DF_X32_TYPELESS_G8X24_UINT
    4,  // DF_R10G10B10A2_TYPELESS
    4,  // DF_R10G10B10A2_UNORM
    4,  // DF_R10G10B10A2_UINT
    3,  // DF_R11G11B10_FLOAT
    4,  // DF_R8G8B8A8_TYPELESS
    4,  // DF_R8G8B8A8_UNORM
    4,  // DF_R8G8B8A8_UNORM_SRGB
    4,  // DF_R8G8B8A8_UINT
    4,  // DF_R8G8B8A8_SNORM
    4,  // DF_R8G8B8A8_SINT
    2,  // DF_R16G16_TYPELESS
    2,  // DF_R16G16_FLOAT
    2,  // DF_R16G16_UNORM
    2,  // DF_R16G16_UINT
    2,  // DF_R16G16_SNORM
    2,  // DF_R16G16_SINT
    1,  // DF_R32_TYPELESS
    1,  // DF_D32_FLOAT
    1,  // DF_R32_FLOAT
    1,  // DF_R32_UINT
    1,  // DF_R32_SINT
    2,  // DF_R24G8_TYPELESS
    2,  // DF_D24_UNORM_S8_UINT
    2,  // DF_R24_UNORM_X8_TYPELESS
    2,  // DF_X24_TYPELESS_G8_UINT
    2,  // DF_R8G8_TYPELESS
    2,  // DF_R8G8_UNORM
    2,  // DF_R8G8_UINT
    2,  // DF_R8G8_SNORM
    2,  // DF_R8G8_SINT
    1,  // DF_R16_TYPELESS
    1,  // DF_R16_FLOAT
    1,  // DF_D16_UNORM
    1,  // DF_R16_UNORM
    1,  // DF_R16_UINT
    1,  // DF_R16_SNORM
    1,  // DF_R16_SINT
    1,  // DF_R8_TYPELESS
    1,  // DF_R8_UNORM
    1,  // DF_R8_UINT
    1,  // DF_R8_SNORM
    1,  // DF_R8_SINT
    1,  // DF_A8_UNORM
    1,  // DF_R1_UNORM
    4,  // DF_R9G9B9E5_SHAREDEXP
    4,  // DF_R8G8_B8G8_UNORM
    4,  // DF_G8R8_G8B8_UNORM
    0,  // DF_BC1_TYPELESS
    0,  // DF_BC1_UNORM
    0,  // DF_BC1_UNORM_SRGB
    0,  // DF_BC2_TYPELESS
    0,  // DF_BC2_UNORM
    0,  // DF_BC2_UNORM_SRGB
    0,  // DF_BC3_TYPELESS
    0,  // DF_BC3_UNORM
    0,  // DF_BC3_UNORM_SRGB
    0,  // DF_BC4_TYPELESS
    0,  // DF_BC4_UNORM
    0,  // DF_BC4_SNORM
    0,  // DF_BC5_TYPELESS
    0,  // DF_BC5_UNORM
    0,  // DF_BC5_SNORM
    2,  // DF_B5G6R5_UNORM
    4,  // DF_B5G5R5A1_UNORM
    4,  // DF_B8G8R8A8_UNORM
    4,  // DF_B8G8R8X8_UNORM
    4,  // DF_R10G10B10_XR_BIAS_A2_UNORM
    4,  // DF_B8G8R8A8_TYPELESS
    4,  // DF_B8G8R8A8_UNORM_SRGB
    4,  // DF_B8G8R8X8_TYPELESS
    4,  // DF_B8G8R8X8_UNORM_SRGB
    0,  // DF_BC6H_TYPELESS
    0,  // DF_BC6H_UF16
    0,  // DF_BC6H_SF16
    0,  // DF_BC7_TYPELESS
    0,  // DF_BC7_UNORM
    0,  // DF_BC7_UNORM_SRGB
    // DX11.1 formats (TODO: Determine number of channels)
    0,  // DF_AYUV
    0,  // DF_Y410
    0,  // DF_Y416
    0,  // DF_NV12
    0,  // DF_P010
    0,  // DF_P016
    0,  // DF_420_OPAQUE
    0,  // DF_YUY2
    0,  // DF_Y210
    0,  // DF_Y216
    0,  // DF_NV11
    0,  // DF_AI44
    0,  // DF_IA44
    0,  // DF_P8
    0,  // DF_A8P8
    0   // DF_B4G4R4A4_UNORM
};

DFChannelType const DataFormat::msChannelType[DF_NUM_FORMATS] =
{
    DF_UNSUPPORTED,     // DF_UNKNOWN
    DF_UNSUPPORTED,     // DF_R32G32B32A32_TYPELESS
    DF_FLOAT,           // DF_R32G32B32A32_FLOAT
    DF_UINT,            // DF_R32G32B32A32_UINT
    DF_INT,             // DF_R32G32B32A32_SINT
    DF_UNSUPPORTED,     // DF_R32G32B32_TYPELESS
    DF_FLOAT,           // DF_R32G32B32_FLOAT
    DF_UINT,            // DF_R32G32B32_UINT
    DF_INT,             // DF_R32G32B32_SINT
    DF_UNSUPPORTED,     // DF_R16G16B16A16_TYPELESS
    DF_HALF_FLOAT,      // DF_R16G16B16A16_FLOAT
    DF_USHORT,          // DF_R16G16B16A16_UNORM
    DF_USHORT,          // DF_R16G16B16A16_UINT
    DF_SHORT,           // DF_R16G16B16A16_SNORM
    DF_SHORT,           // DF_R16G16B16A16_SINT
    DF_UNSUPPORTED,     // DF_R32G32_TYPELESS
    DF_FLOAT,           // DF_R32G32_FLOAT
    DF_UINT,            // DF_R32G32_UINT
    DF_INT,             // DF_R32G32_SINT
    DF_UNSUPPORTED,     // DF_R32G8X24_TYPELESS
    DF_UNSUPPORTED,     // DF_D32_FLOAT_S8X24_UINT
    DF_UNSUPPORTED,     // DF_R32_FLOAT_X8X24_TYPELESS
    DF_UNSUPPORTED,     // DF_X32_TYPELESS_G8X24_UINT
    DF_UNSUPPORTED,     // DF_R10G10B10A2_TYPELESS
    DF_UINT_10_10_2,    // DF_R10G10B10A2_UNORM
    DF_UINT_10_10_2,    // DF_R10G10B10A2_UINT
    DF_FLOAT_11_11_10,  // DF_R11G11B10_FLOAT
    DF_UNSUPPORTED,     // DF_R8G8B8A8_TYPELESS
    DF_UBYTE,           // DF_R8G8B8A8_UNORM
    DF_UBYTE,           // DF_R8G8B8A8_UNORM_SRGB
    DF_UBYTE,           // DF_R8G8B8A8_UINT
    DF_BYTE,            // DF_R8G8B8A8_SNORM
    DF_BYTE,            // DF_R8G8B8A8_SINT
    DF_UNSUPPORTED,     // DF_R16G16_TYPELESS
    DF_FLOAT,           // DF_R16G16_FLOAT
    DF_USHORT,          // DF_R16G16_UNORM
    DF_USHORT,          // DF_R16G16_UINT
    DF_SHORT,           // DF_R16G16_SNORM
    DF_SHORT,           // DF_R16G16_SINT
    DF_UNSUPPORTED,     // DF_R32_TYPELESS
    DF_FLOAT,           // DF_D32_FLOAT
    DF_FLOAT,           // DF_R32_FLOAT
    DF_UINT,            // DF_R32_UINT
    DF_INT,             // DF_R32_SINT
    DF_UNSUPPORTED,     // DF_R24G8_TYPELESS
    DF_UINT_24_8,       // DF_D24_UNORM_S8_UINT
    DF_UNSUPPORTED,     // DF_R24_UNORM_X8_TYPELESS
    DF_UNSUPPORTED,     // DF_X24_TYPELESS_G8_UINT
    DF_UNSUPPORTED,     // DF_R8G8_TYPELESS
    DF_UBYTE,           // DF_R8G8_UNORM
    DF_UBYTE,           // DF_R8G8_UINT
    DF_BYTE,            // DF_R8G8_SNORM
    DF_BYTE,            // DF_R8G8_SINT
    DF_UNSUPPORTED,     // DF_R16_TYPELESS
    DF_HALF_FLOAT,      // DF_R16_FLOAT
    DF_USHORT,          // DF_D16_UNORM
    DF_USHORT,          // DF_R16_UNORM
    DF_USHORT,          // DF_R16_UINT
    DF_SHORT,           // DF_R16_SNORM
    DF_SHORT,           // DF_R16_SINT
    DF_UNSUPPORTED,     // DF_R8_TYPELESS
    DF_UBYTE,           // DF_R8_UNORM
    DF_UBYTE,           // DF_R8_UINT
    DF_BYTE,            // DF_R8_SNORM
    DF_BYTE,            // DF_R8_SINT
    DF_UNSUPPORTED,     // DF_A8_UNORM
    DF_UNSUPPORTED,     // DF_R1_UNORM
    DF_UNSUPPORTED,     // DF_R9G9B9E5_SHAREDEXP
    DF_UNSUPPORTED,     // DF_R8G8_B8G8_UNORM
    DF_UNSUPPORTED,     // DF_G8R8_G8B8_UNORM
    DF_UNSUPPORTED,     // DF_BC1_TYPELESS
    DF_UNSUPPORTED,     // DF_BC1_UNORM
    DF_UNSUPPORTED,     // DF_BC1_UNORM_SRGB
    DF_UNSUPPORTED,     // DF_BC2_TYPELESS
    DF_UNSUPPORTED,     // DF_BC2_UNORM
    DF_UNSUPPORTED,     // DF_BC2_UNORM_SRGB
    DF_UNSUPPORTED,     // DF_BC3_TYPELESS
    DF_UNSUPPORTED,     // DF_BC3_UNORM
    DF_UNSUPPORTED,     // DF_BC3_UNORM_SRGB
    DF_UNSUPPORTED,     // DF_BC4_TYPELESS
    DF_UNSUPPORTED,     // DF_BC4_UNORM
    DF_UNSUPPORTED,     // DF_BC4_SNORM
    DF_UNSUPPORTED,     // DF_BC5_TYPELESS
    DF_UNSUPPORTED,     // DF_BC5_UNORM
    DF_UNSUPPORTED,     // DF_BC5_SNORM
    DF_UNSUPPORTED,     // DF_B5G6R5_UNORM
    DF_UNSUPPORTED,     // DF_B5G5R5A1_UNORM
    DF_UNSUPPORTED,     // DF_B8G8R8A8_UNORM
    DF_UNSUPPORTED,     // DF_B8G8R8X8_UNORM
    DF_UNSUPPORTED,     // DF_R10G10B10_XR_BIAS_A2_UNORM
    DF_UNSUPPORTED,     // DF_B8G8R8A8_TYPELESS
    DF_UNSUPPORTED,     // DF_B8G8R8A8_UNORM_SRGB
    DF_UNSUPPORTED,     // DF_B8G8R8X8_TYPELESS
    DF_UNSUPPORTED,     // DF_B8G8R8X8_UNORM_SRGB
    DF_UNSUPPORTED,     // DF_BC6H_TYPELESS
    DF_UNSUPPORTED,     // DF_BC6H_UF16
    DF_UNSUPPORTED,     // DF_BC6H_SF16
    DF_UNSUPPORTED,     // DF_BC7_TYPELESS
    DF_UNSUPPORTED,     // DF_BC7_UNORM
    DF_UNSUPPORTED,      // DF_BC7_UNORM_SRGB
    // DX11.1 formats (TODO: Determine whether we will support these)
    DF_UNSUPPORTED,     // DF_AYUV
    DF_UNSUPPORTED,     // DF_Y410
    DF_UNSUPPORTED,     // DF_Y416
    DF_UNSUPPORTED,     // DF_NV12
    DF_UNSUPPORTED,     // DF_P010
    DF_UNSUPPORTED,     // DF_P016
    DF_UNSUPPORTED,     // DF_420_OPAQUE
    DF_UNSUPPORTED,     // DF_YUY2
    DF_UNSUPPORTED,     // DF_Y210
    DF_UNSUPPORTED,     // DF_Y216
    DF_UNSUPPORTED,     // DF_NV11
    DF_UNSUPPORTED,     // DF_AI44
    DF_UNSUPPORTED,     // DF_IA44
    DF_UNSUPPORTED,     // DF_P8
    DF_UNSUPPORTED,     // DF_A8P8
    DF_UNSUPPORTED      // DF_B4G4R4A4_UNORM
};

bool const DataFormat::msConvertChannel[DF_NUM_FORMATS] =
{
    false,  // DF_UNKNOWN
    false,  // DF_R32G32B32A32_TYPELESS
    false,  // DF_R32G32B32A32_FLOAT
    false,  // DF_R32G32B32A32_UINT
    false,  // DF_R32G32B32A32_SINT
    false,  // DF_R32G32B32_TYPELESS
    false,  // DF_R32G32B32_FLOAT
    false,  // DF_R32G32B32_UINT
    false,  // DF_R32G32B32_SINT
    false,  // DF_R16G16B16A16_TYPELESS
    false,  // DF_R16G16B16A16_FLOAT
    true,   // DF_R16G16B16A16_UNORM
    false,  // DF_R16G16B16A16_UINT
    true,   // DF_R16G16B16A16_SNORM
    false,  // DF_R16G16B16A16_SINT
    false,  // DF_R32G32_TYPELESS
    false,  // DF_R32G32_FLOAT
    false,  // DF_R32G32_UINT
    false,  // DF_R32G32_SINT
    false,  // DF_R32G8X24_TYPELESS
    false,  // DF_D32_FLOAT_S8X24_UINT
    false,  // DF_R32_FLOAT_X8X24_TYPELESS
    false,  // DF_X32_TYPELESS_G8X24_UINT
    false,  // DF_R10G10B10A2_TYPELESS
    true,   // DF_R10G10B10A2_UNORM
    false,  // DF_R10G10B10A2_UINT
    false,  // DF_R11G11B10_FLOAT
    false,  // DF_R8G8B8A8_TYPELESS
    true,   // DF_R8G8B8A8_UNORM
    true,   // DF_R8G8B8A8_UNORM_SRGB
    false,  // DF_R8G8B8A8_UINT
    true,   // DF_R8G8B8A8_SNORM
    false,  // DF_R8G8B8A8_SINT
    false,  // DF_R16G16_TYPELESS
    false,  // DF_R16G16_FLOAT
    true,   // DF_R16G16_UNORM
    false,  // DF_R16G16_UINT
    true,   // DF_R16G16_SNORM
    false,  // DF_R16G16_SINT
    false,  // DF_R32_TYPELESS
    false,  // DF_D32_FLOAT
    false,  // DF_R32_FLOAT
    false,  // DF_R32_UINT
    false,  // DF_R32_SINT
    false,  // DF_R24G8_TYPELESS
    false,  // DF_D24_UNORM_S8_UINT
    false,  // DF_R24_UNORM_X8_TYPELESS
    false,  // DF_X24_TYPELESS_G8_UINT
    false,  // DF_R8G8_TYPELESS
    true,   // DF_R8G8_UNORM
    false,  // DF_R8G8_UINT
    true,   // DF_R8G8_SNORM
    false,  // DF_R8G8_SINT
    false,  // DF_R16_TYPELESS
    false,  // DF_R16_FLOAT
    true,   // DF_D16_UNORM
    true,   // DF_R16_UNORM
    false,  // DF_R16_UINT
    true,   // DF_R16_SNORM
    false,  // DF_R16_SINT
    false,  // DF_R8_TYPELESS
    true,   // DF_R8_UNORM
    false,  // DF_R8_UINT
    true,   // DF_R8_SNORM
    false,  // DF_R8_SINT
    true,   // DF_A8_UNORM
    true,   // DF_R1_UNORM
    false,  // DF_R9G9B9E5_SHAREDEXP
    true,   // DF_R8G8_B8G8_UNORM
    true,   // DF_G8R8_G8B8_UNORM
    false,  // DF_BC1_TYPELESS
    true,   // DF_BC1_UNORM
    true,   // DF_BC1_UNORM_SRGB
    false,  // DF_BC2_TYPELESS
    true,   // DF_BC2_UNORM
    true,   // DF_BC2_UNORM_SRGB
    false,  // DF_BC3_TYPELESS
    true,   // DF_BC3_UNORM
    true,   // DF_BC3_UNORM_SRGB
    false,  // DF_BC4_TYPELESS
    true,   // DF_BC4_UNORM
    true,   // DF_BC4_SNORM
    false,  // DF_BC5_TYPELESS
    true,   // DF_BC5_UNORM
    true,   // DF_BC5_SNORM
    true,   // DF_B5G6R5_UNORM
    true,   // DF_B5G5R5A1_UNORM
    true,   // DF_B8G8R8A8_UNORM
    true,   // DF_B8G8R8X8_UNORM
    true,   // DF_R10G10B10_XR_BIAS_A2_UNORM
    false,  // DF_B8G8R8A8_TYPELESS
    true,   // DF_B8G8R8A8_UNORM_SRGB
    false,  // DF_B8G8R8X8_TYPELESS
    true,   // DF_B8G8R8X8_UNORM_SRGB
    false,  // DF_BC6H_TYPELESS
    false,  // DF_BC6H_UF16
    false,  // DF_BC6H_SF16
    false,  // DF_BC7_TYPELESS
    true,   // DF_BC7_UNORM
    true,   // DF_BC7_UNORM_SRGB
    // DX11.1 formats (TODO: Determine the appropriate bool value)
    false,  // DF_AYUV
    false,  // DF_Y410
    false,  // DF_Y416
    false,  // DF_NV12
    false,  // DF_P010
    false,  // DF_P016
    false,  // DF_420_OPAQUE
    false,  // DF_YUY2
    false,  // DF_Y210
    false,  // DF_Y216
    false,  // DF_NV11
    false,  // DF_AI44
    false,  // DF_IA44
    false,  // DF_P8
    false,  // DF_A8P8
    false   // DF_B4G4R4A4_UNORM
};

bool const DataFormat::msSupported[DF_NUM_FORMATS] =
{
    false,  // DF_UNKNOWN
    true,   // DF_R32G32B32A32_TYPELESS
    true,   // DF_R32G32B32A32_FLOAT
    true,   // DF_R32G32B32A32_UINT
    true,   // DF_R32G32B32A32_SINT
    true,   // DF_R32G32B32_TYPELESS
    true,   // DF_R32G32B32_FLOAT
    true,   // DF_R32G32B32_UINT
    true,   // DF_R32G32B32_SINT
    true,   // DF_R16G16B16A16_TYPELESS
    true,   // DF_R16G16B16A16_FLOAT
    true,   // DF_R16G16B16A16_UNORM
    true,   // DF_R16G16B16A16_UINT
    true,   // DF_R16G16B16A16_SNORM
    true,   // DF_R16G16B16A16_SINT
    true,   // DF_R32G32_TYPELESS
    true,   // DF_R32G32_FLOAT
    true,   // DF_R32G32_UINT
    true,   // DF_R32G32_SINT
    true,   // DF_R32G8X24_TYPELESS
    true,   // DF_D32_FLOAT_S8X24_UINT
    true,   // DF_R32_FLOAT_X8X24_TYPELESS
    true,   // DF_X32_TYPELESS_G8X24_UINT
    true,   // DF_R10G10B10A2_TYPELESS
    true,   // DF_R10G10B10A2_UNORM
    true,   // DF_R10G10B10A2_UINT
    true,   // DF_R11G11B10_FLOAT
    true,   // DF_R8G8B8A8_TYPELESS
    true,   // DF_R8G8B8A8_UNORM
    true,   // DF_R8G8B8A8_UNORM_SRGB
    true,   // DF_R8G8B8A8_UINT
    true,   // DF_R8G8B8A8_SNORM
    true,   // DF_R8G8B8A8_SINT
    true,   // DF_R16G16_TYPELESS
    true,   // DF_R16G16_FLOAT
    true,   // DF_R16G16_UNORM
    true,   // DF_R16G16_UINT
    true,   // DF_R16G16_SNORM
    true,   // DF_R16G16_SINT
    true,   // DF_R32_TYPELESS
    true,   // DF_D32_FLOAT
    true,   // DF_R32_FLOAT
    true,   // DF_R32_UINT
    true,   // DF_R32_SINT
    true,   // DF_R24G8_TYPELESS
    true,   // DF_D24_UNORM_S8_UINT
    true,   // DF_R24_UNORM_X8_TYPELESS
    true,   // DF_X24_TYPELESS_G8_UINT
    true,   // DF_R8G8_TYPELESS
    true,   // DF_R8G8_UNORM
    true,   // DF_R8G8_UINT
    true,   // DF_R8G8_SNORM
    true,   // DF_R8G8_SINT
    true,   // DF_R16_TYPELESS
    true,   // DF_R16_FLOAT
    true,   // DF_D16_UNORM
    true,   // DF_R16_UNORM
    true,   // DF_R16_UINT
    true,   // DF_R16_SNORM
    true,   // DF_R16_SINT
    true,   // DF_R8_TYPELESS
    true,   // DF_R8_UNORM
    true,   // DF_R8_UINT
    true,   // DF_R8_SNORM
    true,   // DF_R8_SINT
    true,   // DF_A8_UNORM
    false,  // DF_R1_UNORM
    true,   // DF_R9G9B9E5_SHAREDEXP
    true,   // DF_R8G8_B8G8_UNORM
    true,   // DF_G8R8_G8B8_UNORM
    false,  // DF_BC1_TYPELESS
    false,  // DF_BC1_UNORM
    false,  // DF_BC1_UNORM_SRGB
    false,  // DF_BC2_TYPELESS
    false,  // DF_BC2_UNORM
    false,  // DF_BC2_UNORM_SRGB
    false,  // DF_BC3_TYPELESS
    false,  // DF_BC3_UNORM
    false,  // DF_BC3_UNORM_SRGB
    false,  // DF_BC4_TYPELESS
    false,  // DF_BC4_UNORM
    false,  // DF_BC4_SNORM
    false,  // DF_BC5_TYPELESS
    false,  // DF_BC5_UNORM
    false,  // DF_BC5_SNORM
    true,   // DF_B5G6R5_UNORM
    true,   // DF_B5G5R5A1_UNORM
    true,   // DF_B8G8R8A8_UNORM
    true,   // DF_B8G8R8X8_UNORM
    true,   // DF_R10G10B10_XR_BIAS_A2_UNORM
    true,   // DF_B8G8R8A8_TYPELESS
    true,   // DF_B8G8R8A8_UNORM_SRGB
    true,   // DF_B8G8R8X8_TYPELESS
    true,   // DF_B8G8R8X8_UNORM_SRGB
    false,  // DF_BC6H_TYPELESS
    false,  // DF_BC6H_UF16
    false,  // DF_BC6H_SF16
    false,  // DF_BC7_TYPELESS
    false,  // DF_BC7_UNORM
    false,  // DF_BC7_UNORM_SRGB
    // DX11.1 formats (TODO: Determine whether we will support these)
    false,  // DF_AYUV
    false,  // DF_Y410
    false,  // DF_Y416
    false,  // DF_NV12
    false,  // DF_P010
    false,  // DF_P016
    false,  // DF_420_OPAQUE
    false,  // DF_YUY2
    false,  // DF_Y210
    false,  // DF_Y216
    false,  // DF_NV11
    false,  // DF_AI44
    false,  // DF_IA44
    false,  // DF_P8
    false,  // DF_A8P8
    false   // DF_B4G4R4A4_UNORM
};
