// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/core/assembler.h>
#include <asmjit/core/codewriter_p.h>
#include <asmjit/core/logger.h>
#include <asmjit/support/support.h>

#include <algorithm>
#include <tuple>

ASMJIT_BEGIN_NAMESPACE

// CodeHolder - X86 Utilities
// ==========================

//! Encodes a MOD byte.
static inline uint32_t x86_encode_mod(uint32_t m, uint32_t o, uint32_t rm) noexcept {
  return (m << 6) | (o << 3) | rm;
}

// CodeHolder - LabelEntry Globals & Utilities
// ===========================================

static constexpr LabelEntry::ExtraData CodeHolder_make_shared_label_extra_data() noexcept {
  LabelEntry::ExtraData extra_data {};
  extra_data._section_id = Globals::kInvalidId;
  extra_data._parent_id = Globals::kInvalidId;
  return extra_data;
}

static constexpr LabelEntry::ExtraData CodeHolder_shared_label_extra_data = CodeHolder_make_shared_label_extra_data();

class ResolveFixupIterator {
public:
  Fixup* _fixup {};
  Fixup** _prev {};
  size_t _resolved_count {};
  size_t _unresolved_count {};

  ASMJIT_INLINE_NODEBUG explicit ResolveFixupIterator(Fixup** prev_fixup_ptr) noexcept { reset(prev_fixup_ptr); }
  ASMJIT_INLINE_NODEBUG bool is_valid() const noexcept { return _fixup != nullptr; }
  ASMJIT_INLINE_NODEBUG Fixup* fixup() const noexcept { return _fixup; }

  ASMJIT_INLINE void reset(Fixup** prev_fixup_ptr) noexcept {
    _prev = prev_fixup_ptr;
    _fixup = *_prev;
  }

  ASMJIT_INLINE void next() noexcept {
    _prev = &_fixup->next;
    _fixup = *_prev;
    _unresolved_count++;
  }

  ASMJIT_INLINE void resolve_and_next(CodeHolder* code) noexcept {
    Fixup* fixup_to_delete = _fixup;

    _fixup = _fixup->next;
    *_prev = _fixup;

    _resolved_count++;
    code->_fixup_data_pool.release(fixup_to_delete);
  }

  ASMJIT_INLINE_NODEBUG size_t resolved_count() const noexcept { return _resolved_count; }
  ASMJIT_INLINE_NODEBUG size_t unresolved_count() const noexcept { return _unresolved_count; }
};

// CodeHolder - Section Globals & Utilities
// ========================================

static const char Section_address_table_name[] = ".addrtab";

static ASMJIT_INLINE void Section_init_name(
  Section* section,
  char c0 = 0, char c1 = 0, char c2 = 0, char c3 = 0,
  char c4 = 0, char c5 = 0, char c6 = 0, char c7 = 0) noexcept {

  section->_name.u32[0] = Support::bytepack32_4x8(uint8_t(c0), uint8_t(c1), uint8_t(c2), uint8_t(c3));
  section->_name.u32[1] = Support::bytepack32_4x8(uint8_t(c4), uint8_t(c5), uint8_t(c6), uint8_t(c7));
  section->_name.u32[2] = 0u;
  section->_name.u32[3] = 0u;
}

static ASMJIT_INLINE void Section_init_data(Section* section, uint32_t section_id, SectionFlags flags, uint32_t alignment, int order) noexcept {
  section->_section_id = section_id;

  // These two fields are not used by sections (see \ref LabelEntry for more details about why).
  section->_internal_label_type = LabelType::kAnonymous;
  section->_internal_label_flags = LabelFlags::kNone;

  section->assign_flags(flags);
  section->_alignment = alignment;
  section->_order = order;
  section->_offset = 0;
  section->_virtual_size = 0;
}

static ASMJIT_INLINE void Section_init_buffer(Section* section) noexcept {
  section->_buffer = CodeBuffer{};
}

static ASMJIT_INLINE void Section_release_buffer(Section* section) noexcept {
  if (Support::bool_and(section->_buffer.data() != nullptr, !section->_buffer.is_external())) {
    ::free(section->_buffer._data);
  }
}

// CodeHolder - Utilities
// ======================

static ASMJIT_INLINE Error CodeHolder_init_section_storage(CodeHolder* self) noexcept {
  Error err1 = self->_sections.reserve_additional(self->_arena);
  Error err2 = self->_sections_by_order.reserve_additional(self->_arena);

  return Error(uint32_t(err1) | uint32_t(err2));
}

static ASMJIT_INLINE void CodeHolder_add_text_section(CodeHolder* self) noexcept {
  Section* text_section = &self->_text_section;

  Section_init_data(text_section, 0u, SectionFlags::kExecutable | SectionFlags::kReadOnly | SectionFlags::kBuiltIn, 0u, 0);
  Section_init_name(text_section, '.', 't', 'e', 'x', 't');

  self->_sections.append_unchecked(text_section);
  self->_sections_by_order.append_unchecked(text_section);
}

static ASMJIT_NOINLINE void CodeHolder_detach_emitters(CodeHolder* self) noexcept {
  BaseEmitter* emitter = self->_attached_first;

  while (emitter) {
    BaseEmitter* next = emitter->_attached_next;

    emitter->_attached_prev = nullptr;
    (void)emitter->on_detach(*self);
    emitter->_attached_next = nullptr;
    emitter->_code = nullptr;

    emitter = next;
    self->_attached_first = next;
  }

  self->_attached_last = nullptr;
}

static ASMJIT_INLINE void CodeHolder_reset_env_and_attached_logger_and_eh(CodeHolder* self) noexcept {
  self->_environment.reset();
  self->_cpu_features.reset();
  self->_base_address = Globals::kNoBaseAddress;
  self->_logger = nullptr;
  self->_error_handler = nullptr;
}

// Reset sections.
static ASMJIT_INLINE void CodeHolder_reset_sections(CodeHolder* self, ResetPolicy reset_policy) noexcept {
  // Reset all sections except the first one (.text section).
  uint32_t from_section = reset_policy == ResetPolicy::kHard ? 0u : 1u;
  uint32_t section_count = self->_sections._size;

  for (uint32_t i = from_section; i < section_count; i++) {
    Section* section = self->_sections[i];

    Section_release_buffer(section);
    section->_buffer._data = nullptr;
    section->_buffer._capacity = 0;
  }
}

// Reset arena and all containers using it.
static ASMJIT_INLINE void CodeHolder_reset_containers(CodeHolder* self, ResetPolicy reset_policy) noexcept {
  // Soft reset won't wipe out the .text section, so set its size to 0 for future reuse.
  self->_text_section._buffer._size = 0;

  self->_named_labels.reset();
  self->_relocations.reset();
  self->_label_entries.reset();

  self->_fixups = nullptr;
  self->_fixup_data_pool.reset();
  self->_unresolved_fixup_count = 0;

  self->_sections.reset();
  self->_sections_by_order.reset();

  self->_address_table_section = nullptr;
  self->_address_table_entries.reset();

  self->_arena.reset(reset_policy);
}

// Reset sections and containers.
static ASMJIT_NOINLINE void CodeHolder_reset_sections_and_containers(CodeHolder* self, ResetPolicy reset_policy) noexcept {
  CodeHolder_reset_sections(self, reset_policy);
  CodeHolder_reset_containers(self, reset_policy);
}

static ASMJIT_INLINE void CodeHolder_on_settings_updated(CodeHolder* self) noexcept {
  // Notify all attached emitters about a settings update.
  BaseEmitter* emitter = self->_attached_first;
  while (emitter) {
    emitter->on_settings_updated();
    emitter = emitter->_attached_next;
  }
}

// CodeHolder - Construction & Destruction
// =======================================

CodeHolder::CodeHolder(Span<uint8_t> static_arena_memory) noexcept
  : _environment(),
    _cpu_features{},
    _base_address(Globals::kNoBaseAddress),
    _logger(nullptr),
    _error_handler(nullptr),
    _arena(16u * 1024u, static_arena_memory),
    _attached_first(nullptr),
    _attached_last(nullptr),
    _fixups(nullptr),
    _unresolved_fixup_count(0),
    _text_section{},
    _address_table_section(nullptr) {}

CodeHolder::~CodeHolder() noexcept {
  if (is_initialized()) {
    CodeHolder_detach_emitters(this);
    CodeHolder_reset_sections(this, ResetPolicy::kHard);
  }
  else {
    Section_release_buffer(&_text_section);
  }
}

// CodeHolder - Initialization & Reset
// ===================================

Error CodeHolder::init(const Environment& environment, uint64_t base_address) noexcept {
  return init(environment, CpuFeatures{}, base_address);
}

Error CodeHolder::init(const Environment& environment, const CpuFeatures& cpu_features, uint64_t base_address) noexcept {
  // Cannot initialize if it's already initialized or the environment passed is invalid.
  if (ASMJIT_UNLIKELY(Support::bool_or(is_initialized(), !environment.is_initialized()))) {
    Error err = is_initialized() ? Error::kAlreadyInitialized : Error::kInvalidArgument;
    return make_error(err);
  }

  // If we are just initializing there should be no emitters attached.
  ASMJIT_ASSERT(_attached_first == nullptr);
  ASMJIT_ASSERT(_attached_last == nullptr);

  // Create a default section and insert it to the `_sections` array.
  Error err = CodeHolder_init_section_storage(this);
  if (ASMJIT_UNLIKELY(err != Error::kOk)) {
    _arena.reset();
    return make_error(Error::kOutOfMemory);
  }

  _environment = environment;
  _cpu_features = cpu_features;
  _base_address = base_address;

  CodeHolder_add_text_section(this);
  return Error::kOk;
}

Error CodeHolder::reinit() noexcept {
  // Cannot reinitialize if it's not initialized.
  if (ASMJIT_UNLIKELY(!is_initialized())) {
    return make_error(Error::kNotInitialized);
  }

  CodeHolder_reset_sections_and_containers(this, ResetPolicy::kSoft);

  // Create a default section and insert it to the `_sections` array.
  (void)CodeHolder_init_section_storage(this);
  CodeHolder_add_text_section(this);

  BaseEmitter* emitter = _attached_first;
  while (emitter) {
    emitter->on_reinit(*this);
    emitter = emitter->_attached_next;
  }

  return Error::kOk;
}

void CodeHolder::reset(ResetPolicy reset_policy) noexcept {
  if (is_initialized()) {
    CodeHolder_detach_emitters(this);
    CodeHolder_reset_env_and_attached_logger_and_eh(this);
    CodeHolder_reset_sections_and_containers(this, reset_policy);
  }
}

// CodeHolder - Attach / Detach
// ============================

Error CodeHolder::attach(BaseEmitter* emitter) noexcept {
  // Catch a possible misuse of the API.
  if (ASMJIT_UNLIKELY(!emitter)) {
    return make_error(Error::kInvalidArgument);
  }

  // Invalid emitter, this should not be possible.
  EmitterType type = emitter->emitter_type();
  if (ASMJIT_UNLIKELY(type == EmitterType::kNone || uint32_t(type) > uint32_t(EmitterType::kMaxValue))) {
    return make_error(Error::kInvalidState);
  }

  uint64_t arch_mask = emitter->_arch_mask;
  if (ASMJIT_UNLIKELY(!(arch_mask & (uint64_t(1) << uint32_t(arch()))))) {
    return make_error(Error::kInvalidArch);
  }

  // This is suspicious, but don't fail if `emitter` is already attached
  // to this code holder. This is not error, but it's not recommended.
  if (emitter->_code != nullptr) {
    if (emitter->_code == this) {
      return Error::kOk;
    }
    return make_error(Error::kInvalidState);
  }

  // Reserve the space now as we cannot fail after `on_attach()` succeeded.
  ASMJIT_PROPAGATE(emitter->on_attach(*this));

  // Make sure CodeHolder <-> BaseEmitter are connected.
  ASMJIT_ASSERT(emitter->_code == this);

  // Add `emitter` to a double linked-list.
  {
    BaseEmitter* last = _attached_last;

    emitter->_attached_prev = last;
    _attached_last = emitter;

    if (last) {
      last->_attached_next = emitter;
    }
    else {
      _attached_first = emitter;
    }
  }

  return Error::kOk;
}

Error CodeHolder::detach(BaseEmitter* emitter) noexcept {
  if (ASMJIT_UNLIKELY(!emitter)) {
    return make_error(Error::kInvalidArgument);
  }

  if (ASMJIT_UNLIKELY(emitter->_code != this)) {
    return make_error(Error::kInvalidState);
  }

  // NOTE: We always detach if we were asked to, if error happens during
  // `emitter->on_detach()` we just propagate it, but the BaseEmitter will
  // be detached.
  Error err = Error::kOk;
  if (!emitter->is_destroyed()) {
    err = emitter->on_detach(*this);
  }

  // Remove `emitter` from a double linked-list.
  {
    BaseEmitter* prev = emitter->_attached_prev;
    BaseEmitter* next = emitter->_attached_next;

    if (prev) { prev->_attached_next = next; } else { _attached_first = next; }
    if (next) { next->_attached_prev = prev; } else { _attached_last = prev; }

    emitter->_code = nullptr;
    emitter->_attached_prev = nullptr;
    emitter->_attached_next = nullptr;
  }

  return err;
}

// CodeHolder - Logging
// ====================

void CodeHolder::set_logger(Logger* logger) noexcept {
#ifndef ASMJIT_NO_LOGGING
  _logger = logger;
  CodeHolder_on_settings_updated(this);
#else
  Support::maybe_unused(logger);
#endif
}

// CodeHolder - Error Handling
// ===========================

void CodeHolder::set_error_handler(ErrorHandler* error_handler) noexcept {
  _error_handler = error_handler;
  CodeHolder_on_settings_updated(this);
}

// CodeHolder - Code Buffer
// ========================

static Error CodeHolder_reserve_internal(CodeHolder* self, CodeBuffer* cb, size_t n) noexcept {
  uint8_t* old_data = cb->_data;
  uint8_t* new_data;

  if (old_data && !cb->is_external()) {
    new_data = static_cast<uint8_t*>(::realloc(old_data, n));
  }
  else {
    new_data = static_cast<uint8_t*>(::malloc(n));
  }

  if (ASMJIT_UNLIKELY(!new_data)) {
    return make_error(Error::kOutOfMemory);
  }

  cb->_data = new_data;
  cb->_capacity = n;

  // Update pointers used by assemblers, if attached.
  BaseEmitter* emitter = self->_attached_first;
  while (emitter) {
    if (emitter->is_assembler()) {
      BaseAssembler* a = static_cast<BaseAssembler*>(emitter);
      if (&a->_section->_buffer == cb) {
        size_t offset = a->offset();

        a->_buffer_data = new_data;
        a->_buffer_end  = new_data + n;
        a->_buffer_ptr  = new_data + offset;
      }
    }
    emitter = emitter->_attached_next;
  }

  return Error::kOk;
}

Error CodeHolder::grow_buffer(CodeBuffer* cb, size_t n) noexcept {
  // The size of the section must be valid.
  size_t size = cb->size();
  if (ASMJIT_UNLIKELY(n > std::numeric_limits<uintptr_t>::max() - size)) {
    return make_error(Error::kOutOfMemory);
  }

  // We can now check if growing the buffer is really necessary. It's unlikely
  // that this function is called while there is still room for `n` bytes.
  size_t capacity = cb->capacity();
  size_t required = cb->size() + n;

  if (ASMJIT_UNLIKELY(required <= capacity)) {
    return Error::kOk;
  }

  if (cb->is_fixed()) {
    return make_error(Error::kTooLarge);
  }

  size_t kInitialCapacity = 8192u - Globals::kAllocOverhead;
  if (capacity < kInitialCapacity) {
    capacity = kInitialCapacity;
  }
  else {
    capacity += Globals::kAllocOverhead;
  }

  do {
    size_t old = capacity;
    size_t capacity_increase = capacity < Globals::kGrowThreshold ? capacity : Globals::kGrowThreshold;

    capacity += capacity_increase;

    // Overflow.
    if (ASMJIT_UNLIKELY(old > capacity)) {
      return make_error(Error::kOutOfMemory);
    }
  } while (capacity - Globals::kAllocOverhead < required);

  return CodeHolder_reserve_internal(this, cb, capacity - Globals::kAllocOverhead);
}

Error CodeHolder::reserve_buffer(CodeBuffer* cb, size_t n) noexcept {
  size_t capacity = cb->capacity();

  if (n <= capacity) {
    return Error::kOk;
  }

  if (cb->is_fixed()) {
    return make_error(Error::kTooLarge);
  }

  return CodeHolder_reserve_internal(this, cb, n);
}

// CodeHolder - Sections
// =====================

Error CodeHolder::new_section(Out<Section*> section_out, const char* name, size_t name_size, SectionFlags flags, uint32_t alignment, int32_t order) noexcept {
  *section_out = nullptr;

  if (ASMJIT_UNLIKELY(!Support::is_zero_or_power_of_2(alignment))) {
    return make_error(Error::kInvalidArgument);
  }

  if (name_size == SIZE_MAX) {
    name_size = strlen(name);
  }

  if (ASMJIT_UNLIKELY(name_size > Globals::kMaxSectionNameSize)) {
    return make_error(Error::kInvalidSectionName);
  }

  uint32_t section_id = _sections._size;
  if (ASMJIT_UNLIKELY(section_id == Globals::kInvalidId)) {
    return make_error(Error::kTooManySections);
  }

  ASMJIT_PROPAGATE(_sections.reserve_additional(_arena));
  ASMJIT_PROPAGATE(_sections_by_order.reserve_additional(_arena));

  Section* section = _arena.alloc_oneshot<Section>();
  if (ASMJIT_UNLIKELY(!section)) {
    return make_error(Error::kOutOfMemory);
  }

  if (alignment == 0u) {
    alignment = 1u;
  }

  Section_init_data(section, section_id, flags, alignment, order);
  Section_init_buffer(section);
  memcpy(section->_name.str, name, name_size);

  Section** insert_position = std::lower_bound(_sections_by_order.begin(), _sections_by_order.end(), section, [](const Section* a, const Section* b) {
    return std::make_tuple(a->order(), a->section_id()) < std::make_tuple(b->order(), b->section_id());
  });

  _sections.append_unchecked(section);
  _sections_by_order.insert_unchecked((size_t)(insert_position - _sections_by_order.data()), section);

  *section_out = section;
  return Error::kOk;
}

Section* CodeHolder::section_by_name(const char* name, size_t name_size) const noexcept {
  if (name_size == SIZE_MAX) {
    name_size = strlen(name);
  }

  // This could be also put in a hash-table similarly like we do with labels, however it's questionable as
  // the number of sections should be pretty low in general. Create an issue if this becomes a problem.
  if (name_size <= Globals::kMaxSectionNameSize) {
    for (Section* section : _sections) {
      if (memcmp(section->_name.str, name, name_size) == 0 && section->_name.str[name_size] == '\0') {
        return section;
      }
    }
  }

  return nullptr;
}

Section* CodeHolder::ensure_address_table_section() noexcept {
  if (_address_table_section) {
    return _address_table_section;
  }

  new_section(Out(_address_table_section),
             Section_address_table_name,
             sizeof(Section_address_table_name) - 1,
             SectionFlags::kNone,
             _environment.register_size(),
             std::numeric_limits<int32_t>::max());
  return _address_table_section;
}

Error CodeHolder::add_address_to_address_table(uint64_t address) noexcept {
  AddressTableEntry* entry = _address_table_entries.get(address);
  if (entry) {
    return Error::kOk;
  }

  Section* section = ensure_address_table_section();
  if (ASMJIT_UNLIKELY(!section)) {
    return make_error(Error::kOutOfMemory);
  }

  entry = _arena.new_oneshot<AddressTableEntry>(address);
  if (ASMJIT_UNLIKELY(!entry)) {
    return make_error(Error::kOutOfMemory);
  }

  _address_table_entries.insert(entry);
  section->_virtual_size += _environment.register_size();

  return Error::kOk;
}

// CodeHolder - Labels & Symbols
// =============================

//! Only used to lookup a label from `_named_labels`.
class LabelByName {
public:
  const char* _key {};
  uint32_t _key_size {};
  uint32_t _hash_code {};
  uint32_t _parent_id {};

  inline LabelByName(const char* key, size_t key_size, uint32_t hash_code, uint32_t parent_id) noexcept
    : _key(key),
      _key_size(uint32_t(key_size)),
      _hash_code(hash_code),
      _parent_id(parent_id) {}

  [[nodiscard]]
  inline uint32_t hash_code() const noexcept { return _hash_code; }

  [[nodiscard]]
  inline bool matches(const CodeHolder::NamedLabelExtraData* node) const noexcept {
    return Support::bool_and(node->extra_data._name_size == _key_size,
                             node->extra_data._parent_id == _parent_id) &&
           ::memcmp(node->extra_data.name(), _key, _key_size) == 0;
  }
};

// Returns a hash of `name` and fixes `name_size` if it's `SIZE_MAX`.
static uint32_t CodeHolder_hash_name_and_get_size(const char* name, size_t& name_size) noexcept {
  uint32_t hash_code = 0;
  if (name_size == SIZE_MAX) {
    size_t i = 0;
    for (;;) {
      uint8_t c = uint8_t(name[i]);
      if (!c) {
        break;
      }
      hash_code = Support::hash_char(hash_code, c);
      i++;
    }
    name_size = i;
  }
  else {
    for (size_t i = 0; i < name_size; i++) {
      uint8_t c = uint8_t(name[i]);
      if (ASMJIT_UNLIKELY(!c)) {
        name_size = i;
        break;
      }
      hash_code = Support::hash_char(hash_code, c);
    }
  }
  return hash_code;
}

Fixup* CodeHolder::new_fixup(LabelEntry& le, uint32_t section_id, size_t offset, intptr_t rel, const OffsetFormat& format) noexcept {
  // Cannot be bound if we are creating a link.
  ASMJIT_ASSERT(!le.is_bound());

  Fixup* link = _fixup_data_pool.alloc(_arena);
  if (ASMJIT_UNLIKELY(!link)) {
    return nullptr;
  }

  link->next = le._get_fixups();
  link->section_id = section_id;
  link->label_or_reloc_id = Globals::kInvalidId;
  link->offset = offset;
  link->rel = rel;
  link->format = format;

  le._set_fixups(link);
  _unresolved_fixup_count++;

  return link;
}

Error CodeHolder::new_label_id(Out<uint32_t> label_id_out) noexcept {
  uint32_t label_id = _label_entries._size;
  Error err = _label_entries.reserve_additional(_arena);

  if (ASMJIT_UNLIKELY(err != Error::kOk)) {
    label_id_out = Globals::kInvalidId;
    return err;
  }
  else {
    label_id_out = label_id;
    _label_entries.append_unchecked(LabelEntry{const_cast<LabelEntry::ExtraData*>(&CodeHolder_shared_label_extra_data), uint64_t(0)});
    return Error::kOk;
  }
}

Error CodeHolder::new_named_label_id(Out<uint32_t> label_id_out, const char* name, size_t name_size, LabelType type, uint32_t parent_id) noexcept {
  uint32_t label_id = _label_entries._size;
  uint32_t hash_code = CodeHolder_hash_name_and_get_size(name, name_size);

  label_id_out = Globals::kInvalidId;
  ASMJIT_PROPAGATE(_label_entries.reserve_additional(_arena));

  if (name_size == 0) {
    if (type != LabelType::kAnonymous) {
      return make_error(Error::kInvalidLabelName);
    }

    label_id_out = label_id;
    _label_entries.append_unchecked(LabelEntry{const_cast<LabelEntry::ExtraData*>(&CodeHolder_shared_label_extra_data), uint64_t(0)});
    return Error::kOk;
  }

  if (ASMJIT_UNLIKELY(name_size > Globals::kMaxLabelNameSize)) {
    return make_error(Error::kLabelNameTooLong);
  }

  size_t extra_data_size = sizeof(LabelEntry::ExtraData) + name_size + 1u;

  switch (type) {
    case LabelType::kAnonymous: {
      // Anonymous labels cannot have a parent (or more specifically, parent is useless here).
      if (ASMJIT_UNLIKELY(parent_id != Globals::kInvalidId)) {
        return make_error(Error::kInvalidParentLabel);
      }

      LabelEntry::ExtraData* extra_data = _arena.alloc_oneshot<LabelEntry::ExtraData>(Arena::aligned_size(extra_data_size));
      if (ASMJIT_UNLIKELY(!extra_data)) {
        return make_error(Error::kOutOfMemory);
      }

      char* name_ptr = reinterpret_cast<char*>(extra_data) + sizeof(LabelEntry::ExtraData);
      extra_data->_section_id = Globals::kInvalidId;
      extra_data->_internal_label_type = type;
      extra_data->_internal_label_flags = LabelFlags::kHasOwnExtraData | LabelFlags::kHasName;
      extra_data->_internal_uint16_data = 0;
      extra_data->_parent_id = Globals::kInvalidId;
      extra_data->_name_size = uint32_t(name_size);
      memcpy(name_ptr, name, name_size);
      name_ptr[name_size] = '\0';

      label_id_out = label_id;
      _label_entries.append_unchecked(LabelEntry{extra_data, uint64_t(0)});
      return Error::kOk;
    }

    case LabelType::kLocal: {
      if (ASMJIT_UNLIKELY(parent_id >= _label_entries.size())) {
        return make_error(Error::kInvalidParentLabel);
      }

      hash_code ^= parent_id;
      break;
    }

    case LabelType::kGlobal:
    case LabelType::kExternal: {
      if (ASMJIT_UNLIKELY(parent_id != Globals::kInvalidId)) {
        return make_error(Error::kInvalidParentLabel);
      }
      break;
    }

    default: {
      return make_error(Error::kInvalidArgument);
    }
  }

  extra_data_size += sizeof(ArenaHashNode);

  // Don't allow to insert duplicates. Local labels allow duplicates that have different ids, however, this is
  // already accomplished by having a different hashes between the same label names having different parent labels.
  NamedLabelExtraData* named_node = _named_labels.get(LabelByName(name, name_size, hash_code, parent_id));
  if (ASMJIT_UNLIKELY(named_node)) {
    return make_error(Error::kLabelAlreadyDefined);
  }

  named_node = _arena.alloc_oneshot<NamedLabelExtraData>(Arena::aligned_size(extra_data_size));
  if (ASMJIT_UNLIKELY(!named_node)) {
    return make_error(Error::kOutOfMemory);
  }

  LabelFlags label_flags =
    (parent_id == Globals::kInvalidId)
      ? LabelFlags::kHasOwnExtraData | LabelFlags::kHasName
      : LabelFlags::kHasOwnExtraData | LabelFlags::kHasName | LabelFlags::kHasParent;

  named_node->_hash_next = nullptr;
  named_node->_hash_code = hash_code;
  named_node->_custom_data = label_id;
  named_node->extra_data._section_id = Globals::kInvalidId;
  named_node->extra_data._internal_label_type = type;
  named_node->extra_data._internal_label_flags = label_flags;
  named_node->extra_data._internal_uint16_data = 0;
  named_node->extra_data._parent_id = parent_id;
  named_node->extra_data._name_size = uint32_t(name_size);

  char* name_ptr = reinterpret_cast<char*>(&named_node->extra_data) + sizeof(LabelEntry::ExtraData);
  memcpy(name_ptr, name, name_size);
  name_ptr[name_size] = '\0';

  label_id_out = label_id;
  _label_entries.append_unchecked(LabelEntry{&named_node->extra_data, uint64_t(0)});
  _named_labels.insert(_arena, named_node);

  return Error::kOk;
}

uint32_t CodeHolder::label_id_by_name(const char* name, size_t name_size, uint32_t parent_id) noexcept {
  uint32_t hash_code = CodeHolder_hash_name_and_get_size(name, name_size);
  if (ASMJIT_UNLIKELY(!name_size)) {
    return 0;
  }

  if (parent_id != Globals::kInvalidId) {
    hash_code ^= parent_id;
  }

  NamedLabelExtraData* named_node = _named_labels.get(LabelByName(name, name_size, hash_code, parent_id));
  return named_node ? named_node->label_id() : uint32_t(Globals::kInvalidId);
}


ASMJIT_API Error CodeHolder::resolve_cross_section_fixups() noexcept {
  if (!has_unresolved_fixups()) {
    return Error::kOk;
  }

  Error err = Error::kOk;
  ResolveFixupIterator it(&_fixups);

  while (it.is_valid()) {
    Fixup* fixup = it.fixup();
    LabelEntry& le = label_entry_of(fixup->label_or_reloc_id);

    Support::FastUInt8 of{};
    Section* to_section = _sections[le.section_id()];
    uint64_t to_offset = Support::add_overflow(to_section->offset(), le.offset(), &of);

    Section* from_section = section_by_id(fixup->section_id);
    size_t fixup_offset = fixup->offset;

    CodeBuffer& buf = from_section->buffer();
    ASMJIT_ASSERT(fixup_offset < buf.size());

    // Calculate the offset relative to the start of the virtual base.
    uint64_t from_offset = Support::add_overflow<uint64_t>(from_section->offset(), fixup_offset, &of);
    int64_t displacement = int64_t(to_offset - from_offset + uint64_t(int64_t(fixup->rel)));

    if (ASMJIT_UNLIKELY(of)) {
      err = make_error(Error::kInvalidDisplacement);
    }
    else {
      ASMJIT_ASSERT(size_t(fixup_offset) < buf.size());
      ASMJIT_ASSERT(buf.size() - size_t(fixup_offset) >= fixup->format.value_size());

      // Overwrite a real displacement in the CodeBuffer.
      if (CodeWriterUtils::write_offset(buf._data + fixup_offset, displacement, fixup->format)) {
        it.resolve_and_next(this);
        continue;
      }
    }

    it.next();
  }

  _unresolved_fixup_count -= it.resolved_count();
  return err;
}

ASMJIT_API Error CodeHolder::bind_label(const Label& label, uint32_t to_section_id, uint64_t to_offset) noexcept {
  uint32_t label_id = label.id();

  if (ASMJIT_UNLIKELY(label_id >= _label_entries.size())) {
    return make_error(Error::kInvalidLabel);
  }

  if (ASMJIT_UNLIKELY(to_section_id >= _sections.size())) {
    return make_error(Error::kInvalidSection);
  }

  LabelEntry& le = _label_entries[label_id];

  // Label can be bound only once.
  if (ASMJIT_UNLIKELY(le.is_bound())) {
    return make_error(Error::kLabelAlreadyBound);
  }

  Section* section = _sections[to_section_id];
  CodeBuffer& buf = section->buffer();

  // Bind the label - this either assigns a section to LabelEntry's `_object_data` or `_section_id` in own `ExtraData`.
  // This is basically how this works - when the ExtraData is shared, we replace it by section as the section header
  // is compatible with ExtraData header, and when the LabelEntry has its own ExtraData, the section identifier must
  // be assigned.
  if (le._has_own_extra_data()) {
    le._own_extra_data()->_section_id = to_section_id;
  }
  else {
    le._object_data = section;
  }

  // It must be in this order as _offset_or_fixups as basically a union.
  Fixup* label_fixups = le._get_fixups();
  le._offset_or_fixups = to_offset;

  if (!label_fixups) {
    return Error::kOk;
  }

  // Fix all fixups of this label we have collected so far if they are within the same
  // section. We ignore any cross-section fixups as these have to be fixed later.
  Error err = Error::kOk;

  ResolveFixupIterator it(&label_fixups);
  ASMJIT_ASSERT(it.is_valid());

  do {
    Fixup* fixup = it.fixup();

    uint32_t reloc_id = fixup->label_or_reloc_id;
    uint32_t from_section_id = fixup->section_id;
    size_t from_offset = fixup->offset;

    if (reloc_id != Globals::kInvalidId) {
      // Adjust the relocation payload.
      RelocEntry* re = _relocations[reloc_id];
      re->_payload += to_offset;
      re->_target_section_id = to_section_id;
    }
    else if (from_section_id != to_section_id) {
      fixup->label_or_reloc_id = label_id;
      it.next();
      continue;
    }
    else {
      ASMJIT_ASSERT(from_offset < buf.size());
      int64_t displacement = int64_t(to_offset - uint64_t(from_offset) + uint64_t(int64_t(fixup->rel)));

      // Size of the value we are going to patch.
      ASMJIT_ASSERT(buf.size() - size_t(from_offset) >= fixup->format.region_size());

      // Overwrite a real displacement in the CodeBuffer.
      if (!CodeWriterUtils::write_offset(buf._data + from_offset, displacement, fixup->format)) {
        err = make_error(Error::kInvalidDisplacement);
        fixup->label_or_reloc_id = label_id;
        it.next();
        continue;
      }
    }

    it.resolve_and_next(this);
  } while (it.is_valid());

  if (it.unresolved_count()) {
    *it._prev = _fixups;
    _fixups = label_fixups;
  }

  _unresolved_fixup_count -= it.resolved_count();
  return err;
}

// CodeHolder - Relocations
// ========================

Error CodeHolder::new_reloc_entry(Out<RelocEntry*> dst, RelocType reloc_type) noexcept {
  ASMJIT_PROPAGATE(_relocations.reserve_additional(_arena));

  uint32_t reloc_id = _relocations._size;
  if (ASMJIT_UNLIKELY(reloc_id == Globals::kInvalidId)) {
    return make_error(Error::kTooManyRelocations);
  }

  RelocEntry* re = _arena.alloc_oneshot<RelocEntry>();
  if (ASMJIT_UNLIKELY(!re)) {
    return make_error(Error::kOutOfMemory);
  }

  re->_id = reloc_id;
  re->_reloc_type = reloc_type;
  re->_format = OffsetFormat{};
  re->_source_section_id = Globals::kInvalidId;
  re->_target_section_id = Globals::kInvalidId;
  re->_source_offset = 0;
  re->_payload = 0;
  _relocations.append_unchecked(re);

  dst = re;
  return Error::kOk;
}

// CodeHolder - Expression Evaluation
// ==================================

static Error CodeHolder_evaluate_expression(CodeHolder* self, Expression* exp, uint64_t* out) noexcept {
  uint64_t value[2];
  for (size_t i = 0; i < 2; i++) {
    uint64_t v;
    switch (exp->value_type[i]) {
      case ExpressionValueType::kNone: {
        v = 0;
        break;
      }

      case ExpressionValueType::kConstant: {
        v = exp->value[i].constant;
        break;
      }

      case ExpressionValueType::kLabel: {
        uint32_t label_id = exp->value[i].label_id;
        if (ASMJIT_UNLIKELY(label_id >= self->label_count())) {
          return make_error(Error::kInvalidLabel);
        }

        LabelEntry& le = self->_label_entries[label_id];
        if (!le.is_bound()) {
          return make_error(Error::kExpressionLabelNotBound);
        }

        v = self->_sections[le.section_id()]->offset() + le.offset();
        break;
      }

      case ExpressionValueType::kExpression: {
        Expression* nested = exp->value[i].expression;
        ASMJIT_PROPAGATE(CodeHolder_evaluate_expression(self, nested, &v));
        break;
      }

      default:
        return make_error(Error::kInvalidState);
    }

    value[i] = v;
  }

  uint64_t result;
  uint64_t& a = value[0];
  uint64_t& b = value[1];

  switch (exp->op_type) {
    case ExpressionOpType::kAdd:
      result = a + b;
      break;

    case ExpressionOpType::kSub:
      result = a - b;
      break;

    case ExpressionOpType::kMul:
      result = a * b;
      break;

    case ExpressionOpType::kSll:
      result = (b > 63) ? uint64_t(0) : uint64_t(a << b);
      break;

    case ExpressionOpType::kSrl:
      result = (b > 63) ? uint64_t(0) : uint64_t(a >> b);
      break;

    case ExpressionOpType::kSra:
      result = Support::sar(a, Support::min<uint64_t>(b, 63));
      break;

    default:
      return make_error(Error::kInvalidState);
  }

  *out = result;
  return Error::kOk;
}

// CodeHolder - Utilities
// ======================

Error CodeHolder::flatten() noexcept {
  uint64_t offset = 0;
  for (Section* section : _sections_by_order) {
    uint64_t real_size = section->real_size();
    if (real_size) {
      uint64_t aligned_offset = Support::align_up(offset, section->alignment());
      if (ASMJIT_UNLIKELY(aligned_offset < offset)) {
        return make_error(Error::kTooLarge);
      }

      Support::FastUInt8 of = 0;
      offset = Support::add_overflow(aligned_offset, real_size, &of);

      if (ASMJIT_UNLIKELY(of)) {
        return make_error(Error::kTooLarge);
      }
    }
  }

  // Now we know that we can assign offsets of all sections properly.
  Section* prev = nullptr;
  offset = 0;
  for (Section* section : _sections_by_order) {
    uint64_t real_size = section->real_size();
    if (real_size) {
      offset = Support::align_up(offset, section->alignment());
    }
    section->_offset = offset;

    // Make sure the previous section extends a bit to cover the alignment.
    if (prev) {
      prev->_virtual_size = offset - prev->_offset;
    }

    prev = section;
    offset += real_size;
  }

  return Error::kOk;
}

size_t CodeHolder::code_size() const noexcept {
  Support::FastUInt8 of = 0;
  uint64_t offset = 0;

  for (Section* section : _sections_by_order) {
    uint64_t real_size = section->real_size();

    if (real_size) {
      uint64_t aligned_offset = Support::align_up(offset, section->alignment());
      ASMJIT_ASSERT(aligned_offset >= offset);
      offset = Support::add_overflow(aligned_offset, real_size, &of);
    }
  }

  if ((sizeof(uint64_t) > sizeof(size_t) && offset > uint64_t(SIZE_MAX)) || of) {
    return SIZE_MAX;
  }

  return size_t(offset);
}

Error CodeHolder::relocate_to_base(uint64_t base_address, RelocationSummary* summary_out) noexcept {
  // Make sure `summary_out` pointer is always valid as we want to fill it.
  RelocationSummary summary_tmp;
  if (summary_out == nullptr) {
    summary_out = &summary_tmp;
  }

  // Fill `summary_out` defaults.
  summary_out->code_size_reduction = 0u;

  // Base address must be provided.
  if (ASMJIT_UNLIKELY(base_address == Globals::kNoBaseAddress)) {
    return make_error(Error::kInvalidArgument);
  }

  _base_address = base_address;
  uint32_t address_size = _environment.register_size();

  Section* address_table_section = _address_table_section;
  uint32_t address_table_entry_size = 0;
  uint8_t* address_table_entry_data = nullptr;

  if (address_table_section) {
    ASMJIT_PROPAGATE(reserve_buffer(&address_table_section->_buffer, size_t(address_table_section->virtual_size())));
    address_table_entry_data = address_table_section->_buffer.data();
  }

  // Relocate all recorded locations.
  for (const RelocEntry* re : _relocations) {
    // Possibly deleted or optimized-out entry.
    if (re->reloc_type() == RelocType::kNone) {
      continue;
    }

    Section* source_section = section_by_id(re->source_section_id());
    Section* target_section = nullptr;

    if (re->target_section_id() != Globals::kInvalidId) {
      target_section = section_by_id(re->target_section_id());
    }

    uint64_t value = re->payload();
    uint64_t section_offset = source_section->offset();
    uint64_t source_offset = re->source_offset();

    // Make sure that the `RelocEntry` doesn't go out of bounds.
    size_t region_size = re->format().region_size();
    if (ASMJIT_UNLIKELY(re->source_offset() >= source_section->buffer_size() ||
                        source_section->buffer_size() - size_t(re->source_offset()) < region_size)) {
      return make_error(Error::kInvalidRelocEntry);
    }

    uint8_t* buffer = source_section->data();

    switch (re->reloc_type()) {
      case RelocType::kExpression: {
        Expression* expression = (Expression*)(uintptr_t(value));
        ASMJIT_PROPAGATE(CodeHolder_evaluate_expression(this, expression, &value));
        break;
      }

      case RelocType::kAbsToAbs: {
        break;
      }

      case RelocType::kRelToAbs: {
        // Value is currently a relative offset from the start of its section.
        // We have to convert it to an absolute offset (including base address).
        if (ASMJIT_UNLIKELY(!target_section)) {
          return make_error(Error::kInvalidRelocEntry);
        }

        //value += base_address + section_offset + source_offset + region_size;
        value += base_address + target_section->offset();
        break;
      }

      case RelocType::kAbsToRel: {
        value -= base_address + section_offset + source_offset + region_size;

        // Sign extend as we are not interested in the high 32-bit word in a 32-bit address space.
        if (address_size <= 4) {
          value = uint64_t(int64_t(int32_t(value & 0xFFFFFFFFu)));
        }
        else if (!Support::is_int_n<32>(int64_t(value))) {
          return make_error(Error::kRelocOffsetOutOfRange);
        }

        break;
      }

      case RelocType::kX64AddressEntry: {
        size_t value_offset = size_t(re->source_offset()) + re->format().value_offset();
        if (re->format().value_size() != 4 || value_offset < 2) {
          return make_error(Error::kInvalidRelocEntry);
        }

        // First try whether a relative 32-bit displacement would work.
        value -= base_address + section_offset + source_offset + region_size;
        if (!Support::is_int_n<32>(int64_t(value))) {
          // Relative 32-bit displacement is not possible, use '.addrtab' section.
          AddressTableEntry* at_entry = _address_table_entries.get(re->payload());
          if (ASMJIT_UNLIKELY(!at_entry)) {
            return make_error(Error::kInvalidRelocEntry);
          }

          // Cannot be null as we have just matched the `AddressTableEntry`.
          ASMJIT_ASSERT(address_table_section != nullptr);

          if (!at_entry->has_assigned_slot()) {
            at_entry->_slot = address_table_entry_size++;
          }

          size_t at_entry_index = size_t(at_entry->slot()) * address_size;
          uint64_t addr_src = section_offset + source_offset + region_size;
          uint64_t addr_dst = address_table_section->offset() + uint64_t(at_entry_index);

          value = addr_dst - addr_src;
          if (!Support::is_int_n<32>(int64_t(value))) {
            return make_error(Error::kRelocOffsetOutOfRange);
          }

          // Bytes that replace [REX, OPCODE] bytes.
          uint32_t byte0 = 0xFF;
          uint32_t byte1 = buffer[value_offset - 1];

          if (byte1 == 0xE8) {
            // Patch CALL/MOD byte to FF /2 (-> 0x15).
            byte1 = x86_encode_mod(0, 2, 5);
          }
          else if (byte1 == 0xE9) {
            // Patch JMP/MOD byte to FF /4 (-> 0x25).
            byte1 = x86_encode_mod(0, 4, 5);
          }
          else {
            return make_error(Error::kInvalidRelocEntry);
          }

          // Patch `jmp/call` instruction.
          buffer[value_offset - 2] = uint8_t(byte0);
          buffer[value_offset - 1] = uint8_t(byte1);

          Support::storeu_u64_le(address_table_entry_data + at_entry_index, re->payload());
        }
        break;
      }

      default:
        return make_error(Error::kInvalidRelocEntry);
    }

    if (!CodeWriterUtils::write_offset(buffer + re->source_offset(), int64_t(value), re->format())) {
      return make_error(Error::kInvalidRelocEntry);
    }
  }

  // Fixup the virtual size of the address table if it's the last section.
  if (_sections_by_order.last() == address_table_section) {
    ASMJIT_ASSERT(address_table_section != nullptr);

    size_t reserved_size = size_t(address_table_section->_virtual_size);
    size_t address_table_size = address_table_entry_size * address_size;

    address_table_section->_buffer._size = address_table_size;
    address_table_section->_virtual_size = address_table_size;

    ASMJIT_ASSERT(reserved_size >= address_table_size);
    size_t code_size_reduction = reserved_size - address_table_size;

    summary_out->code_size_reduction = code_size_reduction;
  }

  return Error::kOk;
}

Error CodeHolder::copy_section_data(void* dst, size_t dst_size, uint32_t section_id, CopySectionFlags copy_flags) noexcept {
  if (ASMJIT_UNLIKELY(!is_section_valid(section_id))) {
    return make_error(Error::kInvalidSection);
  }

  Section* section = section_by_id(section_id);
  size_t buffer_size = section->buffer_size();

  if (ASMJIT_UNLIKELY(dst_size < buffer_size)) {
    return make_error(Error::kInvalidArgument);
  }

  memcpy(dst, section->data(), buffer_size);

  if (buffer_size < dst_size && Support::test(copy_flags, CopySectionFlags::kPadSectionBuffer)) {
    size_t padding_size = dst_size - buffer_size;
    memset(static_cast<uint8_t*>(dst) + buffer_size, 0, padding_size);
  }

  return Error::kOk;
}

Error CodeHolder::copy_flattened_data(void* dst, size_t dst_size, CopySectionFlags copy_flags) noexcept {
  size_t end = 0;
  for (Section* section : _sections_by_order) {
    if (section->offset() > dst_size) {
      return make_error(Error::kInvalidArgument);
    }

    size_t buffer_size = section->buffer_size();
    size_t offset = size_t(section->offset());

    if (ASMJIT_UNLIKELY(dst_size - offset < buffer_size)) {
      return make_error(Error::kInvalidArgument);
    }

    uint8_t* dst_target = static_cast<uint8_t*>(dst) + offset;
    size_t padding_size = 0;
    memcpy(dst_target, section->data(), buffer_size);

    if (Support::test(copy_flags, CopySectionFlags::kPadSectionBuffer) && buffer_size < section->virtual_size()) {
      padding_size = Support::min<size_t>(dst_size - offset, size_t(section->virtual_size())) - buffer_size;
      memset(dst_target + buffer_size, 0, padding_size);
    }

    end = Support::max(end, offset + buffer_size + padding_size);
  }

  if (end < dst_size && Support::test(copy_flags, CopySectionFlags::kPadTargetBuffer)) {
    memset(static_cast<uint8_t*>(dst) + end, 0, dst_size - end);
  }

  return Error::kOk;
}

// CodeHolder - Tests
// ==================

#if defined(ASMJIT_TEST)
UNIT(code_holder) {
  CodeHolder code;

  INFO("Verifying CodeHolder::init()");
  Environment env;
  env.init(Arch::kX86);

  code.init(env);
  EXPECT_EQ(code.arch(), Arch::kX86);

  INFO("Verifying named labels");
  uint32_t dummy_id;
  uint32_t label_id1;
  uint32_t label_id2;

  // Anonymous labels can have no-name (this is basically like calling `code.new_label_id()`).
  EXPECT_EQ(code.new_named_label_id(Out(dummy_id), "", SIZE_MAX, LabelType::kAnonymous), Error::kOk);

  // Global labels must have a name - not providing one is an error.
  EXPECT_EQ(code.new_named_label_id(Out(dummy_id), "", SIZE_MAX, LabelType::kGlobal), Error::kInvalidLabelName);

  // A name of a global label cannot repeat.
  EXPECT_EQ(code.new_named_label_id(Out(label_id1), "NamedLabel1", SIZE_MAX, LabelType::kGlobal), Error::kOk);
  EXPECT_EQ(code.new_named_label_id(Out(dummy_id), "NamedLabel1", SIZE_MAX, LabelType::kGlobal), Error::kLabelAlreadyDefined);
  EXPECT_TRUE(code.is_label_valid(label_id1));
  EXPECT_EQ(code.label_entry_of(label_id1).name_size(), 11u);
  EXPECT_EQ(strcmp(code.label_entry_of(label_id1).name(), "NamedLabel1"), 0);
  EXPECT_EQ(code.label_id_by_name("NamedLabel1"), label_id1);

  EXPECT_EQ(code.new_named_label_id(Out(label_id2), "NamedLabel2", SIZE_MAX, LabelType::kGlobal), Error::kOk);
  EXPECT_EQ(code.new_named_label_id(Out(dummy_id), "NamedLabel2", SIZE_MAX, LabelType::kGlobal), Error::kLabelAlreadyDefined);
  EXPECT_TRUE(code.is_label_valid(label_id2));
  EXPECT_EQ(code.label_entry_of(label_id2).name_size(), 11u);
  EXPECT_EQ(strcmp(code.label_entry_of(label_id2).name(), "NamedLabel2"), 0);
  EXPECT_EQ(code.label_id_by_name("NamedLabel2"), label_id2);

  INFO("Verifying section ordering");
  Section* section1;
  EXPECT_EQ(code.new_section(Out(section1), "high-priority", SIZE_MAX, SectionFlags::kNone, 1, -1), Error::kOk);
  EXPECT_EQ(code.sections()[1], section1);
  EXPECT_EQ(code.sections_by_order()[0], section1);

  Section* section0;
  EXPECT_EQ(code.new_section(Out(section0), "higher-priority", SIZE_MAX, SectionFlags::kNone, 1, -2), Error::kOk);
  EXPECT_EQ(code.sections()[2], section0);
  EXPECT_EQ(code.sections_by_order()[0], section0);
  EXPECT_EQ(code.sections_by_order()[1], section1);

  Section* section3;
  EXPECT_EQ(code.new_section(Out(section3), "low-priority", SIZE_MAX, SectionFlags::kNone, 1, 2), Error::kOk);
  EXPECT_EQ(code.sections()[3], section3);
  EXPECT_EQ(code.sections_by_order()[3], section3);
}
#endif

ASMJIT_END_NAMESPACE
