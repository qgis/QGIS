// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_RAASSIGNMENT_P_H_INCLUDED
#define ASMJIT_CORE_RAASSIGNMENT_P_H_INCLUDED

#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_COMPILER

#include <asmjit/core/radefs_p.h>
#include <asmjit/core/rareg_p.h>

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_ra
//! \{

//! Holds the current register assignment.
//!
//! Has two purposes:
//!
//!   1. Holds register assignment of a local register allocator (see \ref RALocalAllocator).
//!   2. Holds register assignment of the entry of basic blocks (see \ref RABlock).
class RAAssignment {
public:
  ASMJIT_NONCOPYABLE(RAAssignment)

  static inline constexpr uint32_t kPhysNone = 0xFF;

  enum DirtyBit : uint32_t {
    kClean = 0,
    kDirty = 1
  };

  struct Layout {
    //! Index of architecture registers per group.
    RARegIndex phys_index;
    //! Count of architecture registers per group.
    RARegCount phys_count;
    //! Count of physical registers of all groups.
    uint32_t phys_total;
    //! Count of work registers.
    uint32_t work_count;
    //! WorkRegs data (vector).
    const ArenaVector<RAWorkReg*>* work_regs;

    inline void reset() noexcept {
      phys_index.reset();
      phys_count.reset();
      phys_total = 0;
      work_count = 0;
      work_regs = nullptr;
    }
  };

  struct PhysToWorkMap {
    //! Assigned registers (each bit represents one physical reg).
    RARegMask assigned;
    //! Dirty registers (spill slot out of sync or no spill slot).
    RARegMask dirty;
    //! PhysReg to WorkReg mapping.
    RAWorkId work_ids[1 /* ... */];

    [[nodiscard]]
    static ASMJIT_INLINE_NODEBUG size_t size_of(size_t count) noexcept {
      return Arena::aligned_size(sizeof(PhysToWorkMap) - sizeof(uint32_t) + count * sizeof(uint32_t));
    }

    ASMJIT_INLINE void reset(size_t count) noexcept {
      assigned.reset();
      dirty.reset();

      for (size_t i = 0; i < count; i++) {
        work_ids[i] = kBadWorkId;
      }
    }

    ASMJIT_INLINE void copy_from(const PhysToWorkMap* other, size_t count) noexcept {
      size_t size = size_of(count);
      memcpy(this, other, size);
    }

    ASMJIT_INLINE void unassign(RegGroup group, uint32_t phys_id, uint32_t index_in_work_ids) noexcept {
      assigned.clear(group, Support::bit_mask<RegMask>(phys_id));
      dirty.clear(group, Support::bit_mask<RegMask>(phys_id));
      work_ids[index_in_work_ids] = kBadWorkId;
    }
  };

  struct WorkToPhysMap {
    //! WorkReg to PhysReg mapping
    uint8_t phys_ids[1 /* ... */];

    [[nodiscard]]
    static ASMJIT_INLINE_NODEBUG size_t size_of(size_t count) noexcept {
      return Arena::aligned_size(size_t(count) * sizeof(uint8_t));
    }

    ASMJIT_INLINE void reset(size_t count) noexcept {
      for (size_t i = 0; i < count; i++) {
        phys_ids[i] = kPhysNone;
      }
    }

    ASMJIT_INLINE void copy_from(const WorkToPhysMap* other, size_t count) noexcept {
      size_t size = size_of(count);
      if (ASMJIT_LIKELY(size)) {
        memcpy(this, other, size);
      }
    }
  };

  //! \name Members
  //! \{

  //! Physical registers layout.
  Layout _layout;
  //! WorkReg to PhysReg mapping.
  WorkToPhysMap* _work_to_phys_map;
  //! PhysReg to WorkReg mapping and assigned/dirty bits.
  PhysToWorkMap* _phys_to_work_map;
  //! Optimization to translate PhysRegs to WorkRegs faster.
  Support::Array<RAWorkId*, Globals::kNumVirtGroups> _phys_to_work_ids;

  //! \}

  //! \name Construction & Destruction
  //! \{

  inline RAAssignment() noexcept {
    _layout.reset();
    reset_maps();
  }

  ASMJIT_INLINE void init_layout(const RARegCount& phys_count, const ArenaVector<RAWorkReg*>& work_regs) noexcept {
    // Layout must be initialized before data.
    ASMJIT_ASSERT(_phys_to_work_map == nullptr);
    ASMJIT_ASSERT(_work_to_phys_map == nullptr);

    _layout.phys_index.build_indexes(phys_count);
    _layout.phys_count = phys_count;
    _layout.phys_total = _layout.phys_index.get(RegGroup::kMaxVirt) + _layout.phys_count.get(RegGroup::kMaxVirt);
    _layout.work_count = uint32_t(work_regs.size());
    _layout.work_regs = &work_regs;
  }

  ASMJIT_INLINE void init_maps(PhysToWorkMap* phys_to_work_map, WorkToPhysMap* work_to_phys_map) noexcept {
    _phys_to_work_map = phys_to_work_map;
    _work_to_phys_map = work_to_phys_map;
    for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
      _phys_to_work_ids[group] = phys_to_work_map->work_ids + _layout.phys_index.get(group);
    }
  }

  ASMJIT_INLINE void reset_maps() noexcept {
    _phys_to_work_map = nullptr;
    _work_to_phys_map = nullptr;
    _phys_to_work_ids.fill(nullptr);
  }

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG PhysToWorkMap* phys_to_work_map() const noexcept { return _phys_to_work_map; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG WorkToPhysMap* work_to_phys_map() const noexcept { return _work_to_phys_map; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RARegMask& assigned() noexcept { return _phys_to_work_map->assigned; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RARegMask& assigned() const noexcept { return _phys_to_work_map->assigned; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t assigned(RegGroup group) const noexcept { return _phys_to_work_map->assigned[group]; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RARegMask& dirty() noexcept { return _phys_to_work_map->dirty; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RARegMask& dirty() const noexcept { return _phys_to_work_map->dirty; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask dirty(RegGroup group) const noexcept { return _phys_to_work_map->dirty[group]; }

  [[nodiscard]]
  inline uint32_t work_to_phys_id(RegGroup group, RAWorkId work_id) const noexcept {
    Support::maybe_unused(group);
    ASMJIT_ASSERT(work_id != kBadWorkId);
    ASMJIT_ASSERT(uint32_t(work_id) < _layout.work_count);
    return _work_to_phys_map->phys_ids[uint32_t(work_id)];
  }

  [[nodiscard]]
  inline RAWorkId phys_to_work_id(RegGroup group, uint32_t phys_id) const noexcept {
    ASMJIT_ASSERT(phys_id < Globals::kMaxPhysRegs);
    return _phys_to_work_ids[group][phys_id];
  }

  [[nodiscard]]
  inline bool is_phys_assigned(RegGroup group, uint32_t phys_id) const noexcept {
    ASMJIT_ASSERT(phys_id < Globals::kMaxPhysRegs);
    return Support::bit_test(_phys_to_work_map->assigned[group], phys_id);
  }

  [[nodiscard]]
  inline bool is_phys_dirty(RegGroup group, uint32_t phys_id) const noexcept {
    ASMJIT_ASSERT(phys_id < Globals::kMaxPhysRegs);
    return Support::bit_test(_phys_to_work_map->dirty[group], phys_id);
  }

  //! \}

  //! \name Assignment
  //!
  //! These are low-level allocation helpers that are used to update the current mappings between physical and
  //! virt/work registers and also to update masks that represent allocated and dirty registers. These functions
  //! don't emit any code; they are only used to update and keep all mappings in sync.
  //!
  //! \{

  //! Assign [VirtReg/WorkReg] to a physical register.
  inline void assign(RegGroup group, RAWorkId work_id, uint32_t phys_id, bool dirty) noexcept {
    ASMJIT_ASSERT(work_to_phys_id(group, work_id) == kPhysNone);
    ASMJIT_ASSERT(phys_to_work_id(group, phys_id) == kBadWorkId);
    ASMJIT_ASSERT(!is_phys_assigned(group, phys_id));
    ASMJIT_ASSERT(!is_phys_dirty(group, phys_id));

    _work_to_phys_map->phys_ids[uint32_t(work_id)] = uint8_t(phys_id);
    _phys_to_work_ids[group][phys_id] = work_id;

    RegMask reg_mask = Support::bit_mask<RegMask>(phys_id);
    _phys_to_work_map->assigned[group] |= reg_mask;
    _phys_to_work_map->dirty[group] |= reg_mask & Support::bool_as_mask<RegMask>(dirty);

    verify();
  }

  //! Reassign [VirtReg/WorkReg] to `dst_phys_id` from `src_phys_id`.
  inline void reassign(RegGroup group, RAWorkId work_id, uint32_t dst_phys_id, uint32_t src_phys_id) noexcept {
    ASMJIT_ASSERT(dst_phys_id != src_phys_id);
    ASMJIT_ASSERT(work_to_phys_id(group, work_id) == src_phys_id);
    ASMJIT_ASSERT(phys_to_work_id(group, src_phys_id) == work_id);
    ASMJIT_ASSERT(is_phys_assigned(group, src_phys_id) == true);
    ASMJIT_ASSERT(is_phys_assigned(group, dst_phys_id) == false);

    _work_to_phys_map->phys_ids[uint32_t(work_id)] = uint8_t(dst_phys_id);
    _phys_to_work_ids[group][src_phys_id] = kBadWorkId;
    _phys_to_work_ids[group][dst_phys_id] = work_id;

    RegMask src_mask = Support::bit_mask<RegMask>(src_phys_id);
    RegMask dst_mask = Support::bit_mask<RegMask>(dst_phys_id);

    bool dirty = (_phys_to_work_map->dirty[group] & src_mask) != 0;
    RegMask reg_mask = dst_mask | src_mask;

    _phys_to_work_map->assigned[group] ^= reg_mask;
    _phys_to_work_map->dirty[group] ^= reg_mask & Support::bool_as_mask<RegMask>(dirty);

    verify();
  }

  inline void swap(RegGroup group, RAWorkId a_work_id, uint32_t a_phys_id, RAWorkId b_work_id, uint32_t b_phys_id) noexcept {
    ASMJIT_ASSERT(a_phys_id != b_phys_id);

    ASMJIT_ASSERT(work_to_phys_id(group, a_work_id) == a_phys_id);
    ASMJIT_ASSERT(work_to_phys_id(group, b_work_id) == b_phys_id);
    ASMJIT_ASSERT(phys_to_work_id(group, a_phys_id) == a_work_id);
    ASMJIT_ASSERT(phys_to_work_id(group, b_phys_id) == b_work_id);

    ASMJIT_ASSERT(is_phys_assigned(group, a_phys_id));
    ASMJIT_ASSERT(is_phys_assigned(group, b_phys_id));

    _work_to_phys_map->phys_ids[uint32_t(a_work_id)] = uint8_t(b_phys_id);
    _work_to_phys_map->phys_ids[uint32_t(b_work_id)] = uint8_t(a_phys_id);
    _phys_to_work_ids[group][a_phys_id] = b_work_id;
    _phys_to_work_ids[group][b_phys_id] = a_work_id;

    RegMask a_mask = Support::bit_mask<RegMask>(a_phys_id);
    RegMask b_mask = Support::bit_mask<RegMask>(b_phys_id);
    RegMask flip_mask = Support::bool_as_mask<RegMask>(((_phys_to_work_map->dirty[group] & a_mask) != 0) ^ ((_phys_to_work_map->dirty[group] & b_mask) != 0));
    RegMask reg_mask = a_mask | b_mask;
    _phys_to_work_map->dirty[group] ^= reg_mask & flip_mask;

    verify();
  }

  //! Unassign [VirtReg/WorkReg] from a physical register.
  inline void unassign(RegGroup group, RAWorkId work_id, uint32_t phys_id) noexcept {
    ASMJIT_ASSERT(phys_id < Globals::kMaxPhysRegs);
    ASMJIT_ASSERT(work_to_phys_id(group, work_id) == phys_id);
    ASMJIT_ASSERT(phys_to_work_id(group, phys_id) == work_id);
    ASMJIT_ASSERT(is_phys_assigned(group, phys_id));

    _work_to_phys_map->phys_ids[uint32_t(work_id)] = kPhysNone;
    _phys_to_work_ids[group][phys_id] = kBadWorkId;

    RegMask reg_mask = Support::bit_mask<RegMask>(phys_id);
    _phys_to_work_map->assigned[group] &= ~reg_mask;
    _phys_to_work_map->dirty[group] &= ~reg_mask;

    verify();
  }

  inline void make_clean(RegGroup group, RAWorkId work_id, uint32_t phys_id) noexcept {
    Support::maybe_unused(work_id);
    RegMask reg_mask = Support::bit_mask<RegMask>(phys_id);
    _phys_to_work_map->dirty[group] &= ~reg_mask;
  }

  inline void make_dirty(RegGroup group, RAWorkId work_id, uint32_t phys_id) noexcept {
    Support::maybe_unused(work_id);
    RegMask reg_mask = Support::bit_mask<RegMask>(phys_id);
    _phys_to_work_map->dirty[group] |= reg_mask;
  }

  //! \}

  //! \name Utilities
  //! \{

  ASMJIT_INLINE void swap(RAAssignment& other) noexcept {
    std::swap(_work_to_phys_map, other._work_to_phys_map);
    std::swap(_phys_to_work_map, other._phys_to_work_map);
    _phys_to_work_ids.swap(other._phys_to_work_ids);
  }

  inline void assign_work_ids_from_phys_ids() noexcept {
    memset(_work_to_phys_map, uint8_t(Reg::kIdBad), WorkToPhysMap::size_of(_layout.work_count));

    for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
      uint32_t phys_base_index = _layout.phys_index.get(group);
      Support::BitWordIterator<RegMask> it(_phys_to_work_map->assigned[group]);

      while (it.has_next()) {
        uint32_t phys_id = it.next();
        RAWorkId work_id = _phys_to_work_map->work_ids[phys_base_index + phys_id];

        ASMJIT_ASSERT(work_id != kBadWorkId);
        _work_to_phys_map->phys_ids[uint32_t(work_id)] = uint8_t(phys_id);
      }
    }
  }

  inline void copy_from(const PhysToWorkMap* phys_to_work_map) noexcept {
    memcpy(_phys_to_work_map, phys_to_work_map, PhysToWorkMap::size_of(_layout.phys_total));
    assign_work_ids_from_phys_ids();
  }

  inline void copy_from(const PhysToWorkMap* phys_to_work_map, const WorkToPhysMap* work_to_phys_map) noexcept {
    memcpy(_phys_to_work_map, phys_to_work_map, PhysToWorkMap::size_of(_layout.phys_total));
    memcpy(_work_to_phys_map, work_to_phys_map, WorkToPhysMap::size_of(_layout.work_count));
  }

  inline void copy_from(const RAAssignment& other) noexcept {
    copy_from(other.phys_to_work_map(), other.work_to_phys_map());
  }

  // Not really useful outside of debugging.
  [[nodiscard]]
  bool equals(const RAAssignment& other) const noexcept {
    // Layout should always match.
    if (_layout.phys_index != other._layout.phys_index ||
        _layout.phys_count != other._layout.phys_count ||
        _layout.phys_total != other._layout.phys_total ||
        _layout.work_count != other._layout.work_count ||
        _layout.work_regs  != other._layout.work_regs) {
      return false;
    }

    uint32_t phys_total = _layout.phys_total;
    uint32_t work_count = _layout.work_count;

    for (uint32_t phys_id = 0; phys_id < phys_total; phys_id++) {
      RAWorkId this_work_id = _phys_to_work_map->work_ids[phys_id];
      RAWorkId other_work_id = other._phys_to_work_map->work_ids[phys_id];
      if (this_work_id != other_work_id) {
        return false;
      }
    }

    for (uint32_t work_id = 0; work_id < work_count; work_id++) {
      uint32_t this_phys_id = _work_to_phys_map->phys_ids[work_id];
      uint32_t other_phys_id = other._work_to_phys_map->phys_ids[work_id];

      if (this_phys_id != other_phys_id) {
        return false;
      }
    }

    if (_phys_to_work_map->assigned != other._phys_to_work_map->assigned ||
        _phys_to_work_map->dirty    != other._phys_to_work_map->dirty    ) {
      return false;
    }

    return true;
  }

#if defined(ASMJIT_BUILD_DEBUG)
  ASMJIT_NOINLINE void verify() noexcept {
    // Verify WorkToPhysMap.
    {
      for (uint32_t work_id = 0; work_id < _layout.work_count; work_id++) {
        uint32_t phys_id = _work_to_phys_map->phys_ids[work_id];
        if (phys_id != kPhysNone) {
          const RAWorkReg* work_reg = _layout.work_regs->at(work_id);
          RegGroup group = work_reg->group();
          ASMJIT_ASSERT(_phys_to_work_ids[group][phys_id] == RAWorkId(work_id));
        }
      }
    }

    // Verify PhysToWorkMap.
    {
      for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
        uint32_t phys_count = _layout.phys_count.get(group);
        for (uint32_t phys_id = 0; phys_id < phys_count; phys_id++) {
          RAWorkId work_id = _phys_to_work_ids[group][phys_id];
          if (work_id != kBadWorkId) {
            ASMJIT_ASSERT(_work_to_phys_map->phys_ids[uint32_t(work_id)] == phys_id);
          }
        }
      }
    }
  }
#else
  inline void verify() noexcept {}
#endif

  //! \}
};

//! Intersection of multiple register assignments.
//!
//! See \ref RAAssignment for more information about register assignments.
class RASharedAssignment {
public:
  //! \name Types
  //! \{

  using PhysToWorkMap = RAAssignment::PhysToWorkMap;
  using WorkToPhysMap = RAAssignment::WorkToPhysMap;

  //! \}

  //! \name Members
  //! \{

  //! Bit-mask of registers that cannot be used upon a block entry, for each block that has this shared assignment.
  //! Scratch registers can come from ISA limits (like jecx/loop instructions on x86) or because the registers are
  //! used by jump/branch instruction that uses registers to perform an indirect jump.
  RegMask _entry_scratch_gp_regs = 0;
  //! Union of all live-in registers.
  ArenaBitSet _live_in {};
  //! Register assignment (PhysToWork).
  PhysToWorkMap* _phys_to_work_map = nullptr;

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _phys_to_work_map == nullptr; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask entry_scratch_gp_regs() const noexcept { return _entry_scratch_gp_regs; }

  ASMJIT_INLINE_NODEBUG void add_entry_scratch_gp_regs(RegMask mask) noexcept { _entry_scratch_gp_regs |= mask; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<const BitWord> live_in() const noexcept { return _live_in.as_span(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG PhysToWorkMap* phys_to_work_map() const noexcept { return _phys_to_work_map; }

  ASMJIT_INLINE_NODEBUG void assign_phys_to_work_map(PhysToWorkMap* phys_to_work_map) noexcept { _phys_to_work_map = phys_to_work_map; }

  //! \}
};

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_CORE_RAASSIGNMENT_P_H_INCLUDED
