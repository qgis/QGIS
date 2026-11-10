// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/array_p.h>
#include <blend2d/core/filesystem.h>
#include <blend2d/core/format_p.h>
#include <blend2d/core/image_p.h>
#include <blend2d/core/imagecodec.h>
#include <blend2d/core/imagedecoder.h>
#include <blend2d/core/imageencoder.h>
#include <blend2d/core/imagescale_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/pixelconverter_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/memops_p.h>

namespace bl {
namespace ImageInternal {

// bl::Image - Globals
// ===================

static BLObjectEternalImpl<BLImagePrivateImpl> default_image;

// bl::Image - Constants
// =====================

static constexpr uint32_t kLargeDataAlignment = 64;
static constexpr uint32_t kLargeDataThreshold = 1024;
static constexpr uint32_t kMaxAddressableOffset = 0x7FFFFFFFu;

// bl::Image - Utilities
// =====================

static BL_INLINE uint32_t stride_for_width(uint32_t width, uint32_t depth) noexcept {
  return (uint32_t(width) * depth + 7u) / 8u;
}

static BL_INLINE bool check_size_and_format(int w, int h, BLFormat format) noexcept {
  return !(unsigned(w) - 1u >= BL_RUNTIME_MAX_IMAGE_SIZE ||
           unsigned(h) - 1u >= BL_RUNTIME_MAX_IMAGE_SIZE ||
           unsigned(format) - 1u >= BL_FORMAT_MAX_VALUE);
}

static BL_INLINE BLResultT<intptr_t> calc_stride_from_create_params(int w, int h, BLFormat format) noexcept {
  if (BL_UNLIKELY(!check_size_and_format(w, h, format))) {
    BLResult result =
      w <= 0 || h <= 0 || unsigned(format - 1) >= BL_FORMAT_MAX_VALUE
        ? BL_ERROR_INVALID_VALUE
        : BL_ERROR_IMAGE_TOO_LARGE;
    return BLResultT<intptr_t>{result, 0};
  }
  else {
    uint32_t bytes_per_line = stride_for_width(uint32_t(w), bl_format_info[format].depth);
    uint64_t bytes_per_image = uint64_t(bytes_per_line) * unsigned(h);

    // NOTE: Align the stride to 16 bytes if bytes_per_line is not too small. The reason is that when multi-threaded
    // rendering is used and bytes_per_line not aligned, some bands could share a cache line, which would potentially
    // affect the performance in a very negative way.
    if (bytes_per_line > 256u)
      bytes_per_line = IntOps::align_up(bytes_per_line, 16);

    BLResult result = bytes_per_image <= kMaxAddressableOffset ? BL_SUCCESS : BL_ERROR_IMAGE_TOO_LARGE;
    return BLResultT<intptr_t>{result, intptr_t(bytes_per_line)};
  }
}

// Make sure that the external image won't cause any kind of overflow in rasterization and texture fetching.
static BL_INLINE BLResult check_create_from_data_params(int w, int h, BLFormat format, intptr_t stride) noexcept {
  uint32_t minimum_stride = stride_for_width(uint32_t(w), bl_format_info[format].depth);
  uintptr_t bytes_per_line = uintptr_t(bl_abs(stride));

  if (BL_UNLIKELY(!check_size_and_format(w, h, format) || bytes_per_line < minimum_stride))
    return BL_ERROR_INVALID_VALUE;

  // Make sure that the image height multiplied by stride is not greater than 2^31 - this makes sure that we
  // can handle also negative strides properly and that we can guarantee that all pixels are addressable via
  // 32-bit offsets, which is required by some SIMD fetchers.
  //
  // NOTE: BytesPerImage also considers all parent images in case this image is indeed a sub-image. The reason
  // is that we have to address all pixels in this sub-image too, so basically we include the parent in the
  // computation as well. Since a sub-image may be managed by the user (outside of Blend2D) we have to check
  // this every time an external pixel data is used.
  uint64_t bytes_per_image = uint64_t(bytes_per_line) * uint64_t(uint32_t(h));
  return bytes_per_line > uintptr_t(kMaxAddressableOffset) || bytes_per_image > kMaxAddressableOffset ? BL_ERROR_IMAGE_TOO_LARGE : BL_SUCCESS;
}

static void copy_image_data(uint8_t* dst_data, intptr_t dst_stride, const uint8_t* src_data, intptr_t src_stride, int w, int h, BLFormat format) noexcept {
  size_t bytes_per_line = (size_t(unsigned(w)) * bl_format_info[format].depth + 7u) / 8u;

  if (intptr_t(bytes_per_line) == dst_stride && intptr_t(bytes_per_line) == src_stride) {
    // Special case that happens often - stride equals bytes-per-line (no gaps).
    memcpy(dst_data, src_data, bytes_per_line * unsigned(h));
    return;
  }
  else {
    // Generic case - there are either gaps or source/destination is a sub-image.
    size_t gap = dst_stride > 0 ? size_t(dst_stride) - bytes_per_line : size_t(0);
    for (unsigned y = unsigned(h); y; y--) {
      memcpy(dst_data, src_data, bytes_per_line);
      bl::MemOps::fill_small_t(dst_data + bytes_per_line, uint8_t(0), gap);

      dst_data += dst_stride;
      src_data += src_stride;
    }
  }
}

// bl::Image - Alloc & Free Impl
// =============================

static BL_INLINE void init_impl_data(BLImagePrivateImpl* impl, int w, int h, BLFormat format, void* pixel_data, intptr_t stride) noexcept {
  impl->pixel_data = pixel_data;
  impl->size.reset(w, h);
  impl->stride = stride;
  impl->format = uint8_t(format);
  impl->flags = uint8_t(0);
  impl->depth = uint16_t(bl_format_info[format].depth);
  memset(impl->reserved, 0, sizeof(impl->reserved));
}

static BL_NOINLINE BLResult alloc_impl(BLImageCore* self, int w, int h, BLFormat format, intptr_t stride) noexcept {
  BL_ASSERT(w > 0);
  BL_ASSERT(h > 0);
  BL_ASSERT(format != BL_FORMAT_NONE);
  BL_ASSERT(format <= BL_FORMAT_MAX_VALUE);
  BL_ASSERT(stride > 0);

  size_t kBaseImplSize = IntOps::align_up(sizeof(BLImagePrivateImpl), BL_OBJECT_IMPL_ALIGNMENT);
  size_t pixel_data_size = size_t(h) * size_t(stride);

  BLObjectImplSize impl_size(kBaseImplSize + pixel_data_size);
  if (pixel_data_size >= kLargeDataThreshold)
    impl_size += kLargeDataAlignment - BL_OBJECT_IMPL_ALIGNMENT;

  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_IMAGE);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLImagePrivateImpl>(self, info, impl_size));

  BLImagePrivateImpl* impl = get_impl(self);
  uint8_t* pixel_data = reinterpret_cast<uint8_t*>(impl) + kBaseImplSize;

  if (pixel_data_size >= kLargeDataThreshold)
    pixel_data = bl::IntOps::align_up(pixel_data, kLargeDataAlignment);

  init_impl_data(impl, w, h, format, pixel_data, stride);
  impl->writer_count = 0;
  return BL_SUCCESS;
}

static BL_NOINLINE BLResult alloc_external(BLImageCore* self, int w, int h, BLFormat format, void* pixel_data, intptr_t stride, bool immutable, BLDestroyExternalDataFunc destroy_func, void* user_data) noexcept {
  BL_ASSERT(w > 0);
  BL_ASSERT(h > 0);
  BL_ASSERT(format != BL_FORMAT_NONE);
  BL_ASSERT(format <= BL_FORMAT_MAX_VALUE);

  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_IMAGE);
  BL_PROPAGATE(ObjectInternal::alloc_impl_external_t<BLImagePrivateImpl>(self, info, immutable, destroy_func, user_data));

  BLImagePrivateImpl* impl = get_impl(self);
  init_impl_data(impl, w, h, format, pixel_data, stride);
  impl->writer_count = 0;
  return BL_SUCCESS;
}

// Must be available outside of BLImage implementation.
BLResult free_impl(BLImagePrivateImpl* impl) noexcept {
  // Postpone the deletion in case that the image still has writers attached. This is required as the rendering
  // context doesn't manipulate the reference count of `BLImage` (otherwise it would not be possible to attach
  // multiple rendering contexts, for example).
  if (impl->writer_count != 0)
    return BL_SUCCESS;

  if (ObjectInternal::is_impl_external(impl))
    ObjectInternal::call_external_destroy_func(impl, impl->pixel_data);

  return ObjectInternal::free_impl(impl);
}

} // {ImageInternal}
} // {bl}

// bl::Image - API - Init & Destroy
// ================================

BL_API_IMPL BLResult bl_image_init(BLImageCore* self) noexcept {
  using namespace bl::ImageInternal;

  self->_d = bl_object_defaults[BL_OBJECT_TYPE_IMAGE]._d;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_image_init_move(BLImageCore* self, BLImageCore* other) noexcept {
  using namespace bl::ImageInternal;

  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_image());

  self->_d = other->_d;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_IMAGE]._d;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_image_init_weak(BLImageCore* self, const BLImageCore* other) noexcept {
  using namespace bl::ImageInternal;

  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_image());

  self->_d = other->_d;
  return retain_instance(self);
}

BL_API_IMPL BLResult bl_image_init_as(BLImageCore* self, int w, int h, BLFormat format) noexcept {
  using namespace bl::ImageInternal;

  self->_d = bl_object_defaults[BL_OBJECT_TYPE_IMAGE]._d;
  return bl_image_create(self, w, h, format);
}

BL_API_IMPL BLResult bl_image_init_as_from_data(
    BLImageCore* self, int w, int h, BLFormat format,
    void* pixel_data, intptr_t stride,
    BLDataAccessFlags access_flags,
    BLDestroyExternalDataFunc destroy_func, void* user_data) noexcept {

  using namespace bl::ImageInternal;

  self->_d = bl_object_defaults[BL_OBJECT_TYPE_IMAGE]._d;
  return bl_image_create_from_data(self, w, h, format, pixel_data, stride, access_flags, destroy_func, user_data);
}

BL_API_IMPL BLResult bl_image_destroy(BLImageCore* self) noexcept {
  using namespace bl::ImageInternal;

  BL_ASSERT(self->_d.is_image());
  return release_instance(self);
}

// bl::Image - API - Reset
// =======================

BL_API_IMPL BLResult bl_image_reset(BLImageCore* self) noexcept {
  using namespace bl::ImageInternal;

  BL_ASSERT(self->_d.is_image());
  return replace_instance(self, static_cast<BLImageCore*>(&bl_object_defaults[BL_OBJECT_TYPE_IMAGE]));
}

// bl::Image - API - Assign
// ========================

BL_API_IMPL BLResult bl_image_assign_move(BLImageCore* self, BLImageCore* other) noexcept {
  using namespace bl::ImageInternal;

  BL_ASSERT(self->_d.is_image());
  BL_ASSERT(other->_d.is_image());

  BLImageCore tmp = *other;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_IMAGE]._d;
  return replace_instance(self, &tmp);
}

BL_API_IMPL BLResult bl_image_assign_weak(BLImageCore* self, const BLImageCore* other) noexcept {
  using namespace bl::ImageInternal;

  BL_ASSERT(self->_d.is_image());
  BL_ASSERT(other->_d.is_image());

  retain_instance(other);
  return replace_instance(self, other);
}

BL_API_IMPL BLResult bl_image_assign_deep(BLImageCore* self, const BLImageCore* other) noexcept {
  using namespace bl::ImageInternal;

  BL_ASSERT(self->_d.is_image());
  BL_ASSERT(other->_d.is_image());

  BLImagePrivateImpl* self_impl = get_impl(self);
  BLImagePrivateImpl* other_impl = get_impl(other);

  BLSizeI size = other_impl->size;
  BLFormat format = BLFormat(other_impl->format);

  if (format == BL_FORMAT_NONE)
    return bl_image_reset(self);

  BLImageData dummy_image_data;
  if (self_impl == other_impl)
    return bl_image_make_mutable(self, &dummy_image_data);

  BL_PROPAGATE(bl_image_create(self, size.w, size.h, format));
  self_impl = get_impl(self);

  copy_image_data(static_cast<uint8_t*>(self_impl->pixel_data), self_impl->stride,
              static_cast<uint8_t*>(other_impl->pixel_data), other_impl->stride, size.w, size.h, format);
  return BL_SUCCESS;
}

// bl::Image - API - Create
// ========================

BL_API_IMPL BLResult bl_image_create(BLImageCore* self, int w, int h, BLFormat format) noexcept {
  using namespace bl::ImageInternal;

  BL_ASSERT(self->_d.is_image());

  BLResultT<intptr_t> result = calc_stride_from_create_params(w, h, format);
  if (BL_UNLIKELY(result.code != BL_SUCCESS)) {
    if ((w | h) == 0 && format == BL_FORMAT_NONE)
      return bl_image_reset(self);
    else
      return bl_make_error(result.code);
  }

  BLImagePrivateImpl* self_impl = get_impl(self);
  if (self_impl->size == BLSizeI(w, h) && self_impl->format == format)
    if (bl::ObjectInternal::is_impl_mutable(self_impl) && !bl::ObjectInternal::is_impl_external(self_impl))
      return BL_SUCCESS;

  BLImageCore newO;
  BL_PROPAGATE(alloc_impl(&newO, w, h, format, result.value));

  return replace_instance(self, &newO);
}

BL_API_IMPL BLResult bl_image_create_from_data(
    BLImageCore* self, int w, int h, BLFormat format,
    void* pixel_data, intptr_t stride,
    BLDataAccessFlags access_flags,
    BLDestroyExternalDataFunc destroy_func, void* user_data) noexcept {

  using namespace bl::ImageInternal;

  BL_ASSERT(self->_d.is_image());

  BLResult result = check_create_from_data_params(w, h, format, stride);
  if (BL_UNLIKELY(result != BL_SUCCESS))
    return bl_make_error(result);

  BLImagePrivateImpl* self_impl = get_impl(self);
  bool immutable = !(access_flags & BL_DATA_ACCESS_WRITE);

  if (bl::ObjectInternal::is_impl_external(self_impl) && bl::ObjectInternal::is_impl_ref_count_equal_to_base(self_impl) && self_impl->writer_count == 0) {
    // OPTIMIZATION: If the user code calls BLImage::create_from_data() for every frame, use the same Impl
    // if the `ref_count == 1` and the Impl is external to avoid a malloc()/free() roundtrip for each call.
    bl::ObjectInternal::call_external_destroy_func(self_impl, self_impl->pixel_data);
    bl::ObjectInternal::init_external_destroy_func(self_impl, destroy_func, user_data);
    bl::ObjectInternal::init_ref_count_to_base(self_impl, immutable);

    init_impl_data(self_impl, w, h, format, pixel_data, stride);
    return BL_SUCCESS;
  }
  else {
    BLImageCore newO;
    BL_PROPAGATE(alloc_external(&newO, w, h, format, pixel_data, stride, immutable, destroy_func, user_data));

    return replace_instance(self, &newO);
  }
}

// bl::Image - API - Accessors
// ===========================

BL_API_IMPL BLResult bl_image_get_data(const BLImageCore* self, BLImageData* data_out) noexcept {
  using namespace bl::ImageInternal;

  BL_ASSERT(self->_d.is_image());
  BLImagePrivateImpl* self_impl = get_impl(self);

  data_out->pixel_data = self_impl->pixel_data;
  data_out->stride = self_impl->stride;
  data_out->size = self_impl->size;
  data_out->format = self_impl->format;
  data_out->flags = 0;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_image_make_mutable(BLImageCore* self, BLImageData* data_out) noexcept {
  using namespace bl::ImageInternal;

  BL_ASSERT(self->_d.is_image());
  BLImagePrivateImpl* self_impl = get_impl(self);

  BLSizeI size = self_impl->size;
  BLFormat format = (BLFormat)self_impl->format;

  if (format != BL_FORMAT_NONE && !is_impl_mutable(self_impl)) {
    BLImageCore newO;
    BL_PROPAGATE(alloc_impl(&newO, size.w, size.h, format,
      intptr_t(stride_for_width(uint32_t(size.w), bl_format_info[format].depth))));

    BLImagePrivateImpl* new_impl = get_impl(&newO);
    data_out->pixel_data = new_impl->pixel_data;
    data_out->stride = new_impl->stride;
    data_out->size = size;
    data_out->format = format;
    data_out->flags = 0;

    copy_image_data(static_cast<uint8_t*>(new_impl->pixel_data), new_impl->stride,
                  static_cast<uint8_t*>(self_impl->pixel_data), self_impl->stride, size.w, size.h, format);

    return replace_instance(self, &newO);
  }
  else {
    data_out->pixel_data = self_impl->pixel_data;
    data_out->stride = self_impl->stride;
    data_out->size = size;
    data_out->format = format;
    data_out->flags = 0;
    return BL_SUCCESS;
  }
}

// bl::Image - API - Convert
// =========================

BL_API_IMPL BLResult bl_image_convert(BLImageCore* self, BLFormat format) noexcept {
  using namespace bl::ImageInternal;

  BL_ASSERT(self->_d.is_image());
  BLImagePrivateImpl* self_impl = get_impl(self);

  bl::FormatExt src_format = bl::FormatExt(self_impl->format);
  bl::FormatExt dst_format = bl::FormatExt(format);

  if (dst_format == src_format)
    return BL_SUCCESS;

  if (dst_format == bl::FormatExt::kXRGB32)
    dst_format = bl::FormatExt::kFRGB32;

  if (src_format == bl::FormatExt::kNone)
    return bl_make_error(BL_ERROR_NOT_INITIALIZED);

  BLResult result = BL_SUCCESS;
  BLPixelConverterCore pc {};

  BLSizeI size = self_impl->size;
  const BLFormatInfo& di = bl_format_info[size_t(dst_format)];
  const BLFormatInfo& si = bl_format_info[size_t(src_format)];

  // Save some cycles by calling `bl_pixel_converter_init_internal` as we don't need to sanitize the destination and
  // source formats in this case.
  if (bl_pixel_converter_init_internal(&pc, di, si, BL_PIXEL_CONVERTER_CREATE_NO_FLAGS) != BL_SUCCESS) {
    // Built-in formats should always have a built-in converter, so report a different error if the initialization
    // failed. This is pretty critical.
    return bl_make_error(BL_ERROR_INVALID_STATE);
  }

  if (di.depth == si.depth && is_impl_mutable(self_impl)) {
    // Prefer in-place conversion if the depths are equal and the image mutable.
    pc.convert_func(&pc, static_cast<uint8_t*>(self_impl->pixel_data), self_impl->stride,
                        static_cast<uint8_t*>(self_impl->pixel_data), self_impl->stride, uint32_t(size.w), uint32_t(size.h), nullptr);
    self_impl->format = uint8_t(format);
  }
  else {
    BLImageCore dst_image;
    result = bl_image_init_as(&dst_image, size.w, size.h, format);

    if (result == BL_SUCCESS) {
      BLImagePrivateImpl* dst_impl = get_impl(&dst_image);
      BLPixelConverterOptions opt {};

      opt.gap = uintptr_t(bl_abs(dst_impl->stride)) - uintptr_t(uint32_t(size.w)) * (dst_impl->depth / 8u);
      pc.convert_func(&pc, static_cast<uint8_t*>(dst_impl->pixel_data), dst_impl->stride,
                          static_cast<uint8_t*>(self_impl->pixel_data), self_impl->stride, uint32_t(size.w), uint32_t(size.h), &opt);

      return replace_instance(self, &dst_image);
    }
  }

  bl_pixel_converter_reset(&pc);
  return result;
}

// bl::Image - API - Equality & Comparison
// =======================================

BL_API_IMPL bool bl_image_equals(const BLImageCore* a, const BLImageCore* b) noexcept {
  using namespace bl::ImageInternal;

  BL_ASSERT(a->_d.is_image());
  BL_ASSERT(b->_d.is_image());

  const BLImagePrivateImpl* a_impl = get_impl(a);
  const BLImagePrivateImpl* b_impl = get_impl(b);

  if (a_impl == b_impl)
    return true;

  if (a_impl->size != b_impl->size || a_impl->format != b_impl->format)
    return false;

  uint32_t w = uint32_t(a_impl->size.w);
  uint32_t h = uint32_t(a_impl->size.h);

  const uint8_t* a_data = static_cast<const uint8_t*>(a_impl->pixel_data);
  const uint8_t* b_data = static_cast<const uint8_t*>(b_impl->pixel_data);

  intptr_t a_stride = a_impl->stride;
  intptr_t b_stride = b_impl->stride;

  size_t bytes_per_line = (w * bl_format_info[a_impl->format].depth + 7u) / 8u;
  for (uint32_t y = 0; y < h; y++) {
    if (memcmp(a_data, b_data, bytes_per_line) != 0)
      return false;
    a_data += a_stride;
    b_data += b_stride;
  }

  return true;
}

// bl::Image - API - Scale
// =======================

BL_API_IMPL BLResult bl_image_scale(BLImageCore* dst, const BLImageCore* src, const BLSizeI* size, BLImageScaleFilter filter) noexcept {
  using namespace bl::ImageInternal;

  BL_ASSERT(dst->_d.is_image());
  BL_ASSERT(src->_d.is_image());

  BLImagePrivateImpl* src_impl = get_impl(src);
  if (src_impl->format == BL_FORMAT_NONE)
    return bl_image_reset(dst);

  bl::ImageScaleContext scale_ctx;
  BL_PROPAGATE(scale_ctx.create(*size, src_impl->size, filter));

  BLFormat format = BLFormat(src_impl->format);
  int tw = scale_ctx.dst_width();
  int th = scale_ctx.src_height();

  BLImage tmp;
  BLImageData buf;

  if (th == scale_ctx.dst_height() || tw == scale_ctx.src_width()) {
    // Only horizontal or vertical scale.

    // Move to `tmp` so it's not destroyed by `dst->create()`.
    if (dst == src)
      tmp = src->dcast();

    BL_PROPAGATE(bl_image_create(dst, scale_ctx.dst_width(), scale_ctx.dst_height(), format));
    BL_PROPAGATE(bl_image_make_mutable(dst, &buf));

    if (th == scale_ctx.dst_height())
      scale_ctx.process_horz_data(static_cast<uint8_t*>(buf.pixel_data), buf.stride, static_cast<const uint8_t*>(src_impl->pixel_data), src_impl->stride, format);
    else
      scale_ctx.process_vert_data(static_cast<uint8_t*>(buf.pixel_data), buf.stride, static_cast<const uint8_t*>(src_impl->pixel_data), src_impl->stride, format);
  }
  else {
    // Both horizontal and vertical scale.
    BL_PROPAGATE(tmp.create(tw, th, format));
    BL_PROPAGATE(tmp.make_mutable(&buf));
    scale_ctx.process_horz_data(static_cast<uint8_t*>(buf.pixel_data), buf.stride, static_cast<const uint8_t*>(src_impl->pixel_data), src_impl->stride, format);

    src_impl = get_impl(&tmp);
    BL_PROPAGATE(bl_image_create(dst, scale_ctx.dst_width(), scale_ctx.dst_height(), format));
    BL_PROPAGATE(bl_image_make_mutable(dst, &buf));

    scale_ctx.process_vert_data(static_cast<uint8_t*>(buf.pixel_data), buf.stride, static_cast<const uint8_t*>(src_impl->pixel_data), src_impl->stride, format);
  }

  return BL_SUCCESS;
}

// bl::Image - API - Read File
// ===========================

BL_API_IMPL BLResult bl_image_read_from_file(BLImageCore* self, const char* file_name, const BLArrayCore* codecs) noexcept {
  using namespace bl::ImageInternal;

  BL_ASSERT(self->_d.is_image());

  BLArray<uint8_t> buffer;
  BL_PROPAGATE(BLFileSystem::read_file(file_name, buffer));

  if (buffer.is_empty())
    return bl_make_error(BL_ERROR_FILE_EMPTY);

  BLImageCodec codec;
  BL_PROPAGATE(bl_image_codec_find_by_data(&codec, buffer.data(), buffer.size(), codecs));

  if (BL_UNLIKELY(!(codec.features() & BL_IMAGE_CODEC_FEATURE_READ)))
    return bl_make_error(BL_ERROR_IMAGE_DECODER_NOT_PROVIDED);

  BLImageDecoder decoder;
  BL_PROPAGATE(codec.create_decoder(&decoder));
  return decoder.read_frame(*self, buffer);
}

// bl::Image - API - Read Data
// ===========================

BL_API_IMPL BLResult bl_image_read_from_data(BLImageCore* self, const void* data, size_t size, const BLArrayCore* codecs) noexcept {
  using namespace bl::ImageInternal;

  BL_ASSERT(self->_d.is_image());

  BLImageCodec codec;
  BL_PROPAGATE(bl_image_codec_find_by_data(&codec, data, size, codecs));

  if (BL_UNLIKELY(!(codec.features() & BL_IMAGE_CODEC_FEATURE_READ)))
    return bl_make_error(BL_ERROR_IMAGE_DECODER_NOT_PROVIDED);

  BLImageDecoder decoder;
  BL_PROPAGATE(codec.create_decoder(&decoder));
  return decoder.read_frame(*self, data, size);
}

// bl::Image - API - Write File
// ============================

namespace bl {
namespace ImageInternal {

static BLResult write_to_file_internal(const BLImageCore* self, const char* file_name, const BLImageCodecCore* codec) noexcept {
  BL_ASSERT(self->_d.is_image());
  BL_ASSERT(codec->_d.is_image_codec());

  BLArray<uint8_t> buffer;
  BL_PROPAGATE(bl_image_write_to_data(self, &buffer, codec));
  return BLFileSystem::write_file(file_name, buffer);
}

} // {ImageInternal}
} // {bl}

BL_API_IMPL BLResult bl_image_write_to_file(const BLImageCore* self, const char* file_name, const BLImageCodecCore* codec) noexcept {
  using namespace bl::ImageInternal;

  BL_ASSERT(self->_d.is_image());

  if (!codec) {
    BLImageCodec local_codec;
    BL_PROPAGATE(local_codec.find_by_extension(file_name));
    return write_to_file_internal(self, file_name, &local_codec);
  }
  else {
    BL_ASSERT(codec->_d.is_image_codec());
    return write_to_file_internal(self, file_name, codec);
  }
}

// bl::Image - API - Write Data
// ============================

BL_API_IMPL BLResult bl_image_write_to_data(const BLImageCore* self, BLArrayCore* dst, const BLImageCodecCore* codec) noexcept {
  using namespace bl::ImageInternal;

  BL_ASSERT(self->_d.is_image());
  BL_ASSERT(codec->_d.is_image_codec());

  if (BL_UNLIKELY(!(codec->dcast().features() & BL_IMAGE_CODEC_FEATURE_WRITE)))
    return bl_make_error(BL_ERROR_IMAGE_ENCODER_NOT_PROVIDED);

  BLImageEncoder encoder;
  BL_PROPAGATE(codec->dcast().create_encoder(&encoder));
  return encoder.write_frame(dst->dcast<BLArray<uint8_t>>(), self->dcast());
}

// bl::Image - Runtime Registration
// ================================

void bl_image_rt_init(BLRuntimeContext* rt) noexcept {
  bl_unused(rt);

  auto& default_image = bl::ImageInternal::default_image;

  bl_object_defaults[BL_OBJECT_TYPE_IMAGE]._d.init_dynamic(
    BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_IMAGE),
    &default_image.impl);
}
