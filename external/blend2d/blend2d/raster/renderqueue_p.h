// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RASTER_RENDERQUEUE_P_H_INCLUDED
#define BLEND2D_RASTER_RENDERQUEUE_P_H_INCLUDED

#include <blend2d/raster/rendercommand_p.h>
#include <blend2d/support/arenalist_p.h>
#include <blend2d/support/fixedbitarray_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_raster_engine_impl
//! \{

namespace bl::RasterEngine {

struct RenderCommand;
struct RenderFetchData;
struct RenderJob;

static constexpr uint32_t kRenderQueueCapacity = 256;
static constexpr uint8_t kInvalidQuantizedCoordinate = 0xFFu;

//! A generic queue used to store rendering jobs and other data.
//!
//! \note `RenderQueueGenericAppender` is used to add items to the queue.
template<typename T>
class RenderGenericQueue : public ArenaListNode<RenderGenericQueue<T>> {
public:
  //! \name Members
  //! \{

  //! Number of items in the queue.
  size_t _size;

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE_NODEBUG void reset() noexcept { _size = 0; }

  BL_INLINE_NODEBUG bool is_empty() const noexcept { return _size == 0; }
  BL_INLINE_NODEBUG size_t size() const noexcept { return _size; }
  BL_INLINE_NODEBUG size_t capacity() const noexcept { return kRenderQueueCapacity; }

  BL_INLINE_NODEBUG T* data() noexcept { return (T*)(this + 1); }
  BL_INLINE_NODEBUG const T* data() const noexcept { return (const T*)(this + 1); }

  BL_INLINE_NODEBUG T* begin() noexcept { return (T*)(this + 1); }
  BL_INLINE_NODEBUG const T* begin() const noexcept { return (const T*)(this + 1); }

  BL_INLINE_NODEBUG T* end() noexcept { return data() + _size; }
  BL_INLINE_NODEBUG const T* end() const noexcept { return data() + _size; }

  BL_INLINE T& at(size_t index) noexcept {
    BL_ASSERT(index < _size);
    return data()[index];
  }

  BL_INLINE const T& at(size_t index) const noexcept {
    BL_ASSERT(index < _size);
    return data()[index];
  }

  //! \}

  //! \name Statics
  //! \{

  static BL_INLINE_CONSTEXPR size_t size_of() noexcept {
    return sizeof(RenderGenericQueue<T>) + sizeof(T) * kRenderQueueCapacity;
  }

  //! \}
};

typedef RenderGenericQueue<RenderJob*> RenderJobQueue;

class RenderCommandQueue : public ArenaListNode<RenderCommandQueue> {
public:
  //! \name Members
  //! \{

  //! Number of items in the queue.
  size_t _size;

  //! Bit-array where each bit represents a valid FetchData in `_data`, that has to be released once the batch is done.
  FixedBitArray<BLBitWord, kRenderQueueCapacity> _fetch_data_marks;

  //! Quantized Y0 coordinate (shifted right by quantizeShiftY).
  uint8_t _quantizedY0[kRenderQueueCapacity];

  //! Array of render commands.
  RenderCommand _data[kRenderQueueCapacity];

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE_NODEBUG RenderCommandQueue() noexcept { reset(); }

  BL_INLINE_NODEBUG void reset() noexcept {
    _size = 0;
    _fetch_data_marks.clear_all();
    memset(_quantizedY0, 0xFF, sizeof(_quantizedY0));
  }

  BL_INLINE_NODEBUG bool is_empty() const noexcept { return _size == 0; }
  BL_INLINE_NODEBUG size_t size() const noexcept { return _size; }
  BL_INLINE_NODEBUG size_t capacity() const noexcept { return kRenderQueueCapacity; }

  BL_INLINE_NODEBUG RenderCommand* data() noexcept { return _data; }
  BL_INLINE_NODEBUG const RenderCommand* data() const noexcept { return _data; }

  BL_INLINE_NODEBUG RenderCommand* begin() noexcept { return _data; }
  BL_INLINE_NODEBUG const RenderCommand* begin() const noexcept { return _data; }

  BL_INLINE_NODEBUG RenderCommand* end() noexcept { return _data + _size; }
  BL_INLINE_NODEBUG const RenderCommand* end() const noexcept { return _data + _size; }

  BL_INLINE RenderCommand& at(size_t command_index) noexcept {
    BL_ASSERT(command_index < kRenderQueueCapacity);
    return _data[command_index];
  }

  BL_INLINE const RenderCommand& at(size_t command_index) const noexcept {
    BL_ASSERT(command_index < kRenderQueueCapacity);
    return _data[command_index];
  }

  BL_INLINE void initQuantizedY0(size_t command_index, uint8_t qy0) noexcept {
    BL_ASSERT(command_index < kRenderQueueCapacity);
    _quantizedY0[command_index] = qy0;
  }

  //! \}

  //! \name Statics
  //! \{

  static BL_INLINE_CONSTEXPR size_t size_of() noexcept {
    return sizeof(RenderCommandQueue);
  }

  //! \}
};

//! A queue appender - appends items to `RenderGenericQueue`.
template<typename T>
class RenderQueueGenericAppender {
public:
  //! \name Members
  //! \{

  //! Current position in the queue (next item will be added exactly here).
  T* _ptr {};
  //! End of the queue.
  T* _end {};

  //! \}

  //! \name Interface
  //! \{

  BL_INLINE_NODEBUG void reset() noexcept {
    _ptr = nullptr;
    _end = nullptr;
  }

  BL_INLINE_NODEBUG void reset(RenderGenericQueue<T>& queue) noexcept {
    _ptr = queue.data();
    _end = queue.data() + queue.capacity();
  }

  BL_INLINE_NODEBUG size_t index(const RenderGenericQueue<T>& queue) const noexcept {
    return (size_t)(_ptr - queue.data());
  }

  BL_INLINE_NODEBUG bool full() const noexcept { return _ptr == _end; }
  BL_INLINE_NODEBUG void done(RenderGenericQueue<T>& queue) noexcept { queue._size = index(queue); }

  BL_INLINE void append(const T& item) noexcept {
    BL_ASSERT(!full());
    *_ptr++ = item;
  }

  //! Used when the data of the next command were already assigned to just advance the pointer. This should be
  //! only used by command queue, other queues should use `append()`.
  BL_INLINE void advance() noexcept {
    BL_ASSERT(!full());
    _ptr++;
  }

  //! \}
};

typedef RenderQueueGenericAppender<RenderJob*> RenderJobAppender;

class RenderCommandAppender {
public:
  //! \name Members
  //! \{

  RenderCommandQueue* _queue {};
  size_t _index {};

  //! \}

  //! \name Interface
  //! \{

  BL_INLINE_NODEBUG void reset() noexcept {
    _queue = nullptr;
    _index = 0;
  }

  BL_INLINE_NODEBUG void reset(RenderCommandQueue& queue) noexcept {
    _queue = &queue;
    _index = 0;
  }

  BL_INLINE_NODEBUG size_t index() const noexcept { return _index; }

  BL_INLINE_NODEBUG bool is_empty() const noexcept { return _index == 0; }
  BL_INLINE_NODEBUG bool full() const noexcept { return _index == kRenderQueueCapacity; }
  BL_INLINE_NODEBUG void done(RenderCommandQueue& queue) noexcept { queue._size = index(); }

  //! Used when the data of the next command were already assigned to just advance the pointer. This should be
  //! only used by command queue, other queues should use `append()`.
  BL_INLINE void advance() noexcept {
    BL_ASSERT(!full());
    _index++;
  }

  BL_INLINE RenderCommand* command(size_t i) const noexcept {
    BL_ASSERT(i < kRenderQueueCapacity);
    return _queue->data() + i;
  }

  BL_INLINE_NODEBUG RenderCommandQueue* queue() const noexcept { return _queue; }

  BL_INLINE_NODEBUG RenderCommand* current_command() const noexcept { return _queue->data() + _index; }

  BL_INLINE void mark_fetch_data() noexcept { _queue->_fetch_data_marks.set_at(_index); }
  BL_INLINE void mark_fetch_data(uint32_t v) noexcept { _queue->_fetch_data_marks.fill_at(_index, v); }

  BL_INLINE void initQuantizedY0(uint8_t qy0) noexcept {
    _queue->initQuantizedY0(_index, qy0);
  }

  //! \}
};

} // {bl::RasterEngine}

//! \}
//! \endcond

#endif // BLEND2D_RASTER_RENDERQUEUE_P_H_INCLUDED
