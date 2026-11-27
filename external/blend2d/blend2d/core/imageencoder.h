// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_IMAGEENCODER_H_INCLUDED
#define BLEND2D_IMAGEENCODER_H_INCLUDED

#include <blend2d/core/imagecodec.h>
#include <blend2d/core/object.h>
#include <blend2d/core/string.h>

//! \addtogroup bl_c_api
//! \{

//! \name BLImageEncoder - C API
//! \{

//! Image encoder [C API].
struct BLImageEncoderCore BL_CLASS_INHERITS(BLObjectCore) {
  BL_DEFINE_OBJECT_DETAIL
  BL_DEFINE_OBJECT_DCAST(BLImageEncoder)

#ifdef __cplusplus
  //! \name Impl Utilities
  //! \{

  //! Returns Impl of the image encoder (only provided for use cases that implement BLImageEncoder).
  template<typename T = BLImageEncoderImpl>
  [[nodiscard]]
  BL_INLINE_NODEBUG T* _impl() const noexcept { return static_cast<T*>(_d.impl); }

  //! \}
#endif
};

//! \cond INTERNAL
//! Image encoder [Virtual Function Table].
struct BLImageEncoderVirt BL_CLASS_INHERITS(BLObjectVirt) {
  BL_DEFINE_VIRT_BASE

  BLResult (BL_CDECL* restart)(BLImageEncoderImpl* impl) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* write_frame)(BLImageEncoderImpl* impl, BLArrayCore* dst, const BLImageCore* image) BL_NOEXCEPT_C;
};

//! Image encoder [Impl].
struct BLImageEncoderImpl BL_CLASS_INHERITS(BLObjectImpl) {
  //! \name Members
  //! \{

  //! Virtual function table.
  const BLImageEncoderVirt* virt;

  //! Image codec that created this encoder.
  BLImageCodecCore codec;

  //! Last faulty result (if failed).
  BLResult last_result;

  //! Handle in case that this encoder wraps a third-party library.
  void* handle;
  //! Current frame index.
  uint64_t frame_index;
  //! Position in source buffer.
  size_t buffer_index;

  //! \}

#ifdef __cplusplus
  //! \name Construction & Destruction
  //! \{

  //! Explicit constructor that constructs this Impl.
  BL_INLINE void ctor(const BLImageEncoderVirt* virt_, const BLImageCodecCore* codec_) noexcept {
    virt = virt_;
    bl_image_codec_init_weak(&codec, codec_);
    last_result = BL_SUCCESS;
    handle = nullptr;
    buffer_index = 0;
    frame_index = 0;
  }

  //! Explicit destructor that destructs this Impl.
  BL_INLINE void dtor() noexcept {
    bl_image_codec_destroy(&codec);
  }

  //! \}
#endif
};
//! \endcond

BL_BEGIN_C_DECLS

BL_API BLResult BL_CDECL bl_image_encoder_init(BLImageEncoderCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_encoder_init_move(BLImageEncoderCore* self, BLImageEncoderCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_encoder_init_weak(BLImageEncoderCore* self, const BLImageEncoderCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_encoder_destroy(BLImageEncoderCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_encoder_reset(BLImageEncoderCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_encoder_assign_move(BLImageEncoderCore* self, BLImageEncoderCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_encoder_assign_weak(BLImageEncoderCore* self, const BLImageEncoderCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_encoder_restart(BLImageEncoderCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_encoder_write_frame(BLImageEncoderCore* self, BLArrayCore* dst, const BLImageCore* image) BL_NOEXCEPT_C;

BL_END_C_DECLS

//! \}
//! \}

//! \addtogroup bl_imaging
//! \{

//! \name BLImageCodec - C++ API
//! \{
#ifdef __cplusplus

//! Image encoder [C++ API].
class BLImageEncoder final : public BLImageEncoderCore {
public:
  //! \cond INTERNAL
  //! \name Internals
  //! \{

  //! Object info values of a default constructed BLImageEncoder.
  static inline constexpr uint32_t kDefaultSignature =
    BLObjectInfo::pack_type_with_marker(BL_OBJECT_TYPE_IMAGE_ENCODER) | BL_OBJECT_INFO_D_FLAG;

  //! Returns Impl of the image codec (only provided for use cases that implement BLImageCodec).
  template<typename T = BLImageEncoderImpl>
  [[nodiscard]]
  BL_INLINE_NODEBUG T* _impl() const noexcept { return static_cast<T*>(_d.impl); }

  //! \}
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  BL_INLINE_NODEBUG BLImageEncoder() noexcept {
    bl_image_encoder_init(this);

    // Assume a default constructed BLImageEncoder.
    BL_ASSUME(_d.info.bits == kDefaultSignature);
  }

  BL_INLINE_NODEBUG BLImageEncoder(BLImageEncoder&& other) noexcept {
    bl_image_encoder_init_move(this, &other);

    // Assume a default initialized `other`.
    BL_ASSUME(other._d.info.bits == kDefaultSignature);
  }

  BL_INLINE_NODEBUG BLImageEncoder(const BLImageEncoder& other) noexcept {
    bl_image_encoder_init_weak(this, &other);
  }

  BL_INLINE_NODEBUG ~BLImageEncoder() {
    if (BLInternal::object_needs_cleanup(_d.info.bits)) {
      bl_image_encoder_destroy(this);
    }
  }

  //! \}

  //! \name Overloaded Operators
  //! \{

  BL_INLINE_NODEBUG explicit operator bool() const noexcept { return is_valid(); }

  BL_INLINE_NODEBUG BLImageEncoder& operator=(BLImageEncoder&& other) noexcept { bl_image_encoder_assign_move(this, &other); return *this; }
  BL_INLINE_NODEBUG BLImageEncoder& operator=(const BLImageEncoder& other) noexcept { bl_image_encoder_assign_weak(this, &other); return *this; }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool operator==(const BLImageEncoder& other) const noexcept { return  equals(other); }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool operator!=(const BLImageEncoder& other) const noexcept { return !equals(other); }

  //! \}

  //! \name Common Functionality
  //! \{

  BL_INLINE_NODEBUG BLResult reset() noexcept {
    BLResult result = bl_image_encoder_reset(this);

    // Reset operation always succeeds.
    BL_ASSUME(result == BL_SUCCESS);
    // Assume a default constructed BLImageEncoder after reset.
    BL_ASSUME(_d.info.bits == kDefaultSignature);

    return result;
  }

  BL_INLINE_NODEBUG void swap(BLImageEncoderCore& other) noexcept { _d.swap(other._d); }

  BL_INLINE_NODEBUG BLResult assign(BLImageEncoderCore&& other) noexcept { return bl_image_encoder_assign_move(this, &other); }
  BL_INLINE_NODEBUG BLResult assign(const BLImageEncoderCore& other) noexcept { return bl_image_encoder_assign_weak(this, &other); }

  //! Tests whether the image encoder is a built-in null instance.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_valid() const noexcept { return _impl()->last_result != BL_ERROR_NOT_INITIALIZED; }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool equals(const BLImageEncoderCore& other) const noexcept { return _d.impl == other._d.impl; }

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  BL_INLINE_NODEBUG BLImageCodec& codec() const noexcept { return _impl()->codec.dcast(); }

  //! Returns the last encoding result.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLResult last_result() const noexcept { return _impl()->last_result; }

  //! Returns the current frame index (yet to be written).
  [[nodiscard]]
  BL_INLINE_NODEBUG uint64_t frame_index() const noexcept { return _impl()->frame_index; }

  //! Returns the position in destination buffer.
  [[nodiscard]]
  BL_INLINE_NODEBUG size_t buffer_index() const noexcept { return _impl()->buffer_index; }

  //! \}

  //! \name Properties
  //! \{

  BL_DEFINE_OBJECT_PROPERTY_API

  //! \}

  //! \name Encoder Functionality
  //! \{

  BL_INLINE_NODEBUG BLResult restart() noexcept {
    return bl_image_encoder_restart(this);
  }

  //! Encodes the given `image` and writes the encoded data to the destination buffer `dst`.
  BL_INLINE_NODEBUG BLResult write_frame(BLArray<uint8_t>& dst, const BLImageCore& image) noexcept {
    return bl_image_encoder_write_frame(this, &dst, &image);
  }

  //! \}
};

#endif
//! \}

//! \}

#endif // BLEND2D_IMAGEENCODER_H_INCLUDED
