// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/array_p.h>
#include <blend2d/core/filesystem.h>
#include <blend2d/core/fontdata_p.h>
#include <blend2d/core/fonttagdata_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/opentype/otcore_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/ptrops_p.h>

namespace bl {
namespace FontDataInternal {

// bl::FontData - Globals
// ======================

static BLObjectEternalVirtualImpl<BLFontDataPrivateImpl, BLFontDataVirt> default_impl;

static BLFontDataVirt mem_font_data_virt;

// bl::FontData - Null Impl
// ========================

static BLResult BL_CDECL null_destroy_impl(BLObjectImpl* impl) noexcept {
  bl_unused(impl);
  return BL_SUCCESS;
}

static BLResult BL_CDECL null_get_table_tags_impl(const BLFontDataImpl* impl, uint32_t face_index, BLArrayCore* out) noexcept {
  bl_unused(impl, face_index);
  return bl_array_clear(out);
}

static size_t BL_CDECL null_get_tables_impl(const BLFontDataImpl* impl, uint32_t face_index, BLFontTable* dst, const BLTag* tags, size_t n) noexcept {
  bl_unused(impl, face_index, tags);
  for (size_t i = 0; i < n; i++)
    dst[i].reset();
  return 0;
}

// bl::FontData - Memory Impl
// ==========================

struct MemFontDataImpl : public BLFontDataPrivateImpl {
  //! Pointer to the start of font data.
  void* data;
  //! Size of `data` [in bytes].
  uint32_t data_size;
  //! Offset to an array that contains offsets for each font face.
  uint32_t offset_array_index;

  //! If the `data` is not external it's held by this array.
  BLArray<uint8_t> data_array;
};

// Destroys `MemFontDataImpl` - this is a real destructor.
static BLResult mem_real_destroy(MemFontDataImpl* impl) noexcept {
  if (ObjectInternal::is_impl_external(impl))
    ObjectInternal::call_external_destroy_func(impl, impl->data);

  bl_call_dtor(impl->face_cache);
  bl_call_dtor(impl->data_array);

  return ObjectInternal::free_impl(impl);
}

static BLResult BL_CDECL mem_destroy_impl(BLObjectImpl* impl) noexcept {
  return mem_real_destroy(static_cast<MemFontDataImpl*>(impl));
}

static BLResult BL_CDECL mem_get_table_tags_impl(const BLFontDataImpl* impl_, uint32_t face_index, BLArrayCore* out) noexcept {
  using namespace OpenType;

  const MemFontDataImpl* impl = static_cast<const MemFontDataImpl*>(impl_);
  const void* font_data = impl->data;
  size_t data_size = impl->data_size;

  if (BL_UNLIKELY(face_index >= impl->face_count)) {
    bl_array_clear(out);
    return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  uint32_t header_offset = 0;
  if (impl->offset_array_index)
    header_offset = PtrOps::offset<UInt32>(font_data, impl->offset_array_index)[face_index].value();

  if (BL_LIKELY(header_offset <= data_size - sizeof(SFNTHeader))) {
    const SFNTHeader* sfnt = PtrOps::offset<SFNTHeader>(font_data, header_offset);
    if (FontTagData::is_open_type_version_tag(sfnt->version_tag())) {
      // We can safely multiply `table_count` as SFNTHeader::num_tables is `UInt16`.
      uint32_t table_count = sfnt->num_tables();
      uint32_t min_data_size = uint32_t(sizeof(SFNTHeader)) + table_count * uint32_t(sizeof(SFNTHeader::TableRecord));

      if (BL_LIKELY(data_size - header_offset >= min_data_size)) {
        uint32_t* dst;
        BL_PROPAGATE(bl_array_modify_op(out, BL_MODIFY_OP_ASSIGN_FIT, table_count, (void**)&dst));

        const SFNTHeader::TableRecord* tables = sfnt->table_records();
        for (uint32_t table_index = 0; table_index < table_count; table_index++)
          dst[table_index] = tables[table_index].tag();
        return BL_SUCCESS;
      }
    }
  }

  bl_array_clear(out);
  return bl_make_error(BL_ERROR_INVALID_DATA);
}

static size_t BL_CDECL mem_get_tables_impl(const BLFontDataImpl* impl_, uint32_t face_index, BLFontTable* dst, const BLTag* tags, size_t n) noexcept {
  using namespace bl::OpenType;

  const MemFontDataImpl* impl = static_cast<const MemFontDataImpl*>(impl_);
  const void* font_data = impl->data;
  size_t data_size = impl->data_size;

  memset(dst, 0, n * sizeof(BLFontTable));

  if (BL_LIKELY(face_index < impl->face_count)) {
    uint32_t header_offset = 0;
    if (impl->offset_array_index)
      header_offset = PtrOps::offset<UInt32>(font_data, impl->offset_array_index)[face_index].value();

    if (BL_LIKELY(header_offset <= data_size - sizeof(SFNTHeader))) {
      const SFNTHeader* sfnt = PtrOps::offset<SFNTHeader>(font_data, header_offset);
      if (FontTagData::is_open_type_version_tag(sfnt->version_tag())) {
        uint32_t table_count = sfnt->num_tables();

        // We can safely multiply `table_count` as SFNTHeader::num_tables is `UInt16`.
        uint32_t min_data_size = uint32_t(sizeof(SFNTHeader)) + table_count * uint32_t(sizeof(SFNTHeader::TableRecord));
        if (BL_LIKELY(data_size - header_offset >= min_data_size)) {
          // Iterate over all tables and try to find all tables as specified by `tags`.
          const SFNTHeader::TableRecord* tables = sfnt->table_records();
          size_t match_count = 0;

          // If all tags are known (convertible to table_id) we can just build a small index and then try
          // to match all tags by doing a linear scan. If there is one or more unknown tags, we would go
          // with linear scan.
          //
          // In general, we do this, because Blend2D's OpenType engine requests all tables in one go, and
          // then inspects them on load - this means that this table lookup is in general optimized and
          // only does a single iteration of 'sfnt' tables.
          if (n >= 3u && n < 255u) {
            constexpr uint32_t kTableIdCountAligned = IntOps::align_up(FontTagData::kTableIdCount, 16);

            uint8_t table_id_to_index[kTableIdCountAligned];
            memset(table_id_to_index, 0xFF, kTableIdCountAligned);

            size_t i = 0;
            while (i < n) {
              uint32_t table_id = FontTagData::table_tag_to_id(tags[i]);
              if (table_id == FontTagData::kInvalidId)
                break;
              table_id_to_index[table_id] = uint8_t(i);
              i++;
            }

            if (i == n) {
              // All requested tags known, table id to dst index lookup built successfully.
              for (uint32_t table_index = 0; table_index < table_count; table_index++) {
                const SFNTHeader::TableRecord& table = tables[table_index];

                BLTag table_tag = table.tag();
                uint32_t table_id = FontTagData::table_tag_to_id(table_tag);

                // Since we know that all requested tags have their unique IDs, we can refuse any tag that doesn't.
                if (table_id == FontTagData::kInvalidId)
                  continue;

                i = table_id_to_index[table_id];
                if (i != 0xFF) {
                  uint32_t table_offset = table.offset();
                  uint32_t table_size = table.length();

                  if (table_offset < data_size && table_size && table_size <= data_size - table_offset) {
                    dst[i].reset(PtrOps::offset<uint8_t>(font_data, table_offset), table_size);
                    match_count++;
                  }
                }
              }
            }

            return match_count;
          }

          // Linear search.
          for (size_t tag_index = 0; tag_index < n; tag_index++) {
            uint32_t tag = IntOps::byteSwap32BE(tags[tag_index]);
            for (uint32_t table_index = 0; table_index < table_count; table_index++) {
              const SFNTHeader::TableRecord& table = tables[table_index];

              if (table.tag.raw_value() == tag) {
                uint32_t table_offset = table.offset();
                uint32_t table_size = table.length();

                if (table_offset < data_size && table_size && table_size <= data_size - table_offset) {
                  dst[tag_index].reset(PtrOps::offset<uint8_t>(font_data, table_offset), table_size);
                  match_count++;
                }

                break;
              }
            }
          }

          return match_count;
        }
      }
    }
  }

  return 0;
}

} // {FontDataInternal}
} // {bl}

// bl::FontData - API - Init & Destroy
// ===================================

BLResult bl_font_data_init(BLFontDataCore* self) noexcept {
  self->_d = bl_object_defaults[BL_OBJECT_TYPE_FONT_DATA]._d;
  return BL_SUCCESS;
}

BLResult bl_font_data_init_move(BLFontDataCore* self, BLFontDataCore* other) noexcept {
  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_font_data());

  self->_d = other->_d;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_FONT_DATA]._d;

  return BL_SUCCESS;
}

BLResult bl_font_data_init_weak(BLFontDataCore* self, const BLFontDataCore* other) noexcept {
  BL_ASSERT(self != other);
  BL_ASSERT(other->_d.is_font_data());

  return bl_object_private_init_weak_tagged(self, other);
}

BLResult bl_font_data_destroy(BLFontDataCore* self) noexcept {
  BL_ASSERT(self->_d.is_font_data());

  return bl::ObjectInternal::release_virtual_instance(self);
}

// bl::FontData - API - Reset
// ==========================

BLResult bl_font_data_reset(BLFontDataCore* self) noexcept {
  BL_ASSERT(self->_d.is_font_data());

  return bl::ObjectInternal::replace_virtual_instance(self, static_cast<BLFontDataCore*>(&bl_object_defaults[BL_OBJECT_TYPE_FONT_DATA]));
}

// bl::FontData - API - Assign
// ===========================

BLResult bl_font_data_assign_move(BLFontDataCore* self, BLFontDataCore* other) noexcept {
  BL_ASSERT(self->_d.is_font_data());
  BL_ASSERT(other->_d.is_font_data());

  BLFontDataCore tmp = *other;
  other->_d = bl_object_defaults[BL_OBJECT_TYPE_FONT_DATA]._d;
  return bl::ObjectInternal::replace_virtual_instance(self, &tmp);
}

BLResult bl_font_data_assign_weak(BLFontDataCore* self, const BLFontDataCore* other) noexcept {
  BL_ASSERT(self->_d.is_font_data());
  BL_ASSERT(other->_d.is_font_data());

  return bl::ObjectInternal::assign_virtual_instance(self, other);
}

// bl::FontData - API - Equality & Comparison
// ==========================================

bool bl_font_data_equals(const BLFontDataCore* a, const BLFontDataCore* b) noexcept {
  BL_ASSERT(a->_d.is_font_data());
  BL_ASSERT(b->_d.is_font_data());

  return a->_d.impl == b->_d.impl;
}

// bl::FontData - API - Create
// ===========================

BLResult bl_font_data_create_from_file(BLFontDataCore* self, const char* file_name, BLFileReadFlags read_flags) noexcept {
  BL_ASSERT(self->_d.is_font_data());

  BLArray<uint8_t> buffer;
  BL_PROPAGATE(BLFileSystem::read_file(file_name, buffer, 0, read_flags));

  if (buffer.is_empty())
    return bl_make_error(BL_ERROR_FILE_EMPTY);

  return bl_font_data_create_from_data_array(self, &buffer);
}

static BLResult bl_font_data_create_from_data_internal(BLFontDataCore* self, const void* data, size_t data_size, BLDestroyExternalDataFunc destroy_func, void* user_data, const BLArray<uint8_t>* array) noexcept {
  using namespace bl::FontDataInternal;
  using namespace bl::OpenType;

  constexpr uint32_t kBaseSize = bl_min<uint32_t>(SFNTHeader::kBaseSize, TTCFHeader::kBaseSize);
  if (BL_UNLIKELY(data_size < kBaseSize))
    return bl_make_error(BL_ERROR_INVALID_DATA);

  if (BL_UNLIKELY(sizeof(size_t) > 4 && data_size > 0xFFFFFFFFu))
    return bl_make_error(BL_ERROR_DATA_TOO_LARGE);

  uint32_t header_tag = bl::PtrOps::offset<const UInt32>(data, 0)->value();
  uint32_t face_count = 1;
  uint32_t data_flags = 0;

  uint32_t offset_array_index = 0;
  const UInt32* offset_array = nullptr;

  if (bl::FontTagData::is_open_type_collection_tag(header_tag)) {
    if (BL_UNLIKELY(data_size < TTCFHeader::kBaseSize))
      return bl_make_error(BL_ERROR_INVALID_DATA);

    const TTCFHeader* header = bl::PtrOps::offset<const TTCFHeader>(data, 0);

    face_count = header->fonts.count();
    if (BL_UNLIKELY(!face_count || face_count > BL_FONT_DATA_MAX_FACE_COUNT))
      return bl_make_error(BL_ERROR_INVALID_DATA);

    size_t ttc_header_size = header->calc_size(face_count);
    if (BL_UNLIKELY(ttc_header_size > data_size))
      return bl_make_error(BL_ERROR_INVALID_DATA);

    offset_array = header->fonts.array();
    offset_array_index = (uint32_t)((uintptr_t)offset_array - (uintptr_t)header);

    data_flags |= BL_FONT_DATA_FLAG_COLLECTION;
  }
  else {
    if (!bl::FontTagData::is_open_type_version_tag(header_tag))
      return bl_make_error(BL_ERROR_INVALID_SIGNATURE);
  }

  BLArray<BLFontFaceImpl*> face_cache;
  BL_PROPAGATE(face_cache.resize(face_count, nullptr));

  BLFontDataCore newO;
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_FONT_DATA);

  if (!destroy_func)
    BL_PROPAGATE(bl::ObjectInternal::alloc_impl_t<MemFontDataImpl>(&newO, info));
  else
    BL_PROPAGATE(bl::ObjectInternal::alloc_impl_external_t<MemFontDataImpl>(&newO, info, true, destroy_func, user_data));

  MemFontDataImpl* new_impl = static_cast<MemFontDataImpl*>(newO._d.impl);
  init_impl(new_impl, &mem_font_data_virt);

  new_impl->face_type = uint8_t(BL_FONT_FACE_TYPE_OPENTYPE);
  new_impl->face_count = face_count;
  new_impl->flags = data_flags;

  bl_call_ctor(new_impl->face_cache, BLInternal::move(face_cache));
  bl_call_ctor(new_impl->data_array);

  if (array) {
    new_impl->data_array = *array;
    data = new_impl->data_array.data();
  }

  new_impl->data = const_cast<void*>(data);
  new_impl->data_size = uint32_t(data_size);
  new_impl->offset_array_index = offset_array_index;

  return bl::ObjectInternal::replace_virtual_instance(self, &newO);
}

BLResult bl_font_data_create_from_data_array(BLFontDataCore* self, const BLArrayCore* data_array) noexcept {
  BL_ASSERT(self->_d.is_font_data());

  if (data_array->_d.raw_type() != BL_OBJECT_TYPE_ARRAY_UINT8)
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  const BLArray<uint8_t>& array = data_array->dcast<BLArray<uint8_t>>();
  const void* data = array.data();
  size_t data_size = array.size();

  return bl_font_data_create_from_data_internal(self, data, data_size, nullptr, nullptr, &array);
}

BLResult bl_font_data_create_from_data(BLFontDataCore* self, const void* data, size_t data_size, BLDestroyExternalDataFunc destroy_func, void* user_data) noexcept {
  return bl_font_data_create_from_data_internal(self, data, data_size, destroy_func, user_data, nullptr);
};

// bl::FontData - API - Accessors
// ==============================

uint32_t bl_font_data_get_face_count(const BLFontDataCore* self) noexcept {
  using namespace bl::FontDataInternal;
  BL_ASSERT(self->_d.is_font_data());

  const BLFontDataPrivateImpl* self_impl = get_impl(self);
  return self_impl->face_count;
}

BLFontFaceType bl_font_data_get_face_type(const BLFontDataCore* self) noexcept {
  using namespace bl::FontDataInternal;
  BL_ASSERT(self->_d.is_font_data());

  const BLFontDataPrivateImpl* self_impl = get_impl(self);
  return BLFontFaceType(self_impl->face_type);
}

BLFontDataFlags bl_font_data_get_flags(const BLFontDataCore* self) noexcept {
  using namespace bl::FontDataInternal;
  BL_ASSERT(self->_d.is_font_data());

  const BLFontDataPrivateImpl* self_impl = get_impl(self);
  return BLFontDataFlags(self_impl->flags);
}

BLResult bl_font_data_get_table_tags(const BLFontDataCore* self, uint32_t face_index, BLArrayCore* dst) noexcept {
  using namespace bl::FontDataInternal;
  BL_ASSERT(self->_d.is_font_data());

  const BLFontDataPrivateImpl* self_impl = get_impl(self);
  return self_impl->virt->get_table_tags(self_impl, face_index, dst);
}

size_t bl_font_data_get_tables(const BLFontDataCore* self, uint32_t face_index, BLFontTable* dst, const BLTag* tags, size_t count) noexcept {
  using namespace bl::FontDataInternal;
  BL_ASSERT(self->_d.is_font_data());

  const BLFontDataPrivateImpl* self_impl = get_impl(self);
  return self_impl->virt->get_tables(self_impl, face_index, dst, tags, count);
}

// bl::FontData - Runtime Registration
// ===================================

void bl_font_data_rt_init(BLRuntimeContext* rt) noexcept {
  using namespace bl::FontDataInternal;

  bl_unused(rt);

  default_impl.virt.base.destroy = null_destroy_impl;
  default_impl.virt.base.get_property = bl_object_impl_get_property;
  default_impl.virt.base.set_property = bl_object_impl_set_property;
  default_impl.virt.get_table_tags = null_get_table_tags_impl;
  default_impl.virt.get_tables = null_get_tables_impl;
  init_impl(&default_impl.impl, &default_impl.virt);

  mem_font_data_virt.base.destroy = mem_destroy_impl;
  mem_font_data_virt.base.get_property = bl_object_impl_get_property;
  mem_font_data_virt.base.set_property = bl_object_impl_set_property;
  mem_font_data_virt.get_table_tags = mem_get_table_tags_impl;
  mem_font_data_virt.get_tables = mem_get_tables_impl;

  bl_object_defaults[BL_OBJECT_TYPE_FONT_DATA]._d.init_dynamic(BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_FONT_DATA), &default_impl.impl);
}
