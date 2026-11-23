// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_SUPPORT_ARENAHASH_H_INCLUDED
#define ASMJIT_SUPPORT_ARENAHASH_H_INCLUDED

#include <asmjit/support/arena.h>

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_support
//! \{

//! Node used by \ref ArenaHash template.
//!
//! You must provide function `bool eq(const Key& key)` in order to make `ArenaHash::get()` working.
class ArenaHashNode {
public:
  ASMJIT_NONCOPYABLE(ArenaHashNode)

  ASMJIT_INLINE_NODEBUG explicit ArenaHashNode(uint32_t hash_code = 0u, uint32_t custom_data = 0u) noexcept
    : _hash_code(hash_code),
      _custom_data(custom_data) {}

  //! Next node in the chain, null if it terminates the chain.
  ArenaHashNode* _hash_next {};
  //! Precalculated hash-code of key.
  uint32_t _hash_code {};
  //! Padding, can be reused by any Node that inherits `ArenaHashNode`.
  uint32_t _custom_data {};
};

//! Base class used by \ref ArenaHash template
class ArenaHashBase {
public:
  ASMJIT_NONCOPYABLE(ArenaHashBase)

  //! Buckets data.
  ArenaHashNode** _data;
  //! Count of records inserted into the hash table.
  size_t _size;
  //! Count of hash buckets.
  uint32_t _buckets_count;
  //! When buckets array should grow (only checked after insertion).
  uint32_t _buckets_grow;
  //! Reciprocal value of `_buckets_count`.
  uint32_t _rcp_value;
  //! How many bits to shift right when hash is multiplied with `_rcp_value`.
  uint8_t _rcp_shift;
  //! Prime value index in internal prime array.
  uint8_t _prime_index;

  //! Embedded data, used by empty hash tables.
  ArenaHashNode* _embedded[1];

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG ArenaHashBase() noexcept {
    reset();
  }

  inline ArenaHashBase(ArenaHashBase&& other) noexcept {
    _data = other._data;
    _size = other._size;
    _buckets_count = other._buckets_count;
    _buckets_grow = other._buckets_grow;
    _rcp_value = other._rcp_value;
    _rcp_shift = other._rcp_shift;
    _prime_index = other._prime_index;
    _embedded[0] = other._embedded[0];

    if (_data == other._embedded) {
      _data = _embedded;
    }
  }

  inline void reset() noexcept {
    _data = _embedded;
    _size = 0;
    _buckets_count = 1;
    _buckets_grow = 1;
    _rcp_value = 1;
    _rcp_shift = 0;
    _prime_index = 0;
    _embedded[0] = nullptr;
  }

  inline void release(Arena& arena) noexcept {
    ArenaHashNode** old_data = _data;
    if (old_data != _embedded) {
      arena.free_reusable(old_data, _buckets_count * sizeof(ArenaHashNode*));
    }
    reset();
  }

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _size == 0; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t size() const noexcept { return _size; }

  //! \}

  //! \name Utilities
  //! \{

  inline void _swap(ArenaHashBase& other) noexcept {
    std::swap(_data, other._data);
    std::swap(_size, other._size);
    std::swap(_buckets_count, other._buckets_count);
    std::swap(_buckets_grow, other._buckets_grow);
    std::swap(_rcp_value, other._rcp_value);
    std::swap(_rcp_shift, other._rcp_shift);
    std::swap(_prime_index, other._prime_index);
    std::swap(_embedded[0], other._embedded[0]);

    if (_data == other._embedded) {
      _data = _embedded;
    }

    if (other._data == _embedded) {
      other._data = other._embedded;
    }
  }

  //! \cond INTERNAL
  inline uint32_t _calc_mod(uint32_t hash) const noexcept {
    uint32_t x = uint32_t((uint64_t(hash) * _rcp_value) >> _rcp_shift);
    return hash - x * _buckets_count;
  }

  ASMJIT_API void _rehash(Arena& arena, uint32_t prime_index) noexcept;
  ASMJIT_API ArenaHashNode* _insert(Arena& arena, ArenaHashNode* node) noexcept;
  ASMJIT_API ArenaHashNode* _remove(Arena& arena, ArenaHashNode* node) noexcept;
  //! \endcond

  //! \}
};

//! Low-level hash table specialized for storing string keys and POD values.
//!
//! This hash table allows duplicates to be inserted (the API is so low level that it's up to you if you allow it or
//! not, as you should first `get()` the node and then modify it or insert a new node by using `insert()`, depending
//! on the intention).
template<typename NodeT>
class ArenaHash : public ArenaHashBase {
public:
  ASMJIT_NONCOPYABLE(ArenaHash)

  using Node = NodeT;

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG ArenaHash() noexcept
    : ArenaHashBase() {}

  ASMJIT_INLINE_NODEBUG ArenaHash(ArenaHash&& other) noexcept
    : ArenaHash(other) {}

  //! \}

  //! \name Utilities
  //! \{

  ASMJIT_INLINE_NODEBUG void swap(ArenaHash& other) noexcept { ArenaHashBase::_swap(other); }

  template<typename KeyT>
  [[nodiscard]]
  inline NodeT* get(const KeyT& key) const noexcept {
    uint32_t hash_mod = _calc_mod(key.hash_code());
    NodeT* node = static_cast<NodeT*>(_data[hash_mod]);

    while (node && !key.matches(node)) {
      node = static_cast<NodeT*>(node->_hash_next);
    }
    return node;
  }

  ASMJIT_INLINE_NODEBUG NodeT* insert(Arena& arena, NodeT* node) noexcept { return static_cast<NodeT*>(_insert(arena, node)); }
  ASMJIT_INLINE_NODEBUG NodeT* remove(Arena& arena, NodeT* node) noexcept { return static_cast<NodeT*>(_remove(arena, node)); }

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_SUPPORT_ARENAHASH_H_INCLUDED
