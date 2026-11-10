// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_FONTMANAGER_P_H_INCLUDED
#define BLEND2D_FONTMANAGER_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/fontmanager.h>
#include <blend2d/support/arenaallocator_p.h>
#include <blend2d/support/arenahashmap_p.h>
#include <blend2d/threading/mutex_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

//! \name BLFontManager - Internals - Memory Management
//! \{

class BLFontManagerPrivateImpl : public BLFontManagerImpl {
public:
  BL_NONCOPYABLE(BLFontManagerPrivateImpl)

  class FamiliesMapNode : public bl::ArenaHashMapNode {
  public:
    BL_NONCOPYABLE(FamiliesMapNode)

    BLString family_name;
    BLArray<BLFontFace> faces;

    BL_INLINE FamiliesMapNode(uint32_t hash_code, const BLString& family_name) noexcept
      : bl::ArenaHashMapNode(hash_code),
        family_name(family_name),
        faces() {}
    BL_INLINE ~FamiliesMapNode() noexcept {}

    BL_INLINE FamiliesMapNode* next() const noexcept { return static_cast<FamiliesMapNode*>(_hash_next); }
  };

  struct FamilyMatcher {
    BLStringView _family;
    uint32_t _hash_code;

    BL_INLINE uint32_t hash_code() const noexcept { return _hash_code; }
    BL_INLINE bool matches(const FamiliesMapNode* node) const noexcept { return node->family_name.equals(_family); }
  };

  class SubstitutionMapNode : public bl::ArenaHashMapNode {
  public:
    BL_NONCOPYABLE(SubstitutionMapNode)

    BLString from;
    BLString to;

    BL_INLINE SubstitutionMapNode(uint32_t hash_code, const BLString& from, const BLString& to) noexcept
      : bl::ArenaHashMapNode(hash_code),
        from(from),
        to(to) {}
    BL_INLINE ~SubstitutionMapNode() noexcept {}

    BL_INLINE SubstitutionMapNode* next() const noexcept { return static_cast<SubstitutionMapNode*>(_hash_next); }
  };

  BLSharedMutex mutex;
  bl::ArenaAllocator allocator;
  bl::ArenaHashMap<FamiliesMapNode> families_map;
  bl::ArenaHashMap<SubstitutionMapNode> substitution_map;
  size_t face_count = 0;

  BL_INLINE BLFontManagerPrivateImpl(const BLFontManagerVirt* virt_) noexcept
    : mutex(),
      allocator(8192),
      families_map(&allocator),
      substitution_map(&allocator) { virt = virt_; }

  BL_INLINE ~BLFontManagerPrivateImpl() noexcept {}
};

namespace bl {
namespace FontManagerInternal {

static BL_INLINE BLFontManagerPrivateImpl* get_impl(const BLFontManagerCore* self) noexcept {
  return static_cast<BLFontManagerPrivateImpl*>(self->_d.impl);
}

} // {FontManagerInternal}
} // {bl}

//! \}

//! \}
//! \endcond

#endif // BLEND2D_FONTMANAGER_P_H_INCLUDED
