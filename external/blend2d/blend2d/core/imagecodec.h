// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_IMAGECODEC_H_INCLUDED
#define BLEND2D_IMAGECODEC_H_INCLUDED

#include <blend2d/core/array.h>
#include <blend2d/core/object.h>
#include <blend2d/core/string.h>

//! \addtogroup bl_c_api
//! \{

//! \name BLImageCodec - C API
//! \{

//! Image codec [C API].
struct BLImageCodecCore BL_CLASS_INHERITS(BLObjectCore) {
  BL_DEFINE_OBJECT_DETAIL
  BL_DEFINE_OBJECT_DCAST(BLImageCodec)
};

//! \cond
//! Image codec [C API Virtual Function Table].
struct BLImageCodecVirt BL_CLASS_INHERITS(BLObjectVirt) {
  BL_DEFINE_VIRT_BASE

  uint32_t (BL_CDECL* inspect_data)(const BLImageCodecImpl* impl, const uint8_t* data, size_t size) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* create_decoder)(const BLImageCodecImpl* impl, BLImageDecoderCore* dst) BL_NOEXCEPT_C;
  BLResult (BL_CDECL* create_encoder)(const BLImageCodecImpl* impl, BLImageEncoderCore* dst) BL_NOEXCEPT_C;
};

//! Image codec [C API Impl].
struct BLImageCodecImpl BL_CLASS_INHERITS(BLObjectImpl) {
  //! \name Members
  //! \{

  //! Virtual function table.
  const BLImageCodecVirt* virt;

  //! Image codec name like "PNG", "JPEG", etc...
  BLStringCore name;
  //! Image codec vendor string, built-in codecs use "Blend2D" as a vendor string.
  BLStringCore vendor;
  //! Mime types.
  BLStringCore mime_type;
  //! Known file extensions used by this image codec separated by "|".
  BLStringCore extensions;

  //! Image codec features.
  uint32_t features;

  //! \}

#ifdef __cplusplus
  //! \name Construction & Destruction
  //! \{

  //! Explicit constructor that constructs this Impl.
  BL_INLINE void ctor(const BLImageCodecVirt* virt_) noexcept {
    virt = virt_;
    bl_call_ctor(name.dcast());
    bl_call_ctor(vendor.dcast());
    bl_call_ctor(mime_type.dcast());
    bl_call_ctor(extensions.dcast());
    features = 0;
  }

  //! Explicit destructor that destructs this Impl.
  BL_INLINE void dtor() noexcept {
    bl_call_dtor(name.dcast());
    bl_call_dtor(vendor.dcast());
    bl_call_dtor(mime_type.dcast());
    bl_call_dtor(extensions.dcast());
  }

  //! \}
#endif
};
//! \endcond

BL_BEGIN_C_DECLS

BL_API BLResult BL_CDECL bl_image_codec_init(BLImageCodecCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_codec_init_move(BLImageCodecCore* self, BLImageCodecCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_codec_init_weak(BLImageCodecCore* self, const BLImageCodecCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_codec_init_by_name(BLImageCodecCore* self, const char* name, size_t size, const BLArrayCore* codecs) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_codec_destroy(BLImageCodecCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_codec_reset(BLImageCodecCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_codec_assign_move(BLImageCodecCore* self, BLImageCodecCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_codec_assign_weak(BLImageCodecCore* self, const BLImageCodecCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_codec_find_by_name(BLImageCodecCore* self, const char* name, size_t size, const BLArrayCore* codecs) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_codec_find_by_extension(BLImageCodecCore* self, const char* name, size_t size, const BLArrayCore* codecs) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_codec_find_by_data(BLImageCodecCore* self, const void* data, size_t size, const BLArrayCore* codecs) BL_NOEXCEPT_C;
BL_API uint32_t BL_CDECL bl_image_codec_inspect_data(const BLImageCodecCore* self, const void* data, size_t size) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_codec_create_decoder(const BLImageCodecCore* self, BLImageDecoderCore* dst) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_codec_create_encoder(const BLImageCodecCore* self, BLImageEncoderCore* dst) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_image_codec_array_init_built_in_codecs(BLArrayCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_codec_array_assign_built_in_codecs(BLArrayCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_codec_add_to_built_in(const BLImageCodecCore* codec) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_image_codec_remove_from_built_in(const BLImageCodecCore* codec) BL_NOEXCEPT_C;

BL_END_C_DECLS

//! \}
//! \}

//! \addtogroup bl_imaging
//! \{

//! \name BLImageCodec - Constants
//! \{

//! Image codec feature bits.
BL_DEFINE_ENUM(BLImageCodecFeatures) {
  //! No features.
  BL_IMAGE_CODEC_NO_FEATURES = 0u,
  //! Image codec supports reading images (can create BLImageDecoder).
  BL_IMAGE_CODEC_FEATURE_READ = 0x00000001u,
  //! Image codec supports writing images (can create BLImageEncoder).
  BL_IMAGE_CODEC_FEATURE_WRITE = 0x00000002u,
  //! Image codec supports lossless compression.
  BL_IMAGE_CODEC_FEATURE_LOSSLESS = 0x00000004u,
  //! Image codec supports loosy compression.
  BL_IMAGE_CODEC_FEATURE_LOSSY = 0x00000008u,
  //! Image codec supports writing multiple frames (GIF).
  BL_IMAGE_CODEC_FEATURE_MULTI_FRAME = 0x00000010u,
  //! Image codec supports IPTC metadata.
  BL_IMAGE_CODEC_FEATURE_IPTC = 0x10000000u,
  //! Image codec supports EXIF metadata.
  BL_IMAGE_CODEC_FEATURE_EXIF = 0x20000000u,
  //! Image codec supports XMP metadata.
  BL_IMAGE_CODEC_FEATURE_XMP = 0x40000000u

  BL_FORCE_ENUM_UINT32(BL_IMAGE_CODEC_FEATURE)
};

//! \}

//! \name BLImageCodec - C++ API
//! \{
#ifdef __cplusplus

//! Image codec [C++ API].
//!
//! Provides a unified interface for inspecting image data and creating image
//! decoders & encoders.
class BLImageCodec final : public BLImageCodecCore {
public:
  //! \cond INTERNAL
  //! \name Internals
  //! \{

  //! Object info values of a default constructed BLImageCodec.
  static inline constexpr uint32_t kDefaultSignature =
    BLObjectInfo::pack_type_with_marker(BL_OBJECT_TYPE_IMAGE_CODEC) | BL_OBJECT_INFO_D_FLAG;

  //! Returns Impl of the image codec (only provided for use cases that implement BLImageCodec).
  template<typename T = BLImageCodecImpl>
  [[nodiscard]]
  BL_INLINE_NODEBUG T* _impl() const noexcept { return static_cast<T*>(_d.impl); }

  //! \}
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  BL_INLINE_NODEBUG BLImageCodec() noexcept {
    bl_image_codec_init(this);

    // Assume a default constructed BLImageCodec.
    BL_ASSUME(_d.info.bits == kDefaultSignature);
  }

  BL_INLINE_NODEBUG BLImageCodec(BLImageCodec&& other) noexcept {
    bl_image_codec_init_move(this, &other);

    // Assume a default initialized `other`.
    BL_ASSUME(other._d.info.bits == kDefaultSignature);
  }

  BL_INLINE_NODEBUG BLImageCodec(const BLImageCodec& other) noexcept {
    bl_image_codec_init_weak(this, &other);
  }

  BL_INLINE_NODEBUG ~BLImageCodec() {
    if (BLInternal::object_needs_cleanup(_d.info.bits)) {
      bl_image_codec_destroy(this);
    }
  }

  //! \}

  //! \name Overloaded Operators
  //! \{

  BL_INLINE_NODEBUG explicit operator bool() const noexcept { return is_valid(); }

  BL_INLINE_NODEBUG BLImageCodec& operator=(const BLImageCodec& other) noexcept {
    bl_image_codec_assign_weak(this, &other);
    return *this;
  }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool operator==(const BLImageCodec& other) const noexcept { return  equals(other); }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool operator!=(const BLImageCodec& other) const noexcept { return !equals(other); }

  //! \}

  //! \name Common Functionality
  //! \{

  BL_INLINE_NODEBUG BLResult reset() noexcept {
    BLResult result = bl_image_codec_reset(this);

    // Reset operation always succeeds.
    BL_ASSUME(result == BL_SUCCESS);
    // Assume a default constructed BLImageCodec after reset.
    BL_ASSUME(_d.info.bits == kDefaultSignature);

    return result;
  }

  BL_INLINE_NODEBUG void swap(BLImageCodecCore& other) noexcept { _d.swap(other._d); }

  BL_INLINE_NODEBUG BLResult assign(const BLImageCodecCore& other) noexcept { return bl_image_codec_assign_weak(this, &other); }

  //! Tests whether the image codec is a built-in null instance.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_valid() const noexcept { return (_impl()->features & (BL_IMAGE_CODEC_FEATURE_READ | BL_IMAGE_CODEC_FEATURE_WRITE)) != 0; }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool equals(const BLImageCodecCore& other) const noexcept { return _d.impl == other._d.impl; }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns image codec name (i.e, "PNG", "JPEG", etc...).
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLString& name() const noexcept { return _impl()->name.dcast(); }

  //! Returns the image codec vendor (i.e. "Blend2D" for all built-in codecs).
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLString& vendor() const noexcept { return _impl()->vendor.dcast(); }

  //! Returns a mime-type associated with the image codec's format.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLString& mime_type() const noexcept { return _impl()->mime_type.dcast(); }

  //! Returns a list of file extensions used to store image of this codec, separated by '|' character.
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLString& extensions() const noexcept { return _impl()->extensions.dcast(); }

  //! Returns image codec flags, see \ref BLImageCodecFeatures.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLImageCodecFeatures features() const noexcept { return BLImageCodecFeatures(_impl()->features); }

  //! Tests whether the image codec has a flag `flag`.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_feature(BLImageCodecFeatures feature) const noexcept { return (_impl()->features & feature) != 0; }

  //! \}

  //! \name Properties
  //! \{

  BL_DEFINE_OBJECT_PROPERTY_API

  //! \}

  //! \name Find Functionality
  //! \{

  BL_INLINE_NODEBUG BLResult find_by_name(const char* name) noexcept {
    return bl_image_codec_find_by_name(this, name, SIZE_MAX, nullptr);
  }

  BL_INLINE_NODEBUG BLResult find_by_name(const char* name, const BLArray<BLImageCodec>& codecs) noexcept {
    return bl_image_codec_find_by_name(this, name, SIZE_MAX, &codecs);
  }

  BL_INLINE_NODEBUG BLResult find_by_name(BLStringView name) noexcept {
    return bl_image_codec_find_by_name(this, name.data, name.size, nullptr);
  }

  BL_INLINE_NODEBUG BLResult find_by_name(BLStringView name, const BLArray<BLImageCodec>& codecs) noexcept {
    return bl_image_codec_find_by_name(this, name.data, name.size, &codecs);
  }

  BL_INLINE_NODEBUG BLResult find_by_extension(const char* name) noexcept {
    return bl_image_codec_find_by_extension(this, name, SIZE_MAX, nullptr);
  }

  BL_INLINE_NODEBUG BLResult find_by_extension(const char* name, const BLArray<BLImageCodec>& codecs) noexcept {
    return bl_image_codec_find_by_extension(this, name, SIZE_MAX, &codecs);
  }

  BL_INLINE_NODEBUG BLResult find_by_extension(BLStringView name) noexcept {
    return bl_image_codec_find_by_extension(this, name.data, name.size, nullptr);
  }

  BL_INLINE_NODEBUG BLResult find_by_extension(BLStringView name, const BLArray<BLImageCodec>& codecs) noexcept {
    return bl_image_codec_find_by_extension(this, name.data, name.size, &codecs);
  }

  BL_INLINE_NODEBUG BLResult find_by_data(const void* data, size_t size) noexcept {
    return bl_image_codec_find_by_data(this, data, size, nullptr);
  }

  BL_INLINE_NODEBUG BLResult find_by_data(const void* data, size_t size, const BLArray<BLImageCodec>& codecs) noexcept {
    return bl_image_codec_find_by_data(this, data, size, &codecs);
  }

  BL_INLINE_NODEBUG BLResult find_by_data(const BLArrayView<uint8_t>& view) noexcept {
    return bl_image_codec_find_by_data(this, view.data, view.size, nullptr);
  }

  BL_INLINE_NODEBUG BLResult find_by_data(const BLArrayView<uint8_t>& view, const BLArray<BLImageCodec>& codecs) noexcept {
    return bl_image_codec_find_by_data(this, view.data, view.size, &codecs);
  }

  BL_INLINE_NODEBUG BLResult find_by_data(const BLArray<uint8_t>& buffer) noexcept {
    return bl_image_codec_find_by_data(this, buffer.data(), buffer.size(), nullptr);
  }

  BL_INLINE_NODEBUG BLResult find_by_data(const BLArray<uint8_t>& buffer, const BLArray<BLImageCodec>& codecs) noexcept {
    return bl_image_codec_find_by_data(this, buffer.data(), buffer.size(), &codecs);
  }

  //! \}

  //! \name Codec Functionality
  //! \{

  [[nodiscard]]
  BL_INLINE_NODEBUG uint32_t inspect_data(const BLArray<uint8_t>& buffer) const noexcept { return inspect_data(buffer.view()); }

  [[nodiscard]]
  BL_INLINE_NODEBUG uint32_t inspect_data(const BLArrayView<uint8_t>& view) const noexcept { return inspect_data(view.data, view.size); }

  [[nodiscard]]
  BL_INLINE_NODEBUG uint32_t inspect_data(const void* data, size_t size) const noexcept { return bl_image_codec_inspect_data(this, static_cast<const uint8_t*>(data), size); }

  BL_INLINE_NODEBUG BLResult create_decoder(BLImageDecoderCore* dst) const noexcept { return bl_image_codec_create_decoder(this, reinterpret_cast<BLImageDecoderCore*>(dst)); }
  BL_INLINE_NODEBUG BLResult create_encoder(BLImageEncoderCore* dst) const noexcept { return bl_image_codec_create_encoder(this, reinterpret_cast<BLImageEncoderCore*>(dst)); }

  //! \}

  //! \name Built-In Codecs
  //! \{

  //! Returns an array of built-in codecs, which are present in a global registry.
  [[nodiscard]]
  static BL_INLINE_NODEBUG BLArray<BLImageCodec> built_in_codecs() noexcept {
    BLArray<BLImageCodec> result;
    bl_image_codec_array_init_built_in_codecs(&result);
    return result;
  }

  //! Adds a codec to a global built-in codecs registry.
  static BL_INLINE_NODEBUG BLResult add_to_built_in(const BLImageCodecCore& codec) noexcept {
    return bl_image_codec_add_to_built_in(&codec);
  }

  //! Removes a codec from a global built-in codecs registry.
  static BL_INLINE_NODEBUG BLResult remove_from_built_in(const BLImageCodecCore& codec) noexcept {
    return bl_image_codec_remove_from_built_in(&codec);
  }

  //! \}
};

#endif
//! \}

//! \}

#endif // BLEND2D_IMAGECODEC_H_INCLUDED
