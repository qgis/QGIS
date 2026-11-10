// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/matrix_p.h>
#include <blend2d/core/path_p.h>
#include <blend2d/core/pathstroke_p.h>
#include <blend2d/geometry/bezier_p.h>
#include <blend2d/support/lookuptable_p.h>
#include <blend2d/support/math_p.h>

namespace bl {
namespace PathInternal {

// bl::Path - Stroke - Constants
// =============================

// Default minimum miter-join length that always bypasses any other join-type. The reason behind this is to
// prevent emitting very small line segments in case that normals of joining segments are almost equal.
static constexpr double kStrokeMiterMinimum = 1e-10;
static constexpr double kStrokeMiterMinimumSq = Math::square(kStrokeMiterMinimum);

// Minimum length for a line/curve the stroker will accept. If the segment is smaller than this it would be skipped.
static constexpr double kStrokeLengthEpsilon = 1e-10;
static constexpr double kStrokeLengthEpsilonSq = Math::square(kStrokeLengthEpsilon);

static constexpr double kStrokeCollinearityEpsilon = 1e-10;

static constexpr double kStrokeCuspTThreshold = 1e-10;
static constexpr double kStrokeDegenerateFlatness = 1e-6;

// Epsilon used to split quadratic bezier curves during offsetting.
static constexpr double kOffsetQuadEpsilonT = 1e-5;

// Minimum vertices that would be required for any join + additional line.
//
// Calculated from:
//   JOIN:
//     bevel: 1 vertex
//     miter: 3 vertices
//     round: 7 vertices (2 cubics at most)
//   ADDITIONAL:
//     end-point: 1 vertex
//     line-to  : 1 vertex
static constexpr size_t kStrokeMaxJoinVertices = 9;

// bl::Path - Stroke - Tables
// ==========================

struct CapVertexCountGen {
  static constexpr uint8_t value(size_t cap) noexcept {
    return BLStrokeCap(cap) == BL_STROKE_CAP_SQUARE       ? 3 :
           BLStrokeCap(cap) == BL_STROKE_CAP_ROUND        ? 6 :
           BLStrokeCap(cap) == BL_STROKE_CAP_ROUND_REV    ? 8 :
           BLStrokeCap(cap) == BL_STROKE_CAP_TRIANGLE     ? 2 :
           BLStrokeCap(cap) == BL_STROKE_CAP_TRIANGLE_REV ? 4 :
           BLStrokeCap(cap) == BL_STROKE_CAP_BUTT         ? 1 : 0;
  }
};

static constexpr auto cap_vertex_count_table =
  make_lookup_table<uint8_t, BL_STROKE_CAP_MAX_VALUE + 1, CapVertexCountGen>();

// bl::Path - Stroke - Utilities
// =============================

enum class Side : size_t {
  kA = 0,
  kB = 1
};

static BL_INLINE Side opposite_side(Side side) noexcept { return Side(size_t(1) - size_t(side)); }

static BL_INLINE Side side_from_normals(const BLPoint& n0, const BLPoint& n1) noexcept {
  return Side(size_t(Geometry::cross(n0, n1) >= 0));
}

static BL_INLINE uint32_t sanity_stroke_cap(uint32_t cap) noexcept {
  return cap <= BL_STROKE_CAP_MAX_VALUE ? cap : BL_STROKE_CAP_BUTT;
}

static BL_INLINE bool is_miter_join_category(uint32_t join_type) noexcept {
  return join_type == BL_STROKE_JOIN_MITER_CLIP  ||
         join_type == BL_STROKE_JOIN_MITER_BEVEL ||
         join_type == BL_STROKE_JOIN_MITER_ROUND ;
}

static BL_INLINE uint32_t miter_join_to_simple_join(uint32_t join_type) {
  if (join_type == BL_STROKE_JOIN_MITER_BEVEL)
    return BL_STROKE_JOIN_BEVEL;
  else if (join_type == BL_STROKE_JOIN_MITER_ROUND)
    return BL_STROKE_JOIN_ROUND;
  else
    return join_type;
}

static BL_INLINE bool test_inner_join_intersecion(const BLPoint& a0, const BLPoint& a1, const BLPoint& b0, const BLPoint& b1, const BLPoint& join) noexcept {
  BLPoint min = bl_max(bl_min(a0, a1), bl_min(b0, b1));
  BLPoint max = bl_min(bl_max(a0, a1), bl_max(b0, b1));

  return (join.x >= min.x) & (join.y >= min.y) &
         (join.x <= max.x) & (join.y <= max.y) ;
}

static BL_INLINE void dull_angle_arc_to(PathAppender& appender, const BLPoint& p0, const BLPoint& pa, const BLPoint& pb, const BLPoint& intersection) noexcept {
  BLPoint pm = (pa + pb) * 0.5;

  double w = Math::sqrt(Geometry::magnitude(p0 - pm) / Geometry::magnitude(p0 - intersection));
  double a = 4 * w / (3.0 * (1.0 + w));

  BLPoint c0 = pa + a * (intersection - pa);
  BLPoint c1 = pb + a * (intersection - pb);

  appender.cubic_to(c0, c1, pb);
};

// bl::Path - Stroke - Implementation
// ==================================

class PathStroker {
public:
  enum Flags : uint32_t {
    kFlagIsOpen   = 0x01,
    kFlagIsClosed = 0x02
  };

  // Stroke input.
  PathIterator _iter;

  // Stroke options.
  const BLStrokeOptions& _options;
  const BLApproximationOptions& _approx;

  struct SideData {
    BLPath* path;            // Output path (outer/inner, per side).
    size_t figure_offset;     // Start of the figure offset in output path (only A path).
    PathAppender appender; // Output path appended (outer/inner, per side).
    double d;                // Distance (StrokeWidth / 2).
    double d2;               // Distance multiplied by 2.
  };

  double _miter_limit;        // Miter limit possibly clamped to a safe range.
  double _miter_limit_sq;      // Miter limit squared.
  uint32_t _join_type;        // Simplified join type.
  SideData _side_data[2];     // A and B data (outer/inner side).

  // Stroke output.
  BLPath* _cPath;            // Output C path.

  // Global state.
  BLPoint _p0;               // Current point.
  BLPoint _n0;               // Unit normal of `_p0`.
  BLPoint _pInitial;         // Initial point (MoveTo).
  BLPoint _nInitial;         // Unit normal of `_pInitial`.
  uint32_t _flags;           // Work flags.

  BL_INLINE PathStroker(const BLPathView& input, const BLStrokeOptions& options, const BLApproximationOptions& approx, BLPath* a, BLPath* b, BLPath* c) noexcept
    : _iter(input),
      _options(options),
      _approx(approx),
      _join_type(options.join),
      _cPath(c) {

    _side_data[0].path = a;
    _side_data[0].figure_offset = 0;
    _side_data[0].d = options.width * 0.5;
    _side_data[0].d2 = options.width;
    _side_data[1].path = b;
    _side_data[1].figure_offset = 0;
    _side_data[1].d = -_side_data[0].d;
    _side_data[1].d2 = -_side_data[0].d2;

    // Initialize miter calculation options. What we do here is to change `_join_type` to a value that would be easier
    // for us to use during joining. We always honor `_miter_limit_sq` even when the `_join_type` is not miter to prevent
    // emitting very small line segments next to next other, which saves vertices and also prevents border cases in
    // additional processing.
    if (is_miter_join_category(_join_type)) {
      // Simplify miter-join type to non-miter join, if possible.
      _join_type = miter_join_to_simple_join(_join_type);

      // Final miter limit is `0.5 * width * miter_limit`.
      _miter_limit = d() * options.miter_limit;
      _miter_limit_sq = Math::square(_miter_limit);
    }
    else {
      _miter_limit = kStrokeMiterMinimum;
      _miter_limit_sq = kStrokeMiterMinimumSq;
    }
  }

  BL_INLINE bool is_open() const noexcept { return (_flags & kFlagIsOpen) != 0; }
  BL_INLINE bool is_closed() const noexcept { return (_flags & kFlagIsClosed) != 0; }

  BL_INLINE SideData& side_data(Side side) noexcept { return _side_data[size_t(side)]; }

  BL_INLINE double d() const noexcept { return _side_data[0].d; }
  BL_INLINE double d(Side side) const noexcept { return _side_data[size_t(side)].d; }

  BL_INLINE double d2() const noexcept { return _side_data[0].d2; }
  BL_INLINE double d2(Side side) const noexcept { return _side_data[size_t(side)].d2; }

  BL_INLINE BLPath* a_path() const noexcept { return _side_data[size_t(Side::kA)].path; }
  BL_INLINE BLPath* b_path() const noexcept { return _side_data[size_t(Side::kB)].path; }
  BL_INLINE BLPath* c_path() const noexcept { return _cPath; }

  BL_INLINE BLPath* outer_path(Side side) const noexcept { return _side_data[size_t(side)].path; }
  BL_INLINE BLPath* inner_path(Side side) const noexcept { return _side_data[size_t(opposite_side(side))].path; }

  BL_INLINE PathAppender& a_out() noexcept { return _side_data[size_t(Side::kA)].appender; }
  BL_INLINE PathAppender& b_out() noexcept { return _side_data[size_t(Side::kB)].appender; }

  BL_INLINE PathAppender& outer_appender(Side side) noexcept { return _side_data[size_t(side)].appender; }
  BL_INLINE PathAppender& inner_appender(Side side) noexcept { return _side_data[size_t(opposite_side(side))].appender; }

  BL_INLINE BLResult ensure_appenders_capacity(size_t a_required, size_t b_required) noexcept {
    uint32_t ok = uint32_t(a_out().remaining_size() >= a_required) &
                  uint32_t(b_out().remaining_size() >= b_required) ;

    if (BL_LIKELY(ok))
      return BL_SUCCESS;

    return a_out().ensure(a_path(), a_required) |
           b_out().ensure(b_path(), b_required) ;
  }

  BL_INLINE_IF_NOT_DEBUG BLResult stroke(BLPathStrokeSinkFunc sink, void* user_data) noexcept {
    size_t figure_start_idx = 0;
    size_t estimated_size = _iter.remaining_forward() * 2u;

    BL_PROPAGATE(a_path()->reserve(a_path()->size() + estimated_size));

    while (!_iter.at_end()) {
      // Start of the figure.
      const uint8_t* figure_start_cmd = _iter.cmd;
      if (BL_UNLIKELY(_iter.cmd[0] != BL_PATH_CMD_MOVE)) {
        if (_iter.cmd[0] != BL_PATH_CMD_CLOSE)
          return bl_make_error(BL_ERROR_INVALID_GEOMETRY);

        _iter++;
        continue;
      }

      figure_start_idx += (size_t)(_iter.cmd - figure_start_cmd);
      figure_start_cmd = _iter.cmd;

      side_data(Side::kA).figure_offset = side_data(Side::kA).path->size();
      BL_PROPAGATE(a_out().begin(a_path(), BL_MODIFY_OP_APPEND_GROW, _iter.remaining_forward()));
      BL_PROPAGATE(b_out().begin(b_path(), BL_MODIFY_OP_ASSIGN_GROW, 48));

      BLPoint poly_pts[4];
      size_t poly_size;

      _p0 = *_iter.vtx;
      _pInitial = _p0;
      _flags = 0;

      // Content of the figure.
      _iter++;
      while (!_iter.at_end()) {
        BL_PROPAGATE(ensure_appenders_capacity(kStrokeMaxJoinVertices, kStrokeMaxJoinVertices));

        uint8_t cmd = _iter.cmd[0]; // Next command.
        BLPoint p1 = _iter.vtx[0];  // Next line-to or control point.
        BLPoint v1;                 // Vector of `p1 - _p0`.
        BLPoint n1;                 // Unit normal of `v1`.

        if (cmd == BL_PATH_CMD_ON) {
          // Line command, collinear curve converted to line or close of the figure.
          _iter++;
LineTo:
          v1 = p1 - _p0;
          if (Geometry::magnitude_squared(v1) < kStrokeLengthEpsilonSq)
            continue;

          n1 = Geometry::normal(Geometry::unit_vector(v1));
          if (!is_open()) {
            BL_PROPAGATE(open_line_to(p1, n1));
            continue;
          }

          for (;;) {
            BL_PROPAGATE(join_line_to(p1, n1));

            if (_iter.at_end())
              break;

            BL_PROPAGATE(ensure_appenders_capacity(kStrokeMaxJoinVertices, kStrokeMaxJoinVertices));

            cmd = _iter.cmd[0];
            p1 = _iter.vtx[0];

            if (cmd != BL_PATH_CMD_ON)
              break;

            _iter++;
            v1 = p1 - _p0;
            if (Geometry::magnitude_squared(v1) < kStrokeLengthEpsilonSq)
              break;

            n1 = Geometry::normal(Geometry::unit_vector(v1));
          }
          continue;

          // This is again to minimize inline expansion of `smooth_poly_to()`.
SmoothPolyTo:
          BL_PROPAGATE(smooth_poly_to(poly_pts, poly_size));
          continue;
        }
        else if (cmd == BL_PATH_CMD_QUAD) {
          // Quadratic curve segment.
          _iter += 2;
          if (BL_UNLIKELY(_iter.after_end()))
            return BL_ERROR_INVALID_GEOMETRY;

          const BLPoint* quad = _iter.vtx - 3;
          BLPoint p2 = quad[2];
          BLPoint v2 = p2 - p1;

          v1 = p1 - _p0;
          n1 = Geometry::normal(Geometry::unit_vector(v1));

          double cm = Geometry::cross(v2, v1);
          if (bl_abs(cm) <= kStrokeCollinearityEpsilon) {
            // All points are [almost] collinear (degenerate case).
            double dot = Geometry::dot(-v1, v2);

            // Check if control point lies outside of the start/end points.
            if (dot > 0.0) {
              // Rotate all points to x-axis.
              double r1 = Geometry::dot(p1 - _p0, v1);
              double r2 = Geometry::dot(p2 - _p0, v1);

              // Parameter of the cusp if it's within (0, 1).
              double t = r1 / (2.0 * r1 - r2);
              if (t > 0.0 && t < 1.0) {
                poly_pts[0] = Geometry::evaluate(Geometry::quad_ref(quad), t);
                poly_pts[1] = p2;
                poly_size = 2;
                goto SmoothPolyTo;
              }
            }

            // Collinear without cusp => straight line.
            p1 = p2;
            goto LineTo;
          }

          // Very small curve segment => straight line.
          if (Geometry::magnitude_squared(v1) < kStrokeLengthEpsilonSq || Geometry::magnitude_squared(v2) < kStrokeLengthEpsilonSq) {
            p1 = p2;
            goto LineTo;
          }

          if (!is_open())
            BL_PROPAGATE(open_curve(n1));
          else
            BL_PROPAGATE(join_curve(n1));

          BL_PROPAGATE(offset_quad(_iter.vtx - 3));
        }
        else if (cmd == BL_PATH_CMD_CUBIC) {
          // Cubic curve segment.
          _iter += 3;
          if (BL_UNLIKELY(_iter.after_end()))
            return BL_ERROR_INVALID_GEOMETRY;

          BLPoint p[7];

          int cusp = 0;
          double tCusp = 0;

          p[0] = _p0;
          p[1] = _iter.vtx[-3];
          p[2] = _iter.vtx[-2];
          p[3] = _iter.vtx[-1];

          // Check if the curve is flat enough to be potentially degenerate.
          if (Geometry::is_cubic_flat(Geometry::cubic_ref(p), kStrokeDegenerateFlatness)) {
            double dot1 = Geometry::dot(p[0] - p[1], p[3] - p[1]);
            double dot2 = Geometry::dot(p[0] - p[2], p[3] - p[2]);

            if (!(dot1 < 0.0) || !(dot2 < 0.0)) {
              // Rotate all points to x-axis.
              const BLPoint r = Geometry::cubic_start_tangent(Geometry::cubic_ref(p));

              double r1 = Geometry::dot(p[1] - p[0], r);
              double r2 = Geometry::dot(p[2] - p[0], r);
              double r3 = Geometry::dot(p[3] - p[0], r);

              double a = 1.0 / (3.0 * r1 - 3.0 * r2 + r3);
              double b = 2.0 * r1 - r2;
              double s = Math::sqrt(r2 * (r2 - r1) - r1 * (r3 - r1));

              // Parameters of the cusps.
              double t1 = a * (b - s);
              double t2 = a * (b + s);

              // Offset the first and second cusps (if they exist).
              poly_size = 0;
              if (t1 > kStrokeCuspTThreshold && t1 < 1.0 - kStrokeCuspTThreshold)
                poly_pts[poly_size++] = Geometry::evaluate(Geometry::cubic_ref(p), t1);

              if (t2 > kStrokeCuspTThreshold && t2 < 1.0 - kStrokeCuspTThreshold)
                poly_pts[poly_size++] = Geometry::evaluate(Geometry::cubic_ref(p), t2);

              if (poly_size == 0) {
                p1 = p[3];
                goto LineTo;
              }

              poly_pts[poly_size++] = p[3];
              goto SmoothPolyTo;
            }
            else {
              p1 = p[3];
              goto LineTo;
            }
          }
          else {
            double tl;
            Geometry::get_cubic_inflection_parameter(Geometry::cubic_ref(p), tCusp, tl);

            if (tl == 0.0 && tCusp > 0.0 && tCusp < 1.0) {
              Geometry::split(Geometry::cubic_ref(p), Geometry::cubic_out(p), Geometry::cubic_out(p + 3));
              cusp = 1;
            }
          }

          for (;;) {
            v1 = p[1] - _p0;
            if (Geometry::is_zero(v1))
              v1 = p[2] - _p0;
            n1 = Geometry::normal(Geometry::unit_vector(v1));

            if (!is_open())
              BL_PROPAGATE(open_curve(n1));
            else if (cusp >= 0)
              BL_PROPAGATE(join_curve(n1));
            else
              BL_PROPAGATE(join_cusp(n1));

            BL_PROPAGATE(offset_cubic(p));
            if (cusp <= 0)
              break;

            BL_PROPAGATE(ensure_appenders_capacity(kStrokeMaxJoinVertices, kStrokeMaxJoinVertices));

            // Second part of the cubic after the cusp. We assign `-1` to `cusp` so we can call `join_cusp()` later.
            // This is a special join that we need in this case.
            cusp = -1;
            p[0] = p[3];
            p[1] = p[4];
            p[2] = p[5];
            p[3] = p[6];
          }
        }
        else {
          // Either invalid command or close of the figure. If the figure is already closed it means that we have
          // already jumped to `LineTo` and we should terminate now. Otherwise we just encountered close or
          // something else which is not part of the current figure.
          if (is_closed())
            break;

          if (cmd != BL_PATH_CMD_CLOSE)
            break;

          // The figure is closed. We just jump to `LineTo` to minimize inlining expansion and mark the figure as
          // closed. Next time we terminate on `is_closed()` condition above.
          _flags |= kFlagIsClosed;
          p1 = _pInitial;
          goto LineTo;
        }
      }

      // Don't emit anything if the figure has no points (and thus no direction).
      _iter += size_t(is_closed());
      if (!is_open()) {
        a_out().done(a_path());
        b_out().done(b_path());
        continue;
      }

      if (is_closed()) {
        // The figure is closed => the end result is two closed figures without caps. In this case only paths
        // A and B have a content, path C will be empty and should be thus ignored by the sink.

        // Allocate space for the end join and close command.
        BL_PROPAGATE(ensure_appenders_capacity(kStrokeMaxJoinVertices + 1, kStrokeMaxJoinVertices + 1));

        BL_PROPAGATE(join_end_point(_nInitial));
        a_out().close();
        b_out().close();
        c_path()->clear();
      }
      else {
        // The figure is open => the end result is a single figure with caps. The paths contain the following:
        //   A - Offset of the figure and end cap.
        //   B - Offset of the figure that MUST BE reversed.
        //   C - Start cap (not reversed).
        uint32_t start_cap = sanity_stroke_cap(_options.start_cap);
        uint32_t end_cap = sanity_stroke_cap(_options.end_cap);

        BL_PROPAGATE(a_out().ensure(a_path(), cap_vertex_count_table[end_cap]));
        BL_PROPAGATE(add_cap(a_out(), _p0, b_out().vtx[-1], end_cap));

        PathAppender c_out;
        BL_PROPAGATE(c_out.begin(c_path(), BL_MODIFY_OP_ASSIGN_GROW, cap_vertex_count_table[start_cap] + 1));
        c_out.move_to(b_path()->vertex_data()[0]);
        BL_PROPAGATE(add_cap(c_out, _pInitial, a_path()->vertex_data()[side_data(Side::kA).figure_offset], start_cap));
        c_out.done(c_path());
      }

      a_out().done(a_path());
      b_out().done(b_path());

      // Call the path to the provided sink with resulting paths.
      size_t figure_end_idx = figure_start_idx + (size_t)(_iter.cmd - figure_start_cmd);
      BL_PROPAGATE(sink(a_path(), b_path(), c_path(), figure_start_idx, figure_end_idx,  user_data));

      figure_start_idx = figure_end_idx;
    }

    return BL_SUCCESS;
  }

  // Opens a new figure with a line segment starting from the current point and ending at `p1`. The `n1` is a
  // normal calculated from a unit vector of `p1 - _p0`.
  //
  // This function can only be called after we have at least two vertices that form the line. These vertices
  // cannot be a single point as that would mean that we cannot calculate unit vector and then normal for the
  // offset. This must be handled before calling `open_line_to()`.
  //
  // NOTE: Path cannot be open when calling this function.
  BL_INLINE_IF_NOT_DEBUG BLResult open_line_to(const BLPoint& p1, const BLPoint& n1) noexcept {
    BL_ASSERT(!is_open());
    BLPoint w = n1 * d();

    a_out().move_to(_p0 + w);
    b_out().move_to(_p0 - w);

    _p0 = p1;
    _n0 = n1;
    _nInitial = n1;

    a_out().line_to(_p0 + w);
    b_out().line_to(_p0 - w);

    _flags |= kFlagIsOpen;
    return BL_SUCCESS;
  }

  // Joins line-to segment described by `p1` point and `n1` normal.
  BL_INLINE_IF_NOT_DEBUG BLResult join_line_to(const BLPoint& p1, const BLPoint& n1) noexcept {
    if (_n0 == n1) {
      // Collinear case - patch the previous point(s) if they connect lines.
      a_out().back(a_out().cmd[-2].value <= BL_PATH_CMD_ON);
      b_out().back(b_out().cmd[-2].value <= BL_PATH_CMD_ON);

      BLPoint w1 = n1 * d();
      a_out().line_to(p1 + w1);
      b_out().line_to(p1 - w1);
    }
    else {
      Side side = side_from_normals(_n0, n1);
      BLPoint m = _n0 + n1;
      BLPoint k = m * d2(side) / Geometry::magnitude_squared(m);
      BLPoint w1 = n1 * d(side);

      size_t miter_flag = 0;

      if (side == Side::kA) {
        PathAppender& outer = a_out();
        outer_join(outer, n1, w1, k, d(side), d2(side), miter_flag);
        outer.back(miter_flag);
        outer.line_to(p1 + w1);

        PathAppender& inner = b_out();
        inner_join_line_to(inner, _p0 - w1, p1 - w1, _p0 - k);
        inner.line_to(p1 - w1);
      }
      else {
        PathAppender& outer = b_out();
        outer_join(outer, n1, w1, k, d(side), d2(side), miter_flag);
        outer.back(miter_flag);
        outer.line_to(p1 + w1);

        PathAppender& inner = a_out();
        inner_join_line_to(inner, _p0 - w1, p1 - w1, _p0 - k);
        inner.line_to(p1 - w1);
      }
    }

    _p0 = p1;
    _n0 = n1;
    return BL_SUCCESS;
  }

  // Opens a new figure at the current point `_p0`. The first vertex (MOVE) is calculated by offsetting `_p0`
  // by the given unit normal `n0`.
  //
  // NOTE: Path cannot be open when calling this function.
  BL_INLINE_IF_NOT_DEBUG BLResult open_curve(const BLPoint& n0) noexcept {
    BL_ASSERT(!is_open());
    BLPoint w = n0 * d();

    a_out().move_to(_p0 + w);
    b_out().move_to(_p0 - w);

    _n0 = n0;
    _nInitial = n0;

    _flags |= kFlagIsOpen;
    return BL_SUCCESS;
  }

  // Joins curve-to segment.
  BL_INLINE_IF_NOT_DEBUG BLResult join_curve(const BLPoint& n1) noexcept {
    // Collinear case - do nothing.
    if (_n0 == n1)
      return BL_SUCCESS;

    Side side = side_from_normals(_n0, n1);
    BLPoint m = _n0 + n1;
    BLPoint k = m * d2(side) / Geometry::magnitude_squared(m);
    BLPoint w1 = n1 * d(side);
    size_t dummy_miter_flag;

    outer_join(outer_appender(side), n1, w1, k, d(side), d2(side), dummy_miter_flag);
    inner_join_curve_to(inner_appender(side), _p0 - w1);

    _n0 = n1;
    return BL_SUCCESS;
  }

  BL_INLINE_IF_NOT_DEBUG BLResult join_cusp(const BLPoint& n1) noexcept {
    Side side = side_from_normals(_n0, n1);
    BLPoint w1 = n1 * d(side);

    dull_round_join(outer_appender(side), d(side), d2(side), w1);
    inner_appender(side).line_to(_p0 - w1);

    _n0 = n1;
    return BL_SUCCESS;
  }

  BL_INLINE_IF_NOT_DEBUG BLResult join_cusp_and_line_to(const BLPoint& n1, const BLPoint& p1) noexcept {
    Side side = side_from_normals(_n0, n1);
    BLPoint w1 = n1 * d(side);

    PathAppender& outer = outer_appender(side);
    dull_round_join(outer, d(side), d2(side), w1);
    outer.line_to(p1 + w1);

    PathAppender& inner = inner_appender(side);
    inner.line_to(_p0 - w1);
    inner.line_to(p1 - w1);

    _n0 = n1;
    _p0 = p1;
    return BL_SUCCESS;
  }

  BL_INLINE_IF_NOT_DEBUG BLResult smooth_poly_to(const BLPoint* poly, size_t count) noexcept {
    BL_ASSERT(count >= 2);

    BLPoint p1 = poly[0];
    BLPoint v1 = p1 - _p0;
    if (Geometry::magnitude_squared(v1) < kStrokeLengthEpsilonSq)
      return BL_SUCCESS;

    BLPoint n1 = Geometry::normal(Geometry::unit_vector(v1));
    if (!is_open())
      BL_PROPAGATE(open_line_to(p1, n1));
    else
      BL_PROPAGATE(join_line_to(p1, n1));

    // We have already ensured vertices for `open_line_to()` and `join_line_to()`,
    // however, we need more vertices for consecutive joins and line segments.
    size_t required_capacity = (count - 1) * kStrokeMaxJoinVertices;
    BL_PROPAGATE(ensure_appenders_capacity(required_capacity, required_capacity));

    for (size_t i = 1; i < count; i++) {
      p1 = poly[i];
      v1 = p1 - _p0;
      if (Geometry::magnitude_squared(v1) < kStrokeLengthEpsilonSq)
        continue;

      n1 = Geometry::normal(Geometry::unit_vector(v1));
      BL_PROPAGATE(join_cusp_and_line_to(n1, p1));
    }

    return BL_SUCCESS;
  }

  // Joins end point that is only applied to closed figures.
  BL_INLINE_IF_NOT_DEBUG BLResult join_end_point(const BLPoint& n1) noexcept {
    if (_n0 == n1) {
      // Collinear case - patch the previous point(s) if they connect lines.
      a_out().back(a_out().cmd[-2].value <= BL_PATH_CMD_ON);
      b_out().back(b_out().cmd[-2].value <= BL_PATH_CMD_ON);
      return BL_SUCCESS;
    }

    Side side = side_from_normals(_n0, n1);
    BLPoint m = _n0 + n1;
    BLPoint w1 = n1 * d(side);
    BLPoint k = m * d2(side) / Geometry::magnitude_squared(m);

    size_t miter_flag = 0;

    BLPathPrivateImpl* outer_impl = get_impl(outer_path(side));
    size_t outer_start = side_data(side).figure_offset;

    PathAppender& outer = outer_appender(side);
    outer_join(outer, n1, w1, k, d(side), d2(side), miter_flag);

    // Shift the start point to be at the miter intersection and remove the
    // Line from the intersection to the start of the path if miter was applied.
    if (miter_flag) {
      if (outer_impl->command_data[outer_start + 1] == BL_PATH_CMD_ON) {
        outer.back();
        outer_impl->vertex_data[outer_start] = outer.vtx[-1];
        outer.back(outer.cmd[-2].value <= BL_PATH_CMD_ON);
      }
    }

    BLPathPrivateImpl* inner_impl = get_impl(inner_path(side));
    size_t inner_start = side_data(opposite_side(side)).figure_offset;

    if (inner_impl->command_data[inner_start + 1] <= BL_PATH_CMD_ON) {
      inner_join_end_point(inner_appender(side), inner_impl->vertex_data[inner_start], inner_impl->vertex_data[inner_start + 1], _p0 - k);
    }

    return BL_SUCCESS;
  }

  BL_INLINE_IF_NOT_DEBUG void inner_join_curve_to(PathAppender& out, const BLPoint& p1) noexcept {
    out.line_to(_p0);
    out.line_to(p1);
  }

  BL_INLINE_IF_NOT_DEBUG void inner_join_line_to(PathAppender& out, const BLPoint& lineP0, const BLPoint& lineP1, const BLPoint& inner_pt) noexcept {
    if (out.cmd[-2].value <= BL_PATH_CMD_ON && test_inner_join_intersecion(out.vtx[-2], out.vtx[-1], lineP0, lineP1, inner_pt)) {
      out.vtx[-1] = inner_pt;
    }
    else {
      out.line_to(_p0);
      out.line_to(lineP0);
    }
  }

  BL_INLINE_IF_NOT_DEBUG void inner_join_end_point(PathAppender& out, BLPoint& lineP0, const BLPoint& lineP1, const BLPoint& inner_pt) noexcept {
    if (out.cmd[-2].value <= BL_PATH_CMD_ON && test_inner_join_intersecion(out.vtx[-2], out.vtx[-1], lineP0, lineP1, inner_pt)) {
      lineP0 = inner_pt;
      out.back(1);
    }
    else {
      out.line_to(_p0);
      out.line_to(lineP0);
    }
  }

  // Calculates outer join to `pb`.
  BL_INLINE_IF_NOT_DEBUG BLResult outer_join(PathAppender& appender, const BLPoint& n1, const BLPoint& w1, const BLPoint& k, double d, double d2, size_t& miter_flag) noexcept {
    BLPoint pb = _p0 + w1;

    if (Geometry::magnitude_squared(k) <= _miter_limit_sq) {
      // Miter condition is met.
      appender.back(appender.cmd[-2].value <= BL_PATH_CMD_ON);
      appender.line_to(_p0 + k);
      appender.line_to(pb);

      miter_flag = 1;
      return BL_SUCCESS;
    }

    if (_join_type == BL_STROKE_JOIN_MITER_CLIP) {
      double b2 = bl_abs(Geometry::cross(k, _n0));

      // Avoid degenerate cases and NaN.
      if (b2 > 0)
        b2 = b2 * _miter_limit / Geometry::magnitude(k);
      else
        b2 = _miter_limit;

      appender.back(appender.cmd[-2].value <= BL_PATH_CMD_ON);
      appender.line_to(_p0 + d * _n0 - b2 * Geometry::normal(_n0));
      appender.line_to(_p0 + d *  n1 + b2 * Geometry::normal(n1));

      miter_flag = 1;
      appender.line_to(pb);
      return BL_SUCCESS;
    }

    if (_join_type == BL_STROKE_JOIN_ROUND) {
      BLPoint pa = appender.vtx[-1];
      if (Geometry::dot(_p0 - pa, _p0 - pb) < 0.0) {
        // Dull angle.
        BLPoint n2 = Geometry::normal(Geometry::unit_vector(pb - pa));
        BLPoint m = _n0 + n2;
        BLPoint k0 = d2 * m / Geometry::magnitude_squared(m);
        BLPoint q = d * n2;

        BLPoint pc1 = _p0 + k0;
        BLPoint pp1 = _p0 + q;
        BLPoint pc2 = Math::lerp(pc1, pp1, 2.0);

        dull_angle_arc_to(appender, _p0, pa, pp1, pc1);
        dull_angle_arc_to(appender, _p0, pp1, pb, pc2);
      }
      else {
        // Acute angle.
        BLPoint pm = Math::lerp(pa, pb);
        BLPoint pi = _p0 + k;

        double w = Math::sqrt(Geometry::length(_p0, pm) / Geometry::length(_p0, pi));
        double a = 4.0 * w / (3.0 * (1.0 + w));

        BLPoint c0 = pa + a * (pi - pa);
        BLPoint c1 = pb + a * (pi - pb);

        appender.cubic_to(c0, c1, pb);
      }
      return BL_SUCCESS;
    }

    // Bevel or unknown `_join_type`.
    appender.line_to(pb);
    return BL_SUCCESS;
  }

  // Calculates round join to `pb` (dull angle), only used by offsetting cusps.
  BL_INLINE_IF_NOT_DEBUG BLResult dull_round_join(PathAppender& out, double d, double d2, const BLPoint& w1) noexcept {
    BLPoint pa = out.vtx[-1];
    BLPoint pb = _p0 + w1;
    BLPoint n2 = Geometry::normal(Geometry::unit_vector(pb - pa));

    if (!Math::is_finite(n2.x))
      return BL_SUCCESS;

    BLPoint m = _n0 + n2;
    BLPoint k = m * d2 / Geometry::magnitude_squared(m);
    BLPoint q = n2 * d;

    BLPoint pc1 = _p0 + k;
    BLPoint pp1 = _p0 + q;
    BLPoint pc2 = Math::lerp(pc1, pp1, 2.0);

    dull_angle_arc_to(out, _p0, pa, pp1, pc1);
    dull_angle_arc_to(out, _p0, pp1, pb, pc2);
    return BL_SUCCESS;
  }

  BL_INLINE_IF_NOT_DEBUG BLResult offset_quad(const BLPoint bez[3]) noexcept {
    double ts[3];
    size_t tn = Geometry::quad_offset_cusp_ts(Geometry::quad_ref(bez), d(), ts);
    ts[tn++] = 1.0;

    Geometry::QuadCurveTsIter iter(Geometry::quad_ref(bez), Span<double>(ts, tn));
    double m = _approx.offset_parameter;

    do {
      for (;;) {
        BL_PROPAGATE(ensure_appenders_capacity(2, 2));

        double t = Geometry::quad_parameter_at_angle(iter.part, m);
        if (!(t > kOffsetQuadEpsilonT && t < 1.0 - kOffsetQuadEpsilonT))
          t = 1.0;

        BLPoint part[3];
        Geometry::split(Geometry::quad_ref(iter.part), Geometry::quad_out(part), Geometry::quad_out(iter.part), t);
        offset_quad_simple(part[0], part[1], part[2]);

        if (t == 1.0)
          break;
      }
    } while (iter.next());

    return BL_SUCCESS;
  }

  BL_INLINE_IF_NOT_DEBUG void offset_quad_simple(const BLPoint& p0, const BLPoint& p1, const BLPoint& p2) noexcept {
    if (p0 == p2)
      return;

    BLPoint v0 = p1 - p0;
    BLPoint v1 = p2 - p1;

    BLPoint m0 = Geometry::normal(Geometry::unit_vector(p0 != p1 ? v0 : v1));
    BLPoint m2 = Geometry::normal(Geometry::unit_vector(p1 != p2 ? v1 : v0));

    _p0 = p2;
    _n0 = m2;

    BLPoint m = m0 + m2;
    BLPoint k1 = m * d2() / Geometry::magnitude_squared(m);
    BLPoint k2 = m2 * d();

    a_out().quad_to(p1 + k1, p2 + k2);
    b_out().quad_to(p1 - k1, p2 - k2);
  }

  struct CubicApproximateSink {
    PathStroker& _stroker;

    BL_INLINE CubicApproximateSink(PathStroker& stroker) noexcept
      : _stroker(stroker) {}

    BL_INLINE BLResult operator()(BLPoint quad[3]) const noexcept {
      return _stroker.offset_quad(quad);
    }
  };

  BL_INLINE_IF_NOT_DEBUG BLResult offset_cubic(const BLPoint bez[4]) noexcept {
    CubicApproximateSink sink(*this);
    return Geometry::approximate_cubic_with_quads(Geometry::cubic_ref(bez), _approx.simplify_tolerance, sink);
  }

  BL_INLINE_IF_NOT_DEBUG BLResult add_cap(PathAppender& out, BLPoint pivot, BLPoint p1, uint32_t cap_type) noexcept {
    BLPoint p0 = out.vtx[-1];
    BLPoint q = Geometry::normal(p1 - p0) * 0.5;

    switch (cap_type) {
      case BL_STROKE_CAP_BUTT:
      default: {
        out.line_to(p1);
        break;
      }

      case BL_STROKE_CAP_SQUARE: {
        out.line_to(p0 + q);
        out.line_to(p1 + q);
        out.line_to(p1);
        break;
      }

      case BL_STROKE_CAP_ROUND: {
        out.arc_quadrant_to(p0 + q, pivot + q);
        out.arc_quadrant_to(p1 + q, p1);
        break;
      }

      case BL_STROKE_CAP_ROUND_REV: {
        out.line_to(p0 + q);
        out.arc_quadrant_to(p0, pivot);
        out.arc_quadrant_to(p1, p1 + q);
        out.line_to(p1);
        break;
      }

      case BL_STROKE_CAP_TRIANGLE: {
        out.line_to(pivot + q);
        out.line_to(p1);
        break;
      }

      case BL_STROKE_CAP_TRIANGLE_REV: {
        out.line_to(p0 + q);
        out.line_to(pivot);
        out.line_to(p1 + q);
        out.line_to(p1);
        break;
      }
    }

    return BL_SUCCESS;
  }
};

// bl::Path - Stroke - Interface
// =============================

BLResult stroke_path(
  const BLPathView& input,
  const BLStrokeOptions& options,
  const BLApproximationOptions& approx,
  BLPath& a,
  BLPath& b,
  BLPath& c,
  BLPathStrokeSinkFunc sink, void* user_data) noexcept {

  return PathStroker(input, options, approx, &a, &b, &c).stroke(sink, user_data);
}

} // {PathInternal}
} // {bl}
