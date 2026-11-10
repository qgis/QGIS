// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_RADEFS_P_H_INCLUDED
#define ASMJIT_CORE_RADEFS_P_H_INCLUDED

#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_COMPILER

#include <asmjit/core/archtraits.h>
#include <asmjit/core/builder.h>
#include <asmjit/core/compilerdefs.h>
#include <asmjit/core/logger.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/type.h>
#include <asmjit/support/arena.h>
#include <asmjit/support/arenabitset_p.h>
#include <asmjit/support/arenavector.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_ra
//! \{

#ifndef ASMJIT_NO_LOGGING
# define ASMJIT_RA_LOG_FORMAT(...)  \
  do {                              \
    if (ASMJIT_UNLIKELY(logger))    \
      logger->logf(__VA_ARGS__);    \
  } while (0)
# define ASMJIT_RA_LOG_COMPLEX(...) \
  do {                              \
    if (ASMJIT_UNLIKELY(logger)) {  \
      __VA_ARGS__                   \
    }                               \
  } while (0)
#else
# define ASMJIT_RA_LOG_FORMAT(...) ((void)0)
# define ASMJIT_RA_LOG_COMPLEX(...) ((void)0)
#endif

// Forward declarations.
struct RATiedReg;

class BaseRAPass;
class RABlock;
class BaseNode;
struct RAStackSlot;

using RAWorkRegVector = ArenaVector<RAWorkReg*>;

//! Work register identifier (RA).
//!
//! Work register is an actual virtual register that is used by the function and subject to register allocation.
enum class RAWorkId : uint32_t {};

//! Basic block identifier (RA).
enum class RABlockId : uint32_t {};

//! Invalid register identifier or unassigned yet.
static constexpr RAWorkId kBadWorkId = RAWorkId(Globals::kInvalidId);

//! Invalid block identifier or unassigned yet.
static constexpr RABlockId kBadBlockId = RABlockId(Globals::kInvalidId);

//! Maximum number of consecutive registers aggregated from all supported backends.
static constexpr uint32_t kMaxConsecutiveRegs = 4;

enum class RAStrategyType : uint8_t {
  kSimple  = 0,
  kComplex = 1
};
ASMJIT_DEFINE_ENUM_COMPARE(RAStrategyType)

enum class RAStrategyFlags : uint8_t {
  kNone = 0
};
ASMJIT_DEFINE_ENUM_FLAGS(RAStrategyFlags)

//! Register allocation strategy.
//!
//! The idea is to select the best register allocation strategy for each virtual register group based on the
//! complexity of the code.
struct RAStrategy {
  //! \name Members
  //! \{

  RAStrategyType _type = RAStrategyType::kSimple;
  RAStrategyFlags _flags = RAStrategyFlags::kNone;

  //! \}

  //! \name Accessors
  //! \{

  ASMJIT_INLINE_NODEBUG void reset() noexcept {
    _type = RAStrategyType::kSimple;
    _flags = RAStrategyFlags::kNone;
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAStrategyType type() const noexcept { return _type; }

  ASMJIT_INLINE_NODEBUG void set_type(RAStrategyType type) noexcept { _type = type; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_simple() const noexcept { return _type == RAStrategyType::kSimple; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_complex() const noexcept { return _type >= RAStrategyType::kComplex; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAStrategyFlags flags() const noexcept { return _flags; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(RAStrategyFlags flag) const noexcept { return Support::test(_flags, flag); }

  ASMJIT_INLINE_NODEBUG void add_flags(RAStrategyFlags flags) noexcept { _flags |= flags; }

  //! \}
};

//! Count of virtual or physical registers per group.
//!
//! \note This class uses 8-bit integers to represent counters, it's only used in places where this is sufficient,
//! for example total count of machine's physical registers, count of virtual registers per instruction, etc...
//! There is also `RALiveCount`, which uses 32-bit integers and is indeed much safer.
struct RARegCount {
  //! \name Members
  //! \{

  uint32_t _counters;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Resets all counters to zero.
  ASMJIT_INLINE_NODEBUG void reset() noexcept { _counters = 0; }

  //! \}

  //! \name Overloaded Operators
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator==(const RARegCount& other) const noexcept { return _counters == other._counters; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator!=(const RARegCount& other) const noexcept { return _counters != other._counters; }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the count of registers by the given register `group`.
  [[nodiscard]]
  ASMJIT_INLINE uint32_t get(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);

    uint32_t shift = uint32_t(group) * 8u;
    return (_counters >> shift) & 0xFFu;
  }

  //! Sets the register count by a register `group`.
  ASMJIT_INLINE void set(RegGroup group, uint32_t n) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    ASMJIT_ASSERT(n <= 0xFFu);

    uint32_t shift = uint32_t(group) * 8u;
    _counters = (_counters & ~uint32_t(0xFFu << shift)) + (n << shift);
  }

  //! Adds the register count by a register `group`.
  ASMJIT_INLINE void add(RegGroup group, uint32_t n = 1) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    ASMJIT_ASSERT(get(group) + n <= 0xFFu);

    uint32_t shift = uint32_t(group) * 8u;
    _counters += n << shift;
  }

  //! \}
};

//! Provides mapping that can be used to fast index architecture register groups.
struct RARegIndex : public RARegCount {
  //! Build register indexes based on the given `count` of registers.
  ASMJIT_INLINE void build_indexes(const RARegCount& count) noexcept {
    ASMJIT_ASSERT(count.get(RegGroup(0)) + count.get(RegGroup(1)) <= 0xFFu);
    ASMJIT_ASSERT(count.get(RegGroup(0)) + count.get(RegGroup(1)) + count.get(RegGroup(2)) <= 0xFFu);

    uint32_t i = count._counters;
    _counters = (i + (i << 8u) + (i << 16)) << 8u;
  }
};

//! Registers mask.
struct RARegMask {
  //! \name Types
  //! \{

  using RegMasks = Support::Array<RegMask, Globals::kNumVirtGroups>;

  //! \}

  //! \name Members
  //! \{

  RegMasks _masks;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Initializes from other `RARegMask`.
  ASMJIT_INLINE_NODEBUG void init(const RARegMask& other) noexcept { _masks = other._masks; }
  //! Initializes directly from an array of masks.
  ASMJIT_INLINE_NODEBUG void init(const RegMasks& masks) noexcept { _masks = masks; }

  //! Resets all register masks to zero.
  ASMJIT_INLINE_NODEBUG void reset() noexcept { _masks.fill(0); }

  //! \}

  //! \name Overloaded Operators
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator==(const RARegMask& other) const noexcept { return _masks == other._masks; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator!=(const RARegMask& other) const noexcept { return _masks != other._masks; }

  template<typename Index>
  [[nodiscard]]
  inline uint32_t& operator[](const Index& index) noexcept { return _masks[index]; }

  template<typename Index>
  [[nodiscard]]
  inline const uint32_t& operator[](const Index& index) const noexcept { return _masks[index]; }

  //! \}

  //! \name Utilities
  //! \{

  //! Tests whether all register masks are zero (empty).
  [[nodiscard]]
  inline bool is_empty() const noexcept {
    return _masks.aggregate<Support::Or>() == 0;
  }

  [[nodiscard]]
  inline bool has(RegGroup group, RegMask mask = 0xFFFFFFFFu) const noexcept {
    return (_masks[group] & mask) != 0;
  }

  template<class Operator>
  inline void op(const RARegMask& other) noexcept {
    _masks.combine<Operator>(other._masks);
  }

  template<class Operator>
  inline void op(RegGroup group, RegMask mask) noexcept {
    _masks[group] = Operator::op(_masks[group], mask);
  }

  inline void clear(RegGroup group, RegMask mask) noexcept {
    _masks[group] = _masks[group] & ~mask;
  }

  inline void clear(const RegMasks& masks) noexcept {
    _masks.combine<Support::AndNot>(masks);
  }

  //! \}
};

//! Information associated with each instruction, propagated to blocks, loops, and the whole function. This
//! information can be used to do minor decisions before the register allocator tries to do its job. For
//! example to use fast register allocation inside a block or loop it cannot have clobbered and/or fixed
//! registers, etc...
class RARegsStats {
public:
  //! \name Constants
  //! \{

  enum Index : uint32_t {
    kIndexUsed       = 0,
    kIndexFixed      = 8,
    kIndexClobbered  = 16
  };

  enum Mask : uint32_t {
    kMaskUsed        = 0xFFu << kIndexUsed,
    kMaskFixed       = 0xFFu << kIndexFixed,
    kMaskClobbered   = 0xFFu << kIndexClobbered
  };

  //! \}

  //! \name Members
  //! \{

  uint32_t _packed = 0;

  //! \}

  //! \name Accessors
  //! \{

  ASMJIT_INLINE_NODEBUG void reset() noexcept { _packed = 0; }
  ASMJIT_INLINE_NODEBUG void combine_with(const RARegsStats& other) noexcept { _packed |= other._packed; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_used() const noexcept { return (_packed & kMaskUsed) != 0u; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_used(RegGroup group) const noexcept { return Support::bit_test(_packed, kIndexUsed + uint32_t(group)); }

  ASMJIT_INLINE_NODEBUG void make_used(RegGroup group) noexcept { _packed |= Support::bit_mask<uint32_t>(kIndexUsed + uint32_t(group)); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_fixed() const noexcept { return (_packed & kMaskFixed) != 0u; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_fixed(RegGroup group) const noexcept { return Support::bit_test(_packed, kIndexFixed + uint32_t(group)); }

  ASMJIT_INLINE_NODEBUG void make_fixed(RegGroup group) noexcept { _packed |= Support::bit_mask<uint32_t>(kIndexFixed + uint32_t(group)); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_clobbered() const noexcept { return (_packed & kMaskClobbered) != 0u; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_clobbered(RegGroup group) const noexcept { return Support::bit_test(_packed, kIndexClobbered + uint32_t(group)); }

  ASMJIT_INLINE_NODEBUG void make_clobbered(RegGroup group) noexcept { _packed |= Support::bit_mask<uint32_t>(kIndexClobbered + uint32_t(group)); }

  //! \}
};

//! Count of live registers, per group.
class RALiveCount {
public:
  //! \name Members
  //! \{

  Support::Array<uint32_t, Globals::kNumVirtGroups> n {};

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG RALiveCount() noexcept = default;
  ASMJIT_INLINE_NODEBUG RALiveCount(const RALiveCount& other) noexcept = default;

  ASMJIT_INLINE_NODEBUG void init(const RALiveCount& other) noexcept { n = other.n; }
  ASMJIT_INLINE_NODEBUG void reset() noexcept { n.fill(0); }

  //! \}

  //! \name Overloaded Operators
  //! \{

  ASMJIT_INLINE_NODEBUG RALiveCount& operator=(const RALiveCount& other) noexcept = default;

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t& operator[](RegGroup group) noexcept { return n[group]; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const uint32_t& operator[](RegGroup group) const noexcept { return n[group]; }

  //! \}

  //! \name Utilities
  //! \{

  template<class Operator>
  inline void op(const RALiveCount& other) noexcept { n.combine<Operator>(other.n); }

  //! \}
};

//! Span that contains start (a) and end (b).
struct RALiveSpan {
  //! \name Constants
  //! \{

  static inline constexpr NodePosition kNaN = NodePosition(0);
  static inline constexpr NodePosition kInf = NodePosition(0xFFFFFFFFu);

  //! \}

  //! \name Members
  //! \{

  NodePosition a {};
  NodePosition b {};

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG RALiveSpan() noexcept = default;
  ASMJIT_INLINE_NODEBUG RALiveSpan(const RALiveSpan& other) noexcept = default;
  ASMJIT_INLINE_NODEBUG RALiveSpan(NodePosition a, NodePosition b) noexcept : a(a), b(b) {}

  ASMJIT_INLINE_NODEBUG void init(NodePosition first, NodePosition last) noexcept {
    a = first;
    b = last;
  }

  ASMJIT_INLINE_NODEBUG void init(const RALiveSpan& other) noexcept { init(other.a, other.b); }
  ASMJIT_INLINE_NODEBUG void reset() noexcept { init(NodePosition(0), NodePosition(0)); }

  //! \}

  //! \name Overloaded Operators
  //! \{

  ASMJIT_INLINE_NODEBUG RALiveSpan& operator=(const RALiveSpan& other) = default;

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_valid() const noexcept { return a < b; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t width() const noexcept { return uint32_t(b) - uint32_t(a); }

  //! \}
};

//! Vector of `RALiveSpan` with additional convenience API.
class RALiveSpans {
public:
  ASMJIT_NONCOPYABLE(RALiveSpans)

  //! \name Members
  //! \{

  ArenaVector<RALiveSpan> _data {};

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG RALiveSpans() noexcept = default;

  ASMJIT_INLINE_NODEBUG void reset() noexcept { _data.reset(); }
  ASMJIT_INLINE_NODEBUG void release(Arena& arena) noexcept { _data.release(arena); }

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _data.is_empty(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t size() const noexcept { return _data.size(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RALiveSpan* data() noexcept { return _data.data(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RALiveSpan* data() const noexcept { return _data.data(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_open() const noexcept {
    size_t size = _data.size();
    return size && _data[size - 1].b == RALiveSpan::kInf;
  }

  //! \}

  //! \name Utilities
  //! \{

  ASMJIT_INLINE_NODEBUG void swap(RALiveSpans& other) noexcept { _data.swap(other._data); }

  //! Open the current live span.
  ASMJIT_INLINE Error open_at(Arena& arena, NodePosition start, NodePosition end) noexcept {
    bool was_open;
    return open_at(arena, start, end, was_open);
  }

  ASMJIT_INLINE Error open_at(Arena& arena, NodePosition start, NodePosition end, bool& was_open) noexcept {
    size_t size = _data.size();
    was_open = false;

    if (size > 0) {
      RALiveSpan& last = _data[size - 1];
      if (last.b >= start) {
        was_open = last.b > start;
        last.b = end;
        return Error::kOk;
      }
    }

    return _data.append(arena, RALiveSpan(start, end));
  }

  ASMJIT_INLINE void close_at(NodePosition end) noexcept {
    ASMJIT_ASSERT(!is_empty());

    size_t size = _data.size();
    _data[size - 1u].b = end;
  }

  //! Returns the sum of width of all spans.
  //!
  //! \note Don't overuse, this iterates over all spans so it's O(N). It should be only called once and then cached.
  inline uint32_t width() const noexcept {
    uint32_t width = 0;
    for (const RALiveSpan& span : _data)
      width += span.width();
    return width;
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RALiveSpan& operator[](uint32_t index) noexcept { return _data[index]; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RALiveSpan& operator[](uint32_t index) const noexcept { return _data[index]; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool intersects(const RALiveSpans& other) const noexcept {
    return intersects(*this, other);
  }

  [[nodiscard]]
  ASMJIT_INLINE Error non_overlapping_union_of(Arena& arena, const RALiveSpans& x, const RALiveSpans& y) noexcept {
    size_t final_size = x.size() + y.size();
    ASMJIT_PROPAGATE(_data.reserve_grow(arena, final_size));

    RALiveSpan* dst_ptr = _data.data();
    const RALiveSpan* x_span = x.data();
    const RALiveSpan* y_span = y.data();

    const RALiveSpan* x_end = x_span + x.size();
    const RALiveSpan* y_end = y_span + y.size();

    // Loop until we have intersection or either `x_span == x_end` or `y_span == y_end`, which means that there is no
    // intersection. We advance either `x_span` or `y_span` depending on their ranges.
    if (x_span != x_end && y_span != y_end) {
      NodePosition xa, ya;
      xa = x_span->a;
      for (;;) {
        while (y_span->b <= xa) {
          dst_ptr->init(*y_span);
          dst_ptr++;
          if (++y_span == y_end) {
            goto Done;
          }
        }

        ya = y_span->a;
        while (x_span->b <= ya) {
          *dst_ptr++ = *x_span;
          if (++x_span == x_end) {
            goto Done;
          }
        }

        // We know that `x_span->b > y_span->a`, so check if `y_span->b > x_span->a`.
        xa = x_span->a;
        if (y_span->b > xa) {
          return Error::kByPass;
        }
      }
    }

  Done:
    while (x_span != x_end) {
      *dst_ptr++ = *x_span++;
    }

    while (y_span != y_end) {
      dst_ptr->init(*y_span);
      dst_ptr++;
      y_span++;
    }

    _data._set_end(dst_ptr);
    return Error::kOk;
  }

  [[nodiscard]]
  static ASMJIT_INLINE bool intersects(const RALiveSpans& x, const RALiveSpans& y) noexcept {
    const RALiveSpan* x_span = x.data();
    const RALiveSpan* y_span = y.data();

    const RALiveSpan* x_end = x_span + x.size();
    const RALiveSpan* y_end = y_span + y.size();

    // Loop until we have intersection or either `x_span == x_end` or `y_span == y_end`, which means that there is no
    // intersection. We advance either `x_span` or `y_span` depending on their end positions.
    if (x_span == x_end || y_span == y_end) {
      return false;
    }

    NodePosition xa, ya;
    xa = x_span->a;

    for (;;) {
      while (y_span->b <= xa) {
        if (++y_span == y_end) {
          return false;
        }
      }

      ya = y_span->a;
      while (x_span->b <= ya) {
        if (++x_span == x_end) {
          return false;
        }
      }

      // We know that `x_span->b > y_span->a`, so check if `y_span->b > x_span->a`.
      xa = x_span->a;
      if (y_span->b > xa) {
        return true;
      }
    }
  }

  //! \}
};

//! Statistics about a register liveness.
class RALiveStats {
public:
  uint32_t _width = 0;
  float _freq = 0.0f;
  float _priority = 0.0f;

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t width() const noexcept { return _width; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG float freq() const noexcept { return _freq; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG float priority() const noexcept { return _priority; }

  //! \}
};

//! Flags used by \ref RATiedReg.
//!
//! Register access information is encoded in 4 flags in total:
//!
//!   - `kRead`  - Register is Read    (ReadWrite if combined with `kWrite`).
//!   - `kWrite` - Register is Written (ReadWrite if combined with `kRead`).
//!   - `kUse`   - Encoded as Read or ReadWrite.
//!   - `kOut`   - Encoded as WriteOnly.
//!
//! Let's describe all of these on two X86 instructions:
//!
//!   - ADD x{R|W|Use},  x{R|Use}              -> {x:R|W|Use            }
//!   - LEA x{  W|Out}, [x{R|Use} + x{R|Out}]  -> {x:R|W|Use|Out        }
//!   - ADD x{R|W|Use},  y{R|Use}              -> {x:R|W|Use     y:R|Use}
//!   - LEA x{  W|Out}, [x{R|Use} + y{R|Out}]  -> {x:R|W|Use|Out y:R|Use}
//!
//! It should be obvious from the example above how these flags get created. Each operand contains READ/WRITE
//! information, which is then merged to RATiedReg's flags. However, we also need to represent the possibility
//! to view the operation as two independent operations - USE and OUT, because the register allocator first
//! allocates USE registers, and then assigns OUT registers independently of USE registers.
enum class RATiedFlags : uint32_t {
  //! No flags.
  kNone = 0,

  // Access Flags
  // ------------

  //! Register is read.
  kRead = uint32_t(OpRWFlags::kRead),
  //! Register is written.
  kWrite = uint32_t(OpRWFlags::kWrite),
  //! Register both read and written.
  kRW = uint32_t(OpRWFlags::kRW),

  // Use / Out Flags
  // ---------------

  //! Register has a USE slot (read/rw).
  kUse = 0x00000004u,
  //! Register has an OUT slot (write-only).
  kOut = 0x00000008u,
  //! Register in USE slot can be patched to memory.
  kUseRM = 0x00000010u,
  //! Register in OUT slot can be patched to memory.
  kOutRM = 0x00000020u,

  //! Register has a fixed USE slot.
  kUseFixed = 0x00000040u,
  //! Register has a fixed OUT slot.
  kOutFixed = 0x00000080u,
  //! Register USE slot has been allocated.
  kUseDone = 0x00000100u,
  //! Register OUT slot has been allocated.
  kOutDone = 0x00000200u,

  // Consecutive Flags / Data
  // ------------------------

  //! Has consecutive registers in USE slots.
  kUseConsecutive = 0x00000400u,
  //! Has consecutive registers in OUT slots.
  kOutConsecutive = 0x00000800u,
  //! This is a consecutive lead (the first that is consecutive).
  kLeadConsecutive = 0x00001000u,
  //! Consecutive data payload.
  kConsecutiveData = 0x00006000u,

  // Other Constraints
  // -----------------

  //! This must be allocated to a unique physical register, not shared with other USE slots.
  kUnique = 0x00008000u,

  // Liveness Flags
  // --------------

  //! Register must be duplicated (function call only).
  kDuplicate = 0x00010000u,
  //! First occurrence of this virtual register in a basic block.
  kFirst = 0x00020000u,
  //! Last occurrence of this virtual register in a basic block.
  kLast = 0x00040000u,
  //! Kill this VirtReg after use.
  kKill = 0x00080000u,

  // X86 Specific Flags
  // ------------------

  // Architecture specific flags are used during RATiedReg building to ensure that architecture-specific constraints
  // are handled properly. These flags are not really needed after RATiedReg[] is built and copied to `RAInst`.

  //! This RATiedReg references GPB-LO or GPB-HI.
  kX86_Gpb = 0x01000000u,

  // Instruction Flags (Never used by RATiedReg)
  // -------------------------------------------

  //! Instruction has been patched to address a memory location instead of a register.
  //!
  //! This is currently only possible on X86 or X86_64 targets. It informs rewriter to rewrite the instruction if
  //! necessary.
  kInst_RegToMemPatched = 0x40000000u,

  //! Instruction is transformable to another instruction if necessary.
  //!
  //! This is flag that is only used by \ref RAInst to inform register allocator that the instruction has some
  //! constraints that can only be solved by transforming the instruction into another instruction, most likely
  //! by changing its InstId.
  kInst_IsTransformable = 0x80000000u
};
ASMJIT_DEFINE_ENUM_FLAGS(RATiedFlags)

static_assert(uint32_t(RATiedFlags::kRead ) == 0x1, "RATiedFlags::kRead must be 0x1");
static_assert(uint32_t(RATiedFlags::kWrite) == 0x2, "RATiedFlags::kWrite must be 0x2");
static_assert(uint32_t(RATiedFlags::kRW   ) == 0x3, "RATiedFlags::kRW must be 0x3");

//! Tied register merges one ore more register operand into a single entity. It contains information about its access
//! (Read|Write) and allocation slots (Use|Out) that are used by the register allocator and liveness analysis.
struct RATiedReg {
  //! \name Members
  //! \{

  RAWorkReg* _work_reg;
  RAWorkReg* _consecutive_parent;

  //! Allocation flags.
  RATiedFlags _flags;

  union {
    struct {
      //! How many times the VirtReg is referenced in all operands.
      uint8_t _ref_count;
      //! Size of a memory operand in case that it's use instead of the register.
      uint8_t _rm_size;
      //! Physical register for use operation (ReadOnly / ReadWrite).
      uint8_t _use_id;
      //! Physical register for out operation (WriteOnly).
      uint8_t _out_id;
    };
    //! Packed data.
    uint32_t _packed;
  };

  //! Registers where inputs {R|X} can be allocated to.
  RegMask _use_reg_mask;
  //! Registers where outputs {W} can be allocated to.
  RegMask _out_reg_mask;
  //! Indexes used to rewrite USE regs.
  uint32_t _use_rewrite_mask;
  //! Indexes used to rewrite OUT regs.
  uint32_t _out_rewrite_mask;

  //! \}

  //! \name Statics
  //! \{

  static inline RATiedFlags consecutive_data_to_flags(uint32_t offset) noexcept {
    ASMJIT_ASSERT(offset < 4);
    constexpr uint32_t kOffsetShift = Support::ctz_const<RATiedFlags::kConsecutiveData>;
    return (RATiedFlags)(offset << kOffsetShift);
  }

  static inline uint32_t consecutive_data_from_flags(RATiedFlags flags) noexcept {
    constexpr uint32_t kOffsetShift = Support::ctz_const<RATiedFlags::kConsecutiveData>;
    return uint32_t(flags & RATiedFlags::kConsecutiveData) >> kOffsetShift;
  }

  //! \}

  //! \name Construction & Destruction
  //! \{

  inline void init(RAWorkReg* work_reg, RATiedFlags flags, RegMask use_reg_mask, uint32_t use_id, uint32_t use_rewrite_mask, RegMask out_reg_mask, uint32_t out_id, uint32_t out_rewrite_mask, uint32_t rm_size = 0, RAWorkReg* consecutive_parent = nullptr) noexcept {
    _work_reg = work_reg;
    _consecutive_parent = consecutive_parent;
    _flags = flags;
    _ref_count = 1;
    _rm_size = uint8_t(rm_size);
    _use_id = uint8_t(use_id);
    _out_id = uint8_t(out_id);
    _use_reg_mask = use_reg_mask;
    _out_reg_mask = out_reg_mask;
    _use_rewrite_mask = use_rewrite_mask;
    _out_rewrite_mask = out_rewrite_mask;
  }

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAWorkReg* work_reg() const noexcept { return _work_reg; }

  //! Returns the associated WorkReg id.
  //[[nodiscard]]
  //ASMJIT_INLINE_NODEBUG RAWorkId work_id() const noexcept { return _work_reg->work_id(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_consecutive_parent() const noexcept { return _consecutive_parent != nullptr; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAWorkReg* consecutive_parent() const noexcept { return _consecutive_parent; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t consecutive_data() const noexcept { return consecutive_data_from_flags(_flags); }

  //! Returns TiedReg flags.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedFlags flags() const noexcept { return _flags; }

  //! Checks if the given `flag` is set.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(RATiedFlags flag) const noexcept { return Support::test(_flags, flag); }

  //! Adds tied register flags.
  ASMJIT_INLINE_NODEBUG void add_flags(RATiedFlags flags) noexcept { _flags |= flags; }

  //! Tests whether the register is read (writes `true` also if it's Read/Write).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_read() const noexcept { return has_flag(RATiedFlags::kRead); }

  //! Tests whether the register is written (writes `true` also if it's Read/Write).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_write() const noexcept { return has_flag(RATiedFlags::kWrite); }

  //! Tests whether the register is read only.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_read_only() const noexcept { return (_flags & RATiedFlags::kRW) == RATiedFlags::kRead; }

  //! Tests whether the register is write only.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_write_only() const noexcept { return (_flags & RATiedFlags::kRW) == RATiedFlags::kWrite; }

  //! Tests whether the register is read and written.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_read_write() const noexcept { return (_flags & RATiedFlags::kRW) == RATiedFlags::kRW; }

  //! Tests whether the tied register has use operand (Read/ReadWrite).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_use() const noexcept { return has_flag(RATiedFlags::kUse); }

  //! Tests whether the tied register has out operand (Write).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_out() const noexcept { return has_flag(RATiedFlags::kOut); }

  //! Tests whether the tied register has \ref RATiedFlags::kLeadConsecutive flag set.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_lead_consecutive() const noexcept { return has_flag(RATiedFlags::kLeadConsecutive); }

  //! Tests whether the tied register has \ref RATiedFlags::kUseConsecutive flag set.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_use_consecutive() const noexcept { return has_flag(RATiedFlags::kUseConsecutive); }

  //! Tests whether the tied register has \ref RATiedFlags::kOutConsecutive flag set.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_out_consecutive() const noexcept { return has_flag(RATiedFlags::kOutConsecutive); }

  //! Tests whether the tied register must be unique (cannot be allocated to any other allocated register).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_unique() const noexcept { return has_flag(RATiedFlags::kUnique); }

  //! Tests whether the tied register has any consecutive flag.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_any_consecutive_flag() const noexcept { return has_flag(RATiedFlags::kLeadConsecutive | RATiedFlags::kUseConsecutive | RATiedFlags::kOutConsecutive); }

  //! Tests whether the USE slot can be patched to memory operand.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_use_rm() const noexcept { return has_flag(RATiedFlags::kUseRM); }

  //! Tests whether the OUT slot can be patched to memory operand.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_out_rm() const noexcept { return has_flag(RATiedFlags::kOutRM); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t rm_size() const noexcept { return _rm_size; }

  inline void make_read_only() noexcept {
    _flags = (_flags & ~(RATiedFlags::kOut | RATiedFlags::kWrite)) | RATiedFlags::kUse;
    _use_rewrite_mask |= _out_rewrite_mask;
    _out_rewrite_mask = 0;
  }

  inline void make_write_only() noexcept {
    _flags = (_flags & ~(RATiedFlags::kUse | RATiedFlags::kRead)) | RATiedFlags::kOut;
    _out_rewrite_mask |= _use_rewrite_mask;
    _use_rewrite_mask = 0;
  }

  //! Tests whether the register would duplicate.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_duplicate() const noexcept { return has_flag(RATiedFlags::kDuplicate); }

  //! Tests whether the register (and the instruction it's part of) appears first in the basic block.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_first() const noexcept { return has_flag(RATiedFlags::kFirst); }

  //! Tests whether the register (and the instruction it's part of) appears last in the basic block.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_last() const noexcept { return has_flag(RATiedFlags::kLast); }

  //! Tests whether the register should be killed after USEd and/or OUTed.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_kill() const noexcept { return has_flag(RATiedFlags::kKill); }

  //! Tests whether the register is OUT or KILL (used internally by local register allocator).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_out_or_kill() const noexcept { return has_flag(RATiedFlags::kOut | RATiedFlags::kKill); }

  //! Returns a register mask that describes allocable USE registers (Read/ReadWrite access).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask use_reg_mask() const noexcept { return _use_reg_mask; }

  //! Returns a register mask that describes allocable OUT registers (WriteOnly access).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask out_reg_mask() const noexcept { return _out_reg_mask; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t ref_count() const noexcept { return _ref_count; }

  ASMJIT_INLINE_NODEBUG void add_ref_count(uint32_t n = 1) noexcept { _ref_count = uint8_t(_ref_count + n); }

  //! Tests whether the register must be allocated to a fixed physical register before it's used.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_use_id() const noexcept { return _use_id != Reg::kIdBad; }

  //! Tests whether the register must be allocated to a fixed physical register before it's written.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_out_id() const noexcept { return _out_id != Reg::kIdBad; }

  //! Returns a physical register id used for 'use' operation.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t use_id() const noexcept { return _use_id; }

  //! Returns a physical register id used for 'out' operation.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t out_id() const noexcept { return _out_id; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t use_rewrite_mask() const noexcept { return _use_rewrite_mask; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t out_rewrite_mask() const noexcept { return _out_rewrite_mask; }

  //! Sets a physical register used for 'use' operation.
  ASMJIT_INLINE_NODEBUG void set_use_id(uint32_t index) noexcept { _use_id = uint8_t(index); }

  //! Sets a physical register used for 'out' operation.
  ASMJIT_INLINE_NODEBUG void set_out_id(uint32_t index) noexcept { _out_id = uint8_t(index); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_use_done() const noexcept { return has_flag(RATiedFlags::kUseDone); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_out_done() const noexcept { return has_flag(RATiedFlags::kOutDone); }

  ASMJIT_INLINE_NODEBUG void mark_use_done() noexcept { add_flags(RATiedFlags::kUseDone); }

  ASMJIT_INLINE_NODEBUG void mark_out_done() noexcept { add_flags(RATiedFlags::kOutDone); }

  //! \}
};

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_CORE_RADEFS_P_H_INCLUDED
