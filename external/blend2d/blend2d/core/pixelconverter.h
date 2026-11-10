// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIXELCONVERTER_H_INCLUDED
#define BLEND2D_PIXELCONVERTER_H_INCLUDED

#include <blend2d/core/api.h>
#include <blend2d/core/format.h>
#include <blend2d/core/geometry.h>

//! \addtogroup bl_imaging
//! \{

//! \cond INTERNAL
//! \name BLPixelConverter - Types
//! \{

//! Pixel converter function.
typedef BLResult (BL_CDECL* BLPixelConverterFunc)(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride,
  uint32_t w, uint32_t h, const BLPixelConverterOptions* options) BL_NOEXCEPT_C;

//! \}
//! \endcond

//! \name BLPixelConverter - Constants
//! \{

//! Flags used by `BLPixelConverter::create()` function.
BL_DEFINE_ENUM(BLPixelConverterCreateFlags) {
  //! No flags.
  BL_PIXEL_CONVERTER_CREATE_NO_FLAGS = 0u,

  //! Specifies that the source palette in `BLFormatInfo` doesn't have to by copied by `BLPixelConverter`. The caller
  //! must ensure that the palette would stay valid until the pixel converter is destroyed.
  BL_PIXEL_CONVERTER_CREATE_FLAG_DONT_COPY_PALETTE = 0x00000001u,

  //! Specifies that the source palette in `BLFormatInfo` is alterable and the pixel converter can modify it when
  //! preparing the conversion. The modification can be irreversible so only use this flag when you are sure that
  //! the palette passed to `BLPixelConverter::create()` won't be needed outside of pixel conversion.
  //!
  //! \note The flag `BL_PIXEL_CONVERTER_CREATE_FLAG_DONT_COPY_PALETTE` must be set as well, otherwise this flag would
  //! be ignored.
  BL_PIXEL_CONVERTER_CREATE_FLAG_ALTERABLE_PALETTE = 0x00000002u,

  //! When there is no built-in conversion between the given pixel formats it's possible to use an intermediate format
  //! that is used during conversion. In such case the base pixel converter creates two more converters that are then
  //! used internally.
  //!
  //! This option disables such feature - creating a pixel converter would fail with `BL_ERROR_NOT_IMPLEMENTED` error
  //! if direct conversion is not possible.
  BL_PIXEL_CONVERTER_CREATE_FLAG_NO_MULTI_STEP = 0x00000004u

  BL_FORCE_ENUM_UINT32(BL_PIXEL_CONVERTER_CREATE_FLAG)
};

//! \}

//! \name BLPixelConverter - Structs
//! \{

//! Pixel conversion options.
struct BLPixelConverterOptions {
  BLPointI origin;
  size_t gap;
};

//! \}
//! \}

//! \addtogroup bl_c_api
//! \{

//! \name BLPixelConverter - C API
//! \{

//! Pixel converter [C API].
struct BLPixelConverterCore {
  union {
    struct {
      //! Converter function.
      BLPixelConverterFunc convert_func;
      //! Internal flags used by the converter - non-zero value means initialized.
      uint8_t internal_flags;
    };

    //! Internal data not exposed to users, aligned to sizeof(void*).
    uint8_t data[80];
  };
};

BL_BEGIN_C_DECLS

BL_API BLResult BL_CDECL bl_pixel_converter_init(BLPixelConverterCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_pixel_converter_init_weak(BLPixelConverterCore* self, const BLPixelConverterCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_pixel_converter_destroy(BLPixelConverterCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_pixel_converter_reset(BLPixelConverterCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_pixel_converter_assign(BLPixelConverterCore* self, const BLPixelConverterCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_pixel_converter_create(BLPixelConverterCore* self, const BLFormatInfo* dst_info, const BLFormatInfo* src_info, BLPixelConverterCreateFlags create_flags) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_pixel_converter_convert(const BLPixelConverterCore* self,
  void* dst_data, intptr_t dst_stride,
  const void* src_data, intptr_t src_stride,
  uint32_t w, uint32_t h, const BLPixelConverterOptions* options) BL_NOEXCEPT_C;

BL_END_C_DECLS

//! \}
//! \}

//! \addtogroup bl_imaging
//! \{

//! \name BLPixelConverter - C++ API
//! \{

#ifdef __cplusplus
//! Pixel converter [C++ API].
//!
//! Provides an interface to convert pixels between various pixel formats. The primary purpose of this class is to
//! allow efficient conversion between pixel formats used natively by Blend2D and pixel formats used elsewhere, for
//! example image codecs or native framebuffers.
//!
//! \note A default-initialized converter has a valid conversion function that would return \ref BL_ERROR_NOT_INITIALIZED
//! if invoked. Use `is_initialized()` member function to test whether the pixel converter was properly initialized.
class BLPixelConverter final : public BLPixelConverterCore {
public:
  //! Creates a new default-initialized pixel converter.
  BL_INLINE BLPixelConverter() noexcept {
    bl_pixel_converter_init(this);
  }

  //! Creates a copy of the `other` converter.
  //!
  //! If the `other` converter has dynamically allocated resources they will be properly managed (reference counting).
  //! Only very specific converters require such resources so this operation should be considered very cheap.
  BL_INLINE BLPixelConverter(const BLPixelConverter& other) noexcept {
    bl_pixel_converter_init_weak(this, &other);
  }

  //! Destroys the pixel-converter and releases all resources allocated by it.
  BL_INLINE ~BLPixelConverter() noexcept {
    bl_pixel_converter_destroy(this);
  }

  BL_INLINE BLPixelConverter& operator=(const BLPixelConverter& other) noexcept {
    bl_pixel_converter_assign(this, &other);
    return *this;
  }

  //! Tests whether if the converter is initialized.
  [[nodiscard]]
  BL_INLINE bool is_initialized() const noexcept {
    // Internal flags are non-zero when the pixel converter is initialized.
    return internal_flags != 0;
  }

  //! Reset the pixel converter.
  BL_INLINE BLResult reset() noexcept {
    return bl_pixel_converter_reset(this);
  }

  //! Assigns the `other` pixel converter into this one.
  BL_INLINE BLResult assign(const BLPixelConverter& other) noexcept {
    return bl_pixel_converter_assign(this, &other);
  }

  //! Creates a new pixel converter that will convert pixels described by `src_info` into pixels described by `dst_info`.
  //!
  //! Use `create_flags` to further specify the parameters of the conversion.
  //!
  //! \note Destination and source format informattion must be valid, otherwise `BL_ERROR_INVALID_VALUE` would be returned.
  BL_INLINE BLResult create(const BLFormatInfo& dst_info, const BLFormatInfo& src_info, BLPixelConverterCreateFlags create_flags = BL_PIXEL_CONVERTER_CREATE_NO_FLAGS) noexcept {
    return bl_pixel_converter_create(this, &dst_info, &src_info, create_flags);
  }

  //! Converts a single span of pixels of `w` width.
  BL_INLINE BLResult convert_span(void* dst_data, const void* src_data, uint32_t w,
                                 const BLPixelConverterOptions* options = nullptr) const noexcept {
    return convert_func(this, static_cast<uint8_t*>(dst_data), 0, static_cast<const uint8_t*>(src_data), 0, w, 1, options);
  }

  //! Converts a rectangular area of pixels from source format to destination.
  BL_INLINE BLResult convert_rect(void* dst_data, intptr_t dst_stride,
                                 const void* src_data, intptr_t src_stride,
                                 uint32_t w, uint32_t h, const BLPixelConverterOptions* options = nullptr) const noexcept {
    return convert_func(this, static_cast<uint8_t*>(dst_data), dst_stride, static_cast<const uint8_t*>(src_data), src_stride, w, h, options);
  }
};

#endif
//! \}

//! \}

#endif // BLEND2D_PIXELCONVERTER_H_INCLUDED
