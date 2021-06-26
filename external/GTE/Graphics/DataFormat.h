// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.12.23

#pragma once

#include <string>

namespace gte
{
    // Data format types; these map directly to DX11 DXGI_FORMAT.  The
    // GL45 formats are chosen to match some of these.

    // DX11.0 formats listed below
    static uint32_t constexpr DF_UNKNOWN = 0;
    static uint32_t constexpr DF_R32G32B32A32_TYPELESS = 1;
    static uint32_t constexpr DF_R32G32B32A32_FLOAT = 2;
    static uint32_t constexpr DF_R32G32B32A32_UINT = 3;
    static uint32_t constexpr DF_R32G32B32A32_SINT = 4;
    static uint32_t constexpr DF_R32G32B32_TYPELESS = 5;
    static uint32_t constexpr DF_R32G32B32_FLOAT = 6;
    static uint32_t constexpr DF_R32G32B32_UINT = 7;
    static uint32_t constexpr DF_R32G32B32_SINT = 8;
    static uint32_t constexpr DF_R16G16B16A16_TYPELESS = 9;
    static uint32_t constexpr DF_R16G16B16A16_FLOAT = 10;
    static uint32_t constexpr DF_R16G16B16A16_UNORM = 11;
    static uint32_t constexpr DF_R16G16B16A16_UINT = 12;
    static uint32_t constexpr DF_R16G16B16A16_SNORM = 13;
    static uint32_t constexpr DF_R16G16B16A16_SINT = 14;
    static uint32_t constexpr DF_R32G32_TYPELESS = 15;
    static uint32_t constexpr DF_R32G32_FLOAT = 16;
    static uint32_t constexpr DF_R32G32_UINT = 17;
    static uint32_t constexpr DF_R32G32_SINT = 18;
    static uint32_t constexpr DF_R32G8X24_TYPELESS = 19;
    static uint32_t constexpr DF_D32_FLOAT_S8X24_UINT = 20;
    static uint32_t constexpr DF_R32_FLOAT_X8X24_TYPELESS = 21;
    static uint32_t constexpr DF_X32_TYPELESS_G8X24_UINT = 22;
    static uint32_t constexpr DF_R10G10B10A2_TYPELESS = 23;
    static uint32_t constexpr DF_R10G10B10A2_UNORM = 24;
    static uint32_t constexpr DF_R10G10B10A2_UINT = 25;
    static uint32_t constexpr DF_R11G11B10_FLOAT = 26;
    static uint32_t constexpr DF_R8G8B8A8_TYPELESS = 27;
    static uint32_t constexpr DF_R8G8B8A8_UNORM = 28;
    static uint32_t constexpr DF_R8G8B8A8_UNORM_SRGB = 29;
    static uint32_t constexpr DF_R8G8B8A8_UINT = 30;
    static uint32_t constexpr DF_R8G8B8A8_SNORM = 31;
    static uint32_t constexpr DF_R8G8B8A8_SINT = 32;
    static uint32_t constexpr DF_R16G16_TYPELESS = 33;
    static uint32_t constexpr DF_R16G16_FLOAT = 34;
    static uint32_t constexpr DF_R16G16_UNORM = 35;
    static uint32_t constexpr DF_R16G16_UINT = 36;
    static uint32_t constexpr DF_R16G16_SNORM = 37;
    static uint32_t constexpr DF_R16G16_SINT = 38;
    static uint32_t constexpr DF_R32_TYPELESS = 39;
    static uint32_t constexpr DF_D32_FLOAT = 40;
    static uint32_t constexpr DF_R32_FLOAT = 41;
    static uint32_t constexpr DF_R32_UINT = 42;
    static uint32_t constexpr DF_R32_SINT = 43;
    static uint32_t constexpr DF_R24G8_TYPELESS = 44;
    static uint32_t constexpr DF_D24_UNORM_S8_UINT = 45;
    static uint32_t constexpr DF_R24_UNORM_X8_TYPELESS = 46;
    static uint32_t constexpr DF_X24_TYPELESS_G8_UINT = 47;
    static uint32_t constexpr DF_R8G8_TYPELESS = 48;
    static uint32_t constexpr DF_R8G8_UNORM = 49;
    static uint32_t constexpr DF_R8G8_UINT = 50;
    static uint32_t constexpr DF_R8G8_SNORM = 51;
    static uint32_t constexpr DF_R8G8_SINT = 52;
    static uint32_t constexpr DF_R16_TYPELESS = 53;
    static uint32_t constexpr DF_R16_FLOAT = 54;
    static uint32_t constexpr DF_D16_UNORM = 55;
    static uint32_t constexpr DF_R16_UNORM = 56;
    static uint32_t constexpr DF_R16_UINT = 57;
    static uint32_t constexpr DF_R16_SNORM = 58;
    static uint32_t constexpr DF_R16_SINT = 59;
    static uint32_t constexpr DF_R8_TYPELESS = 60;
    static uint32_t constexpr DF_R8_UNORM = 61;
    static uint32_t constexpr DF_R8_UINT = 62;
    static uint32_t constexpr DF_R8_SNORM = 63;
    static uint32_t constexpr DF_R8_SINT = 64;
    static uint32_t constexpr DF_A8_UNORM = 65;
    static uint32_t constexpr DF_R1_UNORM = 66;
    static uint32_t constexpr DF_R9G9B9E5_SHAREDEXP = 67;
    static uint32_t constexpr DF_R8G8_B8G8_UNORM = 68;
    static uint32_t constexpr DF_G8R8_G8B8_UNORM = 69;
    static uint32_t constexpr DF_BC1_TYPELESS = 70;
    static uint32_t constexpr DF_BC1_UNORM = 71;
    static uint32_t constexpr DF_BC1_UNORM_SRGB = 72;
    static uint32_t constexpr DF_BC2_TYPELESS = 73;
    static uint32_t constexpr DF_BC2_UNORM = 74;
    static uint32_t constexpr DF_BC2_UNORM_SRGB = 75;
    static uint32_t constexpr DF_BC3_TYPELESS = 76;
    static uint32_t constexpr DF_BC3_UNORM = 77;
    static uint32_t constexpr DF_BC3_UNORM_SRGB = 78;
    static uint32_t constexpr DF_BC4_TYPELESS = 79;
    static uint32_t constexpr DF_BC4_UNORM = 80;
    static uint32_t constexpr DF_BC4_SNORM = 81;
    static uint32_t constexpr DF_BC5_TYPELESS = 82;
    static uint32_t constexpr DF_BC5_UNORM = 83;
    static uint32_t constexpr DF_BC5_SNORM = 84;
    static uint32_t constexpr DF_B5G6R5_UNORM = 85;
    static uint32_t constexpr DF_B5G5R5A1_UNORM = 86;
    static uint32_t constexpr DF_B8G8R8A8_UNORM = 87;
    static uint32_t constexpr DF_B8G8R8X8_UNORM = 88;
    static uint32_t constexpr DF_R10G10B10_XR_BIAS_A2_UNORM = 89;
    static uint32_t constexpr DF_B8G8R8A8_TYPELESS = 90;
    static uint32_t constexpr DF_B8G8R8A8_UNORM_SRGB = 91;
    static uint32_t constexpr DF_B8G8R8X8_TYPELESS = 92;
    static uint32_t constexpr DF_B8G8R8X8_UNORM_SRGB = 93;
    static uint32_t constexpr DF_BC6H_TYPELESS = 94;
    static uint32_t constexpr DF_BC6H_UF16 = 95;
    static uint32_t constexpr DF_BC6H_SF16 = 96;
    static uint32_t constexpr DF_BC7_TYPELESS = 97;
    static uint32_t constexpr DF_BC7_UNORM = 98;
    static uint32_t constexpr DF_BC7_UNORM_SRGB = 99;
    // DX11.1 formats listed below
    static uint32_t constexpr DF_AYUV = 100;
    static uint32_t constexpr DF_Y410 = 101;
    static uint32_t constexpr DF_Y416 = 102;
    static uint32_t constexpr DF_NV12 = 103;
    static uint32_t constexpr DF_P010 = 104;
    static uint32_t constexpr DF_P016 = 105;
    static uint32_t constexpr DF_420_OPAQUE = 106;
    static uint32_t constexpr DF_YUY2 = 107;
    static uint32_t constexpr DF_Y210 = 108;
    static uint32_t constexpr DF_Y216 = 109;
    static uint32_t constexpr DF_NV11 = 110;
    static uint32_t constexpr DF_AI44 = 111;
    static uint32_t constexpr DF_IA44 = 112;
    static uint32_t constexpr DF_P8 = 113;
    static uint32_t constexpr DF_A8P8 = 114;
    static uint32_t constexpr DF_B4G4R4A4_UNORM = 115;
    static uint32_t constexpr DF_NUM_FORMATS = 116;

    using DFType = uint32_t;

    // Enumerations for GL45.
    enum DFChannelType
    {
        DF_UNSUPPORTED,
        DF_BYTE,
        DF_UBYTE,
        DF_SHORT,
        DF_USHORT,
        DF_INT,
        DF_UINT,
        DF_HALF_FLOAT,
        DF_FLOAT,
        DF_DOUBLE,
        DF_INT_10_10_2,
        DF_UINT_10_10_2,
        DF_FLOAT_11_11_10,
        DF_UINT_24_8,
        DF_NUM_CHANNEL_TYPES
    };

    class DataFormat
    {
    public:
        // All data formats are known at compile time.  This class provides
        // queries for format information given the type.

        // A string version of the DF_* enumeration.
        static std::string const& GetName(DFType type);

        // The number of bytes per struct.
        static unsigned int GetNumBytesPerStruct(DFType type);

        // The number of channels per struct.
        static unsigned int GetNumChannels(DFType type);

        // The type of the channel.
        static DFChannelType GetChannelType(DFType type);

        // The conversion semantics for the channel.  When true, signed
        // integers are converted to floats in [-1,1] and unsigned integers
        // are converted to floats in [0,1].  When false, integer data is
        // converted directly to floats.
        static bool ConvertChannel(DFType type);

        // Not all data formats are currently supported.
        static bool IsSupported(DFType type);

        // The struct has a depth format.
        static bool IsDepth(DFType type);

    private:
        // Texel information.
        static std::string const msName[DF_NUM_FORMATS];
        static unsigned int const msNumBytesPerStruct[DF_NUM_FORMATS];
        static unsigned int const msNumChannels[DF_NUM_FORMATS];
        static DFChannelType const msChannelType[DF_NUM_FORMATS];
        static bool const msConvertChannel[DF_NUM_FORMATS];
        static bool const msSupported[DF_NUM_FORMATS];
    };
}
