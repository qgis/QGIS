// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/core/trace_p.h>

// BLDebugTrace - Log
// ==================

void BLDebugTrace::log(uint32_t severity, uint32_t indentation, const char* fmt, ...) noexcept {
  const char* prefix = "";
  if (indentation < 0xFFFFFFFFu) {
    switch (severity) {
      case 1: prefix = "[WARN] "; break;
      case 2: prefix = "[FAIL] "; break;
    }
    bl_runtime_message_fmt("%*s%s", indentation * 2, "", prefix);
  }

  va_list ap;
  va_start(ap, fmt);
  bl_runtime_message_vfmt(fmt, ap);
  va_end(ap);
}
