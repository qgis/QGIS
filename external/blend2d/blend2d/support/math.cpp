// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/support/algorithm_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/math_p.h>
#include <blend2d/support/traits_p.h>

namespace bl {
namespace Math {

// bl::Math - Cubic Roots
// ======================

// Ax^3 + Bx^2 + Cx + D = 0.
//
// Roots3And4.c: Graphics Gems, original author Jochen Schwarze (schwarze@isa.de). See also the wiki article
// at <http://en.wikipedia.org/wiki/Cubic_function> for other equations.
size_t cubic_roots(double* dst, const double* poly, double t_min, double t_max) noexcept {
  constexpr double k1Div3 = 1.0 / 3.0;
  constexpr double k1Div6 = 1.0 / 6.0;
  constexpr double k1Div9 = 1.0 / 9.0;
  constexpr double k1Div27 = 1.0 / 27.0;

  size_t n_roots = 0;
  double norm = poly[0];
  double a = poly[1];
  double b = poly[2];
  double c = poly[3];

  if (norm == 0.0)
    return quad_roots(dst, a, b, c, t_min, t_max);

  // Convert to a normalized form `x^3 + Ax^2 + Bx + C == 0`.
  a /= norm;
  b /= norm;
  c /= norm;

  // Substitute x = y - A/3 to eliminate quadric term `x^3 + px + q = 0`.
  double sa = a * a;
  double p = -k1Div9  * sa + k1Div3 * b;
  double q = (k1Div27 * sa - k1Div6 * b) * a + 0.5 * c;

  // Use Cardano's formula.
  double p3 = p * p * p;
  double d  = q * q + p3;

  // Resubstitution constant.
  double sub = -k1Div3 * a;

  if (is_near_zero(d)) {
    // One triple solution.
    if (is_near_zero(q)) {
      dst[0] = sub;
      return size_t(sub >= t_min && sub <= t_max);
    }

    // One single and one double solution.
    double u = cbrt(-q);
    n_roots = 2;

    dst[0] = sub + 2.0 * u;
    dst[1] = sub - u;

    // Sort.
    if (dst[0] > dst[1])
      BLInternal::swap(dst[0], dst[1]);
  }
  else if (d < 0.0) {
    // Three real solutions.
    double phi = k1Div3 * Math::acos(-q / sqrt(-p3));
    double t = 2.0 * sqrt(-p);

    n_roots = 3;
    dst[0] = sub + t * cos(phi);
    dst[1] = sub - t * cos(phi + kPI_DIV_3);
    dst[2] = sub - t * cos(phi - kPI_DIV_3);

    // Sort.
    if (dst[0] > dst[1]) BLInternal::swap(dst[0], dst[1]);
    if (dst[1] > dst[2]) BLInternal::swap(dst[1], dst[2]);
    if (dst[0] > dst[1]) BLInternal::swap(dst[0], dst[1]);
  }
  else {
    // One real solution.
    double sqrt_d = sqrt(d);
    double u =  cbrt(sqrt_d - q);
    double v = -cbrt(sqrt_d + q);

    n_roots = 1;
    dst[0] = sub + u + v;
  }

  size_t n = 0;
  for (size_t i = 0; i < n_roots; i++)
    if (dst[i] >= t_min && dst[i] <= t_max)
      dst[n++] = dst[i];
  return n;
}

} // {Math}
} // {bl}
