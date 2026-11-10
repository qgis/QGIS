// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_JIT_FETCHUTILSPIXELGATHER_P_H_INCLUDED
#define BLEND2D_PIPELINE_JIT_FETCHUTILSPIXELGATHER_P_H_INCLUDED

#include <blend2d/core/format.h>
#include <blend2d/pipeline/jit/pipecompiler_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_jit
//! \{

namespace bl::Pipeline::JIT {

// Interleave callback is used to interleave a sequence of code into pixel fetching sequence. There are two scenarios in
// general:
//
//   - Fetching is performed by scalar loads and shuffles to form the destination pixel. In this case individual fetches
//     can be interleaved with another code to hide the latency of reading from memory and shuffling.
//   - Fetching is performed by hardware (vpgatherxx). In this case the interleave code is inserted after gather to hide
//     its latency (i.e. to not immediately depend on gathered content).
typedef void (*InterleaveCallback)(uint32_t step, void* data) noexcept;

static void dummy_interleave_callback(uint32_t step, void* data) noexcept { bl_unused(step, data); }

namespace FetchUtils {

enum class IndexLayout : uint32_t {
  //! Consecutive unsigned 16-bit indexes.
  kUInt16,
  //! Consecutive unsigned 32-bit indexes.
  kUInt32,
  //! Unsigned 16-bit indexes in lo 32-bit words (or, even 16-bit indexes).
  kUInt32Lo16,
  //! Unsigned 16-bit indexes in hi 32-bit words (or, odd 16-bit indexes).
  kUInt32Hi16
};

//! Index extractor makes it easy to extract indexes from SIMD registers. We have learned the hard way that the best
//! way of extracting indexes is to use stack instead of dedicated instructions like PEXTRW/PEXTRD. The problem of such
//! instructions is that they have high latency on many older micro-architectures. Newer architectures decreased the
//! latency, but even 2-3 cycles is worse than fetching the index from stack.
class IndexExtractor {
public:
  BL_NONCOPYABLE(IndexExtractor)

  enum Type : uint32_t {
    kTypeNone,
    kTypeInt16,
    kTypeUInt16,
    kTypeInt32,
    kTypeUInt32,
    kTypeCount
  };

  //! \name Members
  //! \{

  PipeCompiler* _pc;
  Vec _vec;
  Mem _mem;
  uint32_t _type;
  uint16_t _index_size;
  uint16_t _mem_size;

  //! \}

  //! Creates a zero-initialized `IndexExtractor`.
  //!
  //! You must call `begin()` to make it usable.
  explicit IndexExtractor(PipeCompiler* pc) noexcept;

  //! Begins index extraction from a SIMD register `vec`.
  void begin(uint32_t type, const Vec& vec) noexcept;
  //! Begins index extraction from memory.
  void begin(uint32_t type, const Mem& mem, uint32_t mem_size) noexcept;
  //! Extracts the given `index` into the destination register `dst`.
  void extract(const Gp& dst, uint32_t index) noexcept;
};

//! Context that is used to fetch more than 1 pixel - used by SIMD fetchers that fetch 2, 4, 8, 16, or 32 pixels per
//! a single loop iteration.
class FetchContext {
public:
  BL_NONCOPYABLE(FetchContext)

  enum class FetchMode : uint8_t {
    //! Uninitialized.
    kNone = 0,

    //! Fetching packed A8 pixels from A8 buffer.
    kA8FromA8_PA,
    //! Fetching unpacked A8 pixels from A8 buffer.
    kA8FromA8_UA,
    //! Fetching packed A8 pixels from RGBA32 buffer.
    kA8FromRGBA32_PA,
    //! Fetching unpacked A8 pixels from RGBA32 buffer.
    kA8FromRGBA32_UA,

    //! Fetching packed RGBA32 pixels from A8 buffer.
    kRGBA32FromA8_PC,
    //! Fetching unpacked RGBA32 pixels from A8 buffer.
    kRGBA32FromA8_UC,

    //! Fetching packed RGBA32 pixels from RGBA32 buffer.
    kRGBA32FromRGBA32_PC,
    //! Fetching unpacked RGBA32 pixels from RGBA32 buffer.
    kRGBA32FromRGBA32_UC,

    //! Fetching packed RGBA64 pixels from RGBA64 buffer.
    kRGBA64FromRGBA64_PC
  };

  //! Widening operation that will be used to widen vectors (128-bit -> 256-bit and 256-bit -> 512-bit).
  enum class WideningOp {
    //! No operation (widening is disabled or not supported due to architecture constraints).
    kNone = 0,
    //! Interleave two vectors into a larger vector.
    kInterleave = 1,
    //! Unpack vector (convert from packed pixel format to unpacked).
    kUnpack = 2,
    //! Unpack 2 128-bit vectors.
    kUnpack2x = 3,
    //! Repeats a vector - repeats DCBA to DDCCBBAA.
    kRepeat = 4,
    //! Repeats a vector of 8 packed A8 pixels into 8 unpacked RGBA32 pixels (AVX-512 only).
    kRepeat8xA8ToRGBA32_UC_AVX512 = 5
  };

  //! \name Members
  //! \{

  //! Pipeline compiler.
  PipeCompiler* _pc {};
  //! Target pixel.
  Pixel* _pixel {};

  //! Required fetch flags, possibly enhanced to make the fetching logic simpler.
  PixelFlags _fetch_flags {};
  //! Pixel fetch information.
  PixelFetchInfo _fetch_info {};
  //! The index of the pixel to be fetched next, starts at 0.
  uint32_t _pixel_index {};
  //! The current vector index where the pixel will be fetched, incremented by `_vec_step`.
  uint32_t _vec_index {};
  //! The step `_vec_index` is incremented by when a vector lane reaches its end.
  uint32_t _vec_step {};
  //! The current lane index where the next pixel will be fetched.
  uint32_t _lane_index {};
  //! Number of lanes to fetch pixels to - when `_lane_index` reaches `_lane_count` it's zeroed and `_vec_index`
  //! incremented by `_vec_step`.
  uint32_t _lane_count {};
  //! Fetch mode - selects the approach used to fetch pixels.
  FetchMode _fetch_mode {};

  //! Fetch is predicated and never contains all the pixels (the fetcher can avoid checking the last pixel).
  GatherMode _gather_mode {};
  //! Number of 128-bit vectors to fetch into.
  uint8_t _p128_count {};
  //! Fetch was completed, `end()` was already called.
  bool _fetch_done = false;

  //! Temporary vector register that can be used as a load destination before inserting to any of _p128[].
  Vec _p_tmp[2];

  //! Temporaries used by the fetcher.
  //!
  //! The maximum vector width of pTmp is 128-bits, because in general we cannot do element-wise insertion to wider
  //! vectors. This is consistent on all backends at the moment as AArch64 only has 128-bit vectors and X86_64 only
  //! allows insertion to 128-bit low part of a vector register, clearing the rest of it.
  Vec _p128[8];

#if defined(BL_JIT_ARCH_X86)
  //! Accumulator used by Alpha only fetches.
  //!
  //! On many targets this may be faster than using SIMD for 8-bit or 16-bit inserts.
  Gp _a_acc;

  //! Current index in `_a_acc` register.
  uint32_t _a_acc_index {};

  //! 256-bit wide vectors that can be used by pipelines taking advantage of AVX2 and AVX-512 extensions.
  VecArray _p256;
  //! 512-bit wide vectors that can be used by pipelines taking advantage of AVX-512 extensions.
  VecArray _p512;

  //! Widening operation used to widen 128-bit vectors to 256-bit vectors.
  WideningOp _widening256_op {};
  //! Widening operation used to widen 256-bit vectors to 512-bit vectors.
  WideningOp _widening512_op {};
#endif // BL_JIT_ARCH_X86

  //! \}

  inline FetchContext(PipeCompiler* pc, Pixel* pixel, PixelCount n, PixelFlags flags, PixelFetchInfo f_info, GatherMode mode = GatherMode::kFetchAll) noexcept
    : _pc(pc),
      _pixel(pixel),
      _fetch_flags(flags),
      _fetch_info(f_info),
      _gather_mode(mode),
      _fetch_done(false) { _init(n); }

  BL_INLINE_NODEBUG FormatExt fetch_format() const noexcept { return _fetch_info.format(); }
  BL_INLINE_NODEBUG FetchMode fetch_mode() const noexcept { return _fetch_mode; }
  BL_INLINE_NODEBUG GatherMode gather_mode() const noexcept { return _gather_mode; }

  void _init(PixelCount n) noexcept;
  void _init_fetch_mode() noexcept;
  void _init_fetch_regs() noexcept;
  void _init_target_pixel() noexcept;

  void fetch_pixel(const Mem& src) noexcept;
  void _fetch_all(const Mem& src, uint32_t src_shift, IndexExtractor& extractor, const uint8_t* indexes, InterleaveCallback cb, void* cb_data) noexcept;
  void _done_vec(uint32_t index) noexcept;

  // Fetches all pixels and allows to interleave the fetch sequence with a lambda function `interleave_func`.
  template<class InterleaveFunc>
  void fetch_all(const Mem& src, uint32_t src_shift, IndexExtractor& extractor, const uint8_t* indexes, InterleaveFunc&& interleave_func) noexcept {
    _fetch_all(src, src_shift, extractor, indexes, [](uint32_t step, void* data) noexcept {
      (*static_cast<const InterleaveFunc*>(data))(step);
    }, (void*)&interleave_func);
  }

  void end() noexcept;
};

void gather_pixels(PipeCompiler* pc, Pixel& p, PixelCount n, PixelFlags flags, PixelFetchInfo f_info, const Mem& src, const Vec& idx, uint32_t shift, IndexLayout index_layout, GatherMode mode, InterleaveCallback cb, void* cb_data) noexcept;

template<class InterleaveFunc>
static void gather_pixels(PipeCompiler* pc, Pixel& p, PixelCount n, PixelFlags flags, PixelFetchInfo f_info, const Mem& src, const Vec& idx, uint32_t shift, IndexLayout index_layout, GatherMode mode, InterleaveFunc&& interleave_func) noexcept {
  gather_pixels(pc, p, n, flags, f_info, src, idx, shift, index_layout, mode, [](uint32_t step, void* data) noexcept {
    (*static_cast<const InterleaveFunc*>(data))(step);
  }, (void*)&interleave_func);
}

} // {FetchUtils}
} // {bl::Pipeline::JIT}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_JIT_FETCHUTILSPIXELGATHER_P_H_INCLUDED
