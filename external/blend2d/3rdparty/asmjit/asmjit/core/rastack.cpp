// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#ifndef ASMJIT_NO_COMPILER

#include <asmjit/core/rastack_p.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

// RAStackAllocator - Slots
// ========================

RAStackSlot* RAStackAllocator::new_slot(uint32_t base_reg_id, uint32_t size, uint32_t alignment, uint32_t flags) noexcept {
  if (ASMJIT_UNLIKELY(_slots.reserve_additional(*arena()) != Error::kOk)) {
    return nullptr;
  }

  RAStackSlot* slot = arena()->alloc_oneshot<RAStackSlot>();
  if (ASMJIT_UNLIKELY(!slot)) {
    return nullptr;
  }

  slot->_base_reg_id = uint8_t(base_reg_id);
  slot->_alignment = uint8_t(Support::max<uint32_t>(alignment, 1));
  slot->_flags = uint16_t(flags);
  slot->_size = size;

  slot->_use_count = 0;
  slot->_weight = 0;
  slot->_offset = 0;

  _alignment = Support::max<uint32_t>(_alignment, alignment);
  _slots.append_unchecked(slot);
  return slot;
}

// RAStackAllocator - Utilities
// ============================

struct RAStackGap {
  inline RAStackGap() noexcept
    : offset(0),
      size(0) {}

  inline RAStackGap(uint32_t offset, uint32_t size) noexcept
    : offset(offset),
      size(size) {}

  inline RAStackGap(const RAStackGap& other) noexcept
    : offset(other.offset),
      size(other.size) {}

  uint32_t offset;
  uint32_t size;
};

Error RAStackAllocator::calculate_stack_frame() noexcept {
  // Base weight added to all registers regardless of their size and alignment.
  uint32_t kBaseRegWeight = 16;

  // STEP 1:
  //
  // Update usage based on the size of the slot. We boost smaller slots in a way that 32-bit register has a higher
  // priority than a 128-bit register, however, if one 128-bit register is used 4 times more than some other 32-bit
  // register it will overweight it.
  for (RAStackSlot* slot : _slots) {
    uint32_t alignment = slot->alignment();
    ASMJIT_ASSERT(alignment > 0);

    uint32_t power = Support::min<uint32_t>(Support::ctz(alignment), 6);
    uint64_t weight;

    if (slot->is_reg_home()) {
      weight = kBaseRegWeight + (uint64_t(slot->use_count()) * (7 - power));
    }
    else {
      weight = power;
    }

    // If overflown, which has less chance of winning a lottery, just use max possible weight. In such case it
    // probably doesn't matter at all.
    if (weight > 0xFFFFFFFFu) {
      weight = 0xFFFFFFFFu;
    }

    slot->set_weight(uint32_t(weight));
  }

  // STEP 2:
  //
  // Sort stack slots based on their newly calculated weight (in descending order).
  _slots.sort([](const RAStackSlot* a, const RAStackSlot* b) noexcept {
    return a->weight() >  b->weight() ? 1 :
           a->weight() == b->weight() ? 0 : -1;
  });

  // STEP 3:
  //
  // Calculate offset of each slot. We start from the slot that has the highest weight and advance to slots with
  // lower weight. It could look that offsets start from the first slot in our list and then simply increase, but
  // it's not always the case as we also try to fill all gaps introduced by the fact that slots are sorted by
  // weight and not by size & alignment, so when we need to align some slot we distribute the gap caused by the
  // alignment to `gaps`.
  uint32_t offset = 0;
  ArenaVector<RAStackGap> gaps[kSizeCount - 1];

  for (RAStackSlot* slot : _slots) {
    if (slot->is_stack_arg()) {
      continue;
    }

    uint32_t slot_alignment = slot->alignment();
    uint32_t aligned_offset = Support::align_up(offset, slot_alignment);

    // Try to find a slot within gaps first, before advancing the `offset`.
    bool found_gap = false;
    uint32_t gap_size = 0;
    uint32_t gap_offset = 0;

    {
      uint32_t slot_size = slot->size();
      if (slot_size < (1u << uint32_t(ASMJIT_ARRAY_SIZE(gaps)))) {
        // Iterate from the lowest to the highest possible.
        uint32_t index = Support::ctz(slot_size);
        do {
          if (!gaps[index].is_empty()) {
            RAStackGap gap = gaps[index].pop();

            ASMJIT_ASSERT(Support::is_aligned(gap.offset, slot_alignment));
            slot->set_offset(int32_t(gap.offset));

            gap_size = gap.size - slot_size;
            gap_offset = gap.offset - slot_size;

            found_gap = true;
            break;
          }
        } while (++index < uint32_t(ASMJIT_ARRAY_SIZE(gaps)));
      }
    }

    // No gap found, we may create a new one(s) if the current offset is not aligned.
    if (!found_gap && offset != aligned_offset) {
      gap_size = aligned_offset - offset;
      gap_offset = aligned_offset;

      offset = aligned_offset;
    }

    // True if we have found a gap and not filled all of it or we aligned the current offset.
    if (gap_size) {
      uint32_t gap_end = gap_size + gap_offset;
      while (gap_offset < gap_end) {
        uint32_t index = Support::ctz(gap_offset);
        uint32_t slot_size = 1u << index;

        // Weird case, better to bail...
        if (gap_end - gap_offset < slot_size) {
          break;
        }

        ASMJIT_PROPAGATE(gaps[index].append(*arena(), RAStackGap(gap_offset, slot_size)));
        gap_offset += slot_size;
      }
    }

    if (!found_gap) {
      ASMJIT_ASSERT(Support::is_aligned(offset, slot_alignment));
      slot->set_offset(int32_t(offset));
      offset += slot->size();
    }
  }

  _stack_size = Support::align_up(offset, _alignment);
  return Error::kOk;
}

Error RAStackAllocator::adjust_slot_offsets(int32_t offset) noexcept {
  for (RAStackSlot* slot : _slots) {
    if (!slot->is_stack_arg()) {
      slot->_offset += offset;
    }
  }
  return Error::kOk;
}

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
