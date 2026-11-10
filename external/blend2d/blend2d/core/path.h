// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PATH_H_INCLUDED
#define BLEND2D_PATH_H_INCLUDED

#include <blend2d/core/array.h>
#include <blend2d/core/geometry.h>
#include <blend2d/core/object.h>

//! \addtogroup bl_geometry
//! \{

//! \name BLPath - Constants
//! \{

//! Path command.
BL_DEFINE_ENUM(BLPathCmd) {
  //! Move-to command (starts a new figure).
  BL_PATH_CMD_MOVE = 0,
  //! On-path command (interpreted as line-to or the end of a curve).
  BL_PATH_CMD_ON = 1,
  //! Quad-to control point.
  BL_PATH_CMD_QUAD = 2,
  //! Conic-to control point
  BL_PATH_CMD_CONIC = 3,
  //! Cubic-to control point (always used as a pair of commands).
  BL_PATH_CMD_CUBIC = 4,
  //! Close path.
  BL_PATH_CMD_CLOSE = 5,

  //! Conic weight.
  //!
  //! \note This is not a point. This is a pair of values from which only the first (x) is used to represent weight
  //! as used by conic curve. The other value (y) is always set to NaN by Blend2D, but can be arbitrary as it has
  //! no meaning.
  BL_PATH_CMD_WEIGHT = 6,

  //! Maximum value of `BLPathCmd`.
  BL_PATH_CMD_MAX_VALUE = 6

  BL_FORCE_ENUM_UINT32(BL_PATH_CMD)
};

//! Path command (never stored in path).
BL_DEFINE_ENUM(BLPathCmdExtra) {
  //! Used by `BLPath::set_vertex_at` to preserve the current command value.
  BL_PATH_CMD_PRESERVE = 0xFFFFFFFFu
};

//! Path flags.
BL_DEFINE_ENUM(BLPathFlags) {
  //! No flags.
  BL_PATH_NO_FLAGS = 0u,
  //! Path is empty (no commands or close commands only).
  BL_PATH_FLAG_EMPTY = 0x00000001u,
  //! Path contains multiple figures.
  BL_PATH_FLAG_MULTIPLE = 0x00000002u,
  //! Path contains one or more quad curves.
  BL_PATH_FLAG_QUADS = 0x00000004u,
  //! Path contains one or more conic curves.
  BL_PATH_FLAG_CONICS = 0x00000008u,
  //! Path contains one or more cubic curves.
  BL_PATH_FLAG_CUBICS = 0x00000010u,
  //! Path is invalid.
  BL_PATH_FLAG_INVALID = 0x40000000u,
  //! Flags are dirty (not reflecting the current status).
  BL_PATH_FLAG_DIRTY = 0x80000000u

  BL_FORCE_ENUM_UINT32(BL_PATH_FLAG)
};

//! Path reversal mode.
BL_DEFINE_ENUM(BLPathReverseMode) {
  //! Reverse each figure and their order as well (default).
  BL_PATH_REVERSE_MODE_COMPLETE = 0,
  //! Reverse each figure separately (keeps their order).
  BL_PATH_REVERSE_MODE_SEPARATE = 1,

  //! Maximum value of `BLPathReverseMode`.
  BL_PATH_REVERSE_MODE_MAX_VALUE = 1

  BL_FORCE_ENUM_UINT32(BL_PATH_REVERSE_MODE)
};

//! Stroke join type.
BL_DEFINE_ENUM(BLStrokeJoin) {
  //! Miter-join possibly clipped at `miter_limit` [default].
  BL_STROKE_JOIN_MITER_CLIP = 0,
  //! Miter-join or bevel-join depending on miter_limit condition.
  BL_STROKE_JOIN_MITER_BEVEL = 1,
  //! Miter-join or round-join depending on miter_limit condition.
  BL_STROKE_JOIN_MITER_ROUND = 2,
  //! Bevel-join.
  BL_STROKE_JOIN_BEVEL = 3,
  //! Round-join.
  BL_STROKE_JOIN_ROUND = 4,

  //! Maximum value of `BLStrokeJoin`.
  BL_STROKE_JOIN_MAX_VALUE = 4

  BL_FORCE_ENUM_UINT32(BL_STROKE_JOIN)
};

//! Position of a stroke-cap.
BL_DEFINE_ENUM(BLStrokeCapPosition) {
  //! Start of the path.
  BL_STROKE_CAP_POSITION_START = 0,
  //! End of the path.
  BL_STROKE_CAP_POSITION_END = 1,

  //! Maximum value of `BLStrokeCapPosition`.
  BL_STROKE_CAP_POSITION_MAX_VALUE = 1

  BL_FORCE_ENUM_UINT32(BL_STROKE_CAP_POSITION)
};

//! A presentation attribute defining the shape to be used at the end of open sub-paths.
BL_DEFINE_ENUM(BLStrokeCap) {
  //! Butt cap [default].
  BL_STROKE_CAP_BUTT = 0,
  //! Square cap.
  BL_STROKE_CAP_SQUARE = 1,
  //! Round cap.
  BL_STROKE_CAP_ROUND = 2,
  //! Round cap reversed.
  BL_STROKE_CAP_ROUND_REV = 3,
  //! Triangle cap.
  BL_STROKE_CAP_TRIANGLE = 4,
  //! Triangle cap reversed.
  BL_STROKE_CAP_TRIANGLE_REV = 5,

  //! Maximum value of `BLStrokeCap`.
  BL_STROKE_CAP_MAX_VALUE = 5

  BL_FORCE_ENUM_UINT32(BL_STROKE_CAP)
};

//! Stroke transform order.
BL_DEFINE_ENUM(BLStrokeTransformOrder) {
  //! Transform after stroke  => `Transform(Stroke(Input))` [default].
  BL_STROKE_TRANSFORM_ORDER_AFTER = 0,
  //! Transform before stroke => `Stroke(Transform(Input))`.
  BL_STROKE_TRANSFORM_ORDER_BEFORE = 1,

  //! Maximum value of `BLStrokeTransformOrder`.
  BL_STROKE_TRANSFORM_ORDER_MAX_VALUE = 1

  BL_FORCE_ENUM_UINT32(BL_STROKE_TRANSFORM_ORDER)
};

//! Mode that specifies how curves are approximated to line segments.
BL_DEFINE_ENUM(BLFlattenMode) {
  //! Use default mode (decided by Blend2D).
  BL_FLATTEN_MODE_DEFAULT = 0,
  //! Recursive subdivision flattening.
  BL_FLATTEN_MODE_RECURSIVE = 1,

  //! Maximum value of `BLFlattenMode`.
  BL_FLATTEN_MODE_MAX_VALUE = 1

  BL_FORCE_ENUM_UINT32(BL_FLATTEN_MODE)
};

//! Mode that specifies how to construct offset curves.
BL_DEFINE_ENUM(BLOffsetMode) {
  //! Use default mode (decided by Blend2D).
  BL_OFFSET_MODE_DEFAULT = 0,
  //! Iterative offset construction.
  BL_OFFSET_MODE_ITERATIVE = 1,

  //! Maximum value of `BLOffsetMode`.
  BL_OFFSET_MODE_MAX_VALUE = 1

  BL_FORCE_ENUM_UINT32(BL_OFFSET_MODE)
};

//! \}

//! \name BLPath - Structs
//! \{

//! Options used to describe how geometry is approximated.
//!
//! This struct cannot be simply zeroed and then passed to functions that accept approximation options.
//! Use `bl_default_approximation_options` to setup defaults and then alter values you want to change.
//!
//! Example of using `BLApproximationOptions`:
//!
//! ```
//! // Initialize with defaults first.
//! BLApproximationOptions approx = bl_default_approximation_options;
//!
//! // Override values you want to change.
//! approx.simplify_tolerance = 0.02;
//!
//! // ... now safely use approximation options in your code ...
//! ```
struct BLApproximationOptions {
  //! Specifies how curves are flattened, see \ref BLFlattenMode.
  uint8_t flatten_mode;
  //! Specifies how curves are offsetted (used by stroking), see \ref BLOffsetMode.
  uint8_t offset_mode;
  //! Reserved for future use, must be zero.
  uint8_t reserved_flags[6];

  //! Tolerance used to flatten curves.
  double flatten_tolerance;
  //! Tolerance used to approximate cubic curves with quadratic curves.
  double simplify_tolerance;
  //! Curve offsetting parameter, exact meaning depends on `offset_mode`.
  double offset_parameter;
};

//! 2D vector path view provides pointers to vertex and command data along with their size.
struct BLPathView {
  const uint8_t* command_data;
  const BLPoint* vertex_data;
  size_t size;

#ifdef __cplusplus
  BL_INLINE_NODEBUG void reset() noexcept { *this = BLPathView{}; }

  BL_INLINE_NODEBUG void reset(const uint8_t* command_data_in, const BLPoint* vertex_data_in, size_t size_in) noexcept {
    command_data = command_data_in;
    vertex_data = vertex_data_in;
    size = size_in;
  }
#endif
};

//! \}

//! \name BLPath - Sinks
//! \{

//! Optional callback that can be used to consume a path data.
typedef BLResult (BL_CDECL* BLPathSinkFunc)(BLPathCore* path, const void* info, void* user_data) BL_NOEXCEPT_C;

//! This is a sink that is used by path offsetting. This sink consumes both `a` and `b` offsets of the path. The sink
//! will be called for each figure and is responsible for joining these paths. If the paths are not closed then the
//! sink must insert start cap, then join `b`, and then insert end cap.
//!
//! The sink must also clean up the paths as this is not done by the offsetter. The reason is that in case the `a` path
//! is the output path you can just keep it and insert `b` path into it (clearing only `b` path after each call).
typedef BLResult (BL_CDECL* BLPathStrokeSinkFunc)(BLPathCore* a, BLPathCore* b, BLPathCore* c, size_t input_start, size_t input_end, void* user_data) BL_NOEXCEPT_C;

//! \}

//! \name BLPath - Globals
//!
//! 2D path functionality is provided by \ref BLPathCore in C API and wrapped by \ref BLPath in C++ API.
//!
//! \{
BL_BEGIN_C_DECLS

//! Default approximation options used by Blend2D.
extern BL_API const BLApproximationOptions bl_default_approximation_options;

BL_END_C_DECLS

//! \}
//! \}

//! \addtogroup bl_c_api
//! \{

//! \name BLPath - C API
//! \{

//! 2D vector path [C API].
struct BLPathCore BL_CLASS_INHERITS(BLObjectCore) {
  BL_DEFINE_OBJECT_DETAIL
  BL_DEFINE_OBJECT_DCAST(BLPath)
};

BL_BEGIN_C_DECLS

BL_API BLResult BL_CDECL bl_path_init(BLPathCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_init_move(BLPathCore* self, BLPathCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_init_weak(BLPathCore* self, const BLPathCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_destroy(BLPathCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_reset(BLPathCore* self) BL_NOEXCEPT_C;
BL_API size_t BL_CDECL bl_path_get_size(const BLPathCore* self) BL_NOEXCEPT_C BL_PURE;
BL_API size_t BL_CDECL bl_path_get_capacity(const BLPathCore* self) BL_NOEXCEPT_C BL_PURE;
BL_API const uint8_t* BL_CDECL bl_path_get_command_data(const BLPathCore* self) BL_NOEXCEPT_C BL_PURE;
BL_API const BLPoint* BL_CDECL bl_path_get_vertex_data(const BLPathCore* self) BL_NOEXCEPT_C BL_PURE;
BL_API BLResult BL_CDECL bl_path_clear(BLPathCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_shrink(BLPathCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_reserve(BLPathCore* self, size_t n) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_modify_op(BLPathCore* self, BLModifyOp op, size_t n, uint8_t** cmd_data_out, BLPoint** vtx_data_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_assign_move(BLPathCore* self, BLPathCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_assign_weak(BLPathCore* self, const BLPathCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_assign_deep(BLPathCore* self, const BLPathCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_set_vertex_at(BLPathCore* self, size_t index, uint32_t cmd, double x, double y) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_move_to(BLPathCore* self, double x0, double y0) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_line_to(BLPathCore* self, double x1, double y1) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_poly_to(BLPathCore* self, const BLPoint* poly, size_t count) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_quad_to(BLPathCore* self, double x1, double y1, double x2, double y2) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_conic_to(BLPathCore* self, double x1, double y1, double x2, double y2, double w) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_cubic_to(BLPathCore* self, double x1, double y1, double x2, double y2, double x3, double y3) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_smooth_quad_to(BLPathCore* self, double x2, double y2) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_smooth_cubic_to(BLPathCore* self, double x2, double y2, double x3, double y3) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_arc_to(BLPathCore* self, double x, double y, double rx, double ry, double start, double sweep, bool force_move_to) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_arc_quadrant_to(BLPathCore* self, double x1, double y1, double x2, double y2) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_elliptic_arc_to(BLPathCore* self, double rx, double ry, double xAxisRotation, bool large_arc_flag, bool sweep_flag, double x1, double y1) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_close(BLPathCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_add_geometry(BLPathCore* self, BLGeometryType geometry_type, const void* geometry_data, const BLMatrix2D* m, BLGeometryDirection dir) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_add_box_i(BLPathCore* self, const BLBoxI* box, BLGeometryDirection dir) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_add_box_d(BLPathCore* self, const BLBox* box, BLGeometryDirection dir) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_add_rect_i(BLPathCore* self, const BLRectI* rect, BLGeometryDirection dir) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_add_rect_d(BLPathCore* self, const BLRect* rect, BLGeometryDirection dir) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_add_path(BLPathCore* self, const BLPathCore* other, const BLRange* range) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_add_translated_path(BLPathCore* self, const BLPathCore* other, const BLRange* range, const BLPoint* p) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_add_transformed_path(BLPathCore* self, const BLPathCore* other, const BLRange* range, const BLMatrix2D* m) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_add_reversed_path(BLPathCore* self, const BLPathCore* other, const BLRange* range, BLPathReverseMode reverse_mode) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_add_stroked_path(BLPathCore* self, const BLPathCore* other, const BLRange* range, const BLStrokeOptionsCore* options, const BLApproximationOptions* approx) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_remove_range(BLPathCore* self, const BLRange* range) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_translate(BLPathCore* self, const BLRange* range, const BLPoint* p) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_transform(BLPathCore* self, const BLRange* range, const BLMatrix2D* m) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_fit_to(BLPathCore* self, const BLRange* range, const BLRect* rect, uint32_t fit_flags) BL_NOEXCEPT_C;
BL_API bool     BL_CDECL bl_path_equals(const BLPathCore* a, const BLPathCore* b) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_get_info_flags(const BLPathCore* self, uint32_t* flags_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_get_control_box(const BLPathCore* self, BLBox* box_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_get_bounding_box(const BLPathCore* self, BLBox* box_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_get_figure_range(const BLPathCore* self, size_t index, BLRange* range_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_get_last_vertex(const BLPathCore* self, BLPoint* vtx_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_path_get_closest_vertex(const BLPathCore* self, const BLPoint* p, double max_distance, size_t* index_out, double* distance_out) BL_NOEXCEPT_C;
BL_API BLHitTest BL_CDECL bl_path_hit_test(const BLPathCore* self, const BLPoint* p, BLFillRule fill_rule) BL_NOEXCEPT_C;

BL_END_C_DECLS

//! Stroke options [C API].
struct BLStrokeOptionsCore {
  union {
    struct {
      uint8_t start_cap;
      uint8_t end_cap;
      uint8_t join;
      uint8_t transform_order;
      uint8_t reserved[4];
    };
    uint8_t caps[BL_STROKE_CAP_POSITION_MAX_VALUE + 1];
    uint64_t hints;
  };

  double width;
  double miter_limit;
  double dash_offset;

#ifdef __cplusplus
  union { BLArray<double> dash_array; };
#else
  BLArrayCore dash_array;
#endif

#ifdef __cplusplus
  BL_INLINE_NODEBUG BLStrokeOptionsCore() noexcept {}
  BL_INLINE_NODEBUG BLStrokeOptionsCore(const BLStrokeOptionsCore& other) noexcept { _copy_from(other); }
  BL_INLINE_NODEBUG ~BLStrokeOptionsCore() noexcept {}
  BL_INLINE_NODEBUG BLStrokeOptionsCore& operator=(const BLStrokeOptionsCore& other) noexcept { _copy_from(other); return *this; }

  BL_INLINE void _copy_from(const BLStrokeOptionsCore& other) noexcept {
    hints = other.hints;
    width = other.width;
    miter_limit = other.miter_limit;
    dash_offset = other.dash_offset;
    dash_array._d = other.dash_array._d;
  }
#endif

  BL_DEFINE_OBJECT_DCAST(BLStrokeOptions)
};

BL_BEGIN_C_DECLS

BL_API BLResult BL_CDECL bl_stroke_options_init(BLStrokeOptionsCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_stroke_options_init_move(BLStrokeOptionsCore* self, BLStrokeOptionsCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_stroke_options_init_weak(BLStrokeOptionsCore* self, const BLStrokeOptionsCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_stroke_options_destroy(BLStrokeOptionsCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_stroke_options_reset(BLStrokeOptionsCore* self) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_stroke_options_equals(const BLStrokeOptionsCore* a, const BLStrokeOptionsCore* b) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_stroke_options_assign_move(BLStrokeOptionsCore* self, BLStrokeOptionsCore* other) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_stroke_options_assign_weak(BLStrokeOptionsCore* self, const BLStrokeOptionsCore* other) BL_NOEXCEPT_C;

BL_API BLResult BL_CDECL bl_path_stroke_to_sink(
  const BLPathCore* self,
  const BLRange* range,
  const BLStrokeOptionsCore* stroke_options,
  const BLApproximationOptions* approximation_options,
  BLPathCore *a,
  BLPathCore *b,
  BLPathCore *c,
  BLPathStrokeSinkFunc sink, void* user_data) BL_NOEXCEPT_C;

BL_END_C_DECLS

//! \}
//! \}

//! \addtogroup bl_geometry
//! \{

//! \cond INTERNAL
//! \name BLPath - Internals
//! \{

//! 2D vector path [Impl].
struct BLPathImpl BL_CLASS_INHERITS(BLObjectImpl) {
  //! Union of either raw path-data or their `view`.
  union {
    struct {
      //! Command data
      uint8_t* command_data;
      //! Vertex data.
      BLPoint* vertex_data;
      //! Vertex/command count.
      size_t size;
    };
    //! Path data as view.
    BLPathView view;
  };

  //! Path vertex/command capacity.
  size_t capacity;

  //! Path flags related to caching.
  uint32_t flags;
};

//! \}
//! \endcond

//! \name BLPath - C++ API
//! \{
#ifdef __cplusplus

// Prevents the following:
//   Base class XXX should be explicitly initialized in the copy constructor [-Wextra]
BL_DIAGNOSTIC_PUSH(BL_DIAGNOSTIC_NO_EXTRA_WARNINGS)

//! Stroke options [C++ API].
//!
//! You should use this as a structure and use members of `BLStrokeOptionsCore` directly.
class BLStrokeOptions final : public BLStrokeOptionsCore {
public:
  BL_INLINE BLStrokeOptions() noexcept { bl_stroke_options_init(this); }
  BL_INLINE BLStrokeOptions(BLStrokeOptions&& other) noexcept { bl_stroke_options_init_move(this, &other); }
  BL_INLINE BLStrokeOptions(const BLStrokeOptions& other) noexcept { bl_stroke_options_init_weak(this, &other); }
  BL_INLINE ~BLStrokeOptions() noexcept { bl_stroke_options_destroy(this); }

  BL_INLINE BLStrokeOptions& operator=(BLStrokeOptions&& other) noexcept { bl_stroke_options_assign_move(this, &other); return *this; }
  BL_INLINE BLStrokeOptions& operator=(const BLStrokeOptions& other) noexcept { bl_stroke_options_assign_weak(this, &other); return *this; }

  BL_INLINE bool operator==(const BLStrokeOptions& other) const noexcept { return  equals(other); }
  BL_INLINE bool operator!=(const BLStrokeOptions& other) const noexcept { return !equals(other); }

  BL_INLINE BLResult reset() noexcept { return bl_stroke_options_reset(this); }
  BL_INLINE bool equals(const BLStrokeOptions& other) const noexcept { return bl_stroke_options_equals(this, &other); }

  BL_INLINE BLResult assign(BLStrokeOptions&& other) noexcept { return bl_stroke_options_assign_move(this, &other); }
  BL_INLINE BLResult assign(const BLStrokeOptions& other) noexcept { return bl_stroke_options_assign_weak(this, &other); }

  BL_INLINE void set_caps(BLStrokeCap stroke_cap) noexcept {
    start_cap = uint8_t(stroke_cap);
    end_cap = uint8_t(stroke_cap);
  }
};

BL_DIAGNOSTIC_POP

namespace BLInternal {

template<typename T>
static BL_INLINE size_t path_segment_count(const T&) noexcept { return T::kVertexCount; }
template<typename T, typename... Args>
static BL_INLINE size_t path_segment_count(const T&, Args&&... args) noexcept { return T::kVertexCount + path_segment_count(BLInternal::forward<Args>(args)...); }

template<typename T> void store_path_segment_cmd(uint8_t* cmd, const T& segment) noexcept = delete;
template<typename T> void store_path_segment_vtx(BLPoint* vtx, const T& segment) noexcept = delete;

template<typename T>
static BL_INLINE void store_path_segments_cmd(uint8_t* cmd, const T& segment) noexcept { store_path_segment_cmd<T>(cmd, segment); }

template<typename T, typename... Args>
static BL_INLINE void store_path_segments_cmd(uint8_t* cmd, const T& segment, Args&&... args) noexcept {
  store_path_segment_cmd<T>(cmd, segment);
  store_path_segments_cmd(cmd + T::kVertexCount, BLInternal::forward<Args>(args)...);
}

template<typename T>
static BL_INLINE void store_path_segments_vtx(BLPoint* vtx, const T& segment) noexcept { store_path_segment_vtx<T>(vtx, segment); }

template<typename T, typename... Args>
static BL_INLINE void store_path_segments_vtx(BLPoint* vtx, const T& segment, Args&&... args) noexcept {
  store_path_segment_vtx<T>(vtx, segment);
  store_path_segments_vtx(vtx + T::kVertexCount, BLInternal::forward<Args>(args)...);
}

} // {BLInternal}

//! 2D vector path [C++ API].
class BLPath /* final */ : public BLPathCore {
public:
  //! \cond INTERNAL

  //! Object info values of a default constructed BLPath.
  static inline constexpr uint32_t kDefaultSignature =
    BLObjectInfo::pack_type_with_marker(BL_OBJECT_TYPE_PATH) | BL_OBJECT_INFO_D_FLAG;

  [[nodiscard]]
  BL_INLINE_NODEBUG BLPathImpl* _impl() const noexcept { return static_cast<BLPathImpl*>(_d.impl); }

  //! \endcond

  //! \name Construction & Destruction
  //! \{

  BL_INLINE_NODEBUG BLPath() noexcept {
    bl_path_init(this);

    // Assume a default constructed BLPath.
    BL_ASSUME(_d.info.bits == kDefaultSignature);
  }

  BL_INLINE_NODEBUG BLPath(BLPath&& other) noexcept {
    bl_path_init_move(this, &other);

    // Assume a default initialized `other`.
    BL_ASSUME(other._d.info.bits == kDefaultSignature);
  }

  BL_INLINE_NODEBUG BLPath(const BLPath& other) noexcept {
    bl_path_init_weak(this, &other);
  }

  BL_INLINE_NODEBUG ~BLPath() noexcept {
    if (BLInternal::object_needs_cleanup(_d.info.bits)) {
      bl_path_destroy(this);
    }
  }

  //! \}

  //! \name Overloaded Operators
  //! \{

  BL_INLINE_NODEBUG explicit operator bool() const noexcept { return !is_empty(); }

  BL_INLINE_NODEBUG BLPath& operator=(BLPath&& other) noexcept { bl_path_assign_move(this, &other); return *this; }
  BL_INLINE_NODEBUG BLPath& operator=(const BLPath& other) noexcept { bl_path_assign_weak(this, &other); return *this; }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool operator==(const BLPath& other) const noexcept { return  equals(other); }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool operator!=(const BLPath& other) const noexcept { return !equals(other); }

  //! \}

  //! \name Common Functionality
  //! \{

  BL_INLINE_NODEBUG BLResult reset() noexcept {
    BLResult result = bl_path_reset(this);

    // Reset operation always succeeds.
    BL_ASSUME(result == BL_SUCCESS);
    // Assume a default constructed BLPath after reset.
    BL_ASSUME(_d.info.bits == kDefaultSignature);

    return result;
  }

  BL_INLINE_NODEBUG void swap(BLPathCore& other) noexcept { _d.swap(other._d); }

  //! \}

  //! \name Accessors
  //! \{

  //! Tests whether the path is empty, which means its size equals to zero.
  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_empty() const noexcept { return size() == 0; }

  //! Returns path size (count of vertices used).
  [[nodiscard]]
  BL_INLINE_NODEBUG size_t size() const noexcept { return _impl()->size; }

  //! Returns path capacity (count of allocated vertices).
  [[nodiscard]]
  BL_INLINE_NODEBUG size_t capacity() const noexcept { return _impl()->capacity; }

  //! Returns path's vertex data (read-only).
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLPoint* vertex_data() const noexcept { return _impl()->vertex_data; }

  //! Returns the end of path's vertex data (read-only).
  [[nodiscard]]
  BL_INLINE_NODEBUG const BLPoint* vertex_data_end() const noexcept { return _impl()->vertex_data + _impl()->size; }

  //! Returns path's command data (read-only).
  [[nodiscard]]
  BL_INLINE_NODEBUG const uint8_t* command_data() const noexcept { return _impl()->command_data; }

  //! Returns the end of path's command data (read-only).
  [[nodiscard]]
  BL_INLINE_NODEBUG const uint8_t* command_data_end() const noexcept { return _impl()->command_data + _impl()->size; }

  //! Returns a read-only path data as `BLPathView`.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLPathView view() const noexcept { return _impl()->view; }

  //! \}

  //! \name Path Construction
  //! \{

  //! Clears the content of the path.
  BL_INLINE_NODEBUG BLResult clear() noexcept {
    return bl_path_clear(this);
  }

  //! Shrinks the capacity of the path to fit the current usage.
  BL_INLINE_NODEBUG BLResult shrink() noexcept {
    return bl_path_shrink(this);
  }

  //! Reserves the capacity of the path for at least `n` vertices and commands.
  BL_INLINE_NODEBUG BLResult reserve(size_t n) noexcept {
    return bl_path_reserve(this, n);
  }

  BL_INLINE_NODEBUG BLResult modify_op(BLModifyOp op, size_t n, uint8_t** cmd_data_out, BLPoint** vtx_data_out) noexcept {
    return bl_path_modify_op(this, op, n, cmd_data_out, vtx_data_out);
  }

  BL_INLINE_NODEBUG BLResult assign(BLPathCore&& other) noexcept {
    return bl_path_assign_move(this, &other);
  }

  BL_INLINE_NODEBUG BLResult assign(const BLPathCore& other) noexcept {
    return bl_path_assign_weak(this, &other);
  }

  BL_INLINE_NODEBUG BLResult assign_deep(const BLPathCore& other) noexcept {
    return bl_path_assign_deep(this, &other);
  }

  //! Sets vertex at `index` to `cmd` and `pt`.
  //!
  //! Pass `BL_PATH_CMD_PRESERVE` in `cmd` to preserve the current command.
  BL_INLINE_NODEBUG BLResult set_vertex_at(size_t index, uint32_t cmd, const BLPoint& pt) noexcept {
    return bl_path_set_vertex_at(this, index, cmd, pt.x, pt.y);
  }

  //! Sets vertex at `index` to `cmd` and `[x, y]`.
  //!
  //! Pass `BL_PATH_CMD_PRESERVE` in `cmd` to preserve the current command.
  BL_INLINE_NODEBUG BLResult set_vertex_at(size_t index, uint32_t cmd, double x, double y) noexcept {
    return bl_path_set_vertex_at(this, index, cmd, x, y);
  }

  //! Moves to `p0`.
  //!
  //! Appends `BL_PATH_CMD_MOVE[p0]` command to the path.
  BL_INLINE_NODEBUG BLResult move_to(const BLPoint& p0) noexcept {
    return bl_path_move_to(this, p0.x, p0.y);
  }

  //! Moves to `[x0, y0]`.
  //!
  //! Appends `BL_PATH_CMD_MOVE[x0, y0]` command to the path.
  BL_INLINE_NODEBUG BLResult move_to(double x0, double y0) noexcept {
    return bl_path_move_to(this, x0, y0);
  }

  //! Adds line to `p1`.
  //!
  //! Appends `BL_PATH_CMD_ON[p1]` command to the path.
  BL_INLINE_NODEBUG BLResult line_to(const BLPoint& p1) noexcept {
    return bl_path_line_to(this, p1.x, p1.y);
  }

  //! Adds line to `[x1, y1]`.
  //!
  //! Appends `BL_PATH_CMD_ON[x1, y1]` command to the path.
  BL_INLINE_NODEBUG BLResult line_to(double x1, double y1) noexcept {
    return bl_path_line_to(this, x1, y1);
  }

  //! Adds a polyline (LineTo) of the given `poly` array of size `count`.
  //!
  //! Appends multiple `BL_PATH_CMD_ON[x[i], y[i]]` commands to the path depending on `count` parameter.
  BL_INLINE_NODEBUG BLResult poly_to(const BLPoint* poly, size_t count) noexcept {
    return bl_path_poly_to(this, poly, count);
  }

  //! Adds a quadratic curve to `p1` and `p2`.
  //!
  //! Appends the following commands to the path:
  //!   - `BL_PATH_CMD_QUAD[p1]`
  //!   - `BL_PATH_CMD_ON[p2]`
  //!
  //! Matches SVG 'Q' path command:
  //!   - https://www.w3.org/TR/SVG/paths.html#PathDataQuadraticBezierCommands
  BL_INLINE_NODEBUG BLResult quad_to(const BLPoint& p1, const BLPoint& p2) noexcept {
    return bl_path_quad_to(this, p1.x, p1.y, p2.x, p2.y);
  }

  //! Adds a quadratic curve to `[x1, y1]` and `[x2, y2]`.
  //!
  //! Appends the following commands to the path:
  //!   - `BL_PATH_CMD_QUAD[x1, y1]`
  //!   - `BL_PATH_CMD_ON[x2, y2]`
  //!
  //! Matches SVG 'Q' path command:
  //!   - https://www.w3.org/TR/SVG/paths.html#PathDataQuadraticBezierCommands
  BL_INLINE_NODEBUG BLResult quad_to(double x1, double y1, double x2, double y2) noexcept {
    return bl_path_quad_to(this, x1, y1, x2, y2);
  }

  BL_INLINE BLResult conic_to(const BLPoint& p1, const BLPoint& p2, double w) noexcept {
    return bl_path_conic_to(this, p1.x, p1.y, p2.x, p2.y, w);
  }

  BL_INLINE BLResult conic_to(double x1, double y1, double x2, double y2, double w) noexcept {
    return bl_path_conic_to(this, x1, y1, x2, y2, w);
  }

  //! Adds a cubic curve to `p1`, `p2`, `p3`.
  //!
  //! Appends the following commands to the path:
  //!   - `BL_PATH_CMD_CUBIC[p1]`
  //!   - `BL_PATH_CMD_CUBIC[p2]`
  //!   - `BL_PATH_CMD_ON[p3]`
  //!
  //! Matches SVG 'C' path command:
  //!   - https://www.w3.org/TR/SVG/paths.html#PathDataCubicBezierCommands
  BL_INLINE_NODEBUG BLResult cubic_to(const BLPoint& p1, const BLPoint& p2, const BLPoint& p3) noexcept {
    return bl_path_cubic_to(this, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
  }

  //! Adds a cubic curve to `[x1, y1]`, `[x2, y2]`, and `[x3, y3]`.
  //!
  //! Appends the following commands to the path:
  //!   - `BL_PATH_CMD_CUBIC[x1, y1]`
  //!   - `BL_PATH_CMD_CUBIC[x2, y2]`
  //!   - `BL_PATH_CMD_ON[x3, y3]`
  //!
  //! Matches SVG 'C' path command:
  //!   - https://www.w3.org/TR/SVG/paths.html#PathDataCubicBezierCommands
  BL_INLINE_NODEBUG BLResult cubic_to(double x1, double y1, double x2, double y2, double x3, double y3) noexcept {
    return bl_path_cubic_to(this, x1, y1, x2, y2, x3, y3);
  }

  //! Adds a smooth quadratic curve to `p2`, calculating `p1` from last points.
  //!
  //! Appends the following commands to the path:
  //!   - `BL_PATH_CMD_QUAD[calculated]`
  //!   - `BL_PATH_CMD_ON[p2]`
  //!
  //! Matches SVG 'T' path command:
  //!   - https://www.w3.org/TR/SVG/paths.html#PathDataQuadraticBezierCommands
  BL_INLINE_NODEBUG BLResult smooth_quad_to(const BLPoint& p2) noexcept {
    return bl_path_smooth_quad_to(this, p2.x, p2.y);
  }

  //! Adds a smooth quadratic curve to `[x2, y2]`, calculating `[x1, y1]` from last points.
  //!
  //! Appends the following commands to the path:
  //!   - `BL_PATH_CMD_QUAD[calculated]`
  //!   - `BL_PATH_CMD_ON[x2, y2]`
  //!
  //! Matches SVG 'T' path command:
  //!   - https://www.w3.org/TR/SVG/paths.html#PathDataQuadraticBezierCommands
  BL_INLINE_NODEBUG BLResult smooth_quad_to(double x2, double y2) noexcept {
    return bl_path_smooth_quad_to(this, x2, y2);
  }

  //! Adds a smooth cubic curve to `p2` and `p3`, calculating `p1` from last points.
  //!
  //! Appends the following commands to the path:
  //!   - `BL_PATH_CMD_CUBIC[calculated]`
  //!   - `BL_PATH_CMD_CUBIC[p2]`
  //!   - `BL_PATH_CMD_ON[p3]`
  //!
  //! Matches SVG 'S' path command:
  //!   - https://www.w3.org/TR/SVG/paths.html#PathDataCubicBezierCommands
  BL_INLINE_NODEBUG BLResult smooth_cubic_to(const BLPoint& p2, const BLPoint& p3) noexcept {
    return bl_path_smooth_cubic_to(this, p2.x, p2.y, p3.x, p3.y);
  }

  //! Adds a smooth cubic curve to `[x2, y2]` and `[x3, y3]`, calculating `[x1, y1]` from last points.
  //!
  //! Appends the following commands to the path:
  //!   - `BL_PATH_CMD_CUBIC[calculated]`
  //!   - `BL_PATH_CMD_CUBIC[x2, y2]`
  //!   - `BL_PATH_CMD_ON[x3, y3]`
  //!
  //! Matches SVG 'S' path command:
  //!   - https://www.w3.org/TR/SVG/paths.html#PathDataCubicBezierCommands
  BL_INLINE_NODEBUG BLResult smooth_cubic_to(double x2, double y2, double x3, double y3) noexcept {
    return bl_path_smooth_cubic_to(this, x2, y2, x3, y3);
  }

  //! Adds an arc to the path.
  //!
  //! The center of the arc is specified by `c` and radius by `r`. Both `start` and `sweep` angles are in radians.
  //! If the last vertex doesn't match the start of the arc then a `line_to()` would be emitted before adding the arc.
  //! Pass `true` in `force_move_to` to always emit `move_to()` at the beginning of the arc, which starts a new figure.
  BL_INLINE_NODEBUG BLResult arc_to(const BLPoint& c, const BLPoint& r, double start, double sweep, bool force_move_to = false) noexcept {
    return bl_path_arc_to(this, c.x, c.y, r.x, r.y, start, sweep, force_move_to);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult arc_to(double cx, double cy, double rx, double ry, double start, double sweep, bool force_move_to = false) noexcept {
    return bl_path_arc_to(this, cx, cy, rx, ry, start, sweep, force_move_to);
  }

  //! Adds an arc quadrant (90deg) to the path. The first point `p1` specifies
  //! the quadrant corner and the last point `p2` specifies the end point.
  BL_INLINE_NODEBUG BLResult arc_quadrant_to(const BLPoint& p1, const BLPoint& p2) noexcept {
    return bl_path_arc_quadrant_to(this, p1.x, p1.y, p2.x, p2.y);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult arc_quadrant_to(double x1, double y1, double x2, double y2) noexcept {
    return bl_path_arc_quadrant_to(this, x1, y1, x2, y2);
  }

  //! Adds an elliptic arc to the path that follows the SVG specification.
  //!
  //! Matches SVG 'A' path command:
  //!   - https://www.w3.org/TR/SVG/paths.html#PathDataEllipticalArcCommands
  BL_INLINE_NODEBUG BLResult elliptic_arc_to(const BLPoint& rp, double xAxisRotation, bool large_arc_flag, bool sweep_flag, const BLPoint& p1) noexcept {
    return bl_path_elliptic_arc_to(this, rp.x, rp.y, xAxisRotation, large_arc_flag, sweep_flag, p1.x, p1.y);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult elliptic_arc_to(double rx, double ry, double xAxisRotation, bool large_arc_flag, bool sweep_flag, double x1, double y1) noexcept {
    return bl_path_elliptic_arc_to(this, rx, ry, xAxisRotation, large_arc_flag, sweep_flag, x1, y1);
  }

  //! Closes the current figure.
  //!
  //! Appends `BL_PATH_CMD_CLOSE` to the path.
  //!
  //! Matches SVG 'Z' path command:
  //!   - https://www.w3.org/TR/SVG/paths.html#PathDataClosePathCommand
  BL_INLINE_NODEBUG BLResult close() noexcept { return bl_path_close(this); }

  //! \}

  //! \name Adding Multiple Segments
  //!
  //! Adding multiple segments API was designed to provide high-performance path building in case that the user knows
  //! the segments that will be added to the path in advance.
  //!
  //! \{

  struct MoveTo {
    static inline constexpr uint32_t kVertexCount = 1;

    double x, y;
  };

  struct LineTo {
    static inline constexpr uint32_t kVertexCount = 1;

    double x, y;
  };

  struct QuadTo {
    static inline constexpr uint32_t kVertexCount = 2;

    double x0, y0, x1, y1;
  };

  struct CubicTo {
    static inline constexpr uint32_t kVertexCount = 3;

    double x0, y0, x1, y1, x2, y2;
  };

  template<typename... Args>
  BL_INLINE BLResult add_segments(Args&&... args) noexcept {
    uint8_t* cmd_ptr;
    BLPoint* vtx_ptr;

    size_t kVertexCount = BLInternal::path_segment_count(BLInternal::forward<Args>(args)...);
    BL_PROPAGATE(modify_op(BL_MODIFY_OP_APPEND_GROW, kVertexCount, &cmd_ptr, &vtx_ptr));

    BLInternal::store_path_segments_cmd(cmd_ptr, BLInternal::forward<Args>(args)...);
    BLInternal::store_path_segments_vtx(vtx_ptr, BLInternal::forward<Args>(args)...);

    return BL_SUCCESS;
  }

  //! \}

  //! \name Adding Figures
  //!
  //! Adding a figure means starting with a move-to segment. For example `add_box()` would start a new figure
  //! by adding `BL_PATH_CMD_MOVE_TO` segment, and then by adding 3 lines, and finally a close command.
  //!
  //! \{

  //! Adds a closed rectangle to the path specified by `box`.
  BL_INLINE_NODEBUG BLResult add_box(const BLBoxI& box, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return bl_path_add_box_i(this, &box, dir);
  }

  //! Adds a closed rectangle to the path specified by `box`.
  BL_INLINE_NODEBUG BLResult add_box(const BLBox& box, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return bl_path_add_box_d(this, &box, dir);
  }

  //! Adds a closed rectangle to the path specified by `[x0, y0, x1, y1]`.
  BL_INLINE_NODEBUG BLResult add_box(double x0, double y0, double x1, double y1, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_box(BLBox(x0, y0, x1, y1), dir);
  }

  //! Adds a closed rectangle to the path specified by `rect`.
  BL_INLINE_NODEBUG BLResult add_rect(const BLRectI& rect, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return bl_path_add_rect_i(this, &rect, dir);
  }

  //! Adds a closed rectangle to the path specified by `rect`.
  BL_INLINE_NODEBUG BLResult add_rect(const BLRect& rect, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return bl_path_add_rect_d(this, &rect, dir);
  }

  //! Adds a closed rectangle to the path specified by `[x, y, w, h]`.
  BL_INLINE_NODEBUG BLResult add_rect(double x, double y, double w, double h, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_rect(BLRect(x, y, w, h), dir);
  }

  //! Adds a geometry to the path.
  BL_INLINE_NODEBUG BLResult add_geometry(BLGeometryType geometry_type, const void* geometry_data, const BLMatrix2D* m = nullptr, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return bl_path_add_geometry(this, geometry_type, geometry_data, m, dir);
  }

  //! Adds a closed circle to the path.
  BL_INLINE_NODEBUG BLResult add_circle(const BLCircle& circle, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_CIRCLE, &circle, nullptr, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_circle(const BLCircle& circle, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_CIRCLE, &circle, &transform, dir);
  }

  //! Adds a closed ellipse to the path.
  BL_INLINE_NODEBUG BLResult add_ellipse(const BLEllipse& ellipse, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_ELLIPSE, &ellipse, nullptr, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_ellipse(const BLEllipse& ellipse, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_ELLIPSE, &ellipse, &transform, dir);
  }

  //! Adds a closed rounded rectangle to the path.
  BL_INLINE_NODEBUG BLResult add_round_rect(const BLRoundRect& rr, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_ROUND_RECT, &rr, nullptr, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_round_rect(const BLRoundRect& rr, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_ROUND_RECT, &rr, &transform, dir);
  }

  //! Adds an unclosed arc to the path.
  BL_INLINE_NODEBUG BLResult add_arc(const BLArc& arc, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_ARC, &arc, nullptr, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_arc(const BLArc& arc, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_ARC, &arc, &transform, dir);
  }

  //! Adds a closed chord to the path.
  BL_INLINE_NODEBUG BLResult add_chord(const BLArc& chord, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_CHORD, &chord, nullptr, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_chord(const BLArc& chord, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_CHORD, &chord, &transform, dir);
  }

  //! Adds a closed pie to the path.
  BL_INLINE_NODEBUG BLResult add_pie(const BLArc& pie, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_PIE, &pie, nullptr, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_pie(const BLArc& pie, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_PIE, &pie, &transform, dir);
  }

  //! Adds an unclosed line to the path.
  BL_INLINE_NODEBUG BLResult add_line(const BLLine& line, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_LINE, &line, nullptr, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_line(const BLLine& line, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_LINE, &line, &transform, dir);
  }

  //! Adds a closed triangle.
  BL_INLINE_NODEBUG BLResult add_triangle(const BLTriangle& triangle, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_TRIANGLE, &triangle, nullptr, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_triangle(const BLTriangle& triangle, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_TRIANGLE, &triangle, &transform, dir);
  }

  //! Adds a polyline.
  BL_INLINE_NODEBUG BLResult add_polyline(const BLArrayView<BLPointI>& poly, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_POLYLINEI, &poly, nullptr, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_polyline(const BLArrayView<BLPointI>& poly, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_POLYLINEI, &poly, &transform, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_polyline(const BLPointI* poly, size_t n, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_polyline(BLArrayView<BLPointI>{poly, n}, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_polyline(const BLPointI* poly, size_t n, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_polyline(BLArrayView<BLPointI>{poly, n}, transform, dir);
  }

  //! Adds a polyline.
  BL_INLINE_NODEBUG BLResult add_polyline(const BLArrayView<BLPoint>& poly, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_POLYLINED, &poly, nullptr, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_polyline(const BLArrayView<BLPoint>& poly, const BLMatrix2D& transform, BLGeometryDirection dir) {
    return add_geometry(BL_GEOMETRY_TYPE_POLYLINED, &poly, &transform, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_polyline(const BLPoint* poly, size_t n, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_polyline(BLArrayView<BLPoint>{poly, n}, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_polyline(const BLPoint* poly, size_t n, const BLMatrix2D& transform, BLGeometryDirection dir) {
    return add_polyline(BLArrayView<BLPoint>{poly, n}, transform, dir);
  }

  //! Adds a polygon.
  BL_INLINE_NODEBUG BLResult add_polygon(const BLArrayView<BLPointI>& poly, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_POLYGONI, &poly, nullptr, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_polygon(const BLArrayView<BLPointI>& poly, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_POLYGONI, &poly, &transform, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_polygon(const BLPointI* poly, size_t n, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_polygon(BLArrayView<BLPointI>{poly, n}, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_polygon(const BLPointI* poly, size_t n, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_polygon(BLArrayView<BLPointI>{poly, n}, transform, dir);
  }

  //! Adds a polygon.
  BL_INLINE_NODEBUG BLResult add_polygon(const BLArrayView<BLPoint>& poly, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_POLYGOND, &poly, nullptr, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_polygon(const BLArrayView<BLPoint>& poly, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_POLYGOND, &poly, &transform, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_polygon(const BLPoint* poly, size_t n, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_polygon(BLArrayView<BLPoint>{poly, n}, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_polygon(const BLPoint* poly, size_t n, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_polygon(BLArrayView<BLPoint>{poly, n}, transform, dir);
  }

  //! Adds an array of closed boxes.
  BL_INLINE_NODEBUG BLResult add_box_array(const BLArrayView<BLBoxI>& array, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXI, &array, nullptr, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_box_array(const BLArrayView<BLBoxI>& array, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXI, &array, &transform, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_box_array(const BLBoxI* data, size_t n, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_box_array(BLArrayView<BLBoxI>{data, n}, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_box_array(const BLBoxI* data, size_t n, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_box_array(BLArrayView<BLBoxI>{data, n}, transform, dir);
  }

  //! Adds an array of closed boxes.
  BL_INLINE_NODEBUG BLResult add_box_array(const BLArrayView<BLBox>& array, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXD, &array, nullptr, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_box_array(const BLArrayView<BLBox>& array, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXD, &array, &transform, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_box_array(const BLBox* data, size_t n, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_box_array(BLArrayView<BLBox>{data, n}, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_box_array(const BLBox* data, size_t n, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_box_array(BLArrayView<BLBox>{data, n}, transform, dir);
  }

  //! Adds an array of closed rectangles.
  BL_INLINE_NODEBUG BLResult add_rect_array(const BLArrayView<BLRectI>& array, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTI, &array, nullptr, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_rect_array(const BLArrayView<BLRectI>& array, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTI, &array, &transform, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_rect_array(const BLRectI* data, size_t n, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_rect_array(BLArrayView<BLRectI>{data, n}, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_rect_array(const BLRectI* data, size_t n, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_rect_array(BLArrayView<BLRectI>{data, n}, transform, dir);
  }

  //! Adds an array of closed rectangles.
  BL_INLINE_NODEBUG BLResult add_rect_array(const BLArrayView<BLRect>& array, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTD, &array, nullptr, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_rect_array(const BLArrayView<BLRect>& array, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_geometry(BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTD, &array, &transform, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_rect_array(const BLRect* data, size_t n, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_rect_array(BLArrayView<BLRect>{data, n}, dir);
  }

  //! \overload
  BL_INLINE_NODEBUG BLResult add_rect_array(const BLRect* data, size_t n, const BLMatrix2D& transform, BLGeometryDirection dir = BL_GEOMETRY_DIRECTION_CW) noexcept {
    return add_rect_array(BLArrayView<BLRect>{data, n}, transform, dir);
  }

  //! \}

  //! \name Adding Paths
  //! \{

  //! Adds other `path` to this path.
  BL_INLINE_NODEBUG BLResult add_path(const BLPath& path) noexcept {
    return bl_path_add_path(this, &path, nullptr);
  }

  //! Adds other `path` sliced by the given `range` to this path.
  BL_INLINE_NODEBUG BLResult add_path(const BLPath& path, const BLRange& range) noexcept {
    return bl_path_add_path(this, &path, &range);
  }

  //! Adds other `path` translated by `p` to this path.
  BL_INLINE_NODEBUG BLResult add_path(const BLPath& path, const BLPoint& p) noexcept {
    return bl_path_add_translated_path(this, &path, nullptr, &p);
  }

  //! Adds other `path` translated by `p` and sliced by the given `range` to this path.
  BL_INLINE_NODEBUG BLResult add_path(const BLPath& path, const BLRange& range, const BLPoint& p) noexcept {
    return bl_path_add_translated_path(this, &path, &range, &p);
  }

  //! Adds other `path` transformed by `m` to this path.
  BL_INLINE_NODEBUG BLResult add_path(const BLPath& path, const BLMatrix2D& transform) noexcept {
    return bl_path_add_transformed_path(this, &path, nullptr, &transform);
  }

  //! Adds other `path` transformed by `m` and sliced by the given `range` to this path.
  BL_INLINE_NODEBUG BLResult add_path(const BLPath& path, const BLRange& range, const BLMatrix2D& transform) noexcept {
    return bl_path_add_transformed_path(this, &path, &range, &transform);
  }

  //! Adds other `path`, but reversed.
  BL_INLINE_NODEBUG BLResult add_reversed_path(const BLPath& path, BLPathReverseMode reverse_mode) noexcept {
    return bl_path_add_reversed_path(this, &path, nullptr, reverse_mode);
  }

  //! Adds other `path`, but reversed.
  BL_INLINE_NODEBUG BLResult add_reversed_path(const BLPath& path, const BLRange& range, BLPathReverseMode reverse_mode) noexcept {
    return bl_path_add_reversed_path(this, &path, &range, reverse_mode);
  }

  //! Adds a stroke of `path` to this path.
  BL_INLINE_NODEBUG BLResult add_stroked_path(const BLPath& path, const BLStrokeOptionsCore& stroke_options, const BLApproximationOptions& approximation_options) noexcept {
    return bl_path_add_stroked_path(this, &path, nullptr, &stroke_options, &approximation_options);
  }
  //! \overload
  BL_INLINE_NODEBUG BLResult add_stroked_path(const BLPath& path, const BLRange& range, const BLStrokeOptionsCore& stroke_options, const BLApproximationOptions& approximation_options) noexcept {
    return bl_path_add_stroked_path(this, &path, &range, &stroke_options, &approximation_options);
  }

  //! \}

  //! \name Manipulation
  //! \{

  BL_INLINE_NODEBUG BLResult remove_range(const BLRange& range) noexcept {
    return bl_path_remove_range(this, &range);
  }

  //! \}

  //! \name Transformations
  //! \{

  //! Translates the whole path by `p`.
  BL_INLINE_NODEBUG BLResult translate(const BLPoint& p) noexcept {
    return bl_path_translate(this, nullptr, &p);
  }

  //! Translates a part of the path specified by the given `range` by `p`.
  BL_INLINE_NODEBUG BLResult translate(const BLRange& range, const BLPoint& p) noexcept {
    return bl_path_translate(this, &range, &p);
  }

  //! Transforms the whole path by matrix `m`.
  BL_INLINE_NODEBUG BLResult transform(const BLMatrix2D& m) noexcept {
    return bl_path_transform(this, nullptr, &m);
  }

  //! Transforms a part of the path specified by the given `range` by matrix `m`.
  BL_INLINE_NODEBUG BLResult transform(const BLRange& range, const BLMatrix2D& m) noexcept {
    return bl_path_transform(this, &range, &m);
  }

  //! Fits the whole path into the given `rect` by taking into account fit flags passed by `fit_flags`.
  BL_INLINE_NODEBUG BLResult fit_to(const BLRect& rect, uint32_t fit_flags) noexcept {
    return bl_path_fit_to(this, nullptr, &rect, fit_flags);
  }

  //! Fits a path of the path specified by the given `range` into the given `rect` by taking into account fit flags
  //! passed by `fit_flags`.
  BL_INLINE_NODEBUG BLResult fit_to(const BLRange& range, const BLRect& rect, uint32_t fit_flags) noexcept {
    return bl_path_fit_to(this, &range, &rect, fit_flags);
  }

  //! \}

  //! \name Equality & Comparison
  //! \{

  //! Tests whether this path and the `other` path are equal.
  //!
  //! The equality check is deep. The data of both paths is examined and binary compared (thus a slight difference
  //! like -0 and +0 would make the equality check to fail).
  [[nodiscard]]
  BL_INLINE_NODEBUG bool equals(const BLPath& other) const noexcept { return bl_path_equals(this, &other); }

  //! \}

  //! \name Path Information
  //! \{

  //! Update a path information if necessary.
  BL_INLINE_NODEBUG BLResult get_info_flags(uint32_t* flags_out) const noexcept {
    return bl_path_get_info_flags(this, flags_out);
  }

  //! Stores a bounding box of all vertices and control points to `box_out`.
  //!
  //! Control box is simply bounds of all vertices the path has without further processing. It contains both on-path
  //! and off-path points. Consider using `get_bounding_box()` if you need a visual bounding box.
  BL_INLINE_NODEBUG BLResult get_control_box(BLBox* box_out) const noexcept {
    return bl_path_get_control_box(this, box_out);
  }

  //! Stores a bounding box of all on-path vertices and curve extrema to `box_out`.
  //!
  //! The bounding box stored to `box_out` could be smaller than a bounding box obtained by `get_control_box()` as it's
  //! calculated by merging only start/end points and curves at their extrema (not control points). The resulting
  //! bounding box represents a visual bounds of the path.
  BL_INLINE_NODEBUG BLResult get_bounding_box(BLBox* box_out) const noexcept {
    return bl_path_get_bounding_box(this, box_out);
  }

  //! Returns the range describing a figure at the given `index`.
  BL_INLINE_NODEBUG BLResult get_figure_range(size_t index, BLRange* range_out) const noexcept {
    return bl_path_get_figure_range(this, index, range_out);
  }

  //! Returns the last vertex of the path and stores it to `vtx_out`. If the very last command of the path is
  //! `BL_PATH_CMD_CLOSE` then the path will be iterated in reverse order to match the initial vertex of the last
  //! figure.
  BL_INLINE_NODEBUG BLResult get_last_vertex(BLPoint* vtx_out) const noexcept {
    return bl_path_get_last_vertex(this, vtx_out);
  }

  BL_INLINE_NODEBUG BLResult get_closest_vertex(const BLPoint& p, double max_distance, size_t* index_out) const noexcept {
    double distance_out;
    return bl_path_get_closest_vertex(this, &p, max_distance, index_out, &distance_out);
  }

  BL_INLINE_NODEBUG BLResult get_closest_vertex(const BLPoint& p, double max_distance, size_t* index_out, double* distance_out) const noexcept {
    return bl_path_get_closest_vertex(this, &p, max_distance, index_out, distance_out);
  }

  //! \}

  //! \name Hit Testing
  //! \{

  //! Hit tests the given point `p` by respecting the given `fill_rule`.
  [[nodiscard]]
  BL_INLINE_NODEBUG BLHitTest hit_test(const BLPoint& p, BLFillRule fill_rule) const noexcept {
    return bl_path_hit_test(this, &p, fill_rule);
  }

  //! \}
};

namespace BLInternal {

template<>
BL_INLINE void store_path_segment_cmd(uint8_t* cmd, const BLPath::MoveTo&) noexcept {
  cmd[0] = uint8_t(BL_PATH_CMD_MOVE);
}

template<>
BL_INLINE void store_path_segment_vtx(BLPoint* vtx, const BLPath::MoveTo& segment) noexcept {
  vtx[0] = BLPoint(segment.x, segment.y);
}

template<>
BL_INLINE void store_path_segment_cmd(uint8_t* cmd, const BLPath::LineTo&) noexcept {
  cmd[0] = uint8_t(BL_PATH_CMD_ON);
}

template<>
BL_INLINE void store_path_segment_vtx(BLPoint* vtx, const BLPath::LineTo& segment) noexcept {
  vtx[0] = BLPoint(segment.x, segment.y);
}

template<>
BL_INLINE void store_path_segment_cmd(uint8_t* cmd, const BLPath::QuadTo&) noexcept {
  cmd[0] = uint8_t(BL_PATH_CMD_QUAD);
  cmd[1] = uint8_t(BL_PATH_CMD_ON);
}

template<>
BL_INLINE void store_path_segment_vtx(BLPoint* vtx, const BLPath::QuadTo& segment) noexcept {
  vtx[0] = BLPoint(segment.x0, segment.y0);
  vtx[1] = BLPoint(segment.x1, segment.y1);
}

template<>
BL_INLINE void store_path_segment_cmd(uint8_t* cmd, const BLPath::CubicTo&) noexcept {
  cmd[0] = uint8_t(BL_PATH_CMD_CUBIC);
  cmd[1] = uint8_t(BL_PATH_CMD_CUBIC);
  cmd[2] = uint8_t(BL_PATH_CMD_ON);
}

template<>
BL_INLINE void store_path_segment_vtx(BLPoint* vtx, const BLPath::CubicTo& segment) noexcept {
  vtx[0] = BLPoint(segment.x0, segment.y0);
  vtx[1] = BLPoint(segment.x1, segment.y1);
  vtx[2] = BLPoint(segment.x2, segment.y2);
}

} // {BLInternal}

#endif
//! \}

//! \}

#endif // BLEND2D_PATH_H_INCLUDED
