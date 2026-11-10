// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_JIT_PIPEPRIMITIVES_P_H_INCLUDED
#define BLEND2D_PIPELINE_JIT_PIPEPRIMITIVES_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/compop_p.h>
#include <blend2d/core/format_p.h>
#include <blend2d/core/image_p.h>
#include <blend2d/core/rgba_p.h>
#include <blend2d/pipeline/pipedefs_p.h>
#include <blend2d/pipeline/jit/jitbase_p.h>
#include <blend2d/tables/tables_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_jit
//! \{

//! \namespace bl::Pipeline::JIT
//! Everything related to JIT pipeline generator and runtime.

namespace bl::Pipeline::JIT {

class PipeCompiler;
class PipePart;

class CompOpPart;

class FetchPart;
class FetchGradientPart;
class FetchPixelPtrPart;
class FetchSolidPart;
class FetchPatternPart;

class FillPart;
class FillBoxAPart;
class FillAnalyticPart;

class GlobalAlpha;

//! Pipeline generator loop-type, used by fillers & compositors.
enum class CMaskLoopType : uint8_t {
  //! Not in a cmask loop mode.
  kNone = 0,
  //! CMask opaque loop (alpha is 1.0).
  kOpaque = 1,
  //! CMask masked loop (alpha is not 1.0).
  kVariant = 2
};

//! Type of the pixel.
//!
//! Not the same as format, PixelType could be a bit simplified.
enum class PixelType : uint8_t {
  kNone = 0,
  kA8 = 1,
  kRGBA32 = 2,
  kRGBA64 = 3
};

enum class PixelFlags : uint32_t {
  kNone = 0,

  //! Scalar alpha or stencil value in `Pixel::sa` (single pixel quantities only).
  kSA = 0x00000001u,
  //! Packed alpha or stencil components stored in `Pixel::pa`.
  kPA = 0x00000002u,
  //! Packed inverted alpha or stencil components stored in `Pixel::pi`.
  kPI = 0x00000004u,
  //! Unpacked alpha or stencil components stored in `Pixel::ua`.
  kUA = 0x00000008u,
  //! Unpacked and inverted alpha or stencil components stored in `Pixel::ui`
  kUI = 0x00000010u,

  //! Packed ARGB32 components stored in `Pixel::pc`.
  kPC = 0x00000020u,
  //! Unpacked ARGB32 components stored in `Pixel::uc`.
  kUC = 0x00000040u,

  //! Last fetch in this scanline, thus at most `N-1` pixels would be used.
  kLastPartial = 0x40000000u,
  //! Fetch read-only, registers won't be modified.
  kImmutable = 0x80000000u,

  kPA_PI_UA_UI = kPA | kPI | kUA | kUI,
  kPC_UC = kPC | kUC
};
BL_DEFINE_ENUM_FLAGS(PixelFlags)

//! Pixel coverage format that is consumed by the compositor.
enum class PixelCoverageFormat : uint8_t {
  //! Uninitialized format (invalid when passed to API that expects an initialized one).
  kNone = 0,
  //! Pixel coverage must be packed.
  kPacked,
  //! Pixel coverage must be unpacked.
  kUnpacked
};

//! Pixel coverage flags.
enum class PixelCoverageFlags : uint8_t {
  //! No coverage flags set.
  kNone = 0,
  //! The coverage is repeated (c-mask fills).
  kRepeated = 0x01,
  //! The coverage is immutable (cannot be altered by the compositor).
  kImmutable = 0x02,

  //! A combination of `kRepeated` and `kImmutable`.
  kRepeatedImmutable = kRepeated | kImmutable
};
BL_DEFINE_ENUM_FLAGS(PixelCoverageFlags)

//! Specifies whether to advance pointers.
enum class AdvanceMode : uint32_t {
  kNoAdvance,
  kAdvance,
  kIgnored
};

//! Specifies gather options.
enum class GatherMode : uint32_t {
  kFetchAll = 0,
  kNeverFull = 1
};

//! Flags used by predicated load and store operations.
enum class PredicateFlags : uint32_t {
  //! No flags specified.
  kNone = 0x00000000u,

  //! Predicate is never full - contains at most `size() - 1` elements to read/write.
  //!
  //! This is a hint to the implementation that can be also used as an assertion.
  kNeverFull = 0x00000001u
};
BL_DEFINE_ENUM_FLAGS(PredicateFlags)

//! Options used by pixel fetchers.
struct PixelFetchInfo {
  //! \name Members
  //! \{

  //! Pixel format.
  FormatExt _format {};

  //! Pixel components, compatible with \ref BLFormatFlags.
  uint8_t _components {};

  //! A byte offset (memory) where the alpha can be accessed.
  //!
  //! This offset can be added to a memory operand on architectures that provide addressing modes with offsets.
  uint8_t _alpha_offset {};

  //! A byte offset already applied to a pointer
  //!
  //! This is used in cases in which the pipeline loads pixels in a scalar way (for example extend modes are applied).
  uint8_t _applied_offset {};

  //! \}

  //! \name Construction & Destruction
  //! \{

  BL_INLINE_NODEBUG PixelFetchInfo() noexcept = default;
  BL_INLINE_NODEBUG PixelFetchInfo(const PixelFetchInfo& other) noexcept = default;
  BL_INLINE_NODEBUG explicit PixelFetchInfo(FormatExt format) noexcept { init(format); }

  //! \}

  //! \name Initialization
  //! \{

  BL_INLINE void init(FormatExt format) noexcept {
    _format = format;
    _components = uint8_t(bl_format_info[size_t(format)].flags & 0xFFu);
    _alpha_offset = uint8_t(bl_format_info[size_t(format)].a_shift / 8u);
    _applied_offset = 0;
  }

  //! Makes the current `byte_offset` applies, which means that ALL source pointers have `byte_offset` incremented.
  BL_INLINE void apply_alpha_offset() noexcept { _applied_offset = _alpha_offset; }

  //! \}

  //! \name Interface
  //! \{

  //! Returns pixel format.
  BL_INLINE_NODEBUG FormatExt format() const noexcept { return _format; }
  //! Returns source pixel format information.
  BL_INLINE_NODEBUG BLFormatInfo format_info() const noexcept { return bl_format_info[size_t(_format)]; }
  //! Returns bytes per pixel.
  BL_INLINE_NODEBUG uint32_t bpp() const noexcept { return bl_format_info[size_t(_format)].depth / 8u; }

  //! Returns a byte offset of the alpha component that can be applied when loading alpha component from memory.
  BL_INLINE_NODEBUG int alpha_offset() const noexcept { return _alpha_offset; }

  //! Returns a byte offset that has been already applied to source pointer(s).
  BL_INLINE_NODEBUG int applied_offset() const noexcept { return _applied_offset; }

  //! Calculates the offset that must be used to fetch a full pixel and not just the alpha.
  BL_INLINE_NODEBUG int fetch_pixel_offset() const noexcept { return -int(_applied_offset); }

  //! Calculates the offset that must be used when fetching alpha component from a possibly adjusted source pointer.
  //!
  //! \note When the offset has been applied the return value should be 0 as that's the purpose of applying it,
  //! however, when the offset hasn't been applied, the returned value would be the same as `byte_offset`.
  BL_INLINE_NODEBUG int fetch_alpha_offset() const noexcept { return int(_alpha_offset) - int(_applied_offset); }

  // Returns whether the pixel format has RGB components.
  BL_INLINE_NODEBUG bool has_rgb() const noexcept { return (_components & BL_FORMAT_FLAG_RGB) != 0; }

  // Returns whether the pixel format has Alpha component.
  BL_INLINE_NODEBUG bool has_alpha() const noexcept { return (_components & BL_FORMAT_FLAG_ALPHA) != 0; }

  //! \}
};

//! Provides an abstraction regarding predicated loads and stores.
//!
//! Predicated composition may improve performance of span tails if the number of pixels to process is greater
//! than 1 and the processing pipeline can efficiently process more than 4 pixels. In that case it's better to
//! always use predicated loads and stores even if it would have to be emitted as branches.
//!
//! Predicates can also be used without masking, however, in that case branches may be emitted instead of
//! predicated (or masked) loads and stores. This is selected automatically depending on the CPU micro-architecture
//! and features.
struct PixelPredicate {
  //! Maximum number of elements that can be loaded / stored.
  //!
  //! This is typically power of 2 minus one - for example 8 pixel wide pipeline would use predicated loads and
  //! stores for 0-7 pixels.
  uint32_t _size {};
  //! Predicate flags.
  PredicateFlags _flags {};

  //! Number of pixels to load/store (starting at #0).
  //!
  //! For example if count is 3, pixels at [0, 1, 2] will be fetched / stored.
  Gp _count;

#if defined(BL_JIT_ARCH_X86)
  static inline constexpr uint32_t kMaterializedMaskCapacity = 2u;

  //! Contains predicates for load/store instructions that were materialized.
  struct MaterializedMask {
    //! The number of elements to access from the end.
    //!
    //! Non-zero offsets are used in cases in which there is multiple registers that are written by using predicates.
    //! In that case the access to the first register can be branched, and only the access to the last register can
    //! actually use predicate, at least this is how it's been designed.
    uint8_t last_n {};

    //! Element size in case this is a vector predicate (always zero when it's a {k} predicate).
    uint8_t element_size {};

    uint8_t reserved[2] {};

    //! Mask register - either an AVX-512 mask (k register) or an xmm/ymm/zmm vector.
    Reg mask {};
  };

  uint32_t _materialized_count {};
  MaterializedMask _materialized_masks[kMaterializedMaskCapacity];
#endif // BL_JIT_ARCH_X86

  static inline constexpr uint32_t kMaterializedEndPtrCapacity = 2u;

  //! Contains two last clamped pointers of `ref`.
  struct MaterializedEndPtr {
    //! Reference pointer (this is the register used to calculate `end1` and `end2`)
    Gp ref;
    //! `unsigned_min(ref + 1 * N, ref + (count - 1) * N)`.
    Gp adjusted1;
    //! `unsigned_min(ref + 2 * N, ref + (count - 1) * N)`.
    Gp adjusted2;
  };

  uint32_t _materialized_end_ptr_count {};
  MaterializedEndPtr _materialized_end_ptr_data[kMaterializedEndPtrCapacity];

  BL_INLINE_NODEBUG PixelPredicate() noexcept = default;
  BL_INLINE explicit PixelPredicate(uint32_t size, PredicateFlags flags, const Gp& count) noexcept { init(size, flags, count); }

  BL_INLINE void init(uint32_t size, PredicateFlags flags, const Gp& count) noexcept {
    _size = size;
    _flags = flags;
    _count = count;
  }

  BL_INLINE_NODEBUG bool is_empty() const noexcept { return _size == 0; }
  BL_INLINE_NODEBUG uint32_t size() const noexcept { return _size; }

  BL_INLINE_NODEBUG PredicateFlags flags() const noexcept { return _flags; }
  BL_INLINE_NODEBUG bool is_never_full() const noexcept { return bl_test_flag(_flags, PredicateFlags::kNeverFull); }

  BL_INLINE_NODEBUG const Gp& count() const noexcept { return _count; }

  BL_INLINE_NODEBUG GatherMode gather_mode() const noexcept {
    return is_never_full() ? GatherMode::kNeverFull : GatherMode::kFetchAll;
  }

  BL_INLINE const MaterializedEndPtr* find_materialized_end_ptr(const Gp& ref) const noexcept {
    for (uint32_t i = 0; i < _materialized_end_ptr_count; i++)
      if (_materialized_end_ptr_data[i].ref.id() == ref.id())
        return &_materialized_end_ptr_data[i];
    return nullptr;
  }

  BL_INLINE void add_materialized_end_ptr(const Gp& ref, const Gp& adjusted1, const Gp& adjusted2) noexcept {
    if (_materialized_end_ptr_count >= kMaterializedEndPtrCapacity)
      return;

    uint32_t i = _materialized_end_ptr_count;
    _materialized_end_ptr_data[i].ref = ref;
    _materialized_end_ptr_data[i].adjusted1 = adjusted1;
    _materialized_end_ptr_data[i].adjusted2 = adjusted2;
    _materialized_end_ptr_count++;
  }
};

//! Represents either Alpha or RGBA pixel.
//!
//! Convention used to define and process pixel components:
//!
//!   - Prefixes:
//!     - "p"  - packed pixel(s) or component(s).
//!     - "u"  - unpacked pixel(s) or component(s).
//!
//!   - Components:
//!     - "c"  - Pixel components (ARGB).
//!     - "a"  - Pixel alpha values (A).
//!     - "i"  - Inverted pixel alpha values (IA).
//!     - "m"  - Mask (not part of the pixel itself, comes from a FillPart).
//!     - "im" - Mask (not part of the pixel itself, comes from a FillPart).
class Pixel {
public:
  PixelType _type;
  char _name[15];

  PixelFlags _flags;
  PixelCount _count;

  //! Scalar alpha component (single value only, no packing/unpacking here).
  Gp sa;
  //! Packed alpha components.
  VecArray pa;
  //! Packed inverted alpha components.
  VecArray pi;
  //! Unpacked alpha components.
  VecArray ua;
  //! Unpacked and inverted alpha components.
  VecArray ui;
  //! Packed ARGB32 pixel(s), maximum 8, 16, or 32, depending on SIMD width.
  VecArray pc;
  //! Unpacked ARGB32 pixel(s), maximum 8, 16, or 32, depending on SIMD width.
  VecArray uc;

  BL_NOINLINE Pixel(PixelType type = PixelType::kNone) noexcept
    : _type(type),
      _name {},
      _flags(PixelFlags::kNone),
      _count(PixelCount(0)) {}

  BL_NOINLINE Pixel(const char* name, PixelType type = PixelType::kNone) noexcept
    : _type(type),
      _name {},
      _flags(PixelFlags::kNone),
      _count(PixelCount(0)) { set_name(name); }

  BL_INLINE void reset(PixelType type = PixelType::kNone) noexcept {
    _type = type;
    memset(_name, 0, BL_ARRAY_SIZE(_name));
    reset_all_except_type_and_name();
  }

  BL_NOINLINE void reset_all_except_type_and_name() noexcept {
    _flags = PixelFlags::kNone;
    _count = PixelCount(0);
    sa.reset();
    pa.reset();
    ua.reset();
    ui.reset();
    pc.reset();
    uc.reset();
  }

  BL_INLINE_NODEBUG PixelType type() const noexcept { return _type; }
  BL_INLINE void set_type(PixelType type) noexcept { _type = type; }

  BL_INLINE_NODEBUG bool isA8() const noexcept { return _type == PixelType::kA8; }
  BL_INLINE_NODEBUG bool isRGBA32() const noexcept { return _type == PixelType::kRGBA32; }
  BL_INLINE_NODEBUG bool isRGBA64() const noexcept { return _type == PixelType::kRGBA64; }

  BL_INLINE_NODEBUG const char* name() const noexcept { return _name; }

  BL_NOINLINE void set_name(const char* name) noexcept {
    size_t len = strnlen(name, BL_ARRAY_SIZE(_name) - 2);
    _name[0] = '\0';

    if (len) {
      memcpy(_name, name, len);
      _name[len + 0] = '.';
      _name[len + 1] = '\0';
    }
  }

  BL_INLINE_NODEBUG PixelFlags flags() const noexcept { return _flags; }
  //! Tests whether all members are immutable (solid fills).
  BL_INLINE_NODEBUG bool is_immutable() const noexcept { return bl_test_flag(_flags, PixelFlags::kImmutable); }
  //! Tests whether this pixel was a partial fetch (the last pixel could be missing).
  BL_INLINE_NODEBUG bool is_last_partial() const noexcept { return bl_test_flag(_flags, PixelFlags::kLastPartial); }

  BL_INLINE void make_immutable() noexcept { _flags |= PixelFlags::kImmutable; }

  BL_INLINE void set_immutable(bool immutable) noexcept {
    _flags = (_flags & ~PixelFlags::kImmutable) | (immutable ? PixelFlags::kImmutable : PixelFlags::kNone);
  }

  BL_INLINE_NODEBUG PixelCount count() const noexcept { return _count; }
  BL_INLINE void set_count(PixelCount count) noexcept { _count = count; }
};

//! Optimized pixel representation used by solid fills.
//!
//! Used by both Alpha and RGBA pixel pipelines.
class SolidPixel {
public:
  BL_INLINE SolidPixel() noexcept { reset(); }

  //! Scalar alpha or stencil value (A8 pipeline).
  Gp sa;
  //! Scalar pre-processed component, shown as "X" in equations.
  Gp sx;
  //! Scalar pre-processed component, shown as "Y" in equations.
  Gp sy;

  //! Packed pre-processed components, shown as "X" in equations.
  Vec px;
  //! Packed pre-processed components, shown as "Y" in equations.
  Vec py;
  //! Unpacked pre-processed components, shown as "X" in equations.
  Vec ux;
  //! Unpacked pre-processed components, shown as "Y" in equations.
  Vec uy;

  //! Mask vector.
  Vec vm;
  //! Inverted mask vector.
  Vec vn;

  BL_INLINE void reset() noexcept {
    sa.reset();
    sx.reset();
    sy.reset();

    px.reset();
    ux.reset();

    py.reset();
    uy.reset();

    vm.reset();
    vn.reset();
  }
};

//! A constant mask (CMASK) stored in either GP or XMM register.
struct PipeCMask {
  //! Mask scalar.
  Gp sm;
  //! Inverted mask scalar.
  Gp sn;

  //! Mask vector.
  Vec vm;
  //! Inverted mask vector.
  Vec vn;

  BL_INLINE void reset() noexcept {
    OpUtils::reset_var_struct<PipeCMask>(this);
  }
};

} // {bl::Pipeline::JIT}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_JIT_PIPEPRIMITIVES_P_H_INCLUDED
