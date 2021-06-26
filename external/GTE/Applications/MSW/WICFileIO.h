// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.06.06

#pragma once

#include <Applications/MSW/WICFileIONative.h>
#include <Graphics/Texture2.h>
#include <memory>

// The WICFileIO class provides simple loading and saving operations for
// texture data. The JPEG operations use lossy compression and the PNG
// file operations use lossless compression. Although the PNG operations
// are typically used for 2D images, you can use this class to save data
// that has nothing to do with images. It is also possible to store 3D
// images by tiling a 2D image with the slices.  After the tiling it is
// possible that not all 2D image elements are occupied, but you can use
// your knowledge of the original 3D data to ignore the unoccupied pixels.

namespace gte
{
    class WICFileIO
    {
    public:
        // All functions in the public interface throw a std::runtime_error
        // exception when the load or save fails.

        // Support for loading from BMP, GIF, ICON, JPEG, PNG, and TIFF. The
        // returned data has a format that matches as close as possible the
        // format on disk.  If the load is not successful, the function
        // returns a null object.
        //
        // The supported formats for loading and the corresponding WIC GUIDs
        // are shown next. If a parenthesized GUID is listed, that is a
        // conversion GUID. The GUID_WICPixelFormat prefix is not shown for
        // readability. The format is DFType; the prefix DF_ is not shown
        // and the suffix (for all but R32_FLOAT) are _UNORM.
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

        // Load from a file.
        static std::shared_ptr<Texture2> Load(std::string const& filename,
            bool wantMipmaps);

        // Load from a resource with direct output. The input 'module' is an
        // HMODULE. The use of void* avoids having to expose the windows.h
        // header file to other source files.
        static std::shared_ptr<Texture2> Load(void* module, std::string const& rtype,
            int resource, bool wantMipmaps);

        // Support for saving to PNG or JPEG.
        //
        // The supported formats for saving and the corresponding WIC GUIDs
        // are shonw next. The GUID_WICPixelFormat prefix is not shown for
        // readability. The format is DFType; the prefix DF_ is not shown
        // and the suffix (for all but R32_FLOAT) are _UNORM.
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

        static void SaveToPNG(std::string const& filename,
            std::shared_ptr<Texture2> const& texture);

        // The image quality is in [0,1], where a value of 0 indicates lowest
        // quality (largest amount of compression) and a value of 1 indicates
        // highest quality (smallest amount of compression).
        static void SaveToJPEG(std::string const& filename,
            std::shared_ptr<Texture2> const& texture, float imageQuality);

    private:
        static uint32_t DFTypeToFormat(DFType type);

        static size_t constexpr NUM_SUPPORTED_FORMATS = 10;
        static std::array<DFType, NUM_SUPPORTED_FORMATS> const msFormatToDFType;
    };
}
