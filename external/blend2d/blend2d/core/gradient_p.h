// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_GRADIENT_P_H_INCLUDED
#define BLEND2D_GRADIENT_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/gradient.h>
#include <blend2d/core/object_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

//! \name BLGradient - Private Structs
//! \{

//! Lookup table that contains interpolated pixels of the gradient in either PRGB32 or PRGB64 format.
struct BLGradientLUT {
  //! \name Members
  //! \{

  //! Reference count.
  size_t ref_count;
  //! Table size - must be a power of 2!
  size_t size;

  //! \}

  //! \name Accessors
  //! \{

  template<typename T = void>
  BL_INLINE_NODEBUG T* data() noexcept { return reinterpret_cast<T*>(this + 1); }

  template<typename T = void>
  const BL_INLINE_NODEBUG T* data() const noexcept { return reinterpret_cast<const T*>(this + 1); }

  //! \}

  //! \name Alloc & Destroy
  //! \{

  static BL_INLINE BLGradientLUT* alloc(size_t size, uint32_t pixel_size) noexcept {
    BLGradientLUT* self = static_cast<BLGradientLUT*>(malloc(sizeof(BLGradientLUT) + size * pixel_size));

    if (BL_UNLIKELY(!self)) {
      return nullptr;
    }

    self->ref_count = 1;
    self->size = size;

    return self;
  }

  static BL_INLINE void destroy(BLGradientLUT* self) noexcept {
    free(self);
  }

  //! \}

  //! \name Reference Counting
  //! \{

  BL_INLINE BLGradientLUT* retain() noexcept {
    bl_atomic_fetch_add_relaxed(&ref_count);
    return this;
  }

  BL_INLINE bool dec_ref_and_test() noexcept {
    return bl_atomic_fetch_sub_strong(&ref_count) == 1;
  }

  BL_INLINE void release() noexcept {
    if (dec_ref_and_test()) {
      destroy(this);
    }
  }

  //! \}
};

//! Additional information maintained by `BLGradientImpl` that is cached and is useful when deciding how to
//! render the gradient and how big the LUT should be.
struct BLGradientInfo {
  //! \name Members
  //! \{

  union {
    uint32_t packed;

    struct {
      //! True if the gradient is a solid color.
      uint8_t solid;
      //! Gradient format (either 32-bit or 64-bit).
      uint8_t format;
      //! Optimal BLGradientLUT size.
      uint16_t _lut_size;
    };
  };

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE_NODEBUG bool is_empty() const noexcept { return packed == 0; }
  BL_INLINE_NODEBUG void reset() noexcept { packed = 0; }

  BL_INLINE uint32_t lut_size(bool high_quality) const noexcept {
    if (high_quality) {
      return bl_min<uint32_t>(_lut_size * 2, 1024);
    }
    else {
      return _lut_size;
    }
  }

  //! \}
};

//! \}

//! \name BLGradient - Private Impl
//! \{

//! Private implementation that extends \ref BLGradientImpl.
struct BLGradientPrivateImpl : public BLGradientImpl {
  union {
    //! Gradient LUT cache as an array.
    BLGradientLUT* lut[2];
    struct {
      //! Gradient LUT cache (32-bit).
      BLGradientLUT* lut32;
      //! Gradient LUT cache (64-bit).
      BLGradientLUT* lut64;
    };
  };
  //! Information regarding gradient stops.
  BLGradientInfo info32;
};

//! \}

namespace bl {
namespace GradientInternal {

//! \name BLGradient - Internals - Common Functionality (Container)
//! \{

static BL_INLINE_CONSTEXPR BLObjectImplSize impl_size_from_capacity(size_t n) noexcept {
  return BLObjectImplSize(sizeof(BLGradientPrivateImpl) + n * sizeof(BLGradientStop));
}

static BL_INLINE_CONSTEXPR size_t capacity_from_impl_size(BLObjectImplSize impl_size) noexcept {
  return (impl_size.value() - sizeof(BLGradientPrivateImpl)) / sizeof(BLGradientStop);
}

//! \}

//! \name BLGradient - Internals - Common Functionality (Impl)
//! \{

static BL_INLINE bool is_impl_mutable(BLGradientImpl* impl) noexcept {
  return ObjectInternal::is_impl_mutable(impl);
}

BL_HIDDEN BLResult free_impl(BLGradientPrivateImpl* impl) noexcept;

template<RCMode kRCMode>
static BL_INLINE BLResult release_impl(BLGradientPrivateImpl* impl) noexcept {
  return ObjectInternal::deref_impl_and_test<kRCMode>(impl) ? free_impl(impl) : BLResult(BL_SUCCESS);
}

//! \}

//! \name BLGradient - Internals - Common Functionality (Instance)
//! \{

static BL_INLINE_NODEBUG BLGradientPrivateImpl* get_impl(const BLGradientCore* self) noexcept {
  return static_cast<BLGradientPrivateImpl*>(self->_d.impl);
}

static BL_INLINE BLResult retain_instance(const BLGradientCore* self, size_t n = 1) noexcept {
  return ObjectInternal::retain_instance(self, n);
}

static BL_INLINE BLResult release_instance(BLGradientCore* self) noexcept {
  return release_impl<RCMode::kMaybe>(get_impl(self));
}

static BL_INLINE BLResult replace_instance(BLGradientCore* self, const BLGradientCore* other) noexcept {
  BLGradientPrivateImpl* impl = get_impl(self);
  self->_d = other->_d;
  return release_impl<RCMode::kMaybe>(impl);
}

//! \}

//! \name BLGradient - Internals - Accessors
//! \{

static BL_INLINE_NODEBUG uint32_t pack_abcp(BLGradientType type, BLExtendMode extend_mode, BLTransformType transform_type) noexcept {
  return BLObjectInfo::pack_abcp(uint32_t(type), uint32_t(extend_mode), uint32_t(transform_type));
}

static BL_INLINE_NODEBUG BLGradientType get_gradient_type(const BLGradientCore* self) noexcept { return BLGradientType(self->_d.info.a_field()); }
static BL_INLINE_NODEBUG BLExtendMode get_extend_mode(const BLGradientCore* self) noexcept { return BLExtendMode(self->_d.info.b_field()); }
static BL_INLINE_NODEBUG BLTransformType get_transform_type(const BLGradientCore* self) noexcept { return BLTransformType(self->_d.info.c_field()); }

static BL_INLINE_NODEBUG void set_gradient_type(BLGradientCore* self, BLGradientType type) noexcept { self->_d.info.set_a_field(uint32_t(type)); }
static BL_INLINE_NODEBUG void set_extend_mode(BLGradientCore* self, BLExtendMode extend_mode) noexcept { self->_d.info.set_b_field(uint32_t(extend_mode)); }
static BL_INLINE_NODEBUG void set_transform_type(BLGradientCore* self, BLTransformType transform_type) noexcept { self->_d.info.set_c_field(uint32_t(transform_type)); }

//! \}

//! \name BLGradient - Internals - LUT Cache
//! \{

BL_HIDDEN BLGradientInfo ensure_info(BLGradientPrivateImpl* impl_) noexcept;
BL_HIDDEN BLGradientLUT* ensure_lut32(BLGradientPrivateImpl* impl_, uint32_t lut_size) noexcept;
BL_HIDDEN BLGradientLUT* ensure_lut64(BLGradientPrivateImpl* impl_, uint32_t lut_size) noexcept;

//! \}

} // {GradientInternal}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_GRADIENT_P_H_INCLUDED
