// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/filesystem.h>
#include <blend2d/core/font_p.h>
#include <blend2d/core/fontface_p.h>
#include <blend2d/core/fontmanager_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/core/string_p.h>
#include <blend2d/support/hashops_p.h>

namespace bl {
namespace FontManagerInternal {

// bl::FontManager - Internals - Globals
// =====================================

static BLObjectEternalVirtualImpl<BLFontManagerPrivateImpl, BLFontManagerVirt> default_impl;

// bl::FontManager - Internals - Constants
// =======================================

enum QueryPrecedenceBits : uint32_t {
  kQueryDiffFamilyNameShift   = 24, // 0xFF000000 [8 bits].
  kQueryDiffStyleValueShift   = 22, // 0x00C00000 [2 bits].
  kQueryDiffStyleSignShift    = 21, // 0x00200000 [1 bit].
  kQueryDiffWeightValueShift  = 10, // 0x001FFC00 [11 bits].
  kQueryDiffWeightSignShift   =  9, // 0x00000200 [1 bit].
  kQueryDiffStretchValueShift =  5, // 0x000001E0 [4 bits].
  kQueryDiffStretchSignShift  =  4  // 0x00000010 [1 bit].
};

static constexpr uint32_t kQueryInvalidDiff = 0xFFFFFFFFu;

// bl::FontManager - Internals - Alloc & Free Impl
// ===============================================

static BLResult alloc_impl(BLFontManagerCore* self) noexcept {
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_FONT_MANAGER);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLFontManagerPrivateImpl>(self, info));

  BLFontManagerPrivateImpl* impl = get_impl(self);
  bl_call_ctor(*impl, &default_impl.virt);
  return BL_SUCCESS;
}

static BLResult BL_CDECL destroy_impl(BLObjectImpl* impl) noexcept {
  bl_call_dtor(*static_cast<BLFontManagerPrivateImpl*>(impl));
  return bl_object_free_impl(impl);
}

// bl::FontManager - Internals - Faces
// ===================================

static BL_INLINE size_t index_of_face(const BLFontFace* array, size_t size, BLFontFaceImpl* face_impl) noexcept {
  for (size_t i = 0; i < size; i++)
    if (array[i]._d.impl == face_impl)
      return i;
  return SIZE_MAX;
}

static BL_INLINE uint32_t calc_face_order(const BLFontFaceImpl* face_impl) noexcept {
  uint32_t style = face_impl->style;
  uint32_t weight = face_impl->weight;

  return (style << kQueryDiffStyleValueShift) |
         (weight << kQueryDiffWeightValueShift);
}

static BL_INLINE size_t index_for_insertion(const BLFontFace* array, size_t size, BLFontFaceImpl* face_impl) noexcept {
  uint32_t face_order = calc_face_order(face_impl);
  size_t i;

  for (i = 0; i < size; i++) {
    BLFontFacePrivateImpl* storedFaceI = FontFaceInternal::get_impl(&array[i]);
    uint32_t stored_face_order = calc_face_order(storedFaceI);
    if (stored_face_order >= face_order) {
      if (stored_face_order == face_order)
        return SIZE_MAX;
      break;
    }
  }

  return i;
}

// bl::FontManager - Query - Utilities
// ===================================

static const BLFontQueryProperties default_query_properties = {
  BL_FONT_STYLE_NORMAL,
  BL_FONT_WEIGHT_NORMAL,
  BL_FONT_STRETCH_NORMAL
};

static bool sanitize_query_properties(BLFontQueryProperties& dst, const BLFontQueryProperties& src) noexcept {
  bool valid = src.weight <= 1000u &&
               src.style <= BL_FONT_STYLE_MAX_VALUE &&
               src.stretch <= BL_FONT_STRETCH_ULTRA_EXPANDED;
  if (!valid)
    return false;

  dst.style = src.style;
  dst.weight = src.weight ? src.weight : BL_FONT_WEIGHT_NORMAL;
  dst.stretch = src.stretch ? src.stretch : BL_FONT_STRETCH_NORMAL;

  return true;
}

// bl::FontManager - Query - Prepared Query
// ========================================

struct PreparedQuery {
  BLStringView _name;
  uint32_t _hash_code;

  typedef BLFontManagerPrivateImpl::FamiliesMapNode FamiliesMapNode;

  BL_INLINE const BLStringView& name() const noexcept { return _name; };
  BL_INLINE uint32_t hash_code() const noexcept { return _hash_code; }

  BL_INLINE bool matches(const FamiliesMapNode* node) const noexcept { return node->family_name.equals(_name); }
};

static bool prepare_query(const BLFontManagerPrivateImpl* impl, const char* name, size_t name_size, PreparedQuery* out) noexcept {
  bl_unused(impl);

  if (name_size == SIZE_MAX)
    name_size = strlen(name);

  out->_name.reset(name, name_size);
  out->_hash_code = HashOps::hash_stringCI(name, name_size);
  return name_size != 0;
}

// bl::FontManager - Query - Diff Calculation
// ==========================================

static BL_INLINE uint32_t calc_family_name_diff(BLStringView a_str, BLStringView b_str) noexcept {
  if (a_str.size != b_str.size)
    return kQueryInvalidDiff;

  uint32_t diff = 0;

  for (size_t i = 0; i < a_str.size; i++) {
    uint32_t a = uint8_t(a_str.data[i]);
    uint32_t b = uint8_t(b_str.data[i]);

    if (a == b)
      continue;

    a = Unicode::ascii_to_lower(a);
    b = Unicode::ascii_to_lower(b);

    if (a != b)
      return kQueryInvalidDiff;

    diff++;
  }

  if (diff > 255)
    diff = 255;

  return diff << kQueryDiffFamilyNameShift;
}

static BL_INLINE uint32_t calc_property_diff(const BLFontFaceImpl* face_impl, const BLFontQueryProperties* properties) noexcept {
  uint32_t diff = 0;

  uint32_t fStyle = face_impl->style;
  uint32_t fWeight = face_impl->weight;
  uint32_t fStretch = face_impl->stretch;

  uint32_t pStyle = properties->style;
  uint32_t pWeight = properties->weight;
  uint32_t pStretch = properties->stretch;

  uint32_t style_diff = uint32_t(bl_abs(int(pStyle) - int(fStyle)));
  uint32_t weight_diff = uint32_t(bl_abs(int(pWeight) - int(fWeight)));
  uint32_t stretch_diff = uint32_t(bl_abs(int(pStretch) - int(fStretch)));

  diff |= style_diff << kQueryDiffStyleValueShift;
  diff |= uint32_t(pStyle < fStyle) << kQueryDiffStyleSignShift;

  diff |= weight_diff << kQueryDiffWeightValueShift;
  diff |= uint32_t(pWeight < fWeight) << kQueryDiffWeightSignShift;

  diff |= stretch_diff << kQueryDiffStretchValueShift;
  diff |= uint32_t(pStretch < fStretch) << kQueryDiffStretchSignShift;

  return diff;
}

// bl::FontManager - Query - Match
// ===============================

class QueryBestMatch {
public:
  const BLFontQueryProperties* properties;
  const BLFontFace* face;
  uint32_t diff;

  BL_INLINE QueryBestMatch(const BLFontQueryProperties* properties) noexcept
    : properties(properties),
      face(nullptr),
      diff(0xFFFFFFFFu) {}

  BL_INLINE bool has_face() const noexcept { return face != nullptr; }

  void match(const BLFontFace& face_in, uint32_t base_diff = 0) noexcept {
    uint32_t local_diff = base_diff + calc_property_diff(face_in._impl(), properties);
    if (diff > local_diff) {
      face = &face_in;
      diff = local_diff;
    }
  }
};

} // {FontManagerInternal}
} // {bl}

// bl::FontManager - API - Init & Destroy
// ======================================

BL_API_IMPL BLResult bl_font_manager_init(BLFontManagerCore* self) noexcept {
  self->_d = bl_object_defaults[BL_OBJECT_TYPE_FONT_MANAGER]._d;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_font_manager_init_move(BLFontManagerCore* self, BLFontManagerCore* other) noexcept {
  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_font_manager());

  self->_d = other->_d;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_FONT_MANAGER]._d;

  return BL_SUCCESS;

}

BL_API_IMPL BLResult bl_font_manager_init_weak(BLFontManagerCore* self, const BLFontManagerCore* other) noexcept {
  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_font_manager());

  return bl_object_private_init_weak_tagged(self, other);
}

BL_API_IMPL BLResult bl_font_manager_init_new(BLFontManagerCore* self) noexcept {
  using namespace bl::FontManagerInternal;

  self->_d = bl_object_defaults[BL_OBJECT_TYPE_FONT_MANAGER]._d;
  return alloc_impl(self);
}

BL_API_IMPL BLResult bl_font_manager_destroy(BLFontManagerCore* self) noexcept {
  BL_ASSERT(self->_d.is_font_manager());

  return bl::ObjectInternal::release_virtual_instance(self);
}

// bl::FontManager - API - Reset
// =============================

BL_API_IMPL BLResult bl_font_manager_reset(BLFontManagerCore* self) noexcept {
  BL_ASSERT(self->_d.is_font_manager());

  return bl::ObjectInternal::replace_virtual_instance(self, static_cast<BLFontManagerCore*>(&bl_object_defaults[BL_OBJECT_TYPE_FONT_MANAGER]));
}

// bl::FontManager - API - Assign
// ==============================

BL_API_IMPL BLResult bl_font_manager_assign_move(BLFontManagerCore* self, BLFontManagerCore* other) noexcept {
  BL_ASSERT(self->_d.is_font_manager());
  BL_ASSERT(other->_d.is_font_manager());

  BLFontManagerCore tmp = *other;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_FONT_MANAGER]._d;
  return bl::ObjectInternal::replace_virtual_instance(self, &tmp);
}

BL_API_IMPL BLResult bl_font_manager_assign_weak(BLFontManagerCore* self, const BLFontManagerCore* other) noexcept {
  BL_ASSERT(self->_d.is_font_manager());
  BL_ASSERT(other->_d.is_font_manager());

  return bl::ObjectInternal::assign_virtual_instance(self, other);
}

// bl::FontManager - API - Equals
// ==============================

bool bl_font_manager_equals(const BLFontManagerCore* a, const BLFontManagerCore* b) noexcept {
  BL_ASSERT(a->_d.is_font_manager());
  BL_ASSERT(b->_d.is_font_manager());

  return a->_d.impl == b->_d.impl;
}

// bl::FontManager - API - Create
// ==============================

BL_API_IMPL BLResult bl_font_manager_create(BLFontManagerCore* self) noexcept {
  using namespace bl::FontManagerInternal;
  BL_ASSERT(self->_d.is_font_manager());

  BLFontManagerCore newO;
  BL_PROPAGATE(alloc_impl(&newO));

  return bl::ObjectInternal::replace_virtual_instance(self, &newO);
}

// bl::FontManager - API - Accessors
// =================================

BL_API_IMPL size_t bl_font_manager_get_face_count(const BLFontManagerCore* self) noexcept {
  using namespace bl::FontManagerInternal;
  BL_ASSERT(self->_d.is_font_manager());

  BLFontManagerPrivateImpl* self_impl = get_impl(self);
  BLSharedLockGuard<BLSharedMutex> guard(self_impl->mutex);

  return self_impl->face_count;
}

BL_API_IMPL size_t bl_font_manager_get_family_count(const BLFontManagerCore* self) noexcept {
  using namespace bl::FontManagerInternal;
  BL_ASSERT(self->_d.is_font_manager());

  BLFontManagerPrivateImpl* self_impl = get_impl(self);
  BLSharedLockGuard<BLSharedMutex> guard(self_impl->mutex);

  return self_impl->families_map.size();
}

// bl::FontManager - Internal Utilities
// ====================================

static BL_INLINE BLResult bl_font_manager_make_mutable(BLFontManagerCore* self) noexcept {
  BL_ASSERT(self->_d.is_font_manager());

  if (!self->dcast().is_valid())
    return bl_font_manager_create(self);

  return BL_SUCCESS;
}

// bl::FontManager - API - Font Face Management
// ============================================

BL_API_IMPL bool bl_font_manager_has_face(const BLFontManagerCore* self, const BLFontFaceCore* face) noexcept {
  using namespace bl::FontManagerInternal;

  BL_ASSERT(self->_d.is_font_manager());
  BL_ASSERT(self->_d.is_font_face());

  BLFontManagerPrivateImpl* self_impl = get_impl(self);
  BLFontFacePrivateImpl* face_impl = bl::FontFaceInternal::get_impl(face);

  uint32_t name_hash = bl::HashOps::hash_stringCI(face_impl->family_name.dcast().view());

  BLSharedLockGuard<BLSharedMutex> guard(self_impl->mutex);
  BLFontManagerPrivateImpl::FamiliesMapNode* families_node =
    self_impl->families_map.get(BLFontManagerPrivateImpl::FamilyMatcher{face_impl->family_name.dcast().view(), name_hash});

  if (!families_node)
    return false;

  size_t index = index_of_face(families_node->faces.data(), families_node->faces.size(), face_impl);
  return index != SIZE_MAX;
}

BL_API_IMPL BLResult bl_font_manager_add_face(BLFontManagerCore* self, const BLFontFaceCore* face) noexcept {
  using namespace bl::FontManagerInternal;

  BL_ASSERT(self->_d.is_font_manager());
  BL_ASSERT(self->_d.is_font_face());

  if (!face->dcast().is_valid())
    return bl_make_error(BL_ERROR_FONT_NOT_INITIALIZED);

  BL_PROPAGATE(bl_font_manager_make_mutable(self));

  BLFontManagerPrivateImpl* self_impl = get_impl(self);
  BLFontFacePrivateImpl* face_impl = bl::FontFaceInternal::get_impl(face);

  uint32_t name_hash = bl::HashOps::hash_stringCI(face_impl->family_name.dcast().view());

  BLLockGuard<BLSharedMutex> guard(self_impl->mutex);
  bl::ArenaAllocator::StatePtr allocator_state = self_impl->allocator.save_state();

  BLFontManagerPrivateImpl::FamiliesMapNode* families_node =
    self_impl->families_map.get(BLFontManagerPrivateImpl::FamilyMatcher{face_impl->family_name.dcast().view(), name_hash});

  if (!families_node) {
    families_node = self_impl->allocator.new_t<BLFontManagerPrivateImpl::FamiliesMapNode>(name_hash, face_impl->family_name.dcast());
    if (!families_node)
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

    // Reserve for only one item at the beginning. This helps to decrease memory footprint when loading a lot of font
    // faces that don't share family names.
    BLResult result = families_node->faces.reserve(1u);
    if (BL_UNLIKELY(result != BL_SUCCESS)) {
      bl_call_dtor(*families_node);
      self_impl->allocator.restore_state(allocator_state);
      return result;
    }

    families_node->faces.append(face->dcast());
    self_impl->families_map.insert(families_node);
  }
  else {
    size_t index = index_for_insertion(families_node->faces.data(), families_node->faces.size(), face_impl);
    if (index == SIZE_MAX)
      return BL_SUCCESS;
    BL_PROPAGATE(families_node->faces.insert(index, face->dcast()));
  }

  self_impl->face_count++;
  return BL_SUCCESS;
}

// bl::FontManager - Query - API
// =============================

BL_API_IMPL BLResult bl_font_manager_query_faces_by_family_name(const BLFontManagerCore* self, const char* name, size_t name_size, BLArrayCore* out) noexcept {
  using namespace bl::FontManagerInternal;
  BL_ASSERT(self->_d.is_font_manager());

  if (BL_UNLIKELY(out->_d.raw_type() != BL_OBJECT_TYPE_ARRAY_OBJECT))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  {
    BLFontManagerPrivateImpl* self_impl = get_impl(self);
    BLSharedLockGuard<BLSharedMutex> guard(self_impl->mutex);

    PreparedQuery query;
    uint32_t candidate_diff = 0xFFFFFFFF;
    BLFontManagerPrivateImpl::FamiliesMapNode* candidate = nullptr;

    if (prepare_query(self_impl, name, name_size, &query)) {
      BLFontManagerPrivateImpl::FamiliesMapNode* node = self_impl->families_map.get(query);
      while (node) {
        uint32_t family_diff = calc_family_name_diff(node->family_name.view(), query.name());
        if (candidate_diff > family_diff) {
          candidate_diff = family_diff;
          candidate = node;
        }
        node = node->next();
      }
    }

    if (candidate)
      return out->dcast<BLArray<BLFontFace>>().assign(candidate->faces);
  }

  // This is not considered to be an error, thus don't use bl_make_error().
  out->dcast<BLArray<BLFontFace>>().clear();
  return BL_ERROR_FONT_NO_MATCH;
}

BL_API_IMPL BLResult bl_font_manager_query_face(
  const BLFontManagerCore* self,
  const char* name, size_t name_size,
  const BLFontQueryProperties* properties,
  BLFontFaceCore* out) noexcept {

  using namespace bl::FontManagerInternal;
  BL_ASSERT(self->_d.is_font_manager());

  if (!properties)
    properties = &default_query_properties;

  BLFontQueryProperties sanitized_properties;
  if (!sanitize_query_properties(sanitized_properties, *properties))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  {
    BLFontManagerPrivateImpl* self_impl = get_impl(self);
    BLSharedLockGuard<BLSharedMutex> guard(self_impl->mutex);

    PreparedQuery query;
    QueryBestMatch best_match(&sanitized_properties);

    if (prepare_query(self_impl, name, name_size, &query)) {
      BLFontManagerPrivateImpl::FamiliesMapNode* node = self_impl->families_map.nodes_by_hash_code(query.hash_code());
      while (node) {
        uint32_t family_diff = calc_family_name_diff(node->family_name.view(), query.name());
        if (family_diff != kQueryInvalidDiff) {
          for (const BLFontFace& face : node->faces.dcast<BLArray<BLFontFace>>())
            best_match.match(face, family_diff);
        }
        node = node->next();
      }
    }

    if (best_match.has_face())
      return out->dcast().assign(*best_match.face);
  }

  // This is not considered to be an error, thus don't use bl_make_error().
  out->dcast().reset();
  return BL_ERROR_FONT_NO_MATCH;
}

// bl::FontManager - Runtime Registration
// ======================================

void bl_font_manager_rt_init(BLRuntimeContext* rt) noexcept {
  bl_unused(rt);

  auto& default_impl = bl::FontManagerInternal::default_impl;

  default_impl.virt.base.destroy = bl::FontManagerInternal::destroy_impl;
  default_impl.virt.base.get_property = bl_object_impl_get_property;
  default_impl.virt.base.set_property = bl_object_impl_set_property;
  default_impl.impl.init(&default_impl.virt);

  bl_object_defaults[BL_OBJECT_TYPE_FONT_MANAGER]._d.init_dynamic(
    BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_FONT_MANAGER) | BLObjectInfo::from_abcp(1), &default_impl.impl);
}
