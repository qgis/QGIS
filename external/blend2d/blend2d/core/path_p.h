// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PATH_P_H_INCLUDED
#define BLEND2D_PATH_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/path.h>
#include <blend2d/support/math_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

//! \name BLPath - Private Structs
//! \{

//! Private implementation that extends \ref BLPathImpl.
struct BLPathPrivateImpl : public BLPathImpl {
  BLBox control_box;
  BLBox bounding_box;
};

//! \}

namespace bl {
namespace PathInternal {

//! \name BLPath - Internals - Common Functionality (Container)
//! \{

static BL_INLINE constexpr size_t capacity_from_impl_size(BLObjectImplSize impl_size) noexcept {
  return (impl_size.value() - sizeof(BLPathPrivateImpl)) / (sizeof(BLPoint) + 1);
}

static BL_INLINE constexpr BLObjectImplSize impl_size_from_capacity(size_t capacity) noexcept {
  return BLObjectImplSize(sizeof(BLPathPrivateImpl) + capacity * (sizeof(BLPoint) + 1));
}

//! \}

//! \name BLPath - Internals - Common Functionality (Impl)
//! \{

static BL_INLINE bool is_impl_mutable(BLPathImpl* impl) noexcept {
  return ObjectInternal::is_impl_mutable(impl);
}

static BL_INLINE BLResult free_impl(BLPathPrivateImpl* impl) noexcept {
  return ObjectInternal::free_impl(impl);
}

template<RCMode kRCMode>
static BL_INLINE BLResult release_impl(BLPathPrivateImpl* impl) noexcept {
  return ObjectInternal::deref_impl_and_test<kRCMode>(impl) ? free_impl(impl) : BLResult(BL_SUCCESS);
}

//! \}

//! \name BLPath - Internals - Common Functionality (Instance)
//! \{

static BL_INLINE BLPathPrivateImpl* get_impl(const BLPathCore* self) noexcept {
  return static_cast<BLPathPrivateImpl*>(self->_d.impl);
}

static BL_INLINE BLResult retain_instance(const BLPathCore* self, size_t n = 1) noexcept {
  return ObjectInternal::retain_instance(self, n);
}

static BL_INLINE BLResult release_instance(BLPathCore* self) noexcept {
  return release_impl<RCMode::kMaybe>(get_impl(self));
}

static BL_INLINE BLResult replace_instance(BLPathCore* self, const BLPathCore* other) noexcept {
  BLPathPrivateImpl* impl = get_impl(self);
  self->_d = other->_d;
  return release_impl<RCMode::kMaybe>(impl);
}

//! \}

//! \name BLPath - Internals - Other
//! \{

static BL_INLINE constexpr BLApproximationOptions make_default_approximation_options() noexcept {
  return BLApproximationOptions {
    BL_FLATTEN_MODE_DEFAULT, // Default flattening mode.
    BL_OFFSET_MODE_DEFAULT,  // Default offset mode.
    { 0, 0, 0, 0, 0, 0 },    // Reserved.
    0.20,                    // Default curve flattening tolerance.
    0.05,                    // Default curve simplification tolerance.
    0.414213562              // Default offset parameter.
  };
}

//! \}

} // {PathPrivate}

//! \name BLPath Iterator
//! \{

//! Path iterator that can iterate over RAW data.
struct PathIterator {
  const uint8_t* cmd;
  const uint8_t* end;
  const BLPoint* vtx;

  BL_INLINE PathIterator() noexcept = default;
  BL_INLINE PathIterator(const BLPathView& view) noexcept { reset(view); }
  BL_INLINE PathIterator(const uint8_t* cmd_, const BLPoint* vtx_, size_t n) noexcept { reset(cmd_, vtx_, n); }

  BL_INLINE PathIterator operator++(int) noexcept { PathIterator out(*this); cmd++; vtx++; return out; }
  BL_INLINE PathIterator operator--(int) noexcept { PathIterator out(*this); cmd--; vtx--; return out; }

  BL_INLINE PathIterator& operator++() noexcept { cmd++; vtx++; return *this; }
  BL_INLINE PathIterator& operator--() noexcept { cmd--; vtx--; return *this; }

  BL_INLINE PathIterator& operator+=(size_t n) noexcept { cmd += n; vtx += n; return *this; }
  BL_INLINE PathIterator& operator-=(size_t n) noexcept { cmd -= n; vtx -= n; return *this; }

  BL_INLINE bool at_end() const noexcept { return cmd == end; }
  BL_INLINE bool after_end() const noexcept { return cmd > end; }
  BL_INLINE bool before_end() const noexcept { return cmd < end; }

  BL_INLINE size_t remaining_forward() const noexcept { return (size_t)(end - cmd); }
  BL_INLINE size_t remaining_backward() const noexcept { return (size_t)(cmd - end); }

  BL_INLINE void reset(const BLPathView& view) noexcept {
    reset(view.command_data, view.vertex_data, view.size);
  }

  BL_INLINE void reset(const uint8_t* cmd_, const BLPoint* vtx_, size_t n) noexcept {
    cmd = cmd_;
    end = cmd_ + n;
    vtx = vtx_;
  }

  BL_INLINE void reverse() noexcept {
    intptr_t n = intptr_t(remaining_forward()) - 1;

    end = cmd - 1;
    cmd += n;
    vtx += n;
  }
};

//! \}

//! \name BLPath Appender
//! \{

//! Low-level interface that can be used to append vertices & commands to an existing path fast. The interface is
//! designed in a way that the user using it must reserve enough space and then call `append...()` functions that
//! can only be called when there is enough storage left for that command. The storage requirements are specified
//! by `begin()` or by `ensure()`. The latter is mostly used to reallocate the array in case more vertices are needed
//! than initially passed to `begin()`.
//!
//! When designing a loop the appender can be used in the following way, where `SourceT` is an iterable object that
//! can provide us some path vertices and commands:
//!
//! ```
//! template<typename Iterator>
//! BLResult append_to_path(BLPath& dst, const Iterator& iter) noexcept {
//!   bl::PathAppender appender;
//!   BL_PROPAGATE(appender.begin_assign(dst, 32));
//!
//!   while (!iter.end()) {
//!     // Maximum number of vertices required by move_to/line_to/quad_to/cubic_to.
//!     BL_PROPAGATE(appender.ensure(dst, 3));
//!
//!     switch (iter.command()) {
//!       case BL_PATH_CMD_MOVE : appender.move_to(iter[0]); break;
//!       case BL_PATH_CMD_ON   : appender.line_to(iter[0]); break;
//!       case BL_PATH_CMD_QUAD : appender.quad_to(iter[0], iter[1]); break;
//!       case BL_PATH_CMD_CUBIC: appender.cubic_to(iter[0], iter[1], iter[2]); break;
//!       case BL_PATH_CMD_CLOSE: appender.close(); break;
//!     }
//!
//!     iter.advance();
//!   }
//!
//!   appender.done(dst);
//!   return BL_SUCCESS;
//! }
//! ```
class PathAppender {
public:
  // Internal struct that represents a single command and prevents compile thinking that it may alias.
  struct Cmd { uint8_t value; };

  Cmd* cmd;
  Cmd* end;
  BLPoint* vtx;

  BL_INLINE PathAppender() noexcept
    : cmd(nullptr) {}

  BL_INLINE void reset() noexcept { cmd = nullptr; }
  BL_INLINE bool is_empty() const noexcept { return cmd == nullptr; }
  BL_INLINE size_t remaining_size() const noexcept { return (size_t)(end - cmd); }

  BL_INLINE size_t current_index(const BLPath& dst) const noexcept {
    return (size_t)(cmd - reinterpret_cast<Cmd*>(PathInternal::get_impl(&dst)->command_data));
  }

  BL_INLINE void _advance(size_t n) noexcept {
    BL_ASSERT(remaining_size() >= n);

    cmd += n;
    vtx += n;
  }

  BL_INLINE BLResult begin(BLPathCore* dst, BLModifyOp op, size_t n) noexcept {
    BLPoint* vtx_ptr_local;
    uint8_t* cmd_ptr_local;
    BL_PROPAGATE(bl_path_modify_op(dst, op, n, &cmd_ptr_local, &vtx_ptr_local));

    BLPathImpl* dst_impl = PathInternal::get_impl(dst);
    vtx = vtx_ptr_local;
    cmd = reinterpret_cast<Cmd*>(cmd_ptr_local);
    end = reinterpret_cast<Cmd*>(dst_impl->command_data + dst_impl->capacity);

    BL_ASSERT(remaining_size() >= n);
    return BL_SUCCESS;
  }

  BL_INLINE BLResult begin_assign(BLPathCore* dst, size_t n) noexcept { return begin(dst, BL_MODIFY_OP_ASSIGN_GROW, n); }
  BL_INLINE BLResult begin_append(BLPathCore* dst, size_t n) noexcept { return begin(dst, BL_MODIFY_OP_APPEND_GROW, n); }

  BL_INLINE BLResult ensure(BLPathCore* dst, size_t n) noexcept {
    if (BL_LIKELY(remaining_size() >= n))
      return BL_SUCCESS;

    BLPathImpl* dst_impl = PathInternal::get_impl(dst);

    dst_impl->size = (size_t)(reinterpret_cast<uint8_t*>(cmd) - dst_impl->command_data);
    BL_ASSERT(dst_impl->size <= dst_impl->capacity);

    uint8_t* cmd_ptr_local;
    BLPoint* vtx_ptr_local;
    BL_PROPAGATE(bl_path_modify_op(dst, BL_MODIFY_OP_APPEND_GROW, n, &cmd_ptr_local, &vtx_ptr_local));

    dst_impl = PathInternal::get_impl(dst);
    vtx = vtx_ptr_local;
    cmd = reinterpret_cast<Cmd*>(cmd_ptr_local);
    end = reinterpret_cast<Cmd*>(dst_impl->command_data + dst_impl->capacity);

    BL_ASSERT(remaining_size() >= n);
    return BL_SUCCESS;
  }

  BL_INLINE void back(size_t n = 1) noexcept {
    cmd -= n;
    vtx -= n;
  }

  BL_INLINE void sync(BLPathCore* dst) noexcept {
    BLPathImpl* dst_impl = PathInternal::get_impl(dst);
    size_t new_size = (size_t)(reinterpret_cast<uint8_t*>(cmd) - dst_impl->command_data);

    BL_ASSERT(!is_empty());
    BL_ASSERT(new_size <= dst_impl->capacity);

    dst_impl->size = new_size;
  }

  BL_INLINE void done(BLPathCore* dst) noexcept {
    sync(dst);
    reset();
  }

  BL_INLINE void move_to(const BLPoint& p0) noexcept { move_to(p0.x, p0.y); }
  BL_INLINE void move_to(const BLPointI& p0) noexcept { move_to(p0.x, p0.y); }

  BL_INLINE void move_to(double x0, double y0) noexcept {
    BL_ASSERT(remaining_size() >= 1);

    cmd[0].value = BL_PATH_CMD_MOVE;
    vtx[0].reset(x0, y0);

    cmd++;
    vtx++;
  }

  BL_INLINE void line_to(const BLPoint& p1) noexcept { line_to(p1.x, p1.y); }
  BL_INLINE void line_to(const BLPointI& p1) noexcept { line_to(p1.x, p1.y); }

  BL_INLINE void line_to(double x1, double y1) noexcept {
    BL_ASSERT(remaining_size() >= 1);

    cmd[0].value = BL_PATH_CMD_ON;
    vtx[0].reset(x1, y1);

    cmd++;
    vtx++;
  }

  BL_INLINE void quad_to(const BLPoint& p1, const BLPoint& p2) noexcept {
    quad_to(p1.x, p1.y, p2.x, p2.y);
  }

  BL_INLINE void quad_to(double x1, double y1, double x2, double y2) noexcept {
    BL_ASSERT(remaining_size() >= 2);

    cmd[0].value = BL_PATH_CMD_QUAD;
    cmd[1].value = BL_PATH_CMD_ON;
    vtx[0].reset(x1, y1);
    vtx[1].reset(x2, y2);

    cmd += 2;
    vtx += 2;
  }

  BL_INLINE void conic_to(const BLPoint& p1, const BLPoint& p2, double w) noexcept {
    BL_ASSERT(remaining_size() >= 3);
    double k = 4.0 * w / (3.0 * (1.0 + w));

    cmd[0].value = BL_PATH_CMD_CUBIC;
    cmd[1].value = BL_PATH_CMD_CUBIC;
    cmd[2].value = BL_PATH_CMD_ON;

    BLPoint p0 = vtx[-1];
    vtx[0] = p0 + (p1 - p0) * k;
    vtx[1] = p2 + (p1 - p2) * k;
    vtx[2] = p2;

    cmd += 3;
    vtx += 3;
  }

/*
  // TODO [Conic]: Use this instead of cubic approximation
  BL_INLINE void conic_to(const BLPoint& p1, const BLPoint& p2, double w) noexcept {
    quad_to(p1.x, p1.y, p2.x, p2.y);
  }

  BL_INLINE void conic_to(double x1, double y1, double x2, double y2, double w) noexcept {
    BL_ASSERT(remaining_size() >= 3);

    cmd[0].value = BL_PATH_CMD_CONIC;
    cmd[1].value = BL_PATH_CMD_WEIGHT;
    cmd[2].value = BL_PATH_CMD_ON;
    vtx[0].reset(x1, y1);
    vtx[1].reset(w, bl::Math::nan<double>());
    vtx[2].reset(x2, y2);

    cmd += 3;
    vtx += 3;
  }
*/
  BL_INLINE void cubic_to(const BLPoint& p1, const BLPoint& p2, const BLPoint& p3) noexcept {
    return cubic_to(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
  }

  BL_INLINE void cubic_to(double x1, double y1, double x2, double y2, double x3, double y3) noexcept {
    BL_ASSERT(remaining_size() >= 3);

    cmd[0].value = BL_PATH_CMD_CUBIC;
    cmd[1].value = BL_PATH_CMD_CUBIC;
    cmd[2].value = BL_PATH_CMD_ON;
    vtx[0].reset(x1, y1);
    vtx[1].reset(x2, y2);
    vtx[2].reset(x3, y3);

    cmd += 3;
    vtx += 3;
  }

  BL_INLINE void arc_quadrant_to(const BLPoint& p1, const BLPoint& p2) noexcept {
    BL_ASSERT(remaining_size() >= 3);

    cmd[0].value = BL_PATH_CMD_CUBIC;
    cmd[1].value = BL_PATH_CMD_CUBIC;
    cmd[2].value = BL_PATH_CMD_ON;

    BLPoint p0 = vtx[-1];
    vtx[0] = p0 + (p1 - p0) * Math::kKAPPA;
    vtx[1] = p2 + (p1 - p2) * Math::kKAPPA;
    vtx[2] = p2;

    cmd += 3;
    vtx += 3;
  }

  BL_INLINE void add_vertex(uint8_t cmd_, const BLPoint& p) noexcept {
    BL_ASSERT(remaining_size() >= 1);

    cmd[0].value = cmd_;
    vtx[0] = p;

    cmd++;
    vtx++;
  }

  BL_INLINE void add_vertex(uint8_t cmd_, double x, double y) noexcept {
    BL_ASSERT(remaining_size() >= 1);

    cmd[0].value = cmd_;
    vtx[0].reset(x, y);

    cmd++;
    vtx++;
  }

  BL_INLINE void close() noexcept {
    BL_ASSERT(remaining_size() >= 1);

    cmd[0].value = BL_PATH_CMD_CLOSE;
    vtx[0].reset(Math::nan<double>(), Math::nan<double>());

    cmd++;
    vtx++;
  }

  BL_INLINE void add_box(double x0, double y0, double x1, double y1, BLGeometryDirection dir) noexcept {
    BL_ASSERT(remaining_size() >= 5);

    cmd[0].value = BL_PATH_CMD_MOVE;
    cmd[1].value = BL_PATH_CMD_ON;
    cmd[2].value = BL_PATH_CMD_ON;
    cmd[3].value = BL_PATH_CMD_ON;
    cmd[4].value = BL_PATH_CMD_CLOSE;

    vtx[0].reset(x0, y0);
    vtx[1].reset(x1, y0);
    vtx[2].reset(x1, y1);
    vtx[3].reset(x0, y1);
    vtx[4].reset(Math::nan<double>(), Math::nan<double>());

    if (dir != BL_GEOMETRY_DIRECTION_CW) {
      vtx[1].reset(x0, y1);
      vtx[3].reset(x1, y0);
    }

    cmd += 5;
    vtx += 5;
  }

  BL_INLINE void addBoxCW(double x0, double y0, double x1, double y1) noexcept {
    add_box(x0, y0, x1, y1, BL_GEOMETRY_DIRECTION_CW);
  }

  BL_INLINE void addBoxCCW(double x0, double y0, double x1, double y1) noexcept {
    add_box(x0, y0, x1, y1, BL_GEOMETRY_DIRECTION_CCW);
  }
};

//! \}

} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_PATH_P_H_INCLUDED
