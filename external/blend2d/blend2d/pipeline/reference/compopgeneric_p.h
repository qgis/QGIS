// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_REFERENCE_COMPOPGENERIC_P_H_INCLUDED
#define BLEND2D_PIPELINE_REFERENCE_COMPOPGENERIC_P_H_INCLUDED

#include <blend2d/core/compop_p.h>
#include <blend2d/pipeline/pipedefs_p.h>
#include <blend2d/pipeline/reference/pixelgeneric_p.h>
#include <blend2d/pipeline/reference/fetchgeneric_p.h>
#include <blend2d/pixelops/scalar_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_reference
//! \{

namespace bl::Pipeline::Reference {
namespace {

using Pixel::Repeat;

template<typename PixelT>
struct CompOp_SrcCopy_Op {
  typedef PixelT PixelType;

  enum : uint32_t {
    kCompOp = BL_COMP_OP_SRC_COPY,
    kOptimizeOpaque = 1
  };

  static BL_INLINE PixelType op_prgb32_prgb32(PixelType d, PixelType s) noexcept {
    bl_unused(d);
    return s;
  }

  static BL_INLINE PixelType op_prgb32_prgb32(PixelType d, PixelType s, uint32_t m) noexcept {
    return (d.unpack() * Repeat{m ^ 0xFFu} + s.unpack() * Repeat{m}).div255().pack();
  }
};

template<typename PixelT>
struct CompOp_SrcOver_Op {
  typedef PixelT PixelType;

  enum : uint32_t {
    kCompOp = BL_COMP_OP_SRC_OVER,
    kOptimizeOpaque = 0
  };

  // Dca' = Sca + Dca.(1 - Sa)
  // Da'  = Sa  + Da .(1 - Sa)
  static BL_INLINE PixelType op_prgb32_prgb32(PixelType d, PixelType s) noexcept {
    return s + (d.unpack() * Repeat{PixelOps::Scalar::neg255(s.a())}).div255().pack();
  }

  // Dca' = Sca.m + Dca.(1 - Sa.m)
  // Da'  = Sa .m + Da .(1 - Sa.m)
  static BL_INLINE PixelType op_prgb32_prgb32(PixelType d, PixelType s, uint32_t m) noexcept {
    return op_prgb32_prgb32(d, (s.unpack() * Repeat{m}).div255().pack());
  }
};

template<typename PixelT>
struct CompOp_Plus_Op {
  typedef PixelT PixelType;

  enum : uint32_t {
    kCompOp = BL_COMP_OP_PLUS,
    kOptimizeOpaque = 0
  };

  static BL_INLINE PixelType op_prgb32_prgb32(PixelType d, PixelType s) noexcept {
    return (d.unpack().addus8(s.unpack())).pack();
  }

  static BL_INLINE PixelType op_prgb32_prgb32(PixelType d, PixelType s, uint32_t m) noexcept {
    return d.unpack().addus8((s.unpack() * Repeat{m}).div255()).pack();
  }
};

template<typename OpT, typename PixelT, typename FetchOp, uint32_t kDstBPP_>
struct CompOp_Base {
  typedef OpT Op;
  typedef PixelT PixelType;

  enum : uint32_t {
    kDstBPP = kDstBPP_,
    kCompOp = Op::kCompOp,
    kOptimizeOpaque = Op::kOptimizeOpaque
  };

  FetchOp fetch_op;

  static constexpr FormatExt kFormat = PixelTypeToFormat<PixelT>::kFormat;

  BL_INLINE void rect_init_fetch(ContextData* ctx_data, const void* fetch_data, uint32_t x_pos, uint32_t y_pos, uint32_t rect_width) noexcept {
    fetch_op.rect_init_fetch(ctx_data, fetch_data, x_pos, y_pos, rect_width);
  }

  BL_INLINE void rectStartX(uint32_t x_pos) noexcept {
    fetch_op.rectStartX(x_pos);
  }

  BL_INLINE void spanInitY(ContextData* ctx_data, const void* fetch_data, uint32_t y_pos) noexcept {
    fetch_op.spanInitY(ctx_data, fetch_data, y_pos);
  }

  BL_INLINE void spanStartX(uint32_t x_pos) noexcept {
    fetch_op.spanStartX(x_pos);
  }

  BL_INLINE void spanAdvanceX(uint32_t x_pos, uint32_t x_diff) noexcept {
    fetch_op.spanAdvanceX(x_pos, x_diff);
  }

  BL_INLINE void spanEndX(uint32_t x_pos) noexcept {
    fetch_op.spanEndX(x_pos);
  }

  BL_INLINE void advance_y() noexcept {
    fetch_op.advance_y();
  }

  BL_INLINE uint8_t* composite_pixel_opaque(uint8_t* dst_ptr) noexcept {
    if (uint32_t(OpT::kCompOp) == BL_COMP_OP_SRC_COPY) {
      PixelIO<PixelT, kFormat>::store(dst_ptr, fetch_op.fetch());
      return dst_ptr + kDstBPP;
    }
    else {
      PixelIO<PixelT, kFormat>::store(dst_ptr, OpT::op_prgb32_prgb32(PixelIO<PixelT, kFormat>::fetch(dst_ptr), fetch_op.fetch()));
      return dst_ptr + kDstBPP;
    }
  }

  BL_INLINE uint8_t* composite_pixel_masked(uint8_t* dst_ptr, uint32_t m) noexcept {
    PixelIO<PixelT, kFormat>::store(dst_ptr, OpT::op_prgb32_prgb32(PixelIO<PixelT, kFormat>::fetch(dst_ptr), fetch_op.fetch(), m));
    return dst_ptr + kDstBPP;
  }

  BL_INLINE uint8_t* compositeCSpanOpaque(uint8_t* dst_ptr, size_t w) noexcept {
    size_t i = w;
    do {
      dst_ptr = composite_pixel_opaque(dst_ptr);
    } while (--i);
    return dst_ptr;
  }

  BL_INLINE uint8_t* compositeCSpanMasked(uint8_t* dst_ptr, size_t w, uint32_t m) noexcept {
    size_t i = w;
    do {
      dst_ptr = composite_pixel_masked(dst_ptr, m);
    } while (--i);
    return dst_ptr;
  }

  BL_INLINE uint8_t* compositeCSpan(uint8_t* dst_ptr, size_t w, uint32_t m) noexcept {
    if (kOptimizeOpaque && m == 255)
      return compositeCSpanOpaque(dst_ptr, w);
    else
      return compositeCSpanMasked(dst_ptr, w, m);
  }

  BL_INLINE uint8_t* compositeVSpanWithGA(uint8_t* BL_RESTRICT dst_ptr, const uint8_t* BL_RESTRICT mask_ptr, size_t w) noexcept {
    size_t i = w;
    do {
      uint32_t msk = mask_ptr[0];
      dst_ptr = composite_pixel_masked(dst_ptr, msk);
      mask_ptr++;
    } while (--i);
    return dst_ptr;
  }

  BL_INLINE uint8_t* compositeVSpanWithoutGA(uint8_t* BL_RESTRICT dst_ptr, const uint8_t* BL_RESTRICT mask_ptr, uint32_t global_alpha, size_t w) noexcept {
    size_t i = w;
    do {
      uint32_t msk = PixelOps::Scalar::udiv255(uint32_t(mask_ptr[0]) * global_alpha);
      mask_ptr++;
      dst_ptr = composite_pixel_masked(dst_ptr, msk);
    } while (--i);
    return dst_ptr;
  }
};

} // {anonymous}
} // {bl::Pipeline::Reference}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_REFERENCE_COMPOPGENERIC_P_H_INCLUDED
