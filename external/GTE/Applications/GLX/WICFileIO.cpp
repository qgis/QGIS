// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

// The functions were written based on the example source code
// https://dev.w3.org/Amaya/libpng/example.c

#include <Applications/GLX/WICFileIO.h>
#include <Graphics/Texture2.h>
#include <png.h>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
using namespace gte;

struct WICFileIOPrivateHelper
{
    WICFileIOPrivateHelper(char const* name, bool readMode)
        :
        fp(fopen(name, (readMode ? "rb" : "wb"))),
        png_ptr(nullptr),
        info_ptr(nullptr),
        isRead(readMode)
    {
    }

    ~WICFileIOPrivateHelper()
    {
        if (fp)
        {
            fclose(fp);
        }

        if (png_ptr)
        {
            if (isRead)
            {
                png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
            }
            else
            {
                png_destroy_write_struct(&png_ptr, &info_ptr);
            }
        }
    }

    FILE* fp;
    png_structp png_ptr;
    png_infop info_ptr;
    bool isRead;
};

std::shared_ptr<Texture2> WICFileIO::Load(std::string const& filename, bool wantMipmaps)
{
    // The destructor for the helper will be called on any return from
    // this function.
    WICFileIOPrivateHelper helper(filename.c_str(), true);
    if (!helper.fp)
    {
        return nullptr;
    }

    helper.png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!helper.png_ptr)
    {
        return nullptr;
    }

    helper.info_ptr = png_create_info_struct(helper.png_ptr);
    if (!helper.info_ptr)
    {
        return nullptr;
    }

    if (setjmp(png_jmpbuf(helper.png_ptr)))
    {
        return nullptr;
    }

    png_init_io(helper.png_ptr, helper.fp);
    png_set_sig_bytes(helper.png_ptr, 0);

    png_read_png(helper.png_ptr, helper.info_ptr, 0, nullptr);

    png_uint_32 width, height;
    int bit_depth, color_type, interlace_method, compression_method, filter_method;
    png_get_IHDR(helper.png_ptr, helper.info_ptr, &width, &height, &bit_depth,
        &color_type, &interlace_method, &compression_method, &filter_method);

    if (bit_depth != 8 && bit_depth != 16)
    {
        // We support only 8-bit and 16-bit pixel types.
        return nullptr;
    }

    if (color_type & PNG_COLOR_MASK_PALETTE)
    {
        // We do not support palettized textures.
        return nullptr;
    }

    if (interlace_method != 0 || filter_method != 0)
    {
        // We do not support interlaced data or filtering.
        return nullptr;
    }

    png_bytepp rows = png_get_rows(helper.png_ptr, helper.info_ptr);

    std::shared_ptr<Texture2> texture;
    if (bit_depth == 8)
    {
        uint8_t const* src = nullptr;
        uint8_t* trg = nullptr;
        if (color_type == PNG_COLOR_TYPE_RGBA)
        {
            texture = std::make_shared<Texture2>(DF_R8G8B8A8_UNORM, width, height, wantMipmaps);
            trg = texture->Get<uint8_t>();
            for (png_uint_32 y = 0; y < height; ++y)
            {
                src = reinterpret_cast<uint8_t const*>(rows[y]);
                for (png_uint_32 x = 0; x < width; ++x)
                {
                    for (int i = 0; i < 4; ++i)
                    {
                        *trg++ = src[4 * x + i];
                    }
                }
            }
        }
        else if (color_type == PNG_COLOR_TYPE_RGB)
        {
            texture = std::make_shared<Texture2>(DF_R8G8B8A8_UNORM, width, height, wantMipmaps);
            trg = texture->Get<uint8_t>();
            for (png_uint_32 y = 0; y < height; ++y)
            {
                src = reinterpret_cast<uint8_t const*>(rows[y]);
                for (png_uint_32 x = 0; x < width; ++x)
                {
                    for (int i = 0; i < 3; ++i)
                    {
                        *trg++ = src[3 * x + i];
                    }
                    *trg++ = 0xFFu;
                }
            }
        }
        else if (color_type == PNG_COLOR_TYPE_GRAY)
        {
            texture = std::make_shared<Texture2>(DF_R8_UNORM, width, height, wantMipmaps);
            trg = texture->Get<uint8_t>();
            for (png_uint_32 y = 0; y < height; ++y)
            {
                src = reinterpret_cast<uint8_t const*>(rows[y]);
                for (png_uint_32 x = 0; x < width; ++x)
                {
                    *trg++ = src[x];
                }
            }
        }
        else if (color_type == PNG_COLOR_TYPE_GA)
        {
            texture = std::make_shared<Texture2>(DF_R8G8_UNORM, width, height, wantMipmaps);
            trg = texture->Get<uint8_t>();
            for (png_uint_32 y = 0; y < height; ++y)
            {
                src = reinterpret_cast<uint8_t const*>(rows[y]);
                for (png_uint_32 x = 0; x < width; ++x)
                {
                    for (int i = 0; i < 2; ++i)
                    {
                        *trg++ = src[2 * x + i];
                    }
                }
            }
        }
    }
    else // bit_depth is 16
    {
        uint16_t const* src = nullptr;
        uint16_t* trg = nullptr;
        if (color_type == PNG_COLOR_TYPE_RGBA)
        {
            texture = std::make_shared<Texture2>(DF_R16G16B16A16_UNORM, width, height, wantMipmaps);
            trg = texture->Get<uint16_t>();
            for (png_uint_32 y = 0; y < height; ++y)
            {
                src = reinterpret_cast<uint16_t const*>(rows[y]);
                for (png_uint_32 x = 0; x < width; ++x)
                {
                    for (int i = 0; i < 4; ++i)
                    {
                        *trg++ = src[4 * x + i];
                    }
                }
            }
        }
        else if (color_type == PNG_COLOR_TYPE_RGB)
        {
            texture = std::make_shared<Texture2>(DF_R8G8B8A8_UNORM, width, height, wantMipmaps);
            trg = texture->Get<uint16_t>();
            for (png_uint_32 y = 0; y < height; ++y)
            {
                src = reinterpret_cast<uint16_t const*>(rows[y]);
                for (png_uint_32 x = 0; x < width; ++x)
                {
                    for (int i = 0; i < 3; ++i)
                    {
                        *trg++ = src[3 * x + i];
                    }
                    *trg++ = 0xFFFFu;
                }
            }
        }
        else if (color_type == PNG_COLOR_TYPE_GRAY)
        {
            texture = std::make_shared<Texture2>(DF_R16_UNORM, width, height, wantMipmaps);
            trg = texture->Get<uint16_t>();
            for (png_uint_32 y = 0; y < height; ++y)
            {
                src = reinterpret_cast<uint16_t const*>(rows[y]);
                for (png_uint_32 x = 0; x < width; ++x)
                {
                    *trg++ = src[x];
                }
            }
        }
        else if (color_type == PNG_COLOR_TYPE_GA)
        {
            texture = std::make_shared<Texture2>(DF_R16G16_UNORM, width, height, wantMipmaps);
            trg = texture->Get<uint16_t>();
            for (png_uint_32 y = 0; y < height; ++y)
            {
                src = reinterpret_cast<uint16_t const*>(rows[y]);
                for (png_uint_32 x = 0; x < width; ++x)
                {
                    for (int i = 0; i < 2; ++i)
                    {
                        *trg++ = src[2 * x + i];
                    }
                }
            }
        }
    }

    return texture;
}

bool WICFileIO::SaveToPNG(std::string const& filename, std::shared_ptr<Texture2> const& texture)
{
    if (!texture)
    {
        return false;
    }

    int bit_depth = 0;
    int color_type = 0;
    png_uint_32 bytes_per_pixel = 0;
    switch (texture->GetFormat())
    {
    case DF_R8G8B8A8_UNORM:
    {
        bit_depth = 8;
        color_type = PNG_COLOR_TYPE_RGBA;
        bytes_per_pixel = 4;
        break;
    }
    case DF_R8G8_UNORM:
        bit_depth = 8;
        color_type = PNG_COLOR_TYPE_GA;
        bytes_per_pixel = 2;
        break;
    case DF_R8_UNORM:
        bit_depth = 8;
        color_type = PNG_COLOR_TYPE_GRAY;
        bytes_per_pixel = 1;
        break;
    case DF_R16G16B16A16_UNORM:
        bit_depth = 16;
        color_type = PNG_COLOR_TYPE_RGBA;
        bytes_per_pixel = 8;
        break;
    case DF_R16G16_UNORM:
        bit_depth = 16;
        color_type = PNG_COLOR_TYPE_GA;
        bytes_per_pixel = 4;
        break;
    case DF_R16_UNORM:
        bit_depth = 16;
        color_type = PNG_COLOR_TYPE_GRAY;
        bytes_per_pixel = 2;
        break;
    default:  // unsupported format
        return false;
    }

    // The destructor for the helper will be called on any return from
    // this function after the constructor call.
    WICFileIOPrivateHelper helper(filename.c_str(), false);
    if (!helper.fp)
    {
        return false;
    }

    helper.png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!helper.png_ptr)
    {
        return false;
    }

    helper.info_ptr = png_create_info_struct(helper.png_ptr);
    if (!helper.info_ptr)
    {
        return false;
    }

    if (setjmp(png_jmpbuf(helper.png_ptr)))
    {
        return false;
    }

    png_init_io(helper.png_ptr, helper.fp);

    png_uint_32 width = texture->GetWidth();
    png_uint_32 height = texture->GetHeight();
    int interlace_method = PNG_INTERLACE_NONE;
    int compression_method = PNG_COMPRESSION_TYPE_BASE;
    int filter_method = PNG_FILTER_TYPE_BASE;

    png_set_IHDR(helper.png_ptr, helper.info_ptr, width, height, bit_depth,
        color_type, interlace_method, compression_method, filter_method);

    std::vector<uint8_t*> row_pointers(height);
    uint8_t* texels = texture->Get<uint8_t>();
    for (png_uint_32 y = 0; y < height; ++y)
    {
        row_pointers[y] = texels + y * width * bytes_per_pixel;
    }

    png_write_info(helper.png_ptr, helper.info_ptr);
    png_write_image(helper.png_ptr, row_pointers.data());
    png_write_end(helper.png_ptr, helper.info_ptr);
    return true;
}
