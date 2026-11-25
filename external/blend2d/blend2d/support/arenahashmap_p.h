// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SUPPORT_ARENAHASHMAP_P_H_INCLUDED
#define BLEND2D_SUPPORT_ARENAHASHMAP_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/support/arenaallocator_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

//! \name Arena Allocated Hash Map
//! \{

//! Node used by `ArenaHash<>` template.
//!
//! You must provide function `bool eq(const Key& key)` in order to make
//! `ArenaHash::get()` working.
class ArenaHashMapNode {
public:
  BL_NONCOPYABLE(ArenaHashMapNode)

  //! Next node in the chain, null if it terminates the chain.
  ArenaHashMapNode* _hash_next;
  //! Precalculated hash-code of key.
  uint32_t _hash_code;
  //! Padding, can be reused by any Node that inherits `ArenaHashMapNode`.
  union {
    uint32_t _custom_data;
    uint16_t _customDataU16[2];
    uint8_t _customDataU8[4];
  };

  BL_INLINE ArenaHashMapNode(uint32_t hash_code = 0, uint32_t custom_data = 0) noexcept
    : _hash_next(nullptr),
      _hash_code(hash_code),
      _custom_data(custom_data) {}
};

//! Base class used by `ArenaHashMap<>` template to share the common functionality.
class ArenaHashMapBase {
public:
  BL_NONCOPYABLE(ArenaHashMapBase)

  // NOTE: There must be at least 2 embedded buckets, otherwise we wouldn't be
  // able to implement division as multiplication and shift in 32-bit mode the
  // way we want. Additionally, if we know that there is always a valid buckets
  // array we won't have to perform null checks.
  enum NullConstants : uint32_t {
    kNullCount = 2,
    kNullGrow = 1,
    kNullRcpValue = 2147483648u, // 2^31
    kNullRcpShift = BL_TARGET_ARCH_BITS >= 64 ? 32 : 0
  };

  ArenaAllocator* _allocator {};
  //! Buckets data.
  ArenaHashMapNode** _data {};
  //! Count of records inserted into the hash table.
  size_t _size {};
  //! Count of hash buckets.
  uint32_t _bucket_count = kNullCount;
  //! When buckets array should grow (only checked after insertion).
  uint32_t _bucket_grow = kNullGrow;
  //! Reciprocal value of `_bucket_count`.
  uint32_t _rcp_value = kNullRcpValue;
  //! How many bits to shift right when hash is multiplied with `_rcp_value`.
  uint8_t _rcp_shift = kNullRcpShift;
  //! Prime value index in internal prime array.
  uint8_t _prime_index = 0;
  //! Padding...
  uint8_t _reserved[2] {};
  //! Embedded and initial hash data.
  ArenaHashMapNode* _embedded[kNullCount] {};

  //! \name Construction & Destruction
  //! \{

  BL_INLINE ArenaHashMapBase(ArenaAllocator* allocator) noexcept
    : _allocator(allocator),
      _data(_embedded) {}

  BL_INLINE ArenaHashMapBase(ArenaHashMapBase&& other) noexcept
    : _allocator(other._allocator),
      _data(other._data),
      _size(other._size),
      _bucket_count(other._bucket_count),
      _bucket_grow(other._bucket_grow),
      _rcp_value(other._rcp_value),
      _rcp_shift(other._rcp_shift),
      _prime_index(other._prime_index) {
    other._data = nullptr;
    other._size = 0;
    other._bucket_count = kNullCount;
    other._bucket_grow = kNullGrow;
    other._rcp_value = kNullRcpValue;
    other._rcp_shift = kNullRcpShift;
    other._prime_index = 0;

    memcpy(_embedded, other._embedded, kNullCount * sizeof(ArenaHashMapNode*));
    memset(other._embedded, 0, kNullCount * sizeof(ArenaHashMapNode*));
  }

  BL_INLINE ~ArenaHashMapBase() noexcept {
    if (_data != _embedded)
      _allocator->release(_data, _bucket_count * sizeof(ArenaHashMapNode*));
  }

  BL_INLINE void reset() noexcept {
    if (_data != _embedded)
      _allocator->release(_data, _bucket_count * sizeof(ArenaHashMapNode*));

    _data = _embedded;
    _size = 0;
    _bucket_count = kNullCount;
    _bucket_grow = kNullGrow;
    _rcp_value = kNullRcpValue;
    _rcp_shift = kNullRcpShift;
    _prime_index = 0;
    memset(_embedded, 0, kNullCount * sizeof(ArenaHashMapNode*));
  }

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE bool is_empty() const noexcept { return _size == 0; }
  BL_INLINE size_t size() const noexcept { return _size; }

  //! \}

  //! \name Internals
  //! \{

  BL_INLINE void _swap(ArenaHashMapBase& other) noexcept {
    BLInternal::swap(_allocator, other._allocator);
    BLInternal::swap(_data, other._data);
    BLInternal::swap(_size, other._size);
    BLInternal::swap(_bucket_count, other._bucket_count);
    BLInternal::swap(_bucket_grow, other._bucket_grow);
    BLInternal::swap(_rcp_value, other._rcp_value);
    BLInternal::swap(_rcp_shift, other._rcp_shift);
    BLInternal::swap(_prime_index, other._prime_index);

    for (uint32_t i = 0; i < kNullCount; i++)
      BLInternal::swap(_embedded[i], other._embedded[i]);

    if (_data == other._embedded) _data = _embedded;
    if (other._data == _embedded) other._data = other._embedded;
  }

  BL_INLINE uint32_t _calc_mod(uint32_t hash) const noexcept {
    uint32_t divided =
      BL_TARGET_ARCH_BITS >= 64
        ? uint32_t((uint64_t(hash) * _rcp_value) >> _rcp_shift)
        : uint32_t((uint64_t(hash) * _rcp_value) >> 32) >> _rcp_shift;

    uint32_t result = hash - divided * _bucket_count;
    BL_ASSERT(result < _bucket_count);
    return result;
  }

  void _rehash(uint32_t prime_index) noexcept;
  void _insert(ArenaHashMapNode* node) noexcept;
  bool _remove(ArenaHashMapNode* node) noexcept;

  //! \}
};

//! Low-level hash table specialized for storing string keys and POD values.
//!
//! This hash table allows duplicates to be inserted (the API is so low
//! level that it's up to you if you allow it or not, as you should first
//! `get()` the node and then modify it or insert a new node by using `insert()`,
//! depending on the intention).
template<typename NodeT>
class ArenaHashMap : public ArenaHashMapBase {
public:
  BL_NONCOPYABLE(ArenaHashMap)

  typedef NodeT Node;

  //! \name Construction & Destruction
  //! \{

  BL_INLINE ArenaHashMap(ArenaAllocator* allocator) noexcept
    : ArenaHashMapBase(allocator) {}

  BL_INLINE ArenaHashMap(ArenaHashMap&& other) noexcept
    : ArenaHashMap(other) {}

  BL_INLINE ~ArenaHashMap() noexcept {
    if constexpr (!std::is_trivially_destructible_v<NodeT>)
      _destroy();
  }

  //! \}

  //! \name Utilities
  //! \{

  BL_INLINE void swap(ArenaHashMap& other) noexcept {
    ArenaHashMapBase::_swap(other);
  }

  BL_NOINLINE void _destroy() noexcept {
    for (size_t i = 0; i < _bucket_count; i++) {
      NodeT* node = static_cast<NodeT*>(_data[i]);
      if (node) {
        do {
          NodeT* next = static_cast<NodeT*>(node->_hash_next);
          bl_call_dtor(*node);
          node = next;
        } while (node);
        _data[i] = nullptr;
      }
    }
  }

  //! \}

  //! \name Functionality
  //! \{

  BL_INLINE NodeT* nodes_by_hash_code(uint32_t hash_code) const noexcept {
    uint32_t hash_mod = _calc_mod(hash_code);
    return static_cast<NodeT*>(_data[hash_mod]);
  }

  template<typename KeyT>
  BL_INLINE NodeT* get(const KeyT& key) const noexcept {
    NodeT* node = nodes_by_hash_code(key.hash_code());
    while (node && !key.matches(node))
      node = static_cast<NodeT*>(node->_hash_next);
    return node;
  }

  BL_INLINE void insert(NodeT* node) noexcept { _insert(node); }
  BL_INLINE bool remove(NodeT* node) noexcept { return _remove(node); }

  template<typename Lambda>
  BL_INLINE void for_each(Lambda&& f) const noexcept {
    ArenaHashMapNode** buckets = _data;
    uint32_t bucket_count = _bucket_count;

    for (uint32_t i = 0; i < bucket_count; i++) {
      Node* node = static_cast<Node*>(buckets[i]);
      while (node) {
        Node* next = static_cast<Node*>(node->_hash_next);
        f(node);
        node = next;
      }
    }
  }

  //! \}
};

//! \}

} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_SUPPORT_ARENAHASHMAP_P_H_INCLUDED
