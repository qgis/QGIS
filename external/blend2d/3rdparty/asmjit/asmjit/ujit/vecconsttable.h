// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_UJIT_VECCONSTTABLE_H_INCLUDED
#define ASMJIT_UJIT_VECCONSTTABLE_H_INCLUDED

#include <asmjit/core/globals.h>

#if !defined(ASMJIT_NO_UJIT)

ASMJIT_BEGIN_SUB_NAMESPACE(ujit)

//! \addtogroup asmjit_ujit
//! \{

template<typename T, size_t W>
struct VecConst;

//! \cond

//! A 64-bit vector constant of type `T` aligned to 64 bits.
template<typename T>
struct ASMJIT_MAY_ALIAS ASMJIT_ALIGNAS(8) VecConst<T, 8> {
  using ElementType = T;

  static inline constexpr size_t kVectorWidth = 8;
  static inline constexpr size_t kElementSize = sizeof(ElementType);
  static inline constexpr size_t kElementCount = kVectorWidth / kElementSize;

  static_assert(kElementCount > 0u, "Vector constant must have at least one element");

  ElementType data[kElementCount];

  template<typename DstT>
  ASMJIT_INLINE_NODEBUG const DstT& as() const noexcept {
    static_assert(sizeof(DstT) <= sizeof(*this), "Size of the destination type DstT must be <= 8");
    return *static_cast<const DstT*>(static_cast<const void*>(this));
  }
};

//! A 128-bit vector constant of type `T` aligned to 128 bits.
template<typename T>
struct ASMJIT_MAY_ALIAS ASMJIT_ALIGNAS(16) VecConst<T, 16> {
  using ElementType = T;

  static inline constexpr size_t kVectorWidth = 16;
  static inline constexpr size_t kElementSize = sizeof(ElementType);
  static inline constexpr size_t kElementCount = kVectorWidth / kElementSize;

  static_assert(kElementCount > 0u, "Vector constant must have at least one element");

  ElementType data[kElementCount];

  template<typename DstT>
  ASMJIT_INLINE_NODEBUG const DstT& as() const noexcept {
    static_assert(sizeof(DstT) <= sizeof(*this), "Size of the destination type DstT must be <= 16");
    return *static_cast<const DstT*>(static_cast<const void*>(this));
  }
};

//! A 256-bit vector constant of type `T` aligned to 256 bits.
template<typename T>
struct ASMJIT_MAY_ALIAS ASMJIT_ALIGNAS(32) VecConst<T, 32> {
  using ElementType = T;

  static inline constexpr size_t kVectorWidth = 32;
  static inline constexpr size_t kElementSize = sizeof(ElementType);
  static inline constexpr size_t kElementCount = kVectorWidth / kElementSize;

  static_assert(kElementCount > 0u, "Vector constant must have at least one element");

  ElementType data[kElementCount];

  template<typename DstT>
  ASMJIT_INLINE_NODEBUG const DstT& as() const noexcept {
    static_assert(sizeof(DstT) <= sizeof(*this), "Size of the destination type DstT must be <= 32");
    return *static_cast<const DstT*>(static_cast<const void*>(this));
  }
};

//! A 512-bit vector constant of type `T` aligned to 512 bits.
template<typename T>
struct ASMJIT_MAY_ALIAS ASMJIT_ALIGNAS(64) VecConst<T, 64> {
  using ElementType = T;

  static inline constexpr size_t kVectorWidth = 64;
  static inline constexpr size_t kElementSize = sizeof(ElementType);
  static inline constexpr size_t kElementCount = kVectorWidth / kElementSize;

  static_assert(kElementCount > 0u, "Vector constant must have at least one element");

  ElementType data[kElementCount];

  template<typename DstT>
  ASMJIT_INLINE_NODEBUG const DstT& as() const noexcept {
    static_assert(sizeof(DstT) <= sizeof(*this), "Size of the destination type DstT must be <= 64");
    return *static_cast<const DstT*>(static_cast<const void*>(this));
  }
};

//! \endcond

template<typename T> using VecConst64 = VecConst<T, 8>;
template<typename T> using VecConst128 = VecConst<T, 16>;
template<typename T> using VecConst256 = VecConst<T, 32>;
template<typename T> using VecConst512 = VecConst<T, 64>;

#if ASMJIT_ARCH_X86
// The maximum native constants we want to use in X86 case is 256-bit ones, because in AVX-512 case
// we want to just broadcast the constant to all lanes. The reason is to not end up with a huge table.
template<typename T> using VecConstNative = VecConst256<T>;
#else
// At the moment all non-x86 platforms support at most 128-bit vectors, thus the maximum width of each
// constant is limited to 128 bits as well. Future additions could include LoongArch, which supports
// 256-bit SIMD vectors natively.
template<typename T> using VecConstNative = VecConst128<T>;
#endif

template<typename V, typename T = typename V::ElementType>
static ASMJIT_INLINE_CONSTEXPR V make_const(T v) noexcept {
  static_assert(V::kElementCount == 1u  ||
                V::kElementCount == 2u  ||
                V::kElementCount == 4u  ||
                V::kElementCount == 8u  ||
                V::kElementCount == 16u ||
                V::kElementCount == 32u ||
                V::kElementCount == 64u);

  if constexpr (V::kElementCount == 1u) {
    return V{{
      v
    }};
  }
  else if constexpr (V::kElementCount == 2u) {
    return V{{
      v, v
    }};
  }
  else if constexpr (V::kElementCount == 4u) {
    return V{{
      v, v, v, v
    }};
  }
  else if constexpr (V::kElementCount == 8u) {
    return V{{
      v, v, v, v, v, v, v, v
    }};
  }
  else if constexpr (V::kElementCount == 16u) {
    return V{{
      v, v, v, v, v, v, v, v,
      v, v, v, v, v, v, v, v
    }};
  }
  else if constexpr (V::kElementCount == 32u) {
    return V{{
      v, v, v, v, v, v, v, v,
      v, v, v, v, v, v, v, v,
      v, v, v, v, v, v, v, v,
      v, v, v, v, v, v, v, v
    }};
  }
  else {
    return V{{
      v, v, v, v, v, v, v, v,
      v, v, v, v, v, v, v, v,
      v, v, v, v, v, v, v, v,
      v, v, v, v, v, v, v, v,
      v, v, v, v, v, v, v, v,
      v, v, v, v, v, v, v, v,
      v, v, v, v, v, v, v, v,
      v, v, v, v, v, v, v, v
    }};
  }
}

template<typename V, typename T = typename V::ElementType>
static ASMJIT_INLINE_CONSTEXPR V make_const(T h, T l) noexcept {
  static_assert(V::kElementCount == 2u  ||
                V::kElementCount == 4u  ||
                V::kElementCount == 8u  ||
                V::kElementCount == 16u ||
                V::kElementCount == 32u ||
                V::kElementCount == 64u);

  if constexpr (V::kElementCount == 2u) {
    return V{{
      l, h
    }};
  }
  else if constexpr (V::kElementCount == 4u) {
    return V{{
      l, h, l, h
    }};
  }
  else if constexpr (V::kElementCount == 8u) {
    return V{{
      l, h, l, h, l, h, l, h
    }};
  }
  else if constexpr (V::kElementCount == 16u) {
    return V{{
      l, h, l, h, l, h, l, h,
      l, h, l, h, l, h, l, h
    }};
  }
  else if constexpr (V::kElementCount == 32u) {
    return V{{
      l, h, l, h, l, h, l, h,
      l, h, l, h, l, h, l, h,
      l, h, l, h, l, h, l, h,
      l, h, l, h, l, h, l, h
    }};
  }
  else {
    return V{{
      l, h, l, h, l, h, l, h,
      l, h, l, h, l, h, l, h,
      l, h, l, h, l, h, l, h,
      l, h, l, h, l, h, l, h,
      l, h, l, h, l, h, l, h,
      l, h, l, h, l, h, l, h,
      l, h, l, h, l, h, l, h,
      l, h, l, h, l, h, l, h
    }};
  }
}

template<typename V, typename T = typename V::ElementType>
static ASMJIT_INLINE_CONSTEXPR V make_const(T d, T c, T b, T a) noexcept {
  static_assert(V::kElementCount == 4u  ||
                V::kElementCount == 8u  ||
                V::kElementCount == 16u ||
                V::kElementCount == 32u ||
                V::kElementCount == 64u);

  if constexpr (V::kElementCount == 4u) {
    return V{{
      a, b, c, d
    }};
  }
  else if constexpr (V::kElementCount == 8u) {
    return V{{
      a, b, c, d, a, b, c, d
    }};
  }
  else if constexpr (V::kElementCount == 16u) {
    return V{{
      a, b, c, d, a, b, c, d,
      a, b, c, d, a, b, c, d
    }};
  }
  else if constexpr (V::kElementCount == 32u) {
    return V{{
      a, b, c, d, a, b, c, d,
      a, b, c, d, a, b, c, d,
      a, b, c, d, a, b, c, d,
      a, b, c, d, a, b, c, d
    }};
  }
  else {
    return V{{
      a, b, c, d, a, b, c, d,
      a, b, c, d, a, b, c, d,
      a, b, c, d, a, b, c, d,
      a, b, c, d, a, b, c, d,
      a, b, c, d, a, b, c, d,
      a, b, c, d, a, b, c, d,
      a, b, c, d, a, b, c, d,
      a, b, c, d, a, b, c, d
    }};
  }
}

template<typename V, typename T = typename V::ElementType>
static ASMJIT_INLINE_CONSTEXPR V make_const(T h, T g, T f, T e, T d, T c, T b, T a) noexcept {
  static_assert(V::kElementCount == 8u  ||
                V::kElementCount == 16u ||
                V::kElementCount == 32u ||
                V::kElementCount == 64u);

  if constexpr (V::kElementCount == 8u) {
    return V{{
      a, b, c, d, e, f, g, h
    }};
  }
  else if constexpr (V::kElementCount == 16u) {
    return V{{
      a, b, c, d, e, f, g, h,
      a, b, c, d, e, f, g, h
    }};
  }
  else if constexpr (V::kElementCount == 32u) {
    return V{{
      a, b, c, d, e, f, g, h,
      a, b, c, d, e, f, g, h,
      a, b, c, d, e, f, g, h,
      a, b, c, d, e, f, g, h
    }};
  }
  else {
    return V{{
      a, b, c, d, e, f, g, h,
      a, b, c, d, e, f, g, h,
      a, b, c, d, e, f, g, h,
      a, b, c, d, e, f, g, h,
      a, b, c, d, e, f, g, h,
      a, b, c, d, e, f, g, h,
      a, b, c, d, e, f, g, h,
      a, b, c, d, e, f, g, h
    }};
  }
}

template<typename V, typename T = typename V::ElementType>
static ASMJIT_INLINE_CONSTEXPR V make_const(
  T p, T o, T n, T m, T l, T k, T j, T i, T h, T g, T f, T e, T d, T c, T b, T a
) noexcept {
  static_assert(V::kElementCount == 16u ||
                V::kElementCount == 32u ||
                V::kElementCount == 64u);

  if constexpr (V::kElementCount == 16u) {
    return V{{
      a, b, c, d, e, f, g, h,
      i, j, k, l, m, n, o, p
    }};
  }
  else if constexpr (V::kElementCount == 32u) {
    return V{{
      a, b, c, d, e, f, g, h,
      i, j, k, l, m, n, o, p,
      a, b, c, d, e, f, g, h,
      i, j, k, l, m, n, o, p
    }};
  }
  else {
    return V{{
      a, b, c, d, e, f, g, h,
      i, j, k, l, m, n, o, p,
      a, b, c, d, e, f, g, h,
      i, j, k, l, m, n, o, p,
      a, b, c, d, e, f, g, h,
      i, j, k, l, m, n, o, p,
      a, b, c, d, e, f, g, h,
      i, j, k, l, m, n, o, p
    }};
  }
}

template<typename V, typename T = typename V::ElementType>
static ASMJIT_INLINE_CONSTEXPR V make_const(
  T p1, T o1, T n1, T m1, T l1, T k1, T j1, T i1, T h1, T g1, T f1, T e1, T d1, T c1, T b1, T a1,
  T p0, T o0, T n0, T m0, T l0, T k0, T j0, T i0, T h0, T g0, T f0, T e0, T d0, T c0, T b0, T a0
) noexcept {
  static_assert(V::kElementCount == 32u ||
                V::kElementCount == 64u);

  if constexpr (V::kElementCount == 32u) {
    return V{{
      a0, b0, c0, d0, e0, f0, g0, h0,
      i0, j0, k0, l0, m0, n0, o0, p0,
      a1, b1, c1, d1, e1, f1, g1, h1,
      i1, j1, k1, l1, m1, n1, o1, p1
    }};
  }
  else {
    return V{{
      a0, b0, c0, d0, e0, f0, g0, h0,
      i0, j0, k0, l0, m0, n0, o0, p0,
      a1, b1, c1, d1, e1, f1, g1, h1,
      i1, j1, k1, l1, m1, n1, o1, p1,
      a0, b0, c0, d0, e0, f0, g0, h0,
      i0, j0, k0, l0, m0, n0, o0, p0,
      a1, b1, c1, d1, e1, f1, g1, h1,
      i1, j1, k1, l1, m1, n1, o1, p1
    }};
  }
}

template<typename V, typename T = typename V::ElementType>
static ASMJIT_INLINE_CONSTEXPR V make_const(
  T p3, T o3, T n3, T m3, T l3, T k3, T j3, T i3, T h3, T g3, T f3, T e3, T d3, T c3, T b3, T a3,
  T p2, T o2, T n2, T m2, T l2, T k2, T j2, T i2, T h2, T g2, T f2, T e2, T d2, T c2, T b2, T a2,
  T p1, T o1, T n1, T m1, T l1, T k1, T j1, T i1, T h1, T g1, T f1, T e1, T d1, T c1, T b1, T a1,
  T p0, T o0, T n0, T m0, T l0, T k0, T j0, T i0, T h0, T g0, T f0, T e0, T d0, T c0, T b0, T a0
) noexcept {
  static_assert(V::kElementCount == 64u);

  return V{{
    a0, b0, c0, d0, e0, f0, g0, h0,
    i0, j0, k0, l0, m0, n0, o0, p0,
    a1, b1, c1, d1, e1, f1, g1, h1,
    i1, j1, k1, l1, m1, n1, o1, p1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    i2, j2, k2, l2, m2, n2, o2, p2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    i3, j3, k3, l3, m3, n3, o3, p3
  }};
}

struct VecConstTable {
  VecConstNative<uint64_t> p_0000000000000000 = make_const<VecConstNative<uint64_t>>(uint64_t(0x0000000000000000u));
  VecConstNative<uint64_t> p_FFFFFFFFFFFFFFFF = make_const<VecConstNative<uint64_t>>(uint64_t(0xFFFFFFFFFFFFFFFFu));

  VecConstNative<uint64_t> p_8080808080808080 = make_const<VecConstNative<uint64_t>>(uint64_t(0x8080808080808080u));
  VecConstNative<uint64_t> p_8000800080008000 = make_const<VecConstNative<uint64_t>>(uint64_t(0x8000800080008000u));
  VecConstNative<uint64_t> p_8000000080000000 = make_const<VecConstNative<uint64_t>>(uint64_t(0x8000000080000000u));
  VecConstNative<uint64_t> p_8000000000000000 = make_const<VecConstNative<uint64_t>>(uint64_t(0x8000000000000000u));

  VecConstNative<uint64_t> p_7F7F7F7F7F7F7F7F = make_const<VecConstNative<uint64_t>>(uint64_t(0x7F7F7F7F7F7F7F7Fu));
  VecConstNative<uint64_t> p_7FFF7FFF7FFF7FFF = make_const<VecConstNative<uint64_t>>(uint64_t(0x7FFF7FFF7FFF7FFFu));
  VecConstNative<uint64_t> p_7FFFFFFF7FFFFFFF = make_const<VecConstNative<uint64_t>>(uint64_t(0x7FFFFFFF7FFFFFFFu));
  VecConstNative<uint64_t> p_7FFFFFFFFFFFFFFF = make_const<VecConstNative<uint64_t>>(uint64_t(0x7FFFFFFFFFFFFFFFu));

  VecConstNative<uint64_t> p_0100010001000100 = make_const<VecConstNative<uint64_t>>(uint64_t(0x0100010001000100u));
  VecConstNative<uint64_t> p_00FF00FF00FF00FF = make_const<VecConstNative<uint64_t>>(uint64_t(0x00FF00FF00FF00FFu));
  VecConstNative<uint64_t> p_0F0F0F0F0F0F0F0F = make_const<VecConstNative<uint64_t>>(uint64_t(0x0F0F0F0F0F0F0F0Fu));

  VecConstNative<uint64_t> p_1010101010101010 = make_const<VecConstNative<uint64_t>>(uint64_t(0x1010101010101010u));

  VecConstNative<uint64_t> p_FFFFFFFF00000000 = make_const<VecConstNative<uint64_t>>(uint64_t(0xFFFFFFFF00000000u));

  VecConstNative<uint64_t> p_0000800000008000 = make_const<VecConstNative<uint64_t>>(uint64_t(0x0000800000008000u));

  VecConst128<uint32_t> sign32_scalar         = make_const<VecConst128<uint32_t>>(0u, 0u, 0u, uint32_t(0x80000000u));
  VecConst128<uint64_t> sign64_scalar         = make_const<VecConst128<uint64_t>>(uint64_t(0u), uint64_t(0x8000000000000000u));

  VecConstNative<uint64_t> f32_0_5_minus_1ulp  = make_const<VecConstNative<uint64_t>>(0x3EFFFFFF3EFFFFFFu); // 0.49999997 (0.5f - 1ulp)
  VecConstNative<float> f32_0_5               = make_const<VecConstNative<float>>(0.5f);
  VecConstNative<float> f32_1                 = make_const<VecConstNative<float>>(1.0f);
  VecConstNative<float> f32_round_magic       = make_const<VecConstNative<float>>(8388608.0f);

  VecConstNative<uint64_t> f64_0_5_minus_1ulp  = make_const<VecConstNative<uint64_t>>(0x3FDFFFFFFFFFFFFFu); // 0.49999999999999994 (0.5 - 1ulp).
  VecConstNative<double> f64_0_5              = make_const<VecConstNative<double>>(0.5);
  VecConstNative<double> f64_1                = make_const<VecConstNative<double>>(1.0);
  VecConstNative<double> f64_round_magic      = make_const<VecConstNative<double>>(4503599627370496.0);
};

ASMJIT_VARAPI const VecConstTable vec_const_table;

struct VecConstTableRef {
  const VecConstTable& table;
  size_t size;
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_UJIT
#endif // ASMJIT_UJIT_VECCONSTTABLE_H_INCLUDED
