// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/array_p.h>
#include <blend2d/core/matrix_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/path_p.h>
#include <blend2d/core/pathstroke_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/geometry/bezier_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/lookuptable_p.h>
#include <blend2d/support/math_p.h>
#include <blend2d/support/ptrops_p.h>
#include <blend2d/support/traits_p.h>

const BLApproximationOptions bl_default_approximation_options = bl::PathInternal::make_default_approximation_options();

// bl::Path - Globals
// ==================

namespace bl {
namespace PathInternal {

static BLObjectEternalImpl<BLPathPrivateImpl> default_path;

static BLResult append_transformed_path_with_type(BLPathCore* self, const BLPathCore* other, const BLRange* range, const BLMatrix2D* transform, uint32_t transform_type) noexcept;
static BLResult transform_with_type(BLPathCore* self, const BLRange* range, const BLMatrix2D* transform, uint32_t transform_type) noexcept;

// bl::Path - Utilities
// ====================

static BL_INLINE bool check_range(const BLPathPrivateImpl* path_impl, const BLRange* range, size_t* start_out, size_t* n_out) noexcept {
  size_t start = 0;
  size_t end = path_impl->size;

  if (range) {
    start = range->start;
    end = bl_min(end, range->end);
  }

  *start_out = start;
  *n_out = end - start;
  return start < end;
}

static BL_INLINE void copy_content(uint8_t* cmd_dst, BLPoint* vtx_dst, const uint8_t* cmd_src, const BLPoint* vtx_src, size_t n) noexcept {
  for (size_t i = 0; i < n; i++) {
    cmd_dst[i] = cmd_src[i];
    vtx_dst[i] = vtx_src[i];
  }
}

// bl::Path - Internals
// ====================

static BL_INLINE BLObjectImplSize expand_impl_size(BLObjectImplSize impl_size) noexcept {
  constexpr size_t kMinimumImplSize = 1024;
  constexpr size_t kMinimumImplMask = kMinimumImplSize - 16;

  return bl_object_expand_impl_size(BLObjectImplSize(impl_size.value() | kMinimumImplMask));
}

static BLObjectImplSize expand_impl_size_with_modify_op(BLObjectImplSize impl_size, BLModifyOp modify_op) noexcept {
  if (bl_modify_op_does_grow(modify_op))
    return expand_impl_size(impl_size);
  else
    return impl_size;
}

static BL_INLINE size_t get_size(const BLPathCore* self) noexcept {
  return get_impl(self)->size;
}

static BL_INLINE void set_size(BLPathCore* self, size_t size) noexcept {
  get_impl(self)->size = size;
}

static BL_INLINE BLResult alloc_impl(BLPathCore* self, size_t size, BLObjectImplSize impl_size) noexcept {
  size_t capacity = capacity_from_impl_size(impl_size);

  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_PATH);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLPathPrivateImpl>(self, info, impl_size));

  BLPathPrivateImpl* impl = get_impl(self);
  BLPoint* vertex_data = PtrOps::offset<BLPoint>(impl, sizeof(BLPathPrivateImpl));
  uint8_t* command_data = PtrOps::offset<uint8_t>(vertex_data, capacity * sizeof(BLPoint));

  impl->command_data = command_data;
  impl->vertex_data = vertex_data;
  impl->size = size;
  impl->capacity = capacity;
  impl->flags = BL_PATH_FLAG_DIRTY;
  return BL_SUCCESS;
}

// Plain realloc - allocates a new path, copies its data into it, and replaces the
// impl in `self`. Flags and cached information are cleared.
static BL_NOINLINE BLResult realloc_path(BLPathCore* self, BLObjectImplSize impl_size) noexcept {
  BLPathPrivateImpl* old_impl = get_impl(self);
  size_t path_size = old_impl->size;

  BLPathCore newO;
  BL_PROPAGATE(alloc_impl(&newO, path_size, impl_size));

  BLPathPrivateImpl* new_impl = get_impl(&newO);
  copy_content(new_impl->command_data, new_impl->vertex_data, old_impl->command_data, old_impl->vertex_data, path_size);
  return replace_instance(self, &newO);
}

// Called by `prepare_add` and some others to create a new path, copy a content from `self` into it, and release
// the current impl. The size of the new path will be set to `new_size` so this function should really be only used
// as an append fallback.
static BL_NOINLINE BLResult realloc_path_to_add(BLPathCore* self, size_t new_size, uint8_t** cmd_out, BLPoint** vtx_out) noexcept {
  BLObjectImplSize impl_size = expand_impl_size(impl_size_from_capacity(new_size));

  BLPathCore newO;
  BL_PROPAGATE(alloc_impl(&newO, new_size, impl_size));

  BLPathPrivateImpl* old_impl = get_impl(self);
  BLPathPrivateImpl* new_impl = get_impl(&newO);

  size_t old_size = old_impl->size;
  copy_content(new_impl->command_data, new_impl->vertex_data, old_impl->command_data, old_impl->vertex_data, old_size);

  *cmd_out = new_impl->command_data + old_size;
  *vtx_out = new_impl->vertex_data + old_size;
  return replace_instance(self, &newO);
}

// Called when adding something to the path. The `n` parameter is always considered safe as it would be
// impossible that a path length would go to half `size_t`. The memory required by each vertex is either:
//
//   -  5 bytes (2*i16 + 1 command byte)
//   -  9 bytes (2*f32 + 1 command byte)
//   - 17 bytes (2*f64 + 1 command byte)
//
// This means that a theoretical maximum size of a path without considering its Impl header would be:
//
//   `SIZE_MAX / (sizeof(vertex) + sizeof(uint8_t))`
//
// which would be always smaller than SIZE_MAX / 2 so we can assume that apending two paths would never overflow
// the maximum theoretical Path capacity represented by `size_t` type.
static BL_INLINE BLResult prepare_add(BLPathCore* self, size_t n, uint8_t** cmd_out, BLPoint** vtx_out) noexcept {
  BLPathPrivateImpl* self_impl = get_impl(self);

  size_t size = self_impl->size;
  size_t size_after = size + n;
  size_t immutable_msk = IntOps::bool_as_mask<size_t>(!is_impl_mutable(self_impl));

  if ((size_after | immutable_msk) > self_impl->capacity)
    return realloc_path_to_add(self, size_after, cmd_out, vtx_out);

  // Likely case, appending to a path that is not shared and has the required capacity. We have to clear FLAGS
  // in addition to set the new size as flags can contain bits regarding BLPathInfo that will no longer hold.
  self_impl->flags = BL_PATH_FLAG_DIRTY;
  self_impl->size = size_after;

  *cmd_out = self_impl->command_data + size;
  *vtx_out = self_impl->vertex_data + size;

  return BL_SUCCESS;
}

static BL_INLINE BLResult make_mutable(BLPathCore* self) noexcept {
  BLPathPrivateImpl* self_impl = get_impl(self);

  if (!is_impl_mutable(self_impl)) {
    BL_PROPAGATE(realloc_path(self, impl_size_from_capacity(self_impl->size)));
    self_impl = get_impl(self);
  }

  self_impl->flags = BL_PATH_FLAG_DIRTY;
  return BL_SUCCESS;
}

} // {PathInternal}
} // {bl}

// bl::StrokeOptions - API - Init & Destroy
// ========================================

BL_API_IMPL BLResult bl_stroke_options_init(BLStrokeOptionsCore* self) noexcept {
  self->hints = 0;
  self->width = 1.0;
  self->miter_limit = 4.0;
  self->dash_offset = 0;
  bl_call_ctor(self->dash_array);

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_stroke_options_init_move(BLStrokeOptionsCore* self, BLStrokeOptionsCore* other) noexcept {
  BL_ASSERT(self != other);

  self->hints = other->hints;
  self->width = other->width;
  self->miter_limit = other->miter_limit;
  self->dash_offset = other->dash_offset;
  return bl_object_private_init_move_tagged(&self->dash_array, &other->dash_array);
}

BL_API_IMPL BLResult bl_stroke_options_init_weak(BLStrokeOptionsCore* self, const BLStrokeOptionsCore* other) noexcept {
  self->hints = other->hints;
  self->width = other->width;
  self->miter_limit = other->miter_limit;
  self->dash_offset = other->dash_offset;
  self->dash_array._d = other->dash_array._d;
  return bl::ArrayInternal::retain_instance(&self->dash_array);
}

BL_API_IMPL BLResult bl_stroke_options_destroy(BLStrokeOptionsCore* self) noexcept {
  return bl::ArrayInternal::release_instance(&self->dash_array);
}

// bl::StrokeOptions - API - Reset
// ===============================

BL_API_IMPL BLResult bl_stroke_options_reset(BLStrokeOptionsCore* self) noexcept {
  self->hints = 0;
  self->width = 1.0;
  self->miter_limit = 4.0;
  self->dash_offset = 0;
  self->dash_array.reset();

  return BL_SUCCESS;
}

// bl::StrokeOptions - API - Assign
// ================================

BL_API_IMPL BLResult bl_stroke_options_assign_move(BLStrokeOptionsCore* self, BLStrokeOptionsCore* other) noexcept {
  self->hints = other->hints;
  self->width = other->width;
  self->miter_limit = other->miter_limit;
  self->dash_offset = other->dash_offset;
  self->dash_array = BLInternal::move(other->dash_array);

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_stroke_options_assign_weak(BLStrokeOptionsCore* self, const BLStrokeOptionsCore* other) noexcept {
  self->hints = other->hints;
  self->width = other->width;
  self->miter_limit = other->miter_limit;
  self->dash_offset = other->dash_offset;
  self->dash_array = other->dash_array;

  return BL_SUCCESS;
}

// bl::StrokeOptions - API - Equality & Comparison
// ===============================================

BL_API_IMPL bool bl_stroke_options_equals(const BLStrokeOptionsCore* a, const BLStrokeOptionsCore* b) noexcept {
  if (unsigned(a->hints == b->hints) &
      unsigned(a->width == b->width) &
      unsigned(a->miter_limit == b->miter_limit) &
      unsigned(a->dash_offset == b->dash_offset)) {
    return a->dash_array.equals(b->dash_array);
  }

  return false;
}

// bl::Path - API - Init & Destroy
// ===============================

BL_API_IMPL BLResult bl_path_init(BLPathCore* self) noexcept {
  self->_d = bl_object_defaults[BL_OBJECT_TYPE_PATH]._d;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_path_init_move(BLPathCore* self, BLPathCore* other) noexcept {
  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_path());

  self->_d = other->_d;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_PATH]._d;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_path_init_weak(BLPathCore* self, const BLPathCore* other) noexcept {
  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_path());

  return bl_object_private_init_weak_tagged(self, other);
}

BL_API_IMPL BLResult bl_path_destroy(BLPathCore* self) noexcept {
  using namespace bl::PathInternal;

  BL_ASSERT(self->_d.is_path());
  return release_instance(self);
}

// bl::Path - API - Reset
// ======================

BL_API_IMPL BLResult bl_path_reset(BLPathCore* self) noexcept {
  using namespace bl::PathInternal;

  BL_ASSERT(self->_d.is_path());
  return replace_instance(self, static_cast<BLPathCore*>(&bl_object_defaults[BL_OBJECT_TYPE_PATH]));
}

// bl::Path - API - Accessors
// ==========================

BL_API_IMPL size_t bl_path_get_size(const BLPathCore* self) BL_NOEXCEPT_C {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  return self_impl->size;
}

BL_API_IMPL size_t bl_path_get_capacity(const BLPathCore* self) BL_NOEXCEPT_C {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  return self_impl->capacity;
}

BL_API_IMPL const uint8_t* bl_path_get_command_data(const BLPathCore* self) BL_NOEXCEPT_C {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  return self_impl->command_data;
}

BL_API_IMPL const BLPoint* bl_path_get_vertex_data(const BLPathCore* self) BL_NOEXCEPT_C {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  return self_impl->vertex_data;
}

BL_API_IMPL BLResult bl_path_clear(BLPathCore* self) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  if (!is_impl_mutable(self_impl))
    return replace_instance(self, static_cast<BLPathCore*>(&bl_object_defaults[BL_OBJECT_TYPE_PATH]));

  self_impl->size = 0;
  self_impl->flags = 0;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_path_shrink(BLPathCore* self) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  size_t size = self_impl->size;
  size_t capacity = self_impl->capacity;

  if (!size)
    return replace_instance(self, static_cast<BLPathCore*>(&bl_object_defaults[BL_OBJECT_TYPE_PATH]));

  BLObjectImplSize fitting_impl_size = impl_size_from_capacity(size);
  BLObjectImplSize current_impl_size = impl_size_from_capacity(capacity);

  if (current_impl_size - fitting_impl_size >= BL_OBJECT_IMPL_ALIGNMENT)
    BL_PROPAGATE(realloc_path(self, fitting_impl_size));

  // Update path info as this this path may be kept alive for some time.
  uint32_t dummy_flags;
  return bl_path_get_info_flags(self, &dummy_flags);
}

BL_API_IMPL BLResult bl_path_reserve(BLPathCore* self, size_t n) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  size_t immutable_msk = bl::IntOps::bool_as_mask<size_t>(!is_impl_mutable(self_impl));

  if ((n | immutable_msk) > self_impl->capacity)
    return realloc_path(self, impl_size_from_capacity(bl_max(n, self_impl->size)));

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_path_modify_op(BLPathCore* self, BLModifyOp op, size_t n, uint8_t** cmd_data_out, BLPoint** vtx_data_out) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  size_t index = bl_modify_op_is_append(op) ? self_impl->size : size_t(0);
  size_t immutable_msk = bl::IntOps::bool_as_mask<size_t>(!is_impl_mutable(self_impl));

  size_t remaining = self_impl->capacity - index;
  size_t size_after = index + n;

  if ((n | immutable_msk) > remaining) {
    *cmd_data_out = nullptr;
    *vtx_data_out = nullptr;

    BLPathCore newO;
    BLObjectImplSize impl_size = expand_impl_size_with_modify_op(impl_size_from_capacity(size_after), op);
    BL_PROPAGATE(alloc_impl(&newO, size_after, impl_size));

    BLPathPrivateImpl* new_impl = get_impl(&newO);
    *cmd_data_out = new_impl->command_data + index;
    *vtx_data_out = new_impl->vertex_data + index;

    copy_content(new_impl->command_data, new_impl->vertex_data, self_impl->command_data, self_impl->vertex_data, index);
    return replace_instance(self, &newO);
  }

  if (n) {
    self_impl->size = size_after;
  }
  else if (!index) {
    bl_path_clear(self);
    self_impl = get_impl(self);
  }

  self_impl->flags = BL_PATH_FLAG_DIRTY;
  *vtx_data_out = self_impl->vertex_data + index;
  *cmd_data_out = self_impl->command_data + index;

  return BL_SUCCESS;
}

// bl::Path - API - Assign
// =======================

BL_API_IMPL BLResult bl_path_assign_move(BLPathCore* self, BLPathCore* other) noexcept {
  using namespace bl::PathInternal;

  BL_ASSERT(self->_d.is_path());
  BL_ASSERT(other->_d.is_path());

  BLPathCore tmp = *other;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_PATH]._d;
  return replace_instance(self, &tmp);
}

BL_API_IMPL BLResult bl_path_assign_weak(BLPathCore* self, const BLPathCore* other) noexcept {
  using namespace bl::PathInternal;

  BL_ASSERT(self->_d.is_path());
  BL_ASSERT(other->_d.is_path());

  retain_instance(other);
  return replace_instance(self, other);
}

BL_API_IMPL BLResult bl_path_assign_deep(BLPathCore* self, const BLPathCore* other) noexcept {
  using namespace bl::PathInternal;

  BL_ASSERT(self->_d.is_path());
  BL_ASSERT(other->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  BLPathPrivateImpl* other_impl = get_impl(other);

  size_t size = other_impl->size;
  if (!size)
    return bl_path_clear(self);

  size_t immutable_msk = bl::IntOps::bool_as_mask<size_t>(!is_impl_mutable(self_impl));
  if ((size | immutable_msk) > self_impl->capacity) {
    BLPathCore newO;
    BL_PROPAGATE(alloc_impl(&newO, size, impl_size_from_capacity(size)));

    BLPathPrivateImpl* new_impl = get_impl(&newO);
    copy_content(new_impl->command_data, new_impl->vertex_data, other_impl->command_data, other_impl->vertex_data, size);
    return replace_instance(self, &newO);
  }

  self_impl->flags = BL_PATH_FLAG_DIRTY;
  self_impl->size = size;

  copy_content(self_impl->command_data, self_impl->vertex_data, other_impl->command_data, other_impl->vertex_data, size);
  return BL_SUCCESS;
}

// bl::Path - Arcs Helpers
// =======================

namespace bl {
namespace PathInternal {

static const double arc90_deg_steps_table[] = {
  Math::kPI_DIV_2,
  Math::kPI,
  Math::kPI_MUL_1p5,
  Math::kPI_MUL_2
};

static void arc_to_cubic_spline(PathAppender& dst, BLPoint c, BLPoint r, double start_angle, double sweep_angle, uint8_t initial_cmd, bool maybe_redundant_line_to = false) noexcept {
  double start_sin = Math::sin(start_angle);
  double start_cos = Math::cos(start_angle);

  BLMatrix2D transform = BLMatrix2D::make_sin_cos(start_sin, start_cos);
  transform.post_scale(r);
  transform.post_translate(c);

  if (sweep_angle < 0) {
    transform.scale(1.0, -1.0);
    sweep_angle = -sweep_angle;
  }

  BLPoint v1(1.0, 0.0);
  BLPoint vc(1.0, 1.0);
  BLPoint v2;

  if (sweep_angle >= Math::kPI_MUL_2 - Math::epsilon<double>()) {
    sweep_angle = Math::kPI_MUL_2;
    v2 = v1;
  }
  else {
    if (Math::is_nan(sweep_angle))
      return;

    double sweep_sin = Math::sin(sweep_angle);
    double sweep_cos = Math::cos(sweep_angle);
    v2 = BLPoint(sweep_cos, sweep_sin);
  }

  BLPoint p0 = transform.map_point(v1);
  dst.add_vertex(initial_cmd, p0);

  if (maybe_redundant_line_to && dst.cmd[-1].value <= BL_PATH_CMD_ON) {
    BL_ASSERT(initial_cmd == BL_PATH_CMD_ON);
    double diff = bl_max(bl_abs(p0.x - dst.vtx[-2].x), bl_abs(p0.y - dst.vtx[-2].y));

    if (diff < Math::epsilon<double>())
      dst.back(1);
  }

  size_t i = 0;
  while (sweep_angle > arc90_deg_steps_table[i]) {
    v1 = Geometry::normal(v1);
    BLPoint p1 = transform.map_point(vc);
    BLPoint p2 = transform.map_point(v1);
    dst.cubic_to(p0 + (p1 - p0) * Math::kKAPPA, p2 + (p1 - p2) * Math::kKAPPA, p2);

    // Full circle.
    if (++i == 4)
      return;

    vc = Geometry::normal(vc);
    p0 = p2;
  }

  // Calculate the remaining control point.
  vc = v1 + v2;
  vc = 2.0 * vc / Geometry::dot(vc, vc);

  // This is actually half of the remaining cos. It is required that v1 dot v2 > -1 holds
  // but we can safely assume it does (only critical for angles close to 180 degrees).
  double w = Math::sqrt(0.5 * Geometry::dot(v1, v2) + 0.5);
  dst.conic_to(transform.map_point(vc), transform.map_point(v2), w);
}

// bl::Path - Info Updater
// =======================

class PathInfoUpdater {
public:
  uint32_t move_to_count;
  uint32_t flags;
  BLBox control_box;
  BLBox bounding_box;

  BL_INLINE PathInfoUpdater() noexcept
    : move_to_count(0),
      flags(0),
      control_box(Traits::max_value<double>(), Traits::max_value<double>(), Traits::min_value<double>(), Traits::min_value<double>()),
      bounding_box(Traits::max_value<double>(), Traits::max_value<double>(), Traits::min_value<double>(), Traits::min_value<double>()) {}

  BLResult update(const BLPathView& view, uint32_t has_prev_vertex = false) noexcept {
    const uint8_t* cmd_data = view.command_data;
    const uint8_t* cmd_end = view.command_data + view.size;
    const BLPoint* vtx_data = view.vertex_data;

    // Iterate over the whole path.
    while (cmd_data != cmd_end) {
      uint32_t c = cmd_data[0];
      switch (c) {
        case BL_PATH_CMD_MOVE: {
          move_to_count++;
          has_prev_vertex = true;

          Geometry::bound(bounding_box, vtx_data[0]);

          cmd_data++;
          vtx_data++;
          break;
        }

        case BL_PATH_CMD_ON: {
          if (!has_prev_vertex)
            return bl_make_error(BL_ERROR_INVALID_GEOMETRY);

          Geometry::bound(bounding_box, vtx_data[0]);

          cmd_data++;
          vtx_data++;
          break;
        }

        case BL_PATH_CMD_QUAD: {
          cmd_data += 2;
          vtx_data += 2;

          if (cmd_data > cmd_end || !has_prev_vertex)
            return bl_make_error(BL_ERROR_INVALID_GEOMETRY);

          flags |= BL_PATH_FLAG_QUADS;
          has_prev_vertex = true;
          Geometry::bound(bounding_box, vtx_data[-1]);

          // Calculate tight bounding-box only when control points are outside the current one.
          const BLPoint& ctrl = vtx_data[-2];

          if (!(ctrl.x >= bounding_box.x0 && ctrl.y >= bounding_box.y0 && ctrl.x <= bounding_box.x1 && ctrl.y <= bounding_box.y1)) {
            BLPoint extrema = Geometry::quad_extrema_point(Geometry::quad_ref(vtx_data - 3));
            Geometry::bound(bounding_box, extrema);
            Geometry::bound(control_box, vtx_data[-2]);
          }
          break;
        }

        case BL_PATH_CMD_CONIC: {
          cmd_data += 3;
          vtx_data += 3;

          if (cmd_data > cmd_end || !has_prev_vertex)
            return bl_make_error(BL_ERROR_INVALID_GEOMETRY);

          flags |= BL_PATH_FLAG_CONICS;
          has_prev_vertex = true;
          Geometry::bound(bounding_box, vtx_data[-1]);

          // Calculate tight bounding-box only when control points are outside the current one.
          const BLPoint& ctrl = vtx_data[-3];

          if (!(ctrl.x >= bounding_box.x0 && ctrl.y >= bounding_box.y0 && ctrl.x <= bounding_box.x1 && ctrl.y <= bounding_box.y1)) {
            BLPoint extrema[2];
            Geometry::get_conic_extrema_points(vtx_data - 4, extrema);
            Geometry::bound(bounding_box, extrema[0]);
            Geometry::bound(bounding_box, extrema[1]);
            Geometry::bound(control_box, vtx_data[-2]);
          }
          break;
        }

        case BL_PATH_CMD_CUBIC: {
          cmd_data += 3;
          vtx_data += 3;

          if (cmd_data > cmd_end || !has_prev_vertex)
            return bl_make_error(BL_ERROR_INVALID_GEOMETRY);

          flags |= BL_PATH_FLAG_CUBICS;
          has_prev_vertex = true;
          Geometry::bound(bounding_box, vtx_data[-1]);

          // Calculate tight bounding-box only when control points are outside of the current one.
          BLPoint ctrl_min = bl_min(vtx_data[-3], vtx_data[-2]);
          BLPoint ctrl_max = bl_max(vtx_data[-3], vtx_data[-2]);

          if (!(ctrl_min.x >= bounding_box.x0 && ctrl_min.y >= bounding_box.y0 && ctrl_max.x <= bounding_box.x1 && ctrl_max.y <= bounding_box.y1)) {
            BLPoint extrema[2];
            Geometry::cubic_extrema_points(Geometry::cubic_ref(vtx_data - 4), extrema);
            Geometry::bound(bounding_box, extrema[0]);
            Geometry::bound(bounding_box, extrema[1]);
            Geometry::bound(control_box, vtx_data[-3]);
            Geometry::bound(control_box, vtx_data[-2]);
          }
          break;
        }

        case BL_PATH_CMD_CLOSE:
          has_prev_vertex = false;

          cmd_data++;
          vtx_data++;
          break;

        default:
          return bl_make_error(BL_ERROR_INVALID_GEOMETRY);
      }
    }

    control_box.x0 = bl_min(control_box.x0, bounding_box.x0);
    control_box.y0 = bl_min(control_box.y0, bounding_box.y0);
    control_box.x1 = bl_max(control_box.x1, bounding_box.x1);
    control_box.y1 = bl_max(control_box.y1, bounding_box.y1);

    if (move_to_count > 1)
      flags |= BL_PATH_FLAG_MULTIPLE;

    if (!Math::is_finite(control_box, bounding_box))
      return bl_make_error(BL_ERROR_INVALID_GEOMETRY);

    return BL_SUCCESS;
  }
};

// bl::Path - API - Path Construction
// ==================================

struct PathVertexCountOfGeometryTypeTableGen {
  static constexpr uint8_t value(size_t i) noexcept {
    return uint8_t(i == BL_GEOMETRY_TYPE_BOXI       ?  5 :
                   i == BL_GEOMETRY_TYPE_BOXD       ?  5 :
                   i == BL_GEOMETRY_TYPE_RECTI      ?  5 :
                   i == BL_GEOMETRY_TYPE_RECTD      ?  5 :
                   i == BL_GEOMETRY_TYPE_CIRCLE     ? 14 :
                   i == BL_GEOMETRY_TYPE_ELLIPSE    ? 14 :
                   i == BL_GEOMETRY_TYPE_ROUND_RECT ? 18 :
                   i == BL_GEOMETRY_TYPE_ARC        ? 13 :
                   i == BL_GEOMETRY_TYPE_CHORD      ? 20 :
                   i == BL_GEOMETRY_TYPE_PIE        ? 20 :
                   i == BL_GEOMETRY_TYPE_LINE       ?  2 :
                   i == BL_GEOMETRY_TYPE_TRIANGLE   ?  4 : 255);
  }
};

static constexpr const auto path_vertex_count_of_geometry_type_table =
  make_lookup_table<uint8_t, BL_GEOMETRY_TYPE_MAX_VALUE + 1, PathVertexCountOfGeometryTypeTableGen>();

static BL_INLINE BLResult append_box_internal(BLPathCore* self, double x0, double y0, double x1, double y1, BLGeometryDirection dir) noexcept {
  uint8_t* cmd_data;
  BLPoint* vtx_data;
  BL_PROPAGATE(prepare_add(self, 5, &cmd_data, &vtx_data));

  vtx_data[0].reset(x0, y0);
  vtx_data[1].reset(x1, y0);
  vtx_data[2].reset(x1, y1);
  vtx_data[3].reset(x0, y1);
  vtx_data[4].reset(Math::nan<double>(), Math::nan<double>());
  cmd_data[0] = BL_PATH_CMD_MOVE;
  cmd_data[1] = BL_PATH_CMD_ON;
  cmd_data[2] = BL_PATH_CMD_ON;
  cmd_data[3] = BL_PATH_CMD_ON;
  cmd_data[4] = BL_PATH_CMD_CLOSE;

  if (dir == BL_GEOMETRY_DIRECTION_CW)
    return BL_SUCCESS;

  vtx_data[1].reset(x0, y1);
  vtx_data[3].reset(x1, y0);
  return BL_SUCCESS;
}

// If the function succeeds then the number of vertices written to destination
// equals `n`. If the function fails you should not rely on the output data.
//
// The algorithm reverses the path, but not the implicit line assumed in case
// of CLOSE command. This means that for example a sequence like:
//
//   [0,0] [0,1] [1,0] [1,1] [CLOSE]
//
// Would be reversed to:
//
//   [1,1] [1,0] [0,1] [0,0] [CLOSE]
//
// Which is what other libraries do as well.
static BLResult copy_content_reversed(PathAppender& dst, PathIterator src, BLPathReverseMode reverse_mode) noexcept {
  for (;;) {
    PathIterator next {};
    if (reverse_mode != BL_PATH_REVERSE_MODE_COMPLETE) {
      // This mode is more complicated as we have to scan the path forward
      // and find the end of each figure so we can then go again backward.
      const uint8_t* p = src.cmd;
      if (p == src.end)
        return BL_SUCCESS;

      uint8_t cmd = p[0];
      if (cmd != BL_PATH_CMD_MOVE)
        return bl_make_error(BL_ERROR_INVALID_GEOMETRY);

      while (++p != src.end) {
        // Terminate on MOVE command, but don't consume it.
        if (p[0] == BL_PATH_CMD_MOVE)
          break;

        // Terminate on CLOSE command and consume it as it's part of the figure.
        if (p[0] == BL_PATH_CMD_CLOSE) {
          p++;
          break;
        }
      }

      size_t figure_size = (size_t)(p - src.cmd);

      next.reset(src.cmd + figure_size, src.vtx + figure_size, src.remaining_forward() - figure_size);
      src.end = src.cmd + figure_size;
    }

    src.reverse();
    while (!src.at_end()) {
      uint8_t cmd = src.cmd[0];
      src--;

      // Initial MOVE means the whole figure consists of just a single MOVE.
      if (cmd == BL_PATH_CMD_MOVE) {
        dst.add_vertex(cmd, src.vtx[1]);
        continue;
      }

      // Only relevant to non-ON commands
      bool has_close = (cmd == BL_PATH_CMD_CLOSE);
      if (cmd != BL_PATH_CMD_ON) {
        // A figure cannot end with anything else than MOVE|ON|CLOSE.
        if (!has_close)
          return bl_make_error(BL_ERROR_INVALID_GEOMETRY);

        // Make sure the next command is ON, continue otherwise.
        if (src.at_end() || src.cmd[0] != BL_PATH_CMD_ON) {
          dst.add_vertex(BL_PATH_CMD_CLOSE, src.vtx[1]);
          continue;
        }
        src--;
      }

      // Each figure starts with MOVE.
      dst.move_to(src.vtx[1]);

      // Iterate the figure.
      while (!src.at_end()) {
        cmd = src.cmd[0];
        if (cmd == BL_PATH_CMD_MOVE) {
          dst.add_vertex(BL_PATH_CMD_ON, src.vtx[0]);
          src--;
          break;
        }

        if (cmd == BL_PATH_CMD_CLOSE)
          break;

        dst.add_vertex(src.cmd[0], src.vtx[0]);
        src--;
      }

      // Emit CLOSE if the figure is closed.
      if (has_close)
        dst.close();
    }

    if (reverse_mode == BL_PATH_REVERSE_MODE_COMPLETE)
      return BL_SUCCESS;
    src = next;
  }
}

static BLResult append_transformed_path_with_type(BLPathCore* self, const BLPathCore* other, const BLRange* range, const BLMatrix2D* transform, uint32_t transform_type) noexcept {
  BL_ASSERT(self->_d.is_path());
  BL_ASSERT(other->_d.is_path());

  BLPathPrivateImpl* other_impl = get_impl(other);
  size_t start, n;

  if (!check_range(other_impl, range, &start, &n))
    return BL_SUCCESS;

  uint8_t* cmd_data;
  BLPoint* vtx_data;

  // Maybe `self` and `other` were the same, so get the `other` impl again.
  BL_PROPAGATE(prepare_add(self, n, &cmd_data, &vtx_data));
  other_impl = get_impl(other);

  memcpy(cmd_data, other_impl->command_data + start, n);
  return TransformInternal::map_pointd_array_funcs[transform_type](transform, vtx_data, other_impl->vertex_data + start, n);
}

} // {PathInternal}
} // {bl}

BL_API_IMPL BLResult bl_path_set_vertex_at(BLPathCore* self, size_t index, uint32_t cmd, double x, double y) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  size_t size = self_impl->size;

  if (BL_UNLIKELY(index >= size))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  BL_PROPAGATE(make_mutable(self));
  self_impl = get_impl(self);

  uint32_t old_cmd = self_impl->command_data[index];
  if (cmd == BL_PATH_CMD_PRESERVE) cmd = old_cmd;

  // NOTE: We don't check `cmd` as we don't care of the value. Invalid commands
  // must always be handled by all Blend2D functions anyway so let it fail at
  // some other place if the given `cmd` is invalid.
  self_impl->command_data[index] = cmd & 0xFFu;
  self_impl->vertex_data[index].reset(x, y);

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_path_move_to(BLPathCore* self, double x0, double y0) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  uint8_t* cmd_data;
  BLPoint* vtx_data;
  BL_PROPAGATE(prepare_add(self, 1, &cmd_data, &vtx_data));

  vtx_data[0].reset(x0, y0);
  cmd_data[0] = BL_PATH_CMD_MOVE;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_path_line_to(BLPathCore* self, double x1, double y1) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  uint8_t* cmd_data;
  BLPoint* vtx_data;
  BL_PROPAGATE(prepare_add(self, 1, &cmd_data, &vtx_data));

  vtx_data[0].reset(x1, y1);
  cmd_data[0] = BL_PATH_CMD_ON;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_path_poly_to(BLPathCore* self, const BLPoint* poly, size_t count) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  uint8_t* cmd_data;
  BLPoint* vtx_data;
  BL_PROPAGATE(prepare_add(self, count, &cmd_data, &vtx_data));

  for (size_t i = 0; i < count; i++) {
    vtx_data[i] = poly[i];
    cmd_data[i] = BL_PATH_CMD_ON;
  }

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_path_quad_to(BLPathCore* self, double x1, double y1, double x2, double y2) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  uint8_t* cmd_data;
  BLPoint* vtx_data;
  BL_PROPAGATE(prepare_add(self, 2, &cmd_data, &vtx_data));

  vtx_data[0].reset(x1, y1);
  vtx_data[1].reset(x2, y2);

  cmd_data[0] = BL_PATH_CMD_QUAD;
  cmd_data[1] = BL_PATH_CMD_ON;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_path_conic_to(BLPathCore* self, double x1, double y1, double x2, double y2, double w) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  uint8_t* cmd_data;
  BLPoint* vtx_data;
  BL_PROPAGATE(prepare_add(self, 3, &cmd_data, &vtx_data));

  vtx_data[0].reset(x1, y1);
  vtx_data[1].reset(w, bl::Math::nan<double>());
  vtx_data[2].reset(x2, y2);

  cmd_data[0] = BL_PATH_CMD_CONIC;
  cmd_data[1] = BL_PATH_CMD_WEIGHT;
  cmd_data[2] = BL_PATH_CMD_ON;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_path_cubic_to(BLPathCore* self, double x1, double y1, double x2, double y2, double x3, double y3) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  uint8_t* cmd_data;
  BLPoint* vtx_data;
  BL_PROPAGATE(prepare_add(self, 3, &cmd_data, &vtx_data));

  vtx_data[0].reset(x1, y1);
  vtx_data[1].reset(x2, y2);
  vtx_data[2].reset(x3, y3);

  cmd_data[0] = BL_PATH_CMD_CUBIC;
  cmd_data[1] = BL_PATH_CMD_CUBIC;
  cmd_data[2] = BL_PATH_CMD_ON;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_path_smooth_quad_to(BLPathCore* self, double x2, double y2) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  size_t size = self_impl->size;

  if (BL_UNLIKELY(!size || self_impl->command_data[size - 1u] >= BL_PATH_CMD_CLOSE))
    return bl_make_error(BL_ERROR_NO_MATCHING_VERTEX);

  uint8_t* cmd_data;
  BLPoint* vtx_data;
  BL_PROPAGATE(prepare_add(self, 2, &cmd_data, &vtx_data));

  double x1 = vtx_data[-1].x;
  double y1 = vtx_data[-1].y;

  if (size >= 2 && cmd_data[-2] == BL_PATH_CMD_QUAD) {
    x1 += x1 - vtx_data[-2].x;
    y1 += y1 - vtx_data[-2].y;
  }

  vtx_data[0].reset(x1, y1);
  vtx_data[1].reset(x2, y2);

  cmd_data[0] = BL_PATH_CMD_QUAD;
  cmd_data[1] = BL_PATH_CMD_ON;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_path_smooth_cubic_to(BLPathCore* self, double x2, double y2, double x3, double y3) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  size_t size = self_impl->size;

  if (BL_UNLIKELY(!size || self_impl->command_data[size - 1u] >= BL_PATH_CMD_CLOSE))
    return bl_make_error(BL_ERROR_NO_MATCHING_VERTEX);

  uint8_t* cmd_data;
  BLPoint* vtx_data;
  BL_PROPAGATE(prepare_add(self, 3, &cmd_data, &vtx_data));

  double x1 = vtx_data[-1].x;
  double y1 = vtx_data[-1].y;

  if (size >= 2 && cmd_data[-2] == BL_PATH_CMD_CUBIC) {
    x1 += x1 - vtx_data[-2].x;
    y1 += y1 - vtx_data[-2].y;
  }

  vtx_data[0].reset(x1, y1);
  vtx_data[1].reset(x2, y2);
  vtx_data[2].reset(x3, y3);

  cmd_data[0] = BL_PATH_CMD_CUBIC;
  cmd_data[1] = BL_PATH_CMD_CUBIC;
  cmd_data[2] = BL_PATH_CMD_ON;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_path_arc_to(BLPathCore* self, double x, double y, double rx, double ry, double start, double sweep, bool force_move_to) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  bl::PathAppender dst;
  uint8_t initial_cmd = BL_PATH_CMD_MOVE;
  bool maybe_redundant_line_to = false;

  if (!force_move_to) {
    BLPathPrivateImpl* self_impl = get_impl(self);
    size_t size = self_impl->size;

    if (size != 0 && self_impl->command_data[size - 1] <= BL_PATH_CMD_ON) {
      initial_cmd = BL_PATH_CMD_ON;
      maybe_redundant_line_to = true;
    }
  }

  BL_PROPAGATE(dst.begin_append(self, 13));
  arc_to_cubic_spline(dst, BLPoint(x, y), BLPoint(rx, ry), start, sweep, initial_cmd, maybe_redundant_line_to);

  dst.done(self);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_path_arc_quadrant_to(BLPathCore* self, double x1, double y1, double x2, double y2) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  size_t size = self_impl->size;

  if (BL_UNLIKELY(!size || self_impl->command_data[size - 1u] >= BL_PATH_CMD_CLOSE))
    return bl_make_error(BL_ERROR_NO_MATCHING_VERTEX);

  uint8_t* cmd_data;
  BLPoint* vtx_data;
  BL_PROPAGATE(prepare_add(self, 3, &cmd_data, &vtx_data));

  BLPoint p0 = vtx_data[-1];
  BLPoint p1(x1, y1);
  BLPoint p2(x2, y2);

  vtx_data[0].reset(p0 + (p1 - p0) * bl::Math::kKAPPA);
  vtx_data[1].reset(p2 + (p1 - p2) * bl::Math::kKAPPA);
  vtx_data[2].reset(p2);

  cmd_data[0] = BL_PATH_CMD_CUBIC;
  cmd_data[1] = BL_PATH_CMD_CUBIC;
  cmd_data[2] = BL_PATH_CMD_ON;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_path_elliptic_arc_to(BLPathCore* self, double rx, double ry, double xAxisRotation, bool large_arc_flag, bool sweep_flag, double x1, double y1) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  size_t size = self_impl->size;

  if (!size || self_impl->command_data[size - 1u] > BL_PATH_CMD_ON)
    return BL_ERROR_NO_MATCHING_VERTEX;

  BLPoint p0 = self_impl->vertex_data[size - 1u]; // Start point.
  BLPoint p1(x1, y1);                        // End point.

  // Special case - out of range radii.
  //   - See https://www.w3.org/TR/SVG/implnote.html#ArcCorrectionOutOfRangeRadii
  rx = bl_abs(rx);
  ry = bl_abs(ry);

  // Special case - out of range parameters:
  //   - See https://www.w3.org/TR/SVG/paths.html#ArcOutOfRangeParameters
  if (p0 == p1)
    return BL_SUCCESS;

  if (unsigned(!(rx > bl::Math::epsilon<double>())) | unsigned(!(ry > bl::Math::epsilon<double>())))
    return bl_path_line_to(self, p1.x, p1.y);

  // Calculate sin/cos for reuse.
  double rot_sin = bl::Math::sin(xAxisRotation);
  double rot_cos = bl::Math::cos(xAxisRotation);

  // Inverse rotation to align the ellipse.
  BLMatrix2D transform = BLMatrix2D::make_sin_cos(-rot_sin, rot_cos);

  // Vector from center (transformed midpoint).
  BLPoint v = transform.map_point((p0 - p1) * 0.5);

  // If scale > 1 the ellipse will need to be rescaled.
  double scale = bl::Math::square(v.x) / bl::Math::square(rx) +
                 bl::Math::square(v.y) / bl::Math::square(ry) ;
  if (scale > 1.0) {
    scale = bl::Math::sqrt(scale);
    rx *= scale;
    ry *= scale;
  }

  // Prepend scale.
  transform.post_scale(1.0 / rx, 1.0 / ry);

  // Calculate unit coordinates.
  BLPoint pp0 = transform.map_point(p0);
  BLPoint pp1 = transform.map_point(p1);

  // New vector from center (unit midpoint).
  v = (pp1 - pp0) * 0.5;
  BLPoint pc = pp0 + v;

  // If length^2 >= 1 the point is already the center.
  double len2 = bl::Geometry::magnitude_squared(v);
  if (len2 < 1.0) {
    v = bl::Math::sqrt(1.0 / len2 - 1.0) * bl::Geometry::normal(v);

    if (large_arc_flag != sweep_flag)
      pc += v;
    else
      pc -= v;
  }

  // Both vectors are unit vectors.
  BLPoint v1 = pp0 - pc;
  BLPoint v2 = pp1 - pc;

  // Set up the final transformation matrix.
  transform.reset_to_sin_cos(v1.y, v1.x);
  transform.post_translate(pc);
  transform.post_scale(rx, ry);
  bl::TransformInternal::multiply(transform, transform, BLMatrix2D::make_sin_cos(rot_sin, rot_cos));

  // We have sin = v1.Cross(v2) / (v1.Length * v2.Length)
  // with length of 'v1' and 'v2' both 1 (unit vectors).
  rot_sin = bl::Geometry::cross(v1, v2);

  // Accordingly cos = v1.Dot(v2) / (v1.Length * v2.Length)
  // to get the angle between 'v1' and 'v2'.
  rot_cos = bl::Geometry::dot(v1, v2);

  // So the sweep angle is Atan2(y, x) = Atan2(sin, cos)
  // https://stackoverflow.com/a/16544330
  double sweep_angle = bl::Math::atan2(rot_sin, rot_cos);
  if (sweep_flag) {
    // Correct the angle if necessary.
    if (sweep_angle < 0) {
      sweep_angle += bl::Math::kPI_MUL_2;
    }

    // |  v1.X  v1.Y  0 |   | v2.X |   | v1.X * v2.X + v1.Y * v2.Y |
    // | -v1.Y  v1.X  0 | * | v2.Y | = | v1.X * v2.Y - v1.Y * v2.X |
    // |  0     0     1 |   | 1    |   | 1                         |
    v2.reset(rot_cos, rot_sin);
  }
  else {
    if (sweep_angle > 0) {
      sweep_angle -= bl::Math::kPI_MUL_2;
    }

    // Flip Y.
    transform.scale(1.0, -1.0);

    v2.reset(rot_cos, -rot_sin);
    sweep_angle = bl_abs(sweep_angle);
  }

  // First quadrant (start and control point).
  v1.reset(1, 0);
  v.reset(1, 1);

  // The the number of 90deg segments we are gonna need. If `i == 1` it means
  // we need one 90deg segment and one smaller segment handled after the loop.
  size_t i = 3;
  if (sweep_angle < bl::Math::kPI_MUL_1p5 + bl::Math::kANGLE_EPSILON) i = 2;
  if (sweep_angle < bl::Math::kPI         + bl::Math::kANGLE_EPSILON) i = 1;
  if (sweep_angle < bl::Math::kPI_DIV_2   + bl::Math::kANGLE_EPSILON) i = 0;

  bl::PathAppender appender;
  BL_PROPAGATE(appender.begin(self, BL_MODIFY_OP_APPEND_GROW, (i + 1) * 3));

  // Process 90 degree segments.
  while (i) {
    v1 = bl::Geometry::normal(v1);

    // Transformed points of the arc segment.
    pp0 = transform.map_point(v);
    pp1 = transform.map_point(v1);
    appender.arc_quadrant_to(pp0, pp1);

    v = bl::Geometry::normal(v);
    i--;
  }

  // Calculate the remaining control point.
  v = v1 + v2;
  v = 2.0 * v / bl::Geometry::dot(v, v);

  // Final arc segment.
  pp0 = transform.map_point(v);
  pp1 = p1;

  // This is actually half of the remaining cos. It is required that v1 dot v2 > -1 holds
  // but we can safely assume it (only critical for angles close to 180 degrees).
  rot_cos = bl::Math::sqrt(0.5 * (1.0 + bl::Geometry::dot(v1, v2)));
  appender.conic_to(pp0, pp1, rot_cos);
  appender.done(self);

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_path_close(BLPathCore* self) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  uint8_t* cmd_data;
  BLPoint* vtx_data;
  BL_PROPAGATE(prepare_add(self, 1, &cmd_data, &vtx_data));

  vtx_data[0].reset(bl::Math::nan<double>(), bl::Math::nan<double>());
  cmd_data[0] = BL_PATH_CMD_CLOSE;

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_path_add_box_i(BLPathCore* self, const BLBoxI* box, BLGeometryDirection dir) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  return append_box_internal(self, double(box->x0), double(box->y0), double(box->x1), double(box->y1), dir);
}

BL_API_IMPL BLResult bl_path_add_box_d(BLPathCore* self, const BLBox* box, BLGeometryDirection dir) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  return append_box_internal(self, box->x0, box->y0, box->x1, box->y1, dir);
}

BL_API_IMPL BLResult bl_path_add_rect_i(BLPathCore* self, const BLRectI* rect, BLGeometryDirection dir) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  double x0 = double(rect->x);
  double y0 = double(rect->y);
  double x1 = double(rect->w) + x0;
  double y1 = double(rect->h) + y0;
  return append_box_internal(self, x0, y0, x1, y1, dir);
}

BL_API_IMPL BLResult bl_path_add_rect_d(BLPathCore* self, const BLRect* rect, BLGeometryDirection dir) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  double x0 = rect->x;
  double y0 = rect->y;
  double x1 = rect->w + x0;
  double y1 = rect->h + y0;
  return append_box_internal(self, x0, y0, x1, y1, dir);
}

BL_API_IMPL BLResult bl_path_add_geometry(BLPathCore* self, BLGeometryType geometry_type, const void* geometry_data, const BLMatrix2D* m, BLGeometryDirection dir) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  if (BL_UNLIKELY(uint32_t(geometry_type) > BL_GEOMETRY_TYPE_MAX_VALUE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  size_t n = path_vertex_count_of_geometry_type_table[geometry_type];
  if (n == 255) {
    switch (geometry_type) {
      // We don't expect this often so that's why we pessimistically check it here...
      case BL_GEOMETRY_TYPE_NONE:
        return BL_SUCCESS;

      case BL_GEOMETRY_TYPE_POLYLINED:
      case BL_GEOMETRY_TYPE_POLYLINEI:
        n = static_cast<const BLArrayView<uint8_t>*>(geometry_data)->size;
        if (!n)
          return BL_SUCCESS;
        break;

      case BL_GEOMETRY_TYPE_POLYGOND:
      case BL_GEOMETRY_TYPE_POLYGONI:
        n = static_cast<const BLArrayView<uint8_t>*>(geometry_data)->size;
        if (!n)
          return BL_SUCCESS;
        n++;
        break;

      case BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXD:
      case BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXI:
      case BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTD:
      case BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTI: {
        n = static_cast<const BLArrayView<uint8_t>*>(geometry_data)->size;
        if (!n)
          return BL_SUCCESS;

        n = bl::IntOps::umul_saturate<size_t>(n, 5);
        break;
      }

      case BL_GEOMETRY_TYPE_PATH: {
        const BLPath* other = static_cast<const BLPath*>(geometry_data);
        n = other->size();
        if (!n)
          return BL_SUCCESS;

        if (dir == BL_GEOMETRY_DIRECTION_CW) {
          if (m)
            return bl_path_add_transformed_path(self, other, nullptr, m);
          else
            return bl_path_add_path(self, other, nullptr);
        }
        break;
      }

      // Should never be reached as we filtered all border cases already...
      default:
        return bl_make_error(BL_ERROR_INVALID_VALUE);
    }
  }

  // Should never be zero if we went here.
  BL_ASSERT(n != 0);
  size_t initial_size = get_size(self);

  bl::PathAppender appender;
  BL_PROPAGATE(appender.begin_append(self, n));

  // For adding 'BLBox', 'BLBoxI', 'BLRect', 'BLRectI', and `BLRoundRect`.
  double x0, y0;
  double x1, y1;

  switch (geometry_type) {
    case BL_GEOMETRY_TYPE_BOXI:
      x0 = double(static_cast<const BLBoxI*>(geometry_data)->x0);
      y0 = double(static_cast<const BLBoxI*>(geometry_data)->y0);
      x1 = double(static_cast<const BLBoxI*>(geometry_data)->x1);
      y1 = double(static_cast<const BLBoxI*>(geometry_data)->y1);
      goto AddBoxD;

    case BL_GEOMETRY_TYPE_BOXD:
      x0 = static_cast<const BLBox*>(geometry_data)->x0;
      y0 = static_cast<const BLBox*>(geometry_data)->y0;
      x1 = static_cast<const BLBox*>(geometry_data)->x1;
      y1 = static_cast<const BLBox*>(geometry_data)->y1;
      goto AddBoxD;

    case BL_GEOMETRY_TYPE_RECTI:
      x0 = double(static_cast<const BLRectI*>(geometry_data)->x);
      y0 = double(static_cast<const BLRectI*>(geometry_data)->y);
      x1 = double(static_cast<const BLRectI*>(geometry_data)->w) + x0;
      y1 = double(static_cast<const BLRectI*>(geometry_data)->h) + y0;
      goto AddBoxD;

    case BL_GEOMETRY_TYPE_RECTD:
      x0 = static_cast<const BLRect*>(geometry_data)->x;
      y0 = static_cast<const BLRect*>(geometry_data)->y;
      x1 = static_cast<const BLRect*>(geometry_data)->w + x0;
      y1 = static_cast<const BLRect*>(geometry_data)->h + y0;

AddBoxD:
      appender.add_box(x0, y0, x1, y1, dir);
      break;

    case BL_GEOMETRY_TYPE_CIRCLE:
    case BL_GEOMETRY_TYPE_ELLIPSE: {
      double rx, kx;
      double ry, ky;

      if (geometry_type == BL_GEOMETRY_TYPE_CIRCLE) {
        const BLCircle* circle = static_cast<const BLCircle*>(geometry_data);
        x0 = circle->cx;
        y0 = circle->cy;
        rx = circle->r;
        ry = bl_abs(rx);
      }
      else {
        const BLEllipse* ellipse = static_cast<const BLEllipse*>(geometry_data);
        x0 = ellipse->cx;
        y0 = ellipse->cy;
        rx = ellipse->rx;
        ry = ellipse->ry;
      }

      if (dir != BL_GEOMETRY_DIRECTION_CW)
        ry = -ry;

      kx = rx * bl::Math::kKAPPA;
      ky = ry * bl::Math::kKAPPA;

      appender.move_to(x0 + rx, y0);
      appender.cubic_to(x0 + rx, y0 + ky, x0 + kx, y0 + ry, x0     , y0 + ry);
      appender.cubic_to(x0 - kx, y0 + ry, x0 - rx, y0 + ky, x0 - rx, y0     );
      appender.cubic_to(x0 - rx, y0 - ky, x0 - kx, y0 - ry, x0     , y0 - ry);
      appender.cubic_to(x0 + kx, y0 - ry, x0 + rx, y0 - ky, x0 + rx, y0     );
      appender.close();
      break;
    }

    case BL_GEOMETRY_TYPE_ROUND_RECT: {
      const BLRoundRect* round = static_cast<const BLRoundRect*>(geometry_data);

      x0 = round->x;
      y0 = round->y;
      x1 = round->x + round->w;
      y1 = round->y + round->h;

      double wHalf = round->w * 0.5;
      double hHalf = round->h * 0.5;

      double rx = bl_min(bl_abs(round->rx), wHalf);
      double ry = bl_min(bl_abs(round->ry), hHalf);

      // Degrade to box if rx/ry are degenerate.
      if (BL_UNLIKELY(!(rx > bl::Math::epsilon<double>() && ry > bl::Math::epsilon<double>())))
        goto AddBoxD;

      double kx = rx * (1.0 - bl::Math::kKAPPA);
      double ky = ry * (1.0 - bl::Math::kKAPPA);

      if (dir == BL_GEOMETRY_DIRECTION_CW) {
        appender.move_to(x0 + rx, y0);
        appender.line_to(x1 - rx, y0);
        appender.cubic_to(x1 - kx, y0, x1, y0 + ky, x1, y0 + ry);
        appender.line_to(x1, y1 - ry);
        appender.cubic_to(x1, y1 - ky, x1 - kx, y1, x1 - rx, y1);
        appender.line_to(x0 + rx, y1);
        appender.cubic_to(x0 + kx, y1, x0, y1 - ky, x0, y1 - ry);
        appender.line_to(x0, y0 + ry);
        appender.cubic_to(x0, y0 + ky, x0 + kx, y0, x0 + rx, y0);
        appender.close();
      }
      else {
        appender.move_to(x0 + rx, y0);
        appender.cubic_to(x0 + kx, y0, x0, y0 + ky, x0, y0 + ry);
        appender.line_to(x0, y1 - ry);
        appender.cubic_to(x0, y1 - ky, x0 + kx, y1, x0 + rx, y1);
        appender.line_to(x1 - rx, y1);
        appender.cubic_to(x1 - kx, y1, x1, y1 - ky, x1, y1 - ry);
        appender.line_to(x1, y0 + ry);
        appender.cubic_to(x1, y0 + ky, x1 - kx, y0, x1 - rx, y0);
        appender.close();
      }
      break;
    }

    case BL_GEOMETRY_TYPE_LINE: {
      const BLPoint* src = static_cast<const BLPoint*>(geometry_data);
      size_t first = dir != BL_GEOMETRY_DIRECTION_CW;

      appender.move_to(src[first]);
      appender.line_to(src[first ^ 1]);
      break;
    }

    case BL_GEOMETRY_TYPE_ARC: {
      const BLArc* arc = static_cast<const BLArc*>(geometry_data);

      BLPoint c(arc->cx, arc->cy);
      BLPoint r(arc->rx, arc->ry);
      double start = arc->start;
      double sweep = arc->sweep;

      if (dir != BL_GEOMETRY_DIRECTION_CW)
        sweep = -sweep;

      arc_to_cubic_spline(appender, c, r, start, sweep, BL_PATH_CMD_MOVE);
      break;
    }

    case BL_GEOMETRY_TYPE_CHORD:
    case BL_GEOMETRY_TYPE_PIE: {
      const BLArc* arc = static_cast<const BLArc*>(geometry_data);

      BLPoint c(arc->cx, arc->cy);
      BLPoint r(arc->rx, arc->ry);
      double start = arc->start;
      double sweep = arc->sweep;

      if (dir != BL_GEOMETRY_DIRECTION_CW)
        sweep = -sweep;

      uint8_t arc_initial_cmd = BL_PATH_CMD_MOVE;
      if (geometry_type == BL_GEOMETRY_TYPE_PIE) {
        appender.move_to(c);
        arc_initial_cmd = BL_PATH_CMD_ON;
      }

      arc_to_cubic_spline(appender, c, r, start, sweep, arc_initial_cmd);
      appender.close();
      break;
    }

    case BL_GEOMETRY_TYPE_TRIANGLE: {
      const BLPoint* src = static_cast<const BLPoint*>(geometry_data);
      size_t cw = dir == BL_GEOMETRY_DIRECTION_CW ? 0 : 2;

      appender.move_to(src[0 + cw]);
      appender.line_to(src[1]);
      appender.line_to(src[2 - cw]);
      appender.close();
      break;
    }

    case BL_GEOMETRY_TYPE_POLYLINEI: {
      const BLArrayView<BLPointI>* array = static_cast<const BLArrayView<BLPointI>*>(geometry_data);
      const BLPointI* src = array->data;

      if (dir == BL_GEOMETRY_DIRECTION_CW) {
        for (size_t i = n; i; i--)
          appender.line_to(*src++);
      }
      else {
        src += n - 1;
        for (size_t i = n; i; i--)
          appender.line_to(*src--);
      }

      appender.cmd[-intptr_t(n)].value = BL_PATH_CMD_MOVE;
      break;
    }

    case BL_GEOMETRY_TYPE_POLYLINED: {
      const BLArrayView<BLPoint>* array = static_cast<const BLArrayView<BLPoint>*>(geometry_data);
      const BLPoint* src = array->data;

      if (dir == BL_GEOMETRY_DIRECTION_CW) {
        for (size_t i = n; i; i--)
          appender.line_to(*src++);
      }
      else {
        src += n - 1;
        for (size_t i = n; i; i--)
          appender.line_to(*src--);
      }

      appender.cmd[-intptr_t(n)].value = BL_PATH_CMD_MOVE;
      break;
    }

    case BL_GEOMETRY_TYPE_POLYGONI: {
      const BLArrayView<BLPointI>* array = static_cast<const BLArrayView<BLPointI>*>(geometry_data);
      const BLPointI* src = array->data;

      if (dir == BL_GEOMETRY_DIRECTION_CW) {
        for (size_t i = n - 1; i; i--)
          appender.line_to(*src++);
      }
      else {
        src += n - 1;
        for (size_t i = n - 1; i; i--)
          appender.line_to(*src--);
      }

      appender.close();
      appender.cmd[-intptr_t(n)].value = BL_PATH_CMD_MOVE;
      break;
    }

    case BL_GEOMETRY_TYPE_POLYGOND: {
      const BLArrayView<BLPoint>* array = static_cast<const BLArrayView<BLPoint>*>(geometry_data);
      const BLPoint* src = array->data;

      if (dir == BL_GEOMETRY_DIRECTION_CW) {
        for (size_t i = n - 1; i; i--)
          appender.line_to(*src++);
      }
      else {
        src += n - 1;
        for (size_t i = n - 1; i; i--)
          appender.line_to(*src--);
      }

      appender.close();
      appender.cmd[-intptr_t(n)].value = BL_PATH_CMD_MOVE;
      break;
    }

    case BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXI: {
      const BLArrayView<BLBoxI>* array = static_cast<const BLArrayView<BLBoxI>*>(geometry_data);
      const BLBoxI* src = array->data;

      if (dir == BL_GEOMETRY_DIRECTION_CW) {
        for (size_t i = n; i != 0; i -= 5, src++) {
          if (!bl::Geometry::is_valid(*src))
            continue;
          appender.addBoxCW(src->x0, src->y0, src->x1, src->y1);
        }
      }
      else {
        src += n - 1;
        for (size_t i = n; i != 0; i -= 5, src--) {
          if (!bl::Geometry::is_valid(*src))
            continue;
          appender.addBoxCCW(src->x0, src->y0, src->x1, src->y1);
        }
      }
      break;
    }

    case BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXD: {
      const BLArrayView<BLBox>* array = static_cast<const BLArrayView<BLBox>*>(geometry_data);
      const BLBox* src = array->data;

      if (dir == BL_GEOMETRY_DIRECTION_CW) {
        for (size_t i = n; i != 0; i -= 5, src++) {
          if (!bl::Geometry::is_valid(*src))
            continue;
          appender.addBoxCW(src->x0, src->y0, src->x1, src->y1);
        }
      }
      else {
        src += n - 1;
        for (size_t i = n; i != 0; i -= 5, src--) {
          if (!bl::Geometry::is_valid(*src))
            continue;
          appender.addBoxCCW(src->x0, src->y0, src->x1, src->y1);
        }
      }
      break;
    }

    case BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTI: {
      const BLArrayView<BLRectI>* array = static_cast<const BLArrayView<BLRectI>*>(geometry_data);
      const BLRectI* src = array->data;

      if (dir == BL_GEOMETRY_DIRECTION_CW) {
        for (size_t i = n; i != 0; i -= 5, src++) {
          if (!bl::Geometry::is_valid(*src))
            continue;

          x0 = double(src->x);
          y0 = double(src->y);
          x1 = double(src->w) + x0;
          y1 = double(src->h) + y0;
          appender.addBoxCW(x0, y0, x1, y1);
        }
      }
      else {
        src += n - 1;
        for (size_t i = n; i != 0; i -= 5, src--) {
          if (!bl::Geometry::is_valid(*src))
            continue;

          x0 = double(src->x);
          y0 = double(src->y);
          x1 = double(src->w) + x0;
          y1 = double(src->h) + y0;
          appender.addBoxCCW(x0, y0, x1, y1);
        }
      }
      break;
    }

    case BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTD: {
      const BLArrayView<BLRect>* array = static_cast<const BLArrayView<BLRect>*>(geometry_data);
      const BLRect* src = array->data;

      if (dir == BL_GEOMETRY_DIRECTION_CW) {
        for (size_t i = n; i != 0; i -= 5, src++) {
          if (!bl::Geometry::is_valid(*src))
            continue;

          x0 = src->x;
          y0 = src->y;
          x1 = src->w + x0;
          y1 = src->h + y0;
          appender.addBoxCW(x0, y0, x1, y1);
        }
      }
      else {
        src += n - 1;
        for (size_t i = n; i != 0; i -= 5, src--) {
          if (!bl::Geometry::is_valid(*src))
            continue;

          x0 = src->x;
          y0 = src->y;
          x1 = src->w + x0;
          y1 = src->h + y0;
          appender.addBoxCCW(x0, y0, x1, y1);
        }
      }
      break;
    }

    case BL_GEOMETRY_TYPE_PATH: {
      // Only for appending path in reverse order, otherwise we use a better approach.
      BL_ASSERT(dir != BL_GEOMETRY_DIRECTION_CW);

      const BLPathPrivateImpl* other_impl = get_impl(static_cast<const BLPath*>(geometry_data));
      BLResult result = copy_content_reversed(appender, bl::PathIterator(other_impl->view), BL_PATH_REVERSE_MODE_COMPLETE);

      if (result != BL_SUCCESS) {
        set_size(self, initial_size);
        return result;
      }
      break;
    }

    default:
      // This is not possible considering even bad input as we have filtered this already.
      BL_NOT_REACHED();
  }

  appender.done(self);
  if (!m)
    return BL_SUCCESS;

  BLPathPrivateImpl* self_impl = get_impl(self);
  BLPoint* vtx_data = self_impl->vertex_data + initial_size;
  return bl_matrix2d_map_pointd_array(m, vtx_data, vtx_data, self_impl->size - initial_size);
}

BL_API_IMPL BLResult bl_path_add_path(BLPathCore* self, const BLPathCore* other, const BLRange* range) noexcept {
  using namespace bl::PathInternal;

  BL_ASSERT(self->_d.is_path());
  BL_ASSERT(other->_d.is_path());

  BLPathPrivateImpl* other_impl = get_impl(other);
  size_t start, n;

  if (!check_range(other_impl, range, &start, &n))
    return BL_SUCCESS;

  uint8_t* cmd_data;
  BLPoint* vtx_data;

  // Maybe `self` and `other` are the same, so get the `other` impl.
  BL_PROPAGATE(prepare_add(self, n, &cmd_data, &vtx_data));
  other_impl = get_impl(other);

  copy_content(cmd_data, vtx_data, other_impl->command_data + start, other_impl->vertex_data + start, n);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_path_add_translated_path(BLPathCore* self, const BLPathCore* other, const BLRange* range, const BLPoint* p) noexcept {
  using namespace bl::PathInternal;

  BL_ASSERT(self->_d.is_path());
  BL_ASSERT(other->_d.is_path());

  BLMatrix2D transform = BLMatrix2D::make_translation(*p);
  return append_transformed_path_with_type(self, other, range, &transform, BL_TRANSFORM_TYPE_TRANSLATE);
}

BL_API_IMPL BLResult bl_path_add_transformed_path(BLPathCore* self, const BLPathCore* other, const BLRange* range, const BLMatrix2D* transform) noexcept {
  using namespace bl::PathInternal;

  BL_ASSERT(self->_d.is_path());
  BL_ASSERT(other->_d.is_path());

  BLPathPrivateImpl* other_impl = get_impl(other);
  size_t start, n;

  if (!check_range(other_impl, range, &start, &n))
    return BL_SUCCESS;

  uint8_t* cmd_data;
  BLPoint* vtx_data;

  // Maybe `self` and `other` were the same, so get the `other` impl again.
  BL_PROPAGATE(prepare_add(self, n, &cmd_data, &vtx_data));
  other_impl = get_impl(other);

  // Only check the transform type if we reach the limit as the check costs some cycles.
  BLTransformType transform_type = (n >= BL_MATRIX_TYPE_MINIMUM_SIZE) ? transform->type() : BL_TRANSFORM_TYPE_AFFINE;

  memcpy(cmd_data, other_impl->command_data + start, n);
  return bl::TransformInternal::map_pointd_array_funcs[transform_type](transform, vtx_data, other_impl->vertex_data + start, n);
}

BL_API_IMPL BLResult bl_path_add_reversed_path(BLPathCore* self, const BLPathCore* other, const BLRange* range, BLPathReverseMode reverse_mode) noexcept {
  using namespace bl::PathInternal;

  BL_ASSERT(self->_d.is_path());
  BL_ASSERT(other->_d.is_path());

  if (BL_UNLIKELY(uint32_t(reverse_mode) > BL_PATH_REVERSE_MODE_MAX_VALUE))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  BLPathPrivateImpl* other_impl = get_impl(other);
  size_t start, n;

  if (!check_range(other_impl, range, &start, &n))
    return BL_SUCCESS;

  size_t initial_size = get_size(self);
  bl::PathAppender dst;
  BL_PROPAGATE(dst.begin_append(self, n));

  // Maybe `self` and `other` were the same, so get the `other` impl again.
  other_impl = get_impl(other);
  bl::PathIterator src(other_impl->command_data + start, other_impl->vertex_data + start, n);

  BLResult result = copy_content_reversed(dst, src, reverse_mode);
  dst.done(self);

  // Don't keep anything if reversal failed.
  if (result != BL_SUCCESS)
    set_size(self, initial_size);
  return result;
}

// bl::Path - API - Stroke
// =======================

namespace bl {
namespace PathInternal {

static BLResult join_figure(PathAppender& dst, PathIterator src) noexcept {
  if (src.at_end())
    return BL_SUCCESS;

  bool is_closed = dst.cmd[-1].value == BL_PATH_CMD_CLOSE;
  uint8_t initial_cmd = uint8_t(is_closed ? BL_PATH_CMD_MOVE : BL_PATH_CMD_ON);

  // Initial vertex (either MOVE or ON). If the initial vertex matches the
  // the last vertex in `dst` we won't emit it as it would be unnecessary.
  if (dst.vtx[-1] != src.vtx[0] || initial_cmd == BL_PATH_CMD_MOVE)
    dst.add_vertex(initial_cmd, src.vtx[0]);

  // Iterate the figure.
  while (!(++src).at_end())
    dst.add_vertex(src.cmd[0], src.vtx[0]);

  return BL_SUCCESS;
}

static BLResult join_reversed_figure(PathAppender& dst, PathIterator src) noexcept {
  if (src.at_end())
    return BL_SUCCESS;

  src.reverse();
  src--;

  bool is_closed = dst.cmd[-1].value == BL_PATH_CMD_CLOSE;
  uint8_t initial_cmd = uint8_t(is_closed ? BL_PATH_CMD_MOVE : BL_PATH_CMD_ON);
  uint8_t cmd = src.cmd[1];

  // Initial MOVE means the whole figure consists of just a single MOVE.
  if (cmd == BL_PATH_CMD_MOVE) {
    dst.add_vertex(initial_cmd, src.vtx[1]);
    return BL_SUCCESS;
  }

  // Get whether the figure is closed.
  BL_ASSERT(cmd == BL_PATH_CMD_CLOSE || cmd == BL_PATH_CMD_ON);
  bool has_close = (cmd == BL_PATH_CMD_CLOSE);

  if (has_close) {
    // Make sure the next command is ON.
    if (src.at_end()) {
      dst.close();
      return BL_SUCCESS;
    }

    // We just encountered CLOSE followed by ON (reversed).
    BL_ASSERT(src.cmd[0] == BL_PATH_CMD_ON);
    src--;
  }

  // Initial vertex (either MOVE or ON). If the initial vertex matches the
  // the last vertex in `dst` we won't emit it as it would be unnecessary.
  if (dst.vtx[-1] != src.vtx[1] || initial_cmd == BL_PATH_CMD_MOVE)
    dst.add_vertex(initial_cmd, src.vtx[1]);

  // Iterate the figure.
  if (!src.at_end()) {
    do {
      dst.add_vertex(src.cmd[0], src.vtx[0]);
      src--;
    } while (!src.at_end());
    // Fix the last vertex to not be MOVE.
    dst.cmd[-1].value = BL_PATH_CMD_ON;
  }

  // Emit CLOSE if the figure is closed.
  if (has_close)
    dst.close();
  return BL_SUCCESS;
}

static BLResult append_stroked_path_sink(BLPathCore* a, BLPathCore* b, BLPathCore* c, size_t figure_start, size_t figure_end, void* user_data) noexcept {
  BL_ASSERT(a->_d.is_path());
  BL_ASSERT(b->_d.is_path());
  BL_ASSERT(c->_d.is_path());

  bl_unused(figure_start, figure_end, user_data);

  PathAppender dst;
  BL_PROPAGATE(dst.begin(a, BL_MODIFY_OP_APPEND_GROW, b->dcast().size() + c->dcast().size() + 1u));

  BLResult result = join_reversed_figure(dst, PathIterator(b->dcast().view()));
  result |= join_figure(dst, PathIterator(c->dcast().view()));

  if (dst.cmd[-1].value != BL_PATH_CMD_CLOSE)
    dst.close();

  dst.done(a);
  return result;
}

} // {PathInternal}
} // {bl}

BL_API_IMPL BLResult bl_path_add_stroked_path(BLPathCore* self, const BLPathCore* other, const BLRange* range, const BLStrokeOptionsCore* options, const BLApproximationOptions* approx) noexcept {
  using namespace bl::PathInternal;

  BL_ASSERT(self->_d.is_path());
  BL_ASSERT(other->_d.is_path());

  const BLPathPrivateImpl* other_impl = get_impl(other);
  size_t start, n;

  if (!check_range(other_impl, range, &start, &n))
    return BL_SUCCESS;

  if (!approx)
    approx = &bl_default_approximation_options;

  BLPathView input { other_impl->command_data + start, other_impl->vertex_data + start, n };
  BLPath b_path;
  BLPath c_path;

  if (self == other) {
    // Border case, we don't want anything to happen to the `other` path during
    // processing. And since stroking may need to reallocate the output path it
    // would be unsafe.
    BLPath tmp(other->dcast());
    return stroke_path(input, options->dcast(), *approx, self->dcast(), b_path, c_path, append_stroked_path_sink, nullptr);
  }
  else {
    return stroke_path(input, options->dcast(), *approx, self->dcast(), b_path, c_path, append_stroked_path_sink, nullptr);
  }
}

BL_API_IMPL BLResult BL_CDECL bl_path_stroke_to_sink(
    const BLPathCore* self,
    const BLRange* range,
    const BLStrokeOptionsCore* stroke_options,
    const BLApproximationOptions* approximation_options,
    BLPathCore *a,
    BLPathCore *b,
    BLPathCore *c,
    BLPathStrokeSinkFunc sink, void* user_data) noexcept {
  using namespace bl::PathInternal;

  BL_ASSERT(self->_d.is_path());
  BL_ASSERT(a->_d.is_path());
  BL_ASSERT(b->_d.is_path());
  BL_ASSERT(c->_d.is_path());

  const BLPathPrivateImpl* self_impl = get_impl(self);
  size_t start, n;
  if (!check_range(self_impl, range, &start, &n))
    return BL_SUCCESS;

  if (!approximation_options)
    approximation_options = &bl_default_approximation_options;

  BLPathView input { self_impl->command_data + start, self_impl->vertex_data + start, n };

  if (a == self || b == self || c == self) {
    BLPath tmp(self->dcast());
    return stroke_path(input, stroke_options->dcast(), *approximation_options, a->dcast(), b->dcast(), c->dcast(), sink, user_data);
  }
  else {
    return stroke_path(input, stroke_options->dcast(), *approximation_options, a->dcast(), b->dcast(), c->dcast(), sink, user_data);
  }
}

// bl::Path - API - Path Manipulation
// ==================================

BL_API_IMPL BLResult bl_path_remove_range(BLPathCore* self, const BLRange* range) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  size_t start, n;

  if (!check_range(self_impl, range, &start, &n))
    return BL_SUCCESS;

  size_t size = self_impl->size;
  size_t end = start + n;

  if (n == size)
    return bl_path_clear(self);

  BLPoint* vtx_data = self_impl->vertex_data;
  uint8_t* cmd_data = self_impl->command_data;

  size_t size_after = size - n;
  if (!is_impl_mutable(self_impl)) {
    BLPathCore newO;
    BL_PROPAGATE(alloc_impl(&newO, size_after, impl_size_from_capacity(size_after)));

    BLPathPrivateImpl* new_impl = get_impl(&newO);
    copy_content(new_impl->command_data, new_impl->vertex_data, cmd_data, vtx_data, start);
    copy_content(new_impl->command_data + start, new_impl->vertex_data + start, cmd_data + end, vtx_data + end, size - end);

    return replace_instance(self, &newO);
  }
  else {
    copy_content(cmd_data + start, vtx_data + start, cmd_data + end, vtx_data + end, size - end);
    self_impl->size = size_after;
    self_impl->flags = BL_PATH_FLAG_DIRTY;
    return BL_SUCCESS;
  }
}

// bl::Path - API - Path Transformations
// =====================================

namespace bl {
namespace PathInternal {

static BLResult transform_with_type(BLPathCore* self, const BLRange* range, const BLMatrix2D* transform, uint32_t transform_type) noexcept {
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  size_t start, n;

  if (!check_range(self_impl, range, &start, &n))
    return BL_SUCCESS;

  BL_PROPAGATE(make_mutable(self));
  self_impl = get_impl(self);

  BLPoint* vtx_data = self_impl->vertex_data + start;
  return bl::TransformInternal::map_pointd_array_funcs[transform_type](transform, vtx_data, vtx_data, n);
}

} // {PathInternal}
} // {bl}

BL_API_IMPL BLResult bl_path_translate(BLPathCore* self, const BLRange* range, const BLPoint* p) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLMatrix2D transform = BLMatrix2D::make_translation(*p);
  return transform_with_type(self, range, &transform, BL_TRANSFORM_TYPE_TRANSLATE);
}

BL_API_IMPL BLResult bl_path_transform(BLPathCore* self, const BLRange* range, const BLMatrix2D* m) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  size_t start, n;

  if (!check_range(self_impl, range, &start, &n))
    return BL_SUCCESS;

  BL_PROPAGATE(make_mutable(self));
  self_impl = get_impl(self);

  // Only check the transform type if we reach the limit as the check costs some cycles.
  BLTransformType transform_type = (n >= BL_MATRIX_TYPE_MINIMUM_SIZE) ? m->type() : BL_TRANSFORM_TYPE_AFFINE;

  BLPoint* vtx_data = self_impl->vertex_data + start;
  return bl::TransformInternal::map_pointd_array_funcs[transform_type](m, vtx_data, vtx_data, n);
}

BL_API_IMPL BLResult bl_path_fit_to(BLPathCore* self, const BLRange* range, const BLRect* rect, uint32_t fit_flags) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  size_t start, n;

  if (!check_range(self_impl, range, &start, &n))
    return BL_SUCCESS;

  if (!bl::Math::is_finite(*rect) || rect->w <= 0.0 || rect->h <= 0.0)
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  PathInfoUpdater updater;
  BL_PROPAGATE(updater.update(BLPathView { self_impl->command_data + start, self_impl->vertex_data + start, n }, true));

  // TODO: Honor `fit_flags`.
  bl_unused(fit_flags);

  const BLBox& b_box = updater.bounding_box;

  double bx = b_box.x0;
  double by = b_box.y0;
  double bw = b_box.x1 - b_box.x0;
  double bh = b_box.y1 - b_box.y0;

  double tx = rect->x;
  double ty = rect->y;
  double sx = rect->w / bw;
  double sy = rect->h / bh;

  tx -= bx * sx;
  ty -= by * sy;

  BLMatrix2D transform(sx, 0.0, 0.0, sy, tx, ty);
  return transform_with_type(self, range, &transform, BL_TRANSFORM_TYPE_SCALE);
}

// bl::Path - API Equals
// =====================

BL_API_IMPL bool bl_path_equals(const BLPathCore* a, const BLPathCore* b) noexcept {
  using namespace bl::PathInternal;

  BL_ASSERT(a->_d.is_path());
  BL_ASSERT(b->_d.is_path());

  const BLPathPrivateImpl* a_impl = get_impl(a);
  const BLPathPrivateImpl* b_impl = get_impl(b);

  if (a_impl == b_impl)
    return true;

  size_t size = a_impl->size;
  if (size != b_impl->size)
    return false;

  return memcmp(a_impl->command_data, b_impl->command_data, size * sizeof(uint8_t)) == 0 &&
         memcmp(a_impl->vertex_data , b_impl->vertex_data , size * sizeof(BLPoint)) == 0;
}

// bl::Path - API Path Info
// ========================

namespace bl {
namespace PathInternal {

static BL_NOINLINE BLResult update_info(BLPathPrivateImpl* self_impl) noexcept {
  // Special-case. The path info is valid, but the path is invalid. We handle it here to simplify `ensure_info()`
  // and to make it a bit shorter.
  if (self_impl->flags & BL_PATH_FLAG_INVALID)
    return bl_make_error(BL_ERROR_INVALID_GEOMETRY);

  PathInfoUpdater updater;
  BLResult result = updater.update(self_impl->view);

  // Path is invalid.
  if (result != BL_SUCCESS) {
    self_impl->flags = updater.flags | BL_PATH_FLAG_INVALID;
    self_impl->control_box.reset();
    self_impl->bounding_box.reset();
    return result;
  }

  // Path is empty.
  if (!(updater.bounding_box.x0 <= updater.bounding_box.x1 &&
        updater.bounding_box.y0 <= updater.bounding_box.y1)) {
    self_impl->flags = updater.flags | BL_PATH_FLAG_EMPTY;
    self_impl->control_box.reset();
    self_impl->bounding_box.reset();
    return BL_SUCCESS;
  }

  // Path is valid.
  self_impl->flags = updater.flags;
  self_impl->control_box = updater.control_box;
  self_impl->bounding_box = updater.bounding_box;
  return BL_SUCCESS;
}

static BL_INLINE BLResult ensure_info(BLPathPrivateImpl* self_impl) noexcept {
  if (self_impl->flags & (BL_PATH_FLAG_INVALID | BL_PATH_FLAG_DIRTY))
    return update_info(self_impl);

  return BL_SUCCESS;
}

} // {PathInternal}
} // {bl}

BL_API_IMPL BLResult bl_path_get_info_flags(const BLPathCore* self, uint32_t* flags_out) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  BLResult result = ensure_info(self_impl);

  *flags_out = self_impl->flags;
  return result;
}

// bl::Path - API - ControlBox & BoundingBox
// =========================================

BL_API_IMPL BLResult bl_path_get_control_box(const BLPathCore* self, BLBox* box_out) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  BLResult result = ensure_info(self_impl);

  *box_out = self_impl->control_box;
  return result;
}

BL_API_IMPL BLResult bl_path_get_bounding_box(const BLPathCore* self, BLBox* box_out) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  BLResult result = ensure_info(self_impl);

  *box_out = self_impl->bounding_box;
  return result;
}

// bl::Path - API - Subpath Range
// ==============================

BL_API_IMPL BLResult bl_path_get_figure_range(const BLPathCore* self, size_t index, BLRange* range_out) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  const BLPathPrivateImpl* self_impl = get_impl(self);
  const uint8_t* cmd_data = self_impl->command_data;
  size_t size = self_impl->size;

  if (index >= size) {
    range_out->reset(0, 0);
    return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  // Find end of the sub-path.
  size_t end = index + 1;
  while (end < size) {
    uint32_t cmd = cmd_data[end];
    if (cmd == BL_PATH_CMD_MOVE)
      break;

    end++;
    if (cmd == BL_PATH_CMD_CLOSE)
      break;
  }

  // Find start of the sub-path.
  if (cmd_data[index] != BL_PATH_CMD_MOVE) {
    while (index > 0) {
      uint32_t cmd = cmd_data[index - 1];

      if (cmd == BL_PATH_CMD_CLOSE)
        break;

      index--;
      if (cmd == BL_PATH_CMD_MOVE)
        break;
    }
  }

  range_out->reset(index, end);
  return BL_SUCCESS;
}

// bl::Path - API - Vertex Queries
// ===============================

BL_API_IMPL BLResult bl_path_get_last_vertex(const BLPathCore* self, BLPoint* vtx_out) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  size_t index = self_impl->size;

  vtx_out->reset();
  if (BL_UNLIKELY(!index))
    return bl_make_error(BL_ERROR_NO_MATCHING_VERTEX);

  const uint8_t* cmd_data = self_impl->command_data;
  uint32_t cmd = cmd_data[--index];

  if (cmd != BL_PATH_CMD_CLOSE) {
    *vtx_out = self_impl->vertex_data[index];
    return BL_SUCCESS;
  }

  for (;;) {
    if (index == 0)
      return bl_make_error(BL_ERROR_NO_MATCHING_VERTEX);

    cmd = cmd_data[--index];
    if (cmd == BL_PATH_CMD_CLOSE)
      return bl_make_error(BL_ERROR_NO_MATCHING_VERTEX);

    if (cmd == BL_PATH_CMD_MOVE)
      break;
  }

  *vtx_out = self_impl->vertex_data[index];
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_path_get_closest_vertex(const BLPathCore* self, const BLPoint* p, double max_distance, size_t* index_out, double* distance_out) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  size_t size = self_impl->size;

  *index_out = SIZE_MAX;
  *distance_out = bl::Math::nan<double>();

  if (BL_UNLIKELY(!size))
    return bl_make_error(BL_ERROR_NO_MATCHING_VERTEX);

  const uint8_t* cmd_data = self_impl->command_data;
  const BLPoint* vtx_data = self_impl->vertex_data;

  size_t best_index = SIZE_MAX;
  double best_distance = bl::Math::inf<double>();
  double best_distance_sq = bl::Math::inf<double>();

  BLPoint pt(*p);
  bool has_max_distance = max_distance > 0.0 && max_distance < bl::Math::inf<double>();

  if (has_max_distance) {
    best_distance = max_distance;
    best_distance_sq = bl::Math::square(best_distance);

    // This code-path can be used to skip the whole path if the given point is
    // too far. We need 'max_distance' to be specified and also bounding-box to
    // be available.
    if (ensure_info(self_impl) != BL_SUCCESS) {
      // If the given point is outside of the path bounding-box extended by
      // `max_distance` then there is no matching vertex to possibly return.
      const BLBox& b_box = self_impl->control_box;
      if (!(pt.x >= b_box.x0 - best_distance && pt.y >= b_box.y0 - best_distance &&
            pt.x <= b_box.x1 + best_distance && pt.y <= b_box.y1 + best_distance))
        return bl_make_error(BL_ERROR_NO_MATCHING_VERTEX);
    }
  }

  for (size_t i = 0; i < size; i++) {
    if (cmd_data[i] != BL_PATH_CMD_CLOSE) {
      double d = bl::Math::square(vtx_data[i].x - pt.x) +
                 bl::Math::square(vtx_data[i].y - pt.y);

      if (d < best_distance_sq) {
        best_index = i;
        best_distance_sq = d;
      }
    }
  }

  if (best_index == SIZE_MAX)
    best_distance = bl::Math::nan<double>();
  else
    best_distance = bl::Math::sqrt(best_distance_sq);

  *index_out = best_index;
  *distance_out = best_distance;

  return BL_SUCCESS;
}

// bl::Path - API - Hit Test
// =========================

BL_API_IMPL BLHitTest bl_path_hit_test(const BLPathCore* self, const BLPoint* p_, BLFillRule fill_rule) noexcept {
  using namespace bl::PathInternal;
  BL_ASSERT(self->_d.is_path());

  BLPathPrivateImpl* self_impl = get_impl(self);
  size_t i = self_impl->size;

  if (!i)
    return BL_HIT_TEST_OUT;

  const uint8_t* cmd_data = self_impl->command_data;
  const BLPoint* vtx_data = self_impl->vertex_data;

  bool has_move_to = false;
  BLPoint start {};
  BLPoint pt(*p_);

  double x0, y0;
  double x1, y1;

  intptr_t winding_number = 0;

  // 10 points - maximum for cubic spline having 3 cubics (1 + 3 + 3 + 3).
  BLPoint spline_data[10];

  do {
    switch (cmd_data[0]) {
      case BL_PATH_CMD_MOVE: {
        if (has_move_to) {
          x0 = vtx_data[-1].x;
          y0 = vtx_data[-1].y;
          x1 = start.x;
          y1 = start.y;

          has_move_to = false;
          goto OnLine;
        }

        start = vtx_data[0];

        cmd_data++;
        vtx_data++;
        i--;

        has_move_to = true;
        break;
      }

      case BL_PATH_CMD_ON: {
        if (BL_UNLIKELY(!has_move_to))
          return BL_HIT_TEST_INVALID;

        x0 = vtx_data[-1].x;
        y0 = vtx_data[-1].y;
        x1 = vtx_data[0].x;
        y1 = vtx_data[0].y;

        cmd_data++;
        vtx_data++;
        i--;

OnLine:
        {
          double dx = x1 - x0;
          double dy = y1 - y0;

          if (dy > 0.0) {
            if (pt.y >= y0 && pt.y < y1) {
              double ix = x0 + (pt.y - y0) * dx / dy;
              winding_number += (pt.x >= ix);
            }
          }
          else if (dy < 0.0) {
            if (pt.y >= y1 && pt.y < y0) {
              double ix = x0 + (pt.y - y0) * dx / dy;
              winding_number -= (pt.x >= ix);
            }
          }
        }
        break;
      }

      case BL_PATH_CMD_QUAD: {
        if (BL_UNLIKELY(!has_move_to || i < 2))
          return BL_HIT_TEST_INVALID;

        const BLPoint* p = vtx_data - 1;

        double min_y = bl_min(p[0].y, p[1].y, p[2].y);
        double max_y = bl_max(p[0].y, p[1].y, p[2].y);

        cmd_data += 2;
        vtx_data += 2;
        i -= 2;

        if (pt.y >= min_y && pt.y <= max_y) {
          if (unsigned(bl::Math::is_near(p[0].y, p[1].y)) &
              unsigned(bl::Math::is_near(p[1].y, p[2].y))) {
            x0 = p[0].x;
            y0 = p[0].y;
            x1 = p[2].x;
            y1 = p[2].y;
            goto OnLine;
          }

          // Subdivide to a quad spline at Y-extrema.
          const BLPoint* spline_ptr = p;
          const BLPoint* spline_end = bl::Geometry::split_with_options<bl::Geometry::QuadSplitOptions::kExtremaY>(bl::Geometry::quad_ref(p), spline_data);

          if (spline_end == spline_data)
            spline_end = vtx_data - 1;
          else
            spline_ptr = spline_data;

          do {
            min_y = bl_min(spline_ptr[0].y, spline_ptr[2].y);
            max_y = bl_max(spline_ptr[0].y, spline_ptr[2].y);

            if (pt.y >= min_y && pt.y < max_y) {
              int dir = 0;
              if (spline_ptr[0].y < spline_ptr[2].y)
                dir = 1;
              else if (spline_ptr[0].y > spline_ptr[2].y)
                dir = -1;

              // It should be only possible to have zero or one solution.
              double ti[2];
              double ix;

              bl::Geometry::QuadCoefficients qc = bl::Geometry::coefficients_of(bl::Geometry::quad_ref(spline_ptr));

              // { At^2 + Bt + C } -> { (At + B)t + C }
              if (bl::Math::quad_roots(ti, qc.a.y, qc.b.y, qc.c.y - pt.y, bl::Math::kAfter0, bl::Math::kBefore1) >= 1) {
                ix = (qc.a.x * ti[0] + qc.b.x) * ti[0] + qc.c.x;
              }
              else if (pt.y - min_y < max_y - pt.y) {
                ix = p[0].x;
              }
              else {
                ix = p[2].x;
              }

              if (pt.x >= ix)
                winding_number += dir;
            }
          } while ((spline_ptr += 2) != spline_end);
        }
        break;
      }

      case BL_PATH_CMD_CUBIC: {
        if (BL_UNLIKELY(!has_move_to || i < 3))
          return BL_HIT_TEST_INVALID;

        const BLPoint* p = vtx_data - 1;

        double min_y = bl_min(p[0].y, p[1].y, p[2].y, p[3].y);
        double max_y = bl_max(p[0].y, p[1].y, p[2].y, p[3].y);

        cmd_data += 3;
        vtx_data += 3;
        i -= 3;

        if (pt.y >= min_y && pt.y <= max_y) {
          if (unsigned(bl::Math::is_near(p[0].y, p[1].y)) &
              unsigned(bl::Math::is_near(p[1].y, p[2].y)) &
              unsigned(bl::Math::is_near(p[2].y, p[3].y))) {
            x0 = p[0].x;
            y0 = p[0].y;
            x1 = p[3].x;
            y1 = p[3].y;
            goto OnLine;
          }

          // Subdivide to a cubic spline at Y-extrema.
          const BLPoint* spline_ptr = p;
          const BLPoint* spline_end = bl::Geometry::split_cubic_to_spline<bl::Geometry::CubicSplitOptions::kExtremaY>(bl::Geometry::cubic_ref(p), spline_data);

          if (spline_end == spline_data)
            spline_end = vtx_data - 1;
          else
            spline_ptr = spline_data;

          do {
            min_y = bl_min(spline_ptr[0].y, spline_ptr[3].y);
            max_y = bl_max(spline_ptr[0].y, spline_ptr[3].y);

            if (pt.y >= min_y && pt.y < max_y) {
              int dir = 0;
              if (spline_ptr[0].y < spline_ptr[3].y)
                dir = 1;
              else if (spline_ptr[0].y > spline_ptr[3].y)
                dir = -1;

              // It should be only possible to have zero or one solution.
              double ti[3];
              double ix;

              bl::Geometry::CubicCoefficients cc = bl::Geometry::coefficients_of(bl::Geometry::cubic_ref(spline_ptr));

              // { At^3 + Bt^2 + Ct + D } -> { ((At + B)t + C)t + D }
              if (bl::Math::cubic_roots(ti, cc.a.y, cc.b.y, cc.c.y, cc.d.y - pt.y, bl::Math::kAfter0, bl::Math::kBefore1) >= 1) {
                ix = ((cc.a.x * ti[0] + cc.b.x) * ti[0] + cc.c.x) * ti[0] + cc.d.x;
              }
              else if (pt.y - min_y < max_y - pt.y) {
                ix = spline_ptr[0].x;
              }
              else {
                ix = spline_ptr[3].x;
              }

              if (pt.x >= ix) {
                winding_number += dir;
              }
            }
          } while ((spline_ptr += 3) != spline_end);
        }
        break;
      }

      case BL_PATH_CMD_CLOSE: {
        if (has_move_to) {
          x0 = vtx_data[-1].x;
          y0 = vtx_data[-1].y;
          x1 = start.x;
          y1 = start.y;

          has_move_to = false;
          goto OnLine;
        }

        cmd_data++;
        vtx_data++;

        i--;
        break;
      }

      default:
        return BL_HIT_TEST_INVALID;
    }
  } while (i);

  // Close the path.
  if (has_move_to) {
    x0 = vtx_data[-1].x;
    y0 = vtx_data[-1].y;
    x1 = start.x;
    y1 = start.y;

    has_move_to = false;
    goto OnLine;
  }

  if (fill_rule == BL_FILL_RULE_EVEN_ODD)
    winding_number &= 1;

  return winding_number != 0 ? BL_HIT_TEST_IN : BL_HIT_TEST_OUT;
}

// bl::Path - Runtime Registration
// ===============================

void bl_path_rt_init(BLRuntimeContext* rt) noexcept {
  bl_unused(rt);

  bl::PathInternal::default_path.impl->flags = BL_PATH_FLAG_EMPTY;

  bl_object_defaults[BL_OBJECT_TYPE_PATH]._d.init_dynamic(
    BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_PATH),
    &bl::PathInternal::default_path.impl);
}
