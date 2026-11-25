// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_FILESYSTEM_P_H_INCLUDED
#define BLEND2D_FILESYSTEM_P_H_INCLUDED

#include <blend2d/core/filesystem.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

//! A thin abstraction over `mmap() / munmap()` (Posix) and `FileMapping` (Windows) to create a read-only file mapping
//! for loading fonts and other resources. File mapping interface is not exposed to users directly, only by high-level
//! functionality.
class BLFileMapping {
public:
  BL_NONCOPYABLE(BLFileMapping)

  void* _data = nullptr;
  size_t _size = 0;

#if defined(_WIN32)
  HANDLE _file_mapping_handle = INVALID_HANDLE_VALUE;
#endif

  //! \name Construction & Destruction
  //! \{

  BL_INLINE BLFileMapping() noexcept {}

  BL_INLINE BLFileMapping(BLFileMapping&& other) noexcept {
    void* data = other._data;
    size_t size = other._size;

    other._data = nullptr;
    other._size = 0;

    _data = data;
    _size = size;

#if defined(_WIN32)
    HANDLE file_mapping_handle = other._file_mapping_handle;
    other._file_mapping_handle = INVALID_HANDLE_VALUE;
    _file_mapping_handle = file_mapping_handle;
#endif
  }

  BL_INLINE ~BLFileMapping() noexcept { unmap(); }

  //! \}

  //! \name Overloaded Operators
  //! \{

  BL_INLINE BLFileMapping& operator=(BLFileMapping&& other) noexcept {
    void* data = other._data;
    size_t size = other._size;

    other._data = nullptr;
    other._size = 0;

#if defined(_WIN32)
    HANDLE file_mapping_handle = other._file_mapping_handle;
    other._file_mapping_handle = INVALID_HANDLE_VALUE;
#endif

    unmap();

    _data = data;
    _size = size;
#if defined(_WIN32)
    _file_mapping_handle = file_mapping_handle;
#endif

    return *this;
  }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns whether the mapping is empty (i.e. not file has been mapped).
  [[nodiscard]]
  BL_INLINE bool is_empty() const noexcept { return _size == 0; }

  //! Returns mapped data casted to `T`.
  template<typename T = void>
  [[nodiscard]]
  BL_INLINE T* data() noexcept { return static_cast<T*>(_data); }

  //! Returns mapped data casted to `T` (const).
  template<typename T = void>
  [[nodiscard]]
  BL_INLINE const T* data() const noexcept { return static_cast<const T*>(_data); }

  //! Returns the size of the mapped data.
  [[nodiscard]]
  BL_INLINE size_t size() const noexcept { return _size; }

#if defined(_WIN32)
  //! Returns a Windows-specific HANDLE of the mapped object.
  [[nodiscard]]
  BL_INLINE HANDLE file_mapping_handle() const noexcept { return _file_mapping_handle; }
#endif

  //! \}

  //! \name Map & Unmap
  //! \{

  //! Maps file `file` to memory. Takes ownership of `file` (moves) on success.
  BL_HIDDEN BLResult map(BLFile& file, size_t size, uint32_t flags = 0) noexcept;

  //! Unmaps previously mapped file or does nothing, if no file was mapped.
  BL_HIDDEN BLResult unmap() noexcept;

  //! \}
};

//! \}
//! \endcond

#endif // BLEND2D_FILESYSTEM_P_H_INCLUDED
