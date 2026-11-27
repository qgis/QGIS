// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SUPPORT_ALGORITHM_P_H_INCLUDED
#define BLEND2D_SUPPORT_ALGORITHM_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

//! \name Binary Search & Bound
//! \{

template<typename T, typename Value, typename Predicate>
[[nodiscard]]
static BL_INLINE size_t lower_bound(const T* data, size_t size, const Value& value, Predicate&& pred) noexcept {
  size_t index = 0;

  while (size) {
    size_t half = size / 2u;
    size_t middle = index + half;

    auto b = pred(data[middle], value);
    size -= half + uint32_t(b);
    middle++;

    if (b)
      index = middle;
    else
      size = half;
  }

  return index;
}

template<typename T, typename Value>
[[nodiscard]]
static BL_INLINE size_t lower_bound(const T* data, size_t size, const Value& value) noexcept {
  return lower_bound(data, size, value, [](const T& a, const Value& b) noexcept -> bool { return a < b; });
}

//! \}

//! \name Binary Search
//! \{

template<typename T, typename V>
[[nodiscard]]
static BL_INLINE size_t binary_search(const T* array, size_t size, const V& value) noexcept {
  if (!size)
    return SIZE_MAX;

  const T* base = array;
  while (size_t half = size / 2u) {
    const T* middle = base + half;
    size -= half;
    if (middle[0] <= value)
      base = middle;
  }

  size_t index = size_t(base - array);
  BL_ASSUME(index != SIZE_MAX);

  return base[0] == value ? index : SIZE_MAX;
}

template<typename T, typename V>
[[nodiscard]]
static BL_INLINE size_t binary_search_closest_first(const T* array, size_t size, const V& value) noexcept {
  if (!size)
    return 0;

  const T* base = array;
  while (size_t half = size / 2u) {
    const T* middle = base + half;
    size -= half;
    if (middle[0] < value)
      base = middle;
  }

  if (base[0] < value)
    base++;

  return size_t(base - array);
}

template<typename T, typename V>
[[nodiscard]]
static BL_INLINE size_t binary_search_closest_last(const T* array, size_t size, const V& value) noexcept {
  if (!size)
    return 0;

  const T* base = array;
  while (size_t half = size / 2u) {
    const T* middle = base + half;
    size -= half;
    if (middle[0] <= value)
      base = middle;
  }

  return size_t(base - array);
}

//! \}

//! \name Sorting
//! \{

enum class SortOrder : uint32_t {
  kAscending = 0,
  kDescending = 1
};

//! A helper class that provides comparison of any user-defined type that
//! implements `<` and `>` operators (primitive types are supported as well).
template<SortOrder Order = SortOrder::kAscending>
struct CompareOp {
  template<typename A, typename B>
  BL_INLINE int operator()(const A& a, const B& b) const noexcept {
    return (Order == SortOrder::kAscending) ? (a < b ? -1 : a > b ?  1 : 0)
                                            : (a < b ?  1 : a > b ? -1 : 0);
  }
};

//! Insertion sort.
template<typename T, typename Compare = CompareOp<SortOrder::kAscending>>
static BL_INLINE void insertion_sort(T* base, size_t size, const Compare& cmp = Compare()) noexcept {
  for (T* pm = base + 1; pm < base + size; pm++)
    for (T* pl = pm; pl > base && cmp(pl[-1], pl[0]) > 0; pl--)
      BLInternal::swap(pl[-1], pl[0]);
}

namespace Internal {

//! Quick-sort implementation.
template<typename T, class Compare>
struct QuickSortImpl {
  enum : size_t {
    kStackSize = 64 * 2,
    kISortThreshold = 7
  };

  // Based on "PDCLib - Public Domain C Library" and rewritten to C++.
  static void sort(T* base, size_t size, const Compare& cmp) noexcept {
    T* end = base + size;
    T* stack[kStackSize];
    T** stackptr = stack;

    for (;;) {
      if ((size_t)(end - base) > kISortThreshold) {
        // We work from second to last - first will be pivot element.
        T* pi = base + 1;
        T* pj = end - 1;
        BLInternal::swap(base[(size_t)(end - base) / 2], base[0]);

        if (cmp(*pi  , *pj  ) > 0) BLInternal::swap(*pi  , *pj  );
        if (cmp(*base, *pj  ) > 0) BLInternal::swap(*base, *pj  );
        if (cmp(*pi  , *base) > 0) BLInternal::swap(*pi  , *base);

        // Now we have the median for pivot element, entering main loop.
        for (;;) {
          while (pi < pj   && cmp(*++pi, *base) < 0) continue; // Move `i` right until `*i >= pivot`.
          while (pj > base && cmp(*--pj, *base) > 0) continue; // Move `j` left  until `*j <= pivot`.

          if (pi > pj) break;
          BLInternal::swap(*pi, *pj);
        }

        // Move pivot into correct place.
        BLInternal::swap(*base, *pj);

        // Larger subfile base / end to stack, sort smaller.
        if (pj - base > end - pi) {
          // Left is larger.
          *stackptr++ = base;
          *stackptr++ = pj;
          base = pi;
        }
        else {
          // Right is larger.
          *stackptr++ = pi;
          *stackptr++ = end;
          end = pj;
        }
        BL_ASSERT(stackptr <= stack + kStackSize);
      }
      else {
        insertion_sort(base, (size_t)(end - base), cmp);
        if (stackptr == stack)
          break;
        end = *--stackptr;
        base = *--stackptr;
      }
    }
  }
};

} // {Internal}

//! Quick sort.
template<typename T, class Compare = CompareOp<SortOrder::kAscending>>
static BL_INLINE void quick_sort(T* base, size_t size, const Compare& cmp = Compare()) noexcept {
  Internal::QuickSortImpl<T, Compare>::sort(base, size, cmp);
}

//! \}

} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_SUPPORT_ALGORITHM_P_H_INCLUDED
