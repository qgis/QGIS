// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_FILESYSTEM_H_INCLUDED
#define BLEND2D_FILESYSTEM_H_INCLUDED

#include <blend2d/core/array.h>

//! \addtogroup bl_filesystem
//! \{

//! \name BLFile API Constants
//! \{

//! File information flags, used by \ref BLFileInfo.
BL_DEFINE_ENUM(BLFileInfoFlags) {
  //! File owner has read permission (compatible with 0400 octal notation).
  BL_FILE_INFO_OWNER_R = 0x00000100u,
  //! File owner has write permission (compatible with 0200 octal notation).
  BL_FILE_INFO_OWNER_W = 0x00000080u,
  //! File owner has execute permission (compatible with 0100 octal notation).
  BL_FILE_INFO_OWNER_X = 0x00000040u,
  //! A combination of \ref BL_FILE_INFO_OWNER_R, \ref BL_FILE_INFO_OWNER_W, and \ref BL_FILE_INFO_OWNER_X.
  BL_FILE_INFO_OWNER_MASK = 0x000001C0u,

  //! File group owner has read permission (compatible with 040 octal notation).
  BL_FILE_INFO_GROUP_R = 0x00000020u,
  //! File group owner has write permission (compatible with 020 octal notation).
  BL_FILE_INFO_GROUP_W = 0x00000010u,
  //! File group owner has execute permission (compatible with 010 octal notation).
  BL_FILE_INFO_GROUP_X = 0x00000008u,
  //! A combination of \ref BL_FILE_INFO_GROUP_R, \ref BL_FILE_INFO_GROUP_W, and \ref BL_FILE_INFO_GROUP_X.
  BL_FILE_INFO_GROUP_MASK = 0x00000038u,

  //! Other users have read permission (compatible with 04 octal notation).
  BL_FILE_INFO_OTHER_R = 0x00000004u,
  //! Other users have write permission (compatible with 02 octal notation).
  BL_FILE_INFO_OTHER_W = 0x00000002u,
  //! Other users have execute permission (compatible with 01 octal notation).
  BL_FILE_INFO_OTHER_X = 0x00000001u,
  //! A combination of \ref BL_FILE_INFO_OTHER_R, \ref BL_FILE_INFO_OTHER_W, and \ref BL_FILE_INFO_OTHER_X.
  BL_FILE_INFO_OTHER_MASK = 0x00000007u,

  //! Set user ID to file owner user ID on execution (compatible with 04000 octal notation).
  BL_FILE_INFO_SUID = 0x00000800u,
  //! Set group ID to file's user group ID on execution (compatible with 02000 octal notation).
  BL_FILE_INFO_SGID = 0x00000400u,

  //! A combination of all file permission bits.
  BL_FILE_INFO_PERMISSIONS_MASK = 0x00000FFFu,

  //! A flag specifying that this is a regular file.
  BL_FILE_INFO_REGULAR = 0x00010000u,
  //! A flag specifying that this is a directory.
  BL_FILE_INFO_DIRECTORY = 0x00020000u,
  //! A flag specifying that this is a symbolic link.
  BL_FILE_INFO_SYMLINK = 0x00040000u,

  //! A flag describing a character device.
  BL_FILE_INFO_CHAR_DEVICE = 0x00100000u,
  //! A flag describing a block device.
  BL_FILE_INFO_BLOCK_DEVICE = 0x00200000u,
  //! A flag describing a FIFO (named pipe).
  BL_FILE_INFO_FIFO = 0x00400000u,
  //! A flag describing a socket.
  BL_FILE_INFO_SOCKET = 0x00800000u,

  //! A flag describing a hidden file (Windows only).
  BL_FILE_INFO_HIDDEN = 0x01000000u,
  //! A flag describing a hidden file (Windows only).
  BL_FILE_INFO_EXECUTABLE = 0x02000000u,
  //! A flag describing an archive (Windows only).
  BL_FILE_INFO_ARCHIVE = 0x04000000u,
  //! A flag describing a system file (Windows only).
  BL_FILE_INFO_SYSTEM = 0x08000000u,

  //! File information is valid (the request succeeded).
  BL_FILE_INFO_VALID = 0x80000000u

  BL_FORCE_ENUM_UINT32(BL_FILE_INFO)
};

//! File open flags, see \ref BLFile::open().
BL_DEFINE_ENUM(BLFileOpenFlags) {
  //! No flags.
  BL_FILE_OPEN_NO_FLAGS = 0u,

  //! Opens the file for reading.
  //!
  //! The following system flags are used when opening the file:
  //!   - `O_RDONLY` (Posix)
  //!   - `GENERIC_READ` (Windows)
  BL_FILE_OPEN_READ = 0x00000001u,

  //! Opens the file for writing:
  //!
  //! The following system flags are used when opening the file:
  //!   - `O_WRONLY` (Posix)
  //!   - `GENERIC_WRITE` (Windows)
  BL_FILE_OPEN_WRITE = 0x00000002u,

  //! Opens the file for reading & writing.
  //!
  //! The following system flags are used when opening the file:
  //!   - `O_RDWR` (Posix)
  //!   - `GENERIC_READ | GENERIC_WRITE` (Windows)
  BL_FILE_OPEN_RW = 0x00000003u,

  //! Creates the file if it doesn't exist or opens it if it does.
  //!
  //! The following system flags are used when opening the file:
  //!   - `O_CREAT` (Posix)
  //!   - `CREATE_ALWAYS` or `OPEN_ALWAYS` depending on other flags (Windows)
  BL_FILE_OPEN_CREATE = 0x00000004u,

  //! Opens the file for deleting or renaming (Windows).
  //!
  //! Adds `DELETE` flag when opening the file to `ACCESS_MASK`.
  BL_FILE_OPEN_DELETE = 0x00000008u,

  //! Truncates the file.
  //!
  //! The following system flags are used when opening the file:
  //!   - `O_TRUNC` (Posix)
  //!   - `TRUNCATE_EXISTING` (Windows)
  BL_FILE_OPEN_TRUNCATE = 0x00000010u,

  //! Opens the file for reading in exclusive mode (Windows).
  //!
  //! Exclusive mode means to not specify the `FILE_SHARE_READ` option.
  BL_FILE_OPEN_READ_EXCLUSIVE = 0x10000000u,

  //! Opens the file for writing in exclusive mode (Windows).
  //!
  //! Exclusive mode means to not specify the `FILE_SHARE_WRITE` option.
  BL_FILE_OPEN_WRITE_EXCLUSIVE = 0x20000000u,

  //! Opens the file for both reading and writing (Windows).
  //!
  //! This is a combination of both `BL_FILE_OPEN_READ_EXCLUSIVE` and `BL_FILE_OPEN_WRITE_EXCLUSIVE`.
  BL_FILE_OPEN_RW_EXCLUSIVE = 0x30000000u,

  //! Creates the file in exclusive mode - fails if the file already exists.
  //!
  //! The following system flags are used when opening the file:
  //!   - `O_EXCL` (Posix)
  //!   - `CREATE_NEW` (Windows)
  BL_FILE_OPEN_CREATE_EXCLUSIVE = 0x40000000u,

  //! Opens the file for deleting or renaming in exclusive mode (Windows).
  //!
  //! Exclusive mode means to not specify the `FILE_SHARE_DELETE` option.
  BL_FILE_OPEN_DELETE_EXCLUSIVE = 0x80000000u

  BL_FORCE_ENUM_UINT32(BL_FILE_OPEN)
};

//! File seek mode, see \ref BLFile::seek().
//!
//! \note Seek constants should be compatible with constants used by both POSIX
//! and Windows API.
BL_DEFINE_ENUM(BLFileSeekType) {
  //! Seek from the beginning of the file (SEEK_SET).
  BL_FILE_SEEK_SET = 0,
  //! Seek from the current position (SEEK_CUR).
  BL_FILE_SEEK_CUR = 1,
  //! Seek from the end of the file (SEEK_END).
  BL_FILE_SEEK_END = 2,

  //! Maximum value of `BLFileSeekType`.
  BL_FILE_SEEK_MAX_VALUE = 3

  BL_FORCE_ENUM_UINT32(BL_FILE_SEEK)
};

//! File read flags used by \ref BLFileSystem::read_file().
BL_DEFINE_ENUM(BLFileReadFlags) {
  //! No flags.
  BL_FILE_READ_NO_FLAGS = 0u,

  //! Use memory mapping to read the content of the file.
  //!
  //! The destination buffer `BLArray<>` would be configured to use the memory mapped buffer instead of allocating its
  //! own.
  BL_FILE_READ_MMAP_ENABLED = 0x00000001u,

  //! Avoid memory mapping of small files.
  //!
  //! The size of small file is determined by Blend2D, however, you should expect it to be 16kB or 64kB depending on
  //! host operating system.
  BL_FILE_READ_MMAP_AVOID_SMALL = 0x00000002u,

  //! Do not fallback to regular read if memory mapping fails. It's worth noting that memory mapping would fail for
  //! files stored on filesystem that is not local (like a mounted network filesystem, etc...).
  BL_FILE_READ_MMAP_NO_FALLBACK = 0x00000008u

  BL_FORCE_ENUM_UINT32(BL_FILE_READ)
};

//! \}

//! \name BLFile C API Structs
//!
//! \{

//! A thin abstraction over a native OS file IO [C API].
struct BLFileCore {
  //! A file handle - either a file descriptor used by POSIX or file handle used by Windows. On both platforms the
  //! handle is always `intptr_t` to make FFI easier (it's basically the size of a pointer / machine register).
  //!
  //! \note A handle of value `-1` is considered invalid and/or uninitialized. This value also matches Windows API
  //! `INVALID_HANDLE_VALUE`, which is also defined to be -1.
  intptr_t handle;
};

//! \}

//! \name BLFileInfo Structs
//!
//! \{

//! File information.
struct BLFileInfo {
  //! \name Members
  //! \{

  uint64_t size;
  int64_t modified_time;
  BLFileInfoFlags flags;
  uint32_t uid;
  uint32_t gid;
  uint32_t reserved[5];

  //! \}

#if defined(__cplusplus)

  //! \name Accessors
  //! \{

  //! Tests whether the file information has the given `flag` set in `flags`.
  BL_INLINE_NODEBUG bool has_flag(BLFileInfoFlags flag) const noexcept { return (flags & flag) != 0; }

  BL_INLINE_NODEBUG bool has_owner_r() const noexcept { return has_flag(BL_FILE_INFO_OWNER_R); }
  BL_INLINE_NODEBUG bool has_owner_w() const noexcept { return has_flag(BL_FILE_INFO_OWNER_W); }
  BL_INLINE_NODEBUG bool has_owner_x() const noexcept { return has_flag(BL_FILE_INFO_OWNER_X); }

  BL_INLINE_NODEBUG bool has_group_r() const noexcept { return has_flag(BL_FILE_INFO_GROUP_R); }
  BL_INLINE_NODEBUG bool has_group_w() const noexcept { return has_flag(BL_FILE_INFO_GROUP_W); }
  BL_INLINE_NODEBUG bool has_group_x() const noexcept { return has_flag(BL_FILE_INFO_GROUP_X); }

  BL_INLINE_NODEBUG bool has_other_r() const noexcept { return has_flag(BL_FILE_INFO_OTHER_R); }
  BL_INLINE_NODEBUG bool has_other_w() const noexcept { return has_flag(BL_FILE_INFO_OTHER_W); }
  BL_INLINE_NODEBUG bool has_other_x() const noexcept { return has_flag(BL_FILE_INFO_OTHER_X); }

  BL_INLINE_NODEBUG bool has_suid() const noexcept { return has_flag(BL_FILE_INFO_SUID); }
  BL_INLINE_NODEBUG bool has_sgid() const noexcept { return has_flag(BL_FILE_INFO_SGID); }

  BL_INLINE_NODEBUG bool is_regular() const noexcept { return has_flag(BL_FILE_INFO_REGULAR); }
  BL_INLINE_NODEBUG bool is_directory() const noexcept { return has_flag(BL_FILE_INFO_DIRECTORY); }
  BL_INLINE_NODEBUG bool is_symlink() const noexcept { return has_flag(BL_FILE_INFO_SYMLINK); }

  BL_INLINE_NODEBUG bool is_char_device() const noexcept { return has_flag(BL_FILE_INFO_CHAR_DEVICE); }
  BL_INLINE_NODEBUG bool is_block_device() const noexcept { return has_flag(BL_FILE_INFO_BLOCK_DEVICE); }
  BL_INLINE_NODEBUG bool is_fifo() const noexcept { return has_flag(BL_FILE_INFO_FIFO); }
  BL_INLINE_NODEBUG bool is_socket() const noexcept { return has_flag(BL_FILE_INFO_SOCKET); }

  BL_INLINE_NODEBUG bool is_hidden() const noexcept { return has_flag(BL_FILE_INFO_HIDDEN); }
  BL_INLINE_NODEBUG bool is_executable() const noexcept { return has_flag(BL_FILE_INFO_EXECUTABLE); }
  BL_INLINE_NODEBUG bool is_archive() const noexcept { return has_flag(BL_FILE_INFO_ARCHIVE); }
  BL_INLINE_NODEBUG bool is_system() const noexcept { return has_flag(BL_FILE_INFO_SYSTEM); }

  BL_INLINE_NODEBUG bool is_valid() const noexcept { return has_flag(BL_FILE_INFO_VALID); }

  //! \}

#endif
};

//! \}

//! \}

//! \addtogroup bl_c_api
//! \{

BL_BEGIN_C_DECLS

//! \name BLFile C API Functions
//!
//! File read/write functionality is provided by \ref BLFileCore in C API and wrapped by \ref BLFile in C++ API.
//!
//! \{

BL_API BLResult BL_CDECL bl_file_init(BLFileCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_file_reset(BLFileCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_file_open(BLFileCore* self, const char* file_name, BLFileOpenFlags open_flags) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_file_close(BLFileCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_file_seek(BLFileCore* self, int64_t offset, BLFileSeekType seek_type, int64_t* position_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_file_read(BLFileCore* self, void* buffer, size_t n, size_t* bytes_read_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_file_write(BLFileCore* self, const void* buffer, size_t n, size_t* bytes_written_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_file_truncate(BLFileCore* self, int64_t max_size) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_file_get_info(BLFileCore* self, BLFileInfo* info_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_file_get_size(BLFileCore* self, uint64_t* file_size_out) BL_NOEXCEPT_C;

//! \}

//! \name BLFileSystem C API Functions
//!
//! \{

BL_API BLResult BL_CDECL bl_file_system_get_info(const char* file_name, BLFileInfo* info_out) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_file_system_read_file(const char* file_name, BLArrayCore* dst, size_t max_size, BLFileReadFlags read_flags) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_file_system_write_file(const char* file_name, const void* data, size_t size, size_t* bytes_written_out) BL_NOEXCEPT_C;

//! \}

BL_END_C_DECLS

//! \}

//! \addtogroup bl_filesystem
//! \{

#ifdef __cplusplus
//! \name BLFile C++ API
//! \{

//! A thin abstraction over a native OS file IO [C++ API].
//!
//! A thin wrapper around a native OS file support. The file handle is always `intptr_t` and it refers to either
//! a file descriptor on POSIX targets and file handle on Windows targets.
class BLFile final : public BLFileCore {
public:
  // Prevent copy-constructor and copy-assignment.
  BL_INLINE_NODEBUG BLFile(const BLFile& other) noexcept = delete;
  BL_INLINE_NODEBUG BLFile& operator=(const BLFile& other) noexcept = delete;

  //! \name Construction & Destruction
  //! \{

  //! Creates an empty file instance, which doesn't represent any open file.
  //!
  //! \note The internal file handle of non-opened files is set to -1.
  BL_INLINE_NODEBUG BLFile() noexcept
    : BLFileCore { -1 } {}

  //! Move constructor - copies file descriptor from `other` to this instance and resets `other` to a default
  //! constructed state.
  BL_INLINE_NODEBUG BLFile(BLFile&& other) noexcept {
    intptr_t h = other.handle;
    other.handle = -1;
    handle = h;
  }

  //! Creates a file instance from an existing file `handle`, which either represents a file descriptor or Windows
  //! `HANDLE` (if compiled for Windows platform).
  BL_INLINE_NODEBUG explicit BLFile(intptr_t handle) noexcept
    : BLFileCore { handle } {}

  BL_INLINE_NODEBUG BLFile& operator=(BLFile&& other) noexcept {
    intptr_t h = other.handle;
    other.handle = -1;

    this->close();
    this->handle = h;

    return *this;
  }

  //! Destroys this file instance - closes the file descriptor or handle when it's referencing an open file.
  BL_INLINE_NODEBUG ~BLFile() noexcept { bl_file_reset(this); }

  //! \}

  //! \name Common Functionality
  //! \{

  BL_INLINE_NODEBUG void swap(BLFile& other) noexcept { BLInternal::swap(this->handle, other.handle); }

  //! \}

  //! \name Interface
  //! \{

  //! Tests whether the file is open.
  BL_INLINE_NODEBUG bool is_open() const noexcept { return handle != -1; }

  //! Attempts to open a file specified by `file_name` with the given `open_flags`.
  BL_INLINE_NODEBUG BLResult open(const char* file_name, BLFileOpenFlags open_flags) noexcept {
    return bl_file_open(this, file_name, open_flags);
  }

  //! Closes the file (if open) and sets the file handle to -1.
  BL_INLINE_NODEBUG BLResult close() noexcept {
    return bl_file_close(this);
  }

  //! Sets the file position of the file to the given `offset` by using the specified `seek_type`.
  BL_INLINE_NODEBUG BLResult seek(int64_t offset, BLFileSeekType seek_type) noexcept {
    int64_t position_out;
    return bl_file_seek(this, offset, seek_type, &position_out);
  }

  //! Sets the file position of the file to the given `offset` by using the specified `seek_type` and writes the new
  //! position into `position_out` output parameter.
  BL_INLINE_NODEBUG BLResult seek(int64_t offset, BLFileSeekType seek_type, int64_t* position_out) noexcept {
    return bl_file_seek(this, offset, seek_type, position_out);
  }

  //! Reads `n` bytes from the file into the given `buffer` and stores the number of bytes actually read into
  //! the `bytes_read_out` output parameter.
  BL_INLINE_NODEBUG BLResult read(void* buffer, size_t n, size_t* bytes_read_out) noexcept {
    return bl_file_read(this, buffer, n, bytes_read_out);
  }

  //! Writes `n` bytes to the file from the given `buffer` and stores the number of bytes actually written into
  //! the `bytes_read_out` output parameter.
  BL_INLINE_NODEBUG BLResult write(const void* buffer, size_t n, size_t* bytes_written_out) noexcept {
    return bl_file_write(this, buffer, n, bytes_written_out);
  }

  //! Truncates the file to the given maximum size `max_size`.
  BL_INLINE_NODEBUG BLResult truncate(int64_t max_size) noexcept {
    return bl_file_truncate(this, max_size);
  }

  //! Queries an information of the file and stores it to `info_out`.
  BL_INLINE_NODEBUG BLResult get_info(BLFileInfo* info_out) noexcept {
    return bl_file_get_info(this, info_out);
  }

  //! Queries a size of the file and stores it to `size_out`.
  BL_INLINE_NODEBUG BLResult get_size(uint64_t* size_out) noexcept {
    return bl_file_get_size(this, size_out);
  }

  //! \}
};

//! File-system utilities.
namespace BLFileSystem {

static BL_INLINE_NODEBUG BLResult file_info(const char* file_name, BLFileInfo* info_out) noexcept {
  return bl_file_system_get_info(file_name, info_out);
}

//! Reads a file into the `dst` buffer.
//!
//! Optionally you can set `max_size` to non-zero value that would restrict the maximum bytes to read to such value.
//! In addition, `read_flags` can be used to enable file mapping. See \ref BLFileReadFlags for more details.
static BL_INLINE_NODEBUG BLResult read_file(const char* file_name, BLArray<uint8_t>& dst, size_t max_size = 0, BLFileReadFlags read_flags = BL_FILE_READ_NO_FLAGS) noexcept {
  return bl_file_system_read_file(file_name, &dst, max_size, read_flags);
}

static BL_INLINE_NODEBUG BLResult write_file(const char* file_name, const void* data, size_t size) noexcept {
  size_t bytes_written_out;
  return bl_file_system_write_file(file_name, data, size, &bytes_written_out);
}

static BL_INLINE_NODEBUG BLResult write_file(const char* file_name, const void* data, size_t size, size_t* bytes_written_out) noexcept {
  return bl_file_system_write_file(file_name, data, size, bytes_written_out);
}

static BL_INLINE_NODEBUG BLResult write_file(const char* file_name, const BLArrayView<uint8_t>& view) noexcept {
  return write_file(file_name, view.data, view.size);
}

static BL_INLINE_NODEBUG BLResult write_file(const char* file_name, const BLArrayView<uint8_t>& view, size_t* bytes_written_out) noexcept {
  return write_file(file_name, view.data, view.size, bytes_written_out);
}

static BL_INLINE_NODEBUG BLResult write_file(const char* file_name, const BLArray<uint8_t>& array) noexcept {
  return write_file(file_name, array.view());
}

static BL_INLINE_NODEBUG BLResult write_file(const char* file_name, const BLArray<uint8_t>& array, size_t* bytes_written_out) noexcept {
  return write_file(file_name, array.view(), bytes_written_out);
}

} // {BLFileSystem}

//! \}
#endif

//! \}

#endif // BLEND2D_FILESYSTEM_H_INCLUDED
