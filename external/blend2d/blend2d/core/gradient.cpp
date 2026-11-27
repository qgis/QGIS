// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/array_p.h>
#include <blend2d/core/format_p.h>
#include <blend2d/core/gradient_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/rgba_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/pixelops/funcs_p.h>
#include <blend2d/support/algorithm_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/lookuptable_p.h>
#include <blend2d/support/math_p.h>
#include <blend2d/support/ptrops_p.h>
#include <blend2d/threading/atomic_p.h>

namespace bl {
namespace GradientInternal {

// bl::Gradient - Globals
// ======================

static BLObjectEternalImpl<BLGradientPrivateImpl> default_impl;

static constexpr const double no_values[BL_GRADIENT_VALUE_MAX_VALUE + 1] = { 0.0 };
static constexpr const BLMatrix2D no_matrix(1.0, 0.0, 0.0, 1.0, 0.0, 0.0);

// bl::Gradient - Tables
// =====================

struct BLGradientValueCountTableGen {
  static constexpr uint8_t value(size_t i) noexcept {
    return i == BL_GRADIENT_TYPE_LINEAR ? uint8_t(sizeof(BLLinearGradientValues ) / sizeof(double)) :
           i == BL_GRADIENT_TYPE_RADIAL ? uint8_t(sizeof(BLRadialGradientValues ) / sizeof(double)) :
           i == BL_GRADIENT_TYPE_CONIC ? uint8_t(sizeof(BLConicGradientValues) / sizeof(double)) : uint8_t(0);
  }
};

static constexpr const auto value_count_table =
  make_lookup_table<uint8_t, BL_GRADIENT_TYPE_MAX_VALUE + 1, BLGradientValueCountTableGen>();

// bl::Gradient - Internals & Utilities
// ====================================

static BL_INLINE_NODEBUG size_t get_size(const BLGradientCore* self) noexcept { return get_impl(self)->size; }
static BL_INLINE_NODEBUG size_t get_capacity(const BLGradientCore* self) noexcept { return get_impl(self)->capacity; }
static BL_INLINE_NODEBUG BLGradientStop* get_stops(const BLGradientCore* self) noexcept { return get_impl(self)->stops; }

static constexpr size_t kInitialImplSize = IntOps::align_up(impl_size_from_capacity(2).value(), BL_OBJECT_IMPL_ALIGNMENT);

// bl::Gradient - Internals - Analysis
// ===================================

static BL_INLINE uint32_t analyze_stop_array(const BLGradientStop* stops, size_t n) noexcept {
  uint32_t result = BL_DATA_ANALYSIS_CONFORMING;
  uint32_t was_same = false;
  double prev = -1.0;

  for (size_t i = 0; i < n; i++) {
    double offset = stops[i].offset;
    if (!((offset >= 0.0) & (offset <= 1.0)))
      return BL_DATA_ANALYSIS_INVALID_VALUE;

    uint32_t is_same = (offset == prev);
    result |= (offset < prev);
    result |= is_same & was_same;

    was_same = is_same;
    prev = offset;
  }

  return result;
}

// bl::Gradient - Internals - Stop Matcher
// =======================================

struct GradientStopMatcher {
  double offset;
  BL_INLINE GradientStopMatcher(double offset) noexcept : offset(offset) {}
};
static BL_INLINE bool operator==(const BLGradientStop& a, const GradientStopMatcher& b) noexcept { return a.offset == b.offset; }
static BL_INLINE bool operator<=(const BLGradientStop& a, const GradientStopMatcher& b) noexcept { return a.offset <= b.offset; }

// bl::Gradient - Internals - AltStop
// ==================================

// Alternative representation of `BLGradientStop` that is used to sort unknown stop array that is either unsorted or
// may contain more than 2 stops that have the same offset. The `index` member is actually an index to the original
// stop array.
struct GradientStopAlt {
  double offset;
  union {
    intptr_t index;
    uint64_t rgba;
  };
};

BL_STATIC_ASSERT(sizeof(GradientStopAlt) == sizeof(BLGradientStop));

// bl::Gradient - Internals - Utilities
// ====================================

static BL_INLINE void init_values(double* dst, const double* src, size_t n) noexcept {
  size_t i;

  BL_NOUNROLL
  for (i = 0; i < n; i++)
    dst[i] = src[i];

  BL_NOUNROLL
  while (i <= BL_GRADIENT_VALUE_MAX_VALUE)
    dst[i++] = 0.0;
}

static BL_INLINE void move_stops(BLGradientStop* dst, const BLGradientStop* src, size_t n) noexcept {
  memmove(dst, src, n * sizeof(BLGradientStop));
}

static BL_INLINE size_t copy_stops(BLGradientStop* dst, const BLGradientStop* src, size_t n) noexcept {
  for (size_t i = 0; i < n; i++)
    dst[i] = src[i];
  return n;
}

static BL_NOINLINE size_t copy_unsafe_stops(BLGradientStop* dst, const BLGradientStop* src, size_t n, uint32_t analysis) noexcept {
  BL_ASSERT(analysis == BL_DATA_ANALYSIS_CONFORMING ||
            analysis == BL_DATA_ANALYSIS_NON_CONFORMING);

  if (analysis == BL_DATA_ANALYSIS_CONFORMING || n == 0u)
    return copy_stops(dst, src, n);

  // First copy source stops into the destination and index them.
  GradientStopAlt* stops = reinterpret_cast<GradientStopAlt*>(dst);
  for (size_t i = 0; i < n; i++) {
    stops[i].offset = src[i].offset;
    stops[i].index = intptr_t(i);
  }

  // Now sort the stops and use both `offset` and `index` as a comparator. After the sort is done we will have
  // preserved the order of all stops that have the same `offset`.
  bl::quick_sort(stops, n, [](const GradientStopAlt& a, const GradientStopAlt& b) noexcept -> intptr_t {
    intptr_t result = 0;
    if (a.offset < b.offset) result = -1;
    if (a.offset > b.offset) result = 1;
    return result ? result : a.index - b.index;
  });

  // Now assign rgba value to the stop and remove all duplicates. If there are 3 or more consecutive stops we
  // remove all except the first/second to make sharp transitions possible.
  size_t j = 0;
  double prev1 = -1.0; // Dummy, cannot be within [0..1] range.
  double prev2 = -1.0;

  for (size_t i = 0; i < n - 1u; i++) {
    double offset = stops[i].offset;
    BLRgba64 rgba = src[size_t(stops[i].index)].rgba;

    j -= size_t((prev1 == prev2) & (prev2 == offset));
    stops[j].offset = offset;
    stops[j].rgba = rgba.value;

    j++;
    prev1 = prev2;
    prev2 = offset;
  }

  // Returns the final number of stops kept. Could be the same as `n` or less.
  return j;
}

static BL_INLINE BLGradientLUT* copyMaybeNullLUT(BLGradientLUT* lut) noexcept {
  return lut ? lut->retain() : nullptr;
}

// Cache invalidation means to remove the cached lut tables from `impl`. Since modification always means to either
// create a copy of it or to modify a unique instance (not shared) it also means that we don't have to worry about
// atomic operations here.
static BL_INLINE BLResult invalidate_lut_cache(BLGradientPrivateImpl* impl) noexcept {
  BLGradientLUT* lut32 = impl->lut32;
  BLGradientLUT* lut64 = impl->lut64;

  if (uintptr_t(lut32) | uintptr_t(lut64)) {
    if (lut32)
      lut32->release();

    if (lut64)
      lut64->release();

    impl->lut32 = nullptr;
    impl->lut64 = nullptr;
  }

  impl->info32.packed = 0;
  return BL_SUCCESS;
}

BLGradientInfo ensure_info(BLGradientPrivateImpl* impl) noexcept {
  BLGradientInfo info;

  info.packed = impl->info32.packed;

  constexpr uint32_t FLAG_ALPHA_NOT_ONE  = 0x1; // Has alpha that is not 1.0.
  constexpr uint32_t FLAG_ALPHA_NOT_ZERO = 0x2; // Has alpha that is not 0.0.
  constexpr uint32_t FLAG_TRANSITION     = 0x4; // Has transition.

  if (info.packed == 0) {
    const BLGradientStop* stops = impl->stops;
    size_t stop_count = impl->size;

    if (stop_count != 0) {
      uint32_t flags = 0;
      uint64_t prev = stops[0].rgba.value & 0xFF00FF00FF00FF00u;
      uint32_t lut_size = 0;

      if (prev < 0xFF00000000000000u)
        flags |= FLAG_ALPHA_NOT_ONE;

      if (prev > 0x00FFFFFFFFFFFFFFu)
        flags |= FLAG_ALPHA_NOT_ZERO;

      for (size_t i = 1; i < stop_count; i++) {
        uint64_t value = stops[i].rgba.value & 0xFF00FF00FF00FF00u;
        if (value == prev)
          continue;

        flags |= FLAG_TRANSITION;
        if (value < 0xFF00000000000000u)
          flags |= FLAG_ALPHA_NOT_ONE;
        if (value > 0x00FFFFFFFFFFFFFFu)
          flags |= FLAG_ALPHA_NOT_ZERO;
        prev = value;
      }

      // If all alpha values are zero then we consider this to be without transition, because the whole transition
      // would result in transparent black.
      if (!(flags & FLAG_ALPHA_NOT_ZERO))
        flags &= ~FLAG_TRANSITION;

      if (!(flags & FLAG_TRANSITION)) {
        // Minimal LUT size for no transition. The engine should always convert such style into solid fill, so such
        // LUT should never be used by the renderer.
        lut_size = 256;
      }
      else {
        // TODO: This is kinda adhoc, it would be much better if we base the calculation on both stops and their
        // offsets and estimate how big the ideal table should be.
        switch (stop_count) {
          case 1: {
            lut_size = 256;
            break;
          }

          case 2: {
            // 2 stops at endpoints only require 256 entries, more stops will use 512.
            double delta = stops[1].offset - stops[0].offset;
            lut_size = (delta >= 0.998) ? 256 : 512;
            break;
          }

          case 3: {
            lut_size = (stops[0].offset <= 0.002 && stops[1].offset == 0.5 && stops[2].offset >= 0.998) ? 512 : 1024;
            break;
          }

          default: {
            lut_size = 1024;
            break;
          }
        }
      }

      info.solid = uint8_t(flags & FLAG_TRANSITION ? 0 : 1);
      info.format = uint8_t(flags & FLAG_ALPHA_NOT_ONE ? FormatExt::kPRGB32 : FormatExt::kFRGB32);
      info._lut_size = uint16_t(lut_size);

      // Update the info. It doesn't have to be atomic.
      impl->info32.packed = info.packed;
    }
  }

  return info;
}

BLGradientLUT* ensure_lut32(BLGradientPrivateImpl* impl, uint32_t lut_size) noexcept {
  BLGradientLUT* lut = impl->lut32;
  if (lut) {
    BL_ASSERT(lut->size == lut_size);
    return lut;
  }

  lut = BLGradientLUT::alloc(lut_size, 4);
  if (BL_UNLIKELY(!lut))
    return nullptr;

  const BLGradientStop* stops = impl->stops;
  PixelOps::funcs.interpolate_prgb32(lut->data<uint32_t>(), lut_size, stops, impl->size);

  // We must drop this LUT if another thread created it meanwhile.
  BLGradientLUT* expected = nullptr;
  if (!bl_atomic_compare_exchange(&impl->lut32, &expected, lut)) {
    BL_ASSERT(expected != nullptr);
    BLGradientLUT::destroy(lut);
    lut = expected;
  }

  return lut;
}

BLGradientLUT* ensure_lut64(BLGradientPrivateImpl* impl, uint32_t lut_size) noexcept {
  BLGradientLUT* lut = impl->lut64;
  if (lut) {
    BL_ASSERT(lut->size == lut_size);
    return lut;
  }

  lut = BLGradientLUT::alloc(lut_size, 8);
  if (BL_UNLIKELY(!lut))
    return nullptr;

  const BLGradientStop* stops = impl->stops;
  PixelOps::funcs.interpolate_prgb64(lut->data<uint64_t>(), lut_size, stops, impl->size);

  // We must drop this LUT if another thread created it meanwhile.
  BLGradientLUT* expected = nullptr;
  if (!bl_atomic_compare_exchange(&impl->lut64, &expected, lut)) {
    BL_ASSERT(expected != nullptr);
    BLGradientLUT::destroy(lut);
    lut = expected;
  }

  return lut;
}

// bl::Gradient - Internals - Alloc & Free Impl
// ============================================

static BLResult alloc_impl(BLGradientCore* self, BLObjectImplSize impl_size, const void* values, size_t value_count, const BLMatrix2D* transform) noexcept {
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_GRADIENT);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLGradientPrivateImpl>(self, info, impl_size));

  BLGradientPrivateImpl* impl = get_impl(self);
  impl->stops = PtrOps::offset<BLGradientStop>(impl, sizeof(BLGradientPrivateImpl));
  impl->size = 0;
  impl->capacity = capacity_from_impl_size(impl_size);
  impl->transform = *transform;
  init_values(impl->values, static_cast<const double*>(values), value_count);
  impl->lut32 = nullptr;
  impl->lut64 = nullptr;
  impl->info32.packed = 0;
  return BL_SUCCESS;
}

BLResult free_impl(BLGradientPrivateImpl* impl) noexcept {
  invalidate_lut_cache(impl);
  return ObjectInternal::free_impl(impl);
}

// bl::Gradient - Internals - Deep Copy & Mutation
// ===============================================

static BL_NOINLINE BLResult deep_copy(BLGradientCore* self, const BLGradientCore* other, bool copy_cache) noexcept {
  uint32_t fields = other->_d.fields();
  const BLGradientPrivateImpl* other_impl = get_impl(other);

  BLGradientCore newO;
  BL_PROPAGATE(alloc_impl(&newO, impl_size_from_capacity(other_impl->capacity), other_impl->values, value_count_table[get_gradient_type(other)], &other_impl->transform));

  newO._d.info.set_fields(fields);
  BLGradientPrivateImpl* new_impl = get_impl(&newO);
  new_impl->size = copy_stops(new_impl->stops, other_impl->stops, other_impl->size);

  if (copy_cache) {
    new_impl->lut32 = copyMaybeNullLUT(other_impl->lut32);
    new_impl->lut64 = copyMaybeNullLUT(other_impl->lut64);
    new_impl->info32.packed = other_impl->info32.packed;
  }

  return replace_instance(self, &newO);
}

static BL_INLINE BLResult make_mutable(BLGradientCore* self, bool copy_cache) noexcept {
  // NOTE: `copy_cache` should be a constant so its handling should have zero cost.
  if (!is_impl_mutable(get_impl(self)))
    return deep_copy(self, self, copy_cache);

  if (!copy_cache)
    return invalidate_lut_cache(get_impl(self));

  return BL_SUCCESS;
}

} // {GradientInternal}
} // {bl}

// bl::Gradient - API - Init & Destroy
// ===================================

BL_API_IMPL BLResult bl_gradient_init(BLGradientCore* self) noexcept {
  using namespace bl::GradientInternal;

  self->_d = bl_object_defaults[BL_OBJECT_TYPE_GRADIENT]._d;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_gradient_init_move(BLGradientCore* self, BLGradientCore* other) noexcept {
  using namespace bl::GradientInternal;

  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_gradient());

  self->_d = other->_d;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_GRADIENT]._d;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_gradient_init_weak(BLGradientCore* self, const BLGradientCore* other) noexcept {
  using namespace bl::GradientInternal;

  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_gradient());

  self->_d = other->_d;
  return retain_instance(self);
}

BL_API_IMPL BLResult bl_gradient_init_as(BLGradientCore* self, BLGradientType type, const void* values, BLExtendMode extend_mode, const BLGradientStop* stops, size_t n, const BLMatrix2D* transform) noexcept {
  using namespace bl::GradientInternal;

  self->_d = bl_object_defaults[BL_OBJECT_TYPE_GRADIENT]._d;
  return bl_gradient_create(self, type, values, extend_mode, stops, n, transform);
}

BL_API_IMPL BLResult bl_gradient_destroy(BLGradientCore* self) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  return release_instance(self);
}

// bl::Gradient - API - Reset
// ==========================

BL_API_IMPL BLResult bl_gradient_reset(BLGradientCore* self) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  return replace_instance(self, static_cast<BLGradientCore*>(&bl_object_defaults[BL_OBJECT_TYPE_GRADIENT]));
}

// bl::Gradient - API - Assign
// ===========================

BL_API_IMPL BLResult bl_gradient_assign_move(BLGradientCore* self, BLGradientCore* other) noexcept {
  using namespace bl::GradientInternal;

  BL_ASSERT(self->_d.is_gradient());
  BL_ASSERT(other->_d.is_gradient());

  BLGradientCore tmp = *other;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_GRADIENT]._d;
  return replace_instance(self, &tmp);
}

BL_API_IMPL BLResult bl_gradient_assign_weak(BLGradientCore* self, const BLGradientCore* other) noexcept {
  using namespace bl::GradientInternal;

  BL_ASSERT(self->_d.is_gradient());
  BL_ASSERT(other->_d.is_gradient());

  retain_instance(other);
  return replace_instance(self, other);
}

BL_API_IMPL BLResult bl_gradient_create(BLGradientCore* self, BLGradientType type, const void* values, BLExtendMode extend_mode, const BLGradientStop* stops, size_t n, const BLMatrix2D* transform) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  if (BL_UNLIKELY((uint32_t(type) > BL_GRADIENT_TYPE_MAX_VALUE) | (uint32_t(extend_mode) > BL_EXTEND_MODE_SIMPLE_MAX_VALUE)))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  if (!values)
    values = no_values;

  BLTransformType transform_type = BL_TRANSFORM_TYPE_IDENTITY;
  if (!transform)
    transform = &no_matrix;
  else
    transform_type = transform->type();

  uint32_t analysis = BL_DATA_ANALYSIS_CONFORMING;
  if (n) {
    if (BL_UNLIKELY(stops == nullptr))
      return bl_make_error(BL_ERROR_INVALID_VALUE);

    analysis = analyze_stop_array(stops, n);
    if (BL_UNLIKELY(analysis >= BL_DATA_ANALYSIS_INVALID_VALUE))
      return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  BLGradientPrivateImpl* self_impl = get_impl(self);
  size_t immutable_msk = bl::IntOps::bool_as_mask<size_t>(!is_impl_mutable(self_impl));

  if ((n | immutable_msk) > self_impl->capacity) {
    BLObjectImplSize impl_size = bl_max(impl_size_from_capacity(n), BLObjectImplSize(kInitialImplSize));

    BLGradientCore newO;
    BL_PROPAGATE(alloc_impl(&newO, impl_size, values, value_count_table[type], transform));

    BLGradientPrivateImpl* new_impl = get_impl(&newO);
    newO._d.info.bits |= pack_abcp(type, extend_mode, transform_type);
    new_impl->size = copy_unsafe_stops(new_impl->stops, stops, n, analysis);
    return replace_instance(self, &newO);
  }
  else {
    self->_d.info.set_fields(pack_abcp(type, extend_mode, transform_type));
    init_values(self_impl->values, static_cast<const double*>(values), value_count_table[type]);
    self_impl->size = copy_unsafe_stops(self_impl->stops, stops, n, analysis);
    self_impl->transform.reset(*transform);

    return invalidate_lut_cache(self_impl);
  }
}

// bl::Gradient - API - Storage
// ============================

BL_API_IMPL BLResult bl_gradient_shrink(BLGradientCore* self) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  BLGradientPrivateImpl* self_impl = get_impl(self);
  BLObjectImplSize current_size = impl_size_from_capacity(self_impl->capacity);
  BLObjectImplSize fitting_size = impl_size_from_capacity(self_impl->size);

  if (current_size - fitting_size < BL_OBJECT_IMPL_ALIGNMENT)
    return BL_SUCCESS;

  BLGradientCore newO;
  BL_PROPAGATE(alloc_impl(&newO, fitting_size, self_impl->values, BL_GRADIENT_VALUE_MAX_VALUE + 1u, &self_impl->transform));

  BLGradientPrivateImpl* new_impl = get_impl(&newO);
  newO._d.info.set_fields(self->_d.fields());
  new_impl->size = copy_stops(new_impl->stops, self_impl->stops, self_impl->size);
  new_impl->lut32 = copyMaybeNullLUT(self_impl->lut32);
  new_impl->lut64 = copyMaybeNullLUT(self_impl->lut64);

  return replace_instance(self, &newO);
}

BL_API_IMPL BLResult bl_gradient_reserve(BLGradientCore* self, size_t n) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  BLGradientPrivateImpl* self_impl = get_impl(self);
  size_t immutable_msk = bl::IntOps::bool_as_mask<size_t>(!is_impl_mutable(self_impl));

  if ((n | immutable_msk) <= self_impl->capacity)
    return BL_SUCCESS;

  BLGradientCore newO;

  BLObjectImplSize impl_size = bl_max(impl_size_from_capacity(n), BLObjectImplSize(kInitialImplSize));
  BL_PROPAGATE(alloc_impl(&newO, impl_size, self_impl->values, BL_GRADIENT_VALUE_MAX_VALUE + 1u, &self_impl->transform));

  BLGradientPrivateImpl* new_impl = get_impl(&newO);
  newO._d.info.set_fields(self->_d.fields());
  new_impl->size = copy_stops(new_impl->stops, self_impl->stops, self_impl->size);
  new_impl->lut32 = copyMaybeNullLUT(self_impl->lut32);
  new_impl->lut64 = copyMaybeNullLUT(self_impl->lut64);

  return replace_instance(self, &newO);
}

// bl::Gradient - API - Accessors
// ==============================

BL_API_IMPL BLGradientType bl_gradient_get_type(const BLGradientCore* self) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  return get_gradient_type(self);
}

BL_API_IMPL BLResult bl_gradient_set_type(BLGradientCore* self, BLGradientType type) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  if (BL_UNLIKELY(uint32_t(type) > BL_GRADIENT_TYPE_MAX_VALUE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  set_gradient_type(self, type);
  return BL_SUCCESS;
}

BL_API_IMPL BLExtendMode bl_gradient_get_extend_mode(const BLGradientCore* self) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  return get_extend_mode(self);
}

BL_API_IMPL BLResult bl_gradient_set_extend_mode(BLGradientCore* self, BLExtendMode extend_mode) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  if (BL_UNLIKELY(extend_mode > BL_EXTEND_MODE_SIMPLE_MAX_VALUE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  set_extend_mode(self, extend_mode);
  return BL_SUCCESS;
}

BL_API_IMPL double bl_gradient_get_value(const BLGradientCore* self, size_t index) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  if (BL_UNLIKELY(index > BL_GRADIENT_VALUE_MAX_VALUE))
    return bl::Math::nan<double>();

  const BLGradientPrivateImpl* self_impl = get_impl(self);
  return self_impl->values[index];
}

BL_API_IMPL BLResult bl_gradient_set_value(BLGradientCore* self, size_t index, double value) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  if (BL_UNLIKELY(index > BL_GRADIENT_VALUE_MAX_VALUE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  BL_PROPAGATE(make_mutable(self, true));

  BLGradientPrivateImpl* self_impl = get_impl(self);
  self_impl->values[index] = value;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_gradient_set_values(BLGradientCore* self, size_t index, const double* values, size_t value_count) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  if (BL_UNLIKELY(index > BL_GRADIENT_VALUE_MAX_VALUE || value_count > BL_GRADIENT_VALUE_MAX_VALUE + 1 - index))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  if (BL_UNLIKELY(!value_count))
    return BL_SUCCESS;

  BL_PROPAGATE(make_mutable(self, true));

  BLGradientPrivateImpl* self_impl = get_impl(self);
  double* dst = self_impl->values + index;

  for (size_t i = 0; i < value_count; i++)
    dst[i] = values[i];

  return BL_SUCCESS;
}

// bl::Gradient - API - Stops
// ==========================

BL_API_IMPL size_t bl_gradient_get_size(const BLGradientCore* self) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  return get_size(self);
}

BL_API_IMPL size_t bl_gradient_get_capacity(const BLGradientCore* self) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  return get_capacity(self);
}

BL_API_IMPL const BLGradientStop* bl_gradient_get_stops(const BLGradientCore* self) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  return get_stops(self);
}

BL_API_IMPL BLResult bl_gradient_reset_stops(BLGradientCore* self) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  if (!get_size(self))
    return BL_SUCCESS;

  BLGradientPrivateImpl* self_impl = get_impl(self);
  if (!is_impl_mutable(self_impl)) {
    BLGradientCore newO;

    BLObjectImplSize impl_size = BLObjectImplSize(kInitialImplSize);
    BL_PROPAGATE(alloc_impl(&newO, impl_size, self_impl->values, BL_GRADIENT_VALUE_MAX_VALUE + 1u, &self_impl->transform));

    newO._d.info.set_fields(self->_d.fields());
    return replace_instance(self, &newO);
  }
  else {
    self_impl->size = 0;
    return invalidate_lut_cache(self_impl);
  }
}

BL_API_IMPL BLResult bl_gradient_assign_stops(BLGradientCore* self, const BLGradientStop* stops, size_t n) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  if (n == 0)
    return bl_gradient_reset_stops(self);

  BLGradientPrivateImpl* self_impl = get_impl(self);
  size_t immutable_msk = bl::IntOps::bool_as_mask<size_t>(!is_impl_mutable(self_impl));
  uint32_t analysis = analyze_stop_array(stops, n);

  if (BL_UNLIKELY(analysis >= BL_DATA_ANALYSIS_INVALID_VALUE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  if ((n | immutable_msk) > self_impl->capacity) {
    BLGradientCore newO;

    BLObjectImplSize impl_size = bl_max(impl_size_from_capacity(n), BLObjectImplSize(kInitialImplSize));
    BL_PROPAGATE(alloc_impl(&newO, impl_size, self_impl->values, BL_GRADIENT_VALUE_MAX_VALUE + 1u, &self_impl->transform));

    BLGradientPrivateImpl* new_impl = get_impl(&newO);
    newO._d.info.set_fields(self->_d.fields());
    new_impl->size = copy_unsafe_stops(new_impl->stops, stops, n, analysis);
    return replace_instance(self, &newO);
  }
  else {
    self_impl->size = copy_unsafe_stops(self_impl->stops, stops, n, analysis);
    return invalidate_lut_cache(self_impl);
  }
}

BL_API_IMPL BLResult bl_gradient_add_stop_rgba32(BLGradientCore* self, double offset, uint32_t rgba32) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  return bl_gradient_add_stop_rgba64(self, offset, bl::RgbaInternal::rgba64FromRgba32(rgba32));
}

BL_API_IMPL BLResult bl_gradient_add_stop_rgba64(BLGradientCore* self, double offset, uint64_t rgba64) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  if (BL_UNLIKELY(!(offset >= 0.0 && offset <= 1.0)))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  BLGradientPrivateImpl* self_impl = get_impl(self);
  BLGradientStop* stops = self_impl->stops;

  size_t i = 0;
  size_t n = self_impl->size;

  if (n && offset >= stops[0].offset) {
    i = bl::binary_search_closest_last(stops, n, GradientStopMatcher(offset));

    // If there are two stops that have the same offset then we would replace the second one. This is supported
    // and it would make a sharp transition.
    if (i > 0 && stops[i - 1].offset == offset)
      return bl_gradient_replace_stop_rgba64(self, i, offset, rgba64);

    // Insert a new stop after `i`.
    i++;
  }

  // If we are here it means that we are going to insert a stop at `i`. All other cases were handled at this point
  // so focus on generic insert, which could be just a special case of append operation, but we don't really care.
  size_t immutable_msk = bl::IntOps::bool_as_mask<size_t>(!is_impl_mutable(self_impl));

  if ((n | immutable_msk) >= self_impl->capacity) {
    BLGradientCore newO;

    BLObjectImplSize impl_size = bl_object_expand_impl_size(impl_size_from_capacity(n + 1));
    BL_PROPAGATE(alloc_impl(&newO, impl_size, self_impl->values, BL_GRADIENT_VALUE_MAX_VALUE + 1u, &self_impl->transform));

    BLGradientPrivateImpl* new_impl = get_impl(&newO);
    newO._d.info.set_fields(self->_d.fields());

    BLGradientStop* new_stops = new_impl->stops;
    copy_stops(new_stops, stops, i);

    new_stops[i].reset(offset, BLRgba64(rgba64));
    copy_stops(new_stops + i + 1, stops + i, n - i);

    new_impl->size = n + 1;
    return replace_instance(self, &newO);
  }
  else {
    move_stops(stops + i + 1, stops + i, n - i);
    stops[i].reset(offset, BLRgba64(rgba64));

    self_impl->size = n + 1;
    return invalidate_lut_cache(self_impl);
  }
}

BL_API_IMPL BLResult bl_gradient_remove_stop(BLGradientCore* self, size_t index) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  return bl_gradient_remove_stops_by_index(self, index, index + 1);
}

BL_API_IMPL BLResult bl_gradient_remove_stop_by_offset(BLGradientCore* self, double offset, uint32_t all) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  if (BL_UNLIKELY(!(offset >= 0.0 && offset <= 1.0)))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  size_t size = get_size(self);
  const BLGradientStop* stops = get_stops(self);

  for (size_t a = 0; a < size; a++) {
    if (stops[a].offset > offset)
      break;

    if (stops[a].offset == offset) {
      size_t b = a + 1;

      if (all) {
        while (b < size) {
          if (stops[b].offset != offset)
            break;
          b++;
        }
      }
      return bl_gradient_remove_stops_by_index(self, a, b);
    }
  }

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_gradient_remove_stops_by_index(BLGradientCore* self, size_t r_start, size_t r_end) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  size_t size = get_size(self);

  size_t index = r_start;
  size_t end = bl_min(r_end, size);

  if (BL_UNLIKELY(index > size || end < index))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  if (BL_UNLIKELY(index == end))
    return BL_SUCCESS;

  BLGradientPrivateImpl* self_impl = get_impl(self);
  BLGradientStop* stops = self_impl->stops;

  size_t removed_count = end - index;
  size_t shifted_count = size - end;
  size_t after_count = size - removed_count;

  if (!is_impl_mutable(self_impl)) {
    BLGradientCore newO;
    BL_PROPAGATE(alloc_impl(&newO, impl_size_from_capacity(after_count), self_impl->values, BL_GRADIENT_VALUE_MAX_VALUE + 1u, &self_impl->transform));

    BLGradientPrivateImpl* new_impl = get_impl(&newO);
    newO._d.info.set_fields(self->_d.fields());

    BLGradientStop* new_stops = new_impl->stops;
    copy_stops(new_stops, stops, index);
    copy_stops(new_stops + index, stops + end, shifted_count);
    return replace_instance(self, &newO);
  }
  else {
    move_stops(stops + index, stops + end, shifted_count);
    self_impl->size = after_count;
    return invalidate_lut_cache(self_impl);
  }
}

BL_API_IMPL BLResult bl_gradient_remove_stops_by_offset(BLGradientCore* self, double offset_min, double offset_max) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  if (BL_UNLIKELY(offset_max < offset_min))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  if (!get_size(self))
    return BL_SUCCESS;

  BLGradientPrivateImpl* self_impl = get_impl(self);
  const BLGradientStop* stops = self_impl->stops;

  size_t size = self_impl->size;
  size_t a, b;

  for (a = 0; a < size && stops[a].offset <  offset_min; a++) continue;
  for (b = a; b < size && stops[b].offset <= offset_max; b++) continue;

  if (a >= b)
    return BL_SUCCESS;

  return bl_gradient_remove_stops_by_index(self, a, b);
}

BL_API_IMPL BLResult bl_gradient_replace_stop_rgba32(BLGradientCore* self, size_t index, double offset, uint32_t rgba32) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  return bl_gradient_replace_stop_rgba64(self, index, offset, bl::RgbaInternal::rgba64FromRgba32(rgba32));
}

BL_API_IMPL BLResult bl_gradient_replace_stop_rgba64(BLGradientCore* self, size_t index, double offset, uint64_t rgba64) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  if (BL_UNLIKELY(index >= get_size(self)))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  BL_PROPAGATE(make_mutable(self, false));

  BLGradientPrivateImpl* self_impl = get_impl(self);
  BLGradientStop* stops = self_impl->stops;

  if (stops[index].offset == offset) {
    stops[index].rgba.value = rgba64;
    return BL_SUCCESS;
  }
  else {
    BL_PROPAGATE(bl_gradient_remove_stop(self, index));
    return bl_gradient_add_stop_rgba64(self, offset, rgba64);
  }
}

BL_API_IMPL size_t bl_gradient_index_of_stop(const BLGradientCore* self, double offset) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  const BLGradientPrivateImpl* self_impl = get_impl(self);
  const BLGradientStop* stops = self_impl->stops;

  size_t n = self_impl->size;
  if (!n)
    return SIZE_MAX;

  size_t i = bl::binary_search(stops, n, GradientStopMatcher(offset));
  if (i == SIZE_MAX)
    return SIZE_MAX;

  if (i > 0 && stops[i - 1].offset == offset)
    i--;

  return i;
}

// bl::Gradient - API - Transform
// ==============================

BL_API_IMPL BLResult bl_gradient_get_transform(const BLGradientCore* self, BLMatrix2D* transform_out) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  *transform_out = get_impl(self)->transform;
  return BL_SUCCESS;
}

BL_API_IMPL BLTransformType bl_gradient_get_transform_type(const BLGradientCore* self) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  return get_transform_type(self);
}

BL_API_IMPL BLResult bl_gradient_apply_transform_op(BLGradientCore* self, BLTransformOp op_type, const void* op_data) noexcept {
  using namespace bl::GradientInternal;
  BL_ASSERT(self->_d.is_gradient());

  if (BL_UNLIKELY(uint32_t(op_type) > BL_TRANSFORM_OP_MAX_VALUE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  if (op_type == BL_TRANSFORM_OP_RESET && get_transform_type(self) == BL_TRANSFORM_TYPE_IDENTITY)
    return BL_SUCCESS;

  BL_PROPAGATE(make_mutable(self, true));
  BLGradientPrivateImpl* self_impl = get_impl(self);

  bl_matrix2d_apply_op(&self_impl->transform, op_type, op_data);
  set_transform_type(self, self_impl->transform.type());

  return BL_SUCCESS;
}

// bl::Gradient - API - Equals
// ===========================

BL_API_IMPL bool bl_gradient_equals(const BLGradientCore* a, const BLGradientCore* b) noexcept {
  using namespace bl::GradientInternal;

  BL_ASSERT(a->_d.is_gradient());
  BL_ASSERT(b->_d.is_gradient());

  const BLGradientPrivateImpl* a_impl = get_impl(a);
  const BLGradientPrivateImpl* b_impl = get_impl(b);

  if (a->_d.info != b->_d.info)
    return false;

  if (a_impl == b_impl)
    return true;

  size_t size = a_impl->size;
  unsigned eq = unsigned(a_impl->transform == b_impl->transform) & unsigned(size == b_impl->size);
  return eq && memcmp(a_impl->stops, b_impl->stops, size * sizeof(BLGradientStop)) == 0;
}

// bl::Gradient - Runtime Registration
// ===================================

void bl_gradient_rt_init(BLRuntimeContext* rt) noexcept {
  bl_unused(rt);

  bl::GradientInternal::default_impl.impl->transform.reset();

  bl_object_defaults[BL_OBJECT_TYPE_GRADIENT]._d.init_dynamic(
    BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_GRADIENT),
    &bl::GradientInternal::default_impl.impl);
}
