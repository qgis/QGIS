// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_TRACE_P_H_INCLUDED
#define BLEND2D_TRACE_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

//! Dummy trace - no tracing, no runtime overhead.
class BLDummyTrace {
public:
  BL_INLINE bool enabled() const noexcept { return false; };
  BL_INLINE void indent() noexcept {}
  BL_INLINE void deindent() noexcept {}

  template<typename... Args>
  BL_INLINE void out(Args&&...) noexcept {}

  template<typename... Args>
  BL_INLINE void info(Args&&...) noexcept {}

  template<typename... Args>
  BL_INLINE bool warn(Args&&...) noexcept { return false; }

  template<typename... Args>
  BL_INLINE bool fail(Args&&...) noexcept { return false; }
};

//! Debug trace - active / enabled trace that can be useful during debugging.
class BLDebugTrace {
public:
  BL_INLINE BLDebugTrace() noexcept
    : indentation(0) {}
  BL_INLINE BLDebugTrace(const BLDebugTrace& other) noexcept
    : indentation(other.indentation) {}

  BL_INLINE bool enabled() const noexcept { return true; };
  BL_INLINE void indent() noexcept { indentation++; }
  BL_INLINE void deindent() noexcept { indentation--; }

  template<typename... Args>
  BL_INLINE void out(Args&&... args) noexcept { log(0, 0xFFFFFFFFu, BLInternal::forward<Args>(args)...); }

  template<typename... Args>
  BL_INLINE void info(Args&&... args) noexcept { log(0, indentation, BLInternal::forward<Args>(args)...); }

  template<typename... Args>
  BL_INLINE bool warn(Args&&... args) noexcept { log(1, indentation, BLInternal::forward<Args>(args)...); return false; }

  template<typename... Args>
  BL_INLINE bool fail(Args&&... args) noexcept { log(2, indentation, BLInternal::forward<Args>(args)...); return false; }

  BL_HIDDEN static void log(uint32_t severity, uint32_t indentation, const char* fmt, ...) noexcept;

  int indentation;
};

//! \}
//! \endcond

#endif // BLEND2D_TRACE_P_H_INCLUDED
