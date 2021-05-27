// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.7.2020.06.06

#pragma once

#include <guiddef.h>
#include <array>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

// The WICFileIONative class provides simple loading and saving operations for
// texture data. The JPEG operations use lossy compression and the PNG file
// operations use lossless compression. Although the PNG operations are
// typically used for 2D images, you can use this class to save data that has
// nothing to do with images. It is also possible to store 3D images by tiling
// a 2D image with the slices. After the tiling it is possible that not all
// 2D image elements are occupied, but you can use your knowledge of the
// original 3D data to ignore the unoccupied pixels.

namespace gte
{
    class WICFileIONative
    {
    public:
        // Supported pixel formats.
        static constexpr uint32_t R10G10B10A2 = 0;
        static constexpr uint32_t R10G10B10_XR_BIAS_A2 = 1;
        static constexpr uint32_t R32_FLOAT = 2;
        static constexpr uint32_t R16G16B16A16 = 3;
        static constexpr uint32_t B5G6R5 = 4;
        static constexpr uint32_t B5G5R5A1 = 5;
        static constexpr uint32_t R1 = 6;
        static constexpr uint32_t R8 = 7;
        static constexpr uint32_t R16 = 8;
        static constexpr uint32_t R8G8B8A8 = 9;
        static constexpr uint32_t B8G8R8A8 = 10;

        // All functions in the public interface throw a std::runtime_error
        // exception when the load or save fails. To allow for creating an
        // object that owns the data which is then loaded by the Load calls,
        // a lambda function can be used. The output format, width and height
        // should be queryable from the implicitly created object.
        using TextureCreator = std::function<uint8_t* (uint32_t, size_t, size_t)>;

        // Support for loading from BMP, GIF, ICON, JPEG, PNG, and TIFF. The
        // returned data has a format that matches as close as possible the
        // format on disk.  If the load is not successful, the function
        // returns a null object.
        //
        // The supported formats for loading and the corresponding WIC GUIDs
        // are shown next. If a parenthesized GUID is listed, that is a
        // conversion GUID. The GUID_WICPixelFormat prefix is not shown for
        // readability.
        //
        //   format                GUID_WICPixelFormat*
        //   --------------------------------------------
        //   R10G10B10A2           32bppRGBA1010102
        //   R10G10B10_XR_BIAS_A2  32bppRGBA1010102XR
        //   R32_FLOAT             32bppGrayFloat
        //   R16G16B16A16          64bppBGRA (64bppRGBA)
        //   B5G6R5                16bppBGR565
        //   B5G5R5A1              16bppBGR555
        //   R1                    BlackWhite (8bppGray)
        //   R8                    2bppGray (8bppGray)
        //   R8                    4bppGray (8bppGray)
        //   R8                    8ppGray
        //   R16                   16bppGray
        //   R8G8B8A8              32bppRGBA
        //   R8G8B8A8              32bppBGRA (32bppRGBA)
        //   R16G16B16A16          64bppRGBA

        // Load from a file with direct output.
        static void Load(std::string const& filename, uint32_t& outFormat,
            size_t& outWidth, size_t& outHeight, std::vector<uint8_t>& outTexels);

        // Load from a file with indirect output stored in the object created
        // by 'creator'.
        static void Load(std::string const& filename, TextureCreator const& creator);

        // Load from a resource with direct output. The input 'module' is an
        // HMODULE. The use of void* avoids having to expose the windows.h
        // header file to other source files.
        static void Load(void* module, std::string const& rtype, int resource,
            uint32_t& outFormat, size_t& outWidth, size_t& outHeight,
            std::vector<uint8_t>& outTexels);

        // Load from a resource with indirect output stored in the object
        // created by 'creator'.
        static void Load(void* module, std::string const& rtype, int resource,
            TextureCreator const& creator);

        // Support for saving to PNG or JPEG.
        //
        // The supported formats for saving and the corresponding WIC GUIDs
        // are shonw next. The GUID_WICPixelFormat prefix is not shown for
        // readability.
        //
        //   format                GUID_WICPixelFormat*
        //   --------------------------------------------
        //   R10G10B10A2           32bppRGBA1010102
        //   R10G10B10_XR_BIAS_A2  32bppRGBA1010102XR
        //   R32_FLOAT             32bppGrayFloat
        //   B5G6R5                16bppBGR565
        //   B5G5R5A1              16bppBGR555
        //   R1                    BlackWhite
        //   R8                    8bppGray
        //   R16                   16bppGray
        //   R8G8B8A8              32bppRGBA
        //   B8G8R8A8              32bppBGRA
        //   R16G16B16A16          64bppRGBA

        // Save to a file of type PNG.
        static void SaveToPNG(std::string const& filename, uint32_t inFormat,
            size_t inWidth, size_t inHeight, uint8_t const* inTexels);

        // Save to a file of type JPG. The image quality is in [0,1], where a
        // value of 0 indicates lowest quality (largest amount of compression)
        // and a value of 1 indicates highest quality (smallest amount of
        // compression).
        static void SaveToJPEG(std::string const& filename, uint32_t inFormat,
            size_t inWidth, size_t inHeight, uint8_t const* inTexels,
            float imageQuality);

    private:
        static std::wstring ConvertNarrowToWide(std::string const& input);

        // The factory is IWICImagingFactory* and the frameDecode is
        // IWICBitmapFrameDecode*. The void* types avoid having to expose
        // <wincodec.h> to other source files.
        static void DoLoad(void* factory, void* frameDecode,
            TextureCreator const& creator);

        // Helper function to share code between saving PNG and JPEG. Set
        // imageQuality to -1.0f for PNG. Set it to a number in [0,1] for
        // JPEG.
        static void SaveTo(std::string const& filename, uint32_t inFormat,
            size_t inWidth, size_t inHeight, uint8_t const* inTexels,
            float imageQuality);

        static size_t constexpr NUM_FORMATS = 11;
        static std::array<uint32_t, NUM_FORMATS> const msBytesPerTexel;

        struct LoadFormatMap
        {
            uint32_t format;
            GUID const* wicInputGUID;
            GUID const* wicConvertGUID;
        };
        static size_t constexpr NUM_LOAD_FORMATS = 14;
        static std::array<LoadFormatMap, NUM_LOAD_FORMATS> const msLoadFormatMap;

        struct SaveFormatMap
        {
            uint32_t format;
            GUID const* wicOutputGUID;
        };
        static size_t constexpr NUM_SAVE_FORMATS = 11;
        static std::array<SaveFormatMap, NUM_SAVE_FORMATS> const msSaveFormatMap;
    };
}
