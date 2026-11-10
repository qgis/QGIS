// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/array_p.h>
#include <blend2d/core/filesystem_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/support/ptrops_p.h>
#include <blend2d/threading/atomic_p.h>
#include <blend2d/unicode/unicode_p.h>

#if !defined(_WIN32)
  #include <errno.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <sys/mman.h>
  #include <sys/stat.h>
  #include <sys/types.h>

  #if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    #define BL_FS_STAT_MTIMESPEC(s) s.st_mtim
  #elif defined(__NetBSD__) || defined(__APPLE__)
    #define BL_FS_STAT_MTIMESPEC(s) s.st_mtimespec
  #endif
#endif

#ifdef _WIN32

// BLUtf16StringTmp
// ================

static constexpr size_t kStaticUTF16StringSize = 1024;

template<size_t N>
class BLUtf16StringTmp {
public:
  uint16_t* _data;
  size_t _size;
  size_t _capacity;
  uint16_t _embedded_data[N + 1];

  BL_NONCOPYABLE(BLUtf16StringTmp);

  BL_INLINE BLUtf16StringTmp() noexcept
    : _data(_embedded_data),
      _size(0),
      _capacity(N) {}

  BL_INLINE ~BLUtf16StringTmp() noexcept {
    if (_data != _embedded_data)
      free(_data);
  }

  BL_INLINE_NODEBUG uint16_t* data() const noexcept { return _data; }
  BL_INLINE_NODEBUG wchar_t* data_as_wchar() const noexcept { return reinterpret_cast<wchar_t*>(_data); }

  BL_INLINE_NODEBUG size_t size() const noexcept { return _size; }
  BL_INLINE_NODEBUG size_t capacity() const noexcept { return _capacity; }

  BL_INLINE_NODEBUG void null_terminate() noexcept { _data[_size] = uint16_t(0); }

  BL_NOINLINE BLResult from_utf8(const char* src) noexcept {
    size_t src_size = strlen(src);
    bl::Unicode::ConversionState conversion_state;

    BLResult result = bl::Unicode::convert_unicode(
      _data, _capacity * 2u, BL_TEXT_ENCODING_UTF16, src, src_size, BL_TEXT_ENCODING_UTF8, conversion_state);

    if (result == BL_SUCCESS) {
      _size = conversion_state.dst_index / 2u;
      null_terminate();
      return result;
    }

    if (result != BL_ERROR_NO_SPACE_LEFT) {
      _size = 0;
      null_terminate();
      return result;
    }

    size_t proc_utf8_size = conversion_state.src_index;
    size_t proc_utf16_size = conversion_state.dst_index / 2u;

    bl::Unicode::ValidationState validation_state;
    BL_PROPAGATE(bl_validate_utf8(src + proc_utf8_size, src_size - proc_utf8_size, validation_state));

    size_t new_size = proc_utf16_size + validation_state.utf16_index;
    uint16_t* new_data = static_cast<uint16_t*>(malloc((new_size + 1) * sizeof(uint16_t)));

    if (BL_UNLIKELY(!new_data)) {
      _size = 0;
      null_terminate();
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);
    }

    memcpy(new_data, _data, proc_utf16_size * sizeof(uint16_t));
    bl::Unicode::convert_unicode(
      new_data + proc_utf16_size, (new_size - proc_utf16_size) * 2u, BL_TEXT_ENCODING_UTF16,
      src + proc_utf8_size, src_size - proc_utf8_size, BL_TEXT_ENCODING_UTF8, conversion_state);
    BL_ASSERT(new_size == proc_utf16_size + conversion_state.dst_index * 2u);

    if (_data != _embedded_data) {
      free(_data);
    }

    _data = new_data;
    _size = new_size;
    _capacity = new_size;

    null_terminate();
    return BL_SUCCESS;
  }
};

#endif

// BLFile - Utilities
// ==================

// Just a helper to cast to `BLFile` and call its `is_open()` as this is the only thing we need from C++ API here.
static BL_INLINE bool is_file_open(const BLFileCore* self) noexcept {
  return static_cast<const BLFile*>(self)->is_open();
}

// BLFile - API - Construction & Destruction
// =========================================

BL_API_IMPL BLResult bl_file_init(BLFileCore* self) noexcept {
  self->handle = -1;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_file_reset(BLFileCore* self) noexcept {
  return bl_file_close(self);
}

#ifdef _WIN32

// BLFileSystem - Windows Implementation (Internal)
// ================================================

namespace bl {
namespace FileSystem {

static BL_INLINE uint64_t combine_hi_lo(uint32_t hi, uint32_t lo) noexcept {
  return (uint64_t(hi) << 32) | lo;
}

static BL_INLINE uint64_t unix_micro_from_file_time(FILETIME ft) noexcept {
  constexpr uint64_t kFileTimeToUnixTimeS = 11644473600;
  constexpr uint32_t kMicrosecondsPerSecond = 1000000;

  // First convert to microseconds, starting from 1601-01-01 [UTC].
  uint64_t t = combine_hi_lo(ft.dwHighDateTime, ft.dwLowDateTime) / 10u;

  return int64_t(t) - int64_t(kFileTimeToUnixTimeS * kMicrosecondsPerSecond);
}

static BLFileInfoFlags file_flags_from_win_file_attributes(DWORD file_attributes) noexcept {
  uint32_t flags = BL_FILE_INFO_VALID;

  if (file_attributes & FILE_ATTRIBUTE_DIRECTORY) {
    flags |= BL_FILE_INFO_DIRECTORY;
  }
  else {
    flags |= BL_FILE_INFO_REGULAR;
  }

  if (file_attributes & FILE_ATTRIBUTE_REPARSE_POINT) {
    flags |= BL_FILE_INFO_SYMLINK;
  }

  if (file_attributes & FILE_ATTRIBUTE_DEVICE) {
    flags |= BL_FILE_INFO_CHAR_DEVICE;
  }

  if (file_attributes & FILE_ATTRIBUTE_HIDDEN) {
    flags |= BL_FILE_INFO_HIDDEN;
  }

  // Windows specific file attributes.
  if (file_attributes & FILE_ATTRIBUTE_ARCHIVE) {
    flags |= BL_FILE_INFO_ARCHIVE;
  }

  if (file_attributes & FILE_ATTRIBUTE_SYSTEM) {
    flags |= BL_FILE_INFO_SYSTEM;
  }

  // Windows specific file attributes.
  if (file_attributes & FILE_ATTRIBUTE_READONLY) {
    flags |= BL_FILE_INFO_OWNER_R |
             BL_FILE_INFO_GROUP_R |
             BL_FILE_INFO_OTHER_R;
  }
  else {
    flags |= BL_FILE_INFO_OWNER_R | BL_FILE_INFO_OWNER_W |
             BL_FILE_INFO_GROUP_R | BL_FILE_INFO_GROUP_W |
             BL_FILE_INFO_OTHER_R | BL_FILE_INFO_OTHER_W ;
  }

  return BLFileInfoFlags(flags);
}

static BLResult file_info_from_win_file_attribute_data(BLFileInfo& info, const WIN32_FILE_ATTRIBUTE_DATA& fa) noexcept {
  info.flags = file_flags_from_win_file_attributes(fa.dwFileAttributes);
  info.size = combine_hi_lo(fa.nFileSizeHigh, fa.nFileSizeLow);
  info.modified_time = unix_micro_from_file_time(fa.ftLastWriteTime);

  return BL_SUCCESS;
}

} // {FileSystem} namespace
} // {bl} namespace

// BLFile - API - Windows Implementation
// =====================================

static const constexpr DWORD kFileBufferSize = 32 * 1024 * 1024; // 32 MB.

BL_API_IMPL BLResult bl_file_open(BLFileCore* self, const char* file_name, BLFileOpenFlags open_flags) noexcept {
  // Desired Access
  // --------------
  //
  // The same flags as O_RDONLY|O_WRONLY|O_RDWR:

  DWORD desired_access = 0;
  switch (open_flags & BL_FILE_OPEN_RW) {
    case BL_FILE_OPEN_READ : desired_access = GENERIC_READ ; break;
    case BL_FILE_OPEN_WRITE: desired_access = GENERIC_WRITE; break;
    case BL_FILE_OPEN_RW   : desired_access = GENERIC_READ | GENERIC_WRITE; break;

    default:
      return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  // Creation Disposition
  // --------------------
  //
  // Since WinAPI documentation is so brief here is a better explanation about various CreationDisposition modes,
  // reformatted from SO:
  //
  //   https://stackoverflow.com/questions/14469607/difference-between-open-always-and-create-always-in-createfile-of-windows-api
  //
  // +-------------------------+-------------+--------------------+
  // | Creation Disposition    | File Exists | File Doesn't Exist |
  // +-------------------------+-------------+--------------------+
  // | CREATE_ALWAYS           | Truncate    | Create New         |
  // | CREATE_NEW              | Fail        | Create New         |
  // | OPEN_ALWAYS             | Open        | Create New         |
  // | OPEN_EXISTING           | Open        | Fail               |
  // | TRUNCATE_EXISTING       | Truncate    | Fail               |
  // +-------------------------+-------------+--------------------+

  uint32_t kExtFlags = BL_FILE_OPEN_CREATE | BL_FILE_OPEN_CREATE_EXCLUSIVE | BL_FILE_OPEN_TRUNCATE;

  if ((open_flags & kExtFlags) && (!(open_flags & BL_FILE_OPEN_WRITE))) {
    return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  DWORD creation_disposition = OPEN_EXISTING;
  if (open_flags & BL_FILE_OPEN_CREATE_EXCLUSIVE) {
    creation_disposition = CREATE_NEW;
  }
  else if ((open_flags & (BL_FILE_OPEN_CREATE | BL_FILE_OPEN_TRUNCATE)) == BL_FILE_OPEN_CREATE) {
    creation_disposition = OPEN_ALWAYS;
  }
  else if ((open_flags & (BL_FILE_OPEN_CREATE | BL_FILE_OPEN_TRUNCATE)) == (BL_FILE_OPEN_CREATE | BL_FILE_OPEN_TRUNCATE)) {
    creation_disposition = CREATE_ALWAYS;
  }
  else if (open_flags & BL_FILE_OPEN_TRUNCATE) {
    creation_disposition = TRUNCATE_EXISTING;
  }

  // Share Mode
  // ----------

  DWORD share_mode = 0;

  auto is_shared = [&](uint32_t access, uint32_t exclusive) noexcept -> bool {
    return (open_flags & (access | exclusive)) == access;
  };

  if (is_shared(BL_FILE_OPEN_READ, BL_FILE_OPEN_READ_EXCLUSIVE)) share_mode |= FILE_SHARE_READ;
  if (is_shared(BL_FILE_OPEN_WRITE, BL_FILE_OPEN_WRITE_EXCLUSIVE)) share_mode |= FILE_SHARE_WRITE;
  if (is_shared(BL_FILE_OPEN_DELETE, BL_FILE_OPEN_DELETE_EXCLUSIVE)) share_mode |= FILE_SHARE_DELETE;

  // WinAPI Call
  // -----------

  // NOTE: Do not close the file before calling `CreateFileW()`. We should behave atomically, which means that
  // we only close the existing file if `CreateFileW()` succeeds, otherwise we do nothing and the previous file
  // would still be open.
  BLUtf16StringTmp<kStaticUTF16StringSize> file_name_w;
  BL_PROPAGATE(file_name_w.from_utf8(file_name));

#if defined(BL_PLATFORM_UWP)
  // UWP platform doesn't provide CreateFileW, but we can use CreateFile2 instead.
  HANDLE handle = CreateFile2(
    file_name_w.data_as_wchar(),
    desired_access,
    share_mode,
    creation_disposition,
    nullptr
  );
#else
  DWORD flags_and_attributes = 0;
  LPSECURITY_ATTRIBUTES security_attributes_ptr = nullptr;

  HANDLE handle = CreateFileW(
    file_name_w.data_as_wchar(),
    desired_access,
    share_mode,
    security_attributes_ptr,
    creation_disposition,
    flags_and_attributes,
    nullptr
  );
#endif

  if (handle == INVALID_HANDLE_VALUE) {
    return bl_make_error(bl_result_from_win_error(GetLastError()));
  }

  bl_file_close(self);
  self->handle = intptr_t(handle);

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_file_close(BLFileCore* self) noexcept {
  // Not sure what should happen if `CloseHandle()` fails, if the handle is invalid or the close can be called
  // again? To ensure compatibility with POSIX implementation we just make it invalid.
  if (is_file_open(self)) {
    HANDLE handle = (HANDLE)self->handle;
    BOOL result = CloseHandle(handle);

    self->handle = -1;
    if (!result) {
      return bl_make_error(bl_result_from_win_error(GetLastError()));
    }
  }

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_file_seek(BLFileCore* self, int64_t offset, BLFileSeekType seek_type, int64_t* position_out) noexcept {
  *position_out = -1;

  DWORD move_method = 0;
  switch (seek_type) {
    case BL_FILE_SEEK_SET: move_method = FILE_BEGIN  ; break;
    case BL_FILE_SEEK_CUR: move_method = FILE_CURRENT; break;
    case BL_FILE_SEEK_END: move_method = FILE_END    ; break;

    default:
      return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  if (!is_file_open(self)) {
    return bl_make_error(BL_ERROR_INVALID_HANDLE);
  }

  LARGE_INTEGER to;
  LARGE_INTEGER prev;

  to.QuadPart = offset;
  prev.QuadPart = 0;

  HANDLE handle = (HANDLE)self->handle;
  BOOL result = SetFilePointerEx(handle, to, &prev, move_method);

  if (!result) {
    return bl_make_error(bl_result_from_win_error(GetLastError()));
  }

  *position_out = prev.QuadPart;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_file_read(BLFileCore* self, void* buffer, size_t n, size_t* bytes_read_out) noexcept {
  *bytes_read_out = 0;
  if (!is_file_open(self)) {
    return bl_make_error(BL_ERROR_INVALID_HANDLE);
  }

  BOOL result = true;
  HANDLE handle = (HANDLE)self->handle;

  size_t remaining_size = n;
  size_t bytes_read_total = 0;

  while (remaining_size) {
    DWORD local_size = static_cast<DWORD>(bl_min<size_t>(remaining_size, kFileBufferSize));
    DWORD bytes_read = 0;

    result = ReadFile(handle, buffer, local_size, &bytes_read, nullptr);
    remaining_size -= local_size;
    bytes_read_total += bytes_read;

    if (bytes_read < local_size || !result) {
      break;
    }

    buffer = bl::PtrOps::offset(buffer, bytes_read);
  }

  *bytes_read_out = bytes_read_total;
  if (!result) {
    DWORD e = GetLastError();
    if (e == ERROR_HANDLE_EOF) {
      return BL_SUCCESS;
    }
    return bl_make_error(bl_result_from_win_error(e));
  }
  else {
    return BL_SUCCESS;
  }
}

BL_API_IMPL BLResult bl_file_write(BLFileCore* self, const void* buffer, size_t n, size_t* bytes_written_out) noexcept {
  *bytes_written_out = 0;
  if (!is_file_open(self)) {
    return bl_make_error(BL_ERROR_INVALID_HANDLE);
  }

  HANDLE handle = (HANDLE)self->handle;
  BOOL result = true;

  size_t remaining_size = n;
  size_t bytes_written_total = 0;

  while (remaining_size) {
    DWORD local_size = static_cast<DWORD>(bl_min<size_t>(remaining_size, kFileBufferSize));
    DWORD bytes_written = 0;

    result = WriteFile(handle, buffer, local_size, &bytes_written, nullptr);
    remaining_size -= local_size;
    bytes_written_total += bytes_written;

    if (bytes_written < local_size || !result) {
      break;
    }

    buffer = bl::PtrOps::offset(buffer, bytes_written);
  }

  *bytes_written_out = bytes_written_total;
  if (!result) {
    return bl_make_error(bl_result_from_win_error(GetLastError()));
  }

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_file_truncate(BLFileCore* self, int64_t max_size) noexcept {
  if (!is_file_open(self)) {
    return bl_make_error(BL_ERROR_INVALID_HANDLE);
  }

  if (BL_UNLIKELY(max_size < 0)) {
    return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  int64_t prev;
  BL_PROPAGATE(bl_file_seek(self, max_size, BL_FILE_SEEK_SET, &prev));

  HANDLE handle = (HANDLE)self->handle;
  BOOL result = SetEndOfFile(handle);

  if (prev < max_size) {
    bl_file_seek(self, prev, BL_FILE_SEEK_SET, &prev);
  }

  if (!result) {
    return bl_make_error(bl_result_from_win_error(GetLastError()));
  }
  else {
    return BL_SUCCESS;
  }
}

BL_API_IMPL BLResult bl_file_get_info(BLFileCore* self, BLFileInfo* info_out) noexcept {
  *info_out = BLFileInfo{};

  if (!is_file_open(self)) {
    return bl_make_error(BL_ERROR_INVALID_HANDLE);
  }

  HANDLE handle = (HANDLE)self->handle;
  BY_HANDLE_FILE_INFORMATION fi;

  if (!GetFileInformationByHandle(handle, &fi)) {
    return bl_make_error(bl_result_from_win_error(GetLastError()));
  }

  info_out->size = bl::FileSystem::combine_hi_lo(fi.nFileSizeHigh, fi.nFileSizeLow);
  info_out->modified_time = bl::FileSystem::unix_micro_from_file_time(fi.ftLastWriteTime);
  info_out->flags = bl::FileSystem::file_flags_from_win_file_attributes(fi.dwFileAttributes);

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_file_get_size(BLFileCore* self, uint64_t* file_size_out) noexcept {
  *file_size_out = 0;

  if (!is_file_open(self)) {
    return bl_make_error(BL_ERROR_INVALID_HANDLE);
  }

  HANDLE handle = (HANDLE)self->handle;
  LARGE_INTEGER size;

  if (!GetFileSizeEx(handle, &size)) {
    return bl_make_error(bl_result_from_win_error(GetLastError()));
  }

  *file_size_out = uint64_t(size.QuadPart);
  return BL_SUCCESS;
}

// BLFileSystem - API - Windows Implementation
// ===========================================

BL_API_IMPL BLResult BL_CDECL bl_file_system_get_info(const char* file_name, BLFileInfo* info_out) noexcept {
  *info_out = BLFileInfo{};

  BLUtf16StringTmp<kStaticUTF16StringSize> file_name_w;
  BL_PROPAGATE(file_name_w.from_utf8(file_name));

  WIN32_FILE_ATTRIBUTE_DATA fa;
  if (!GetFileAttributesExW(file_name_w.data_as_wchar(), GetFileExInfoStandard, &fa)) {
    return bl_make_error(bl_result_from_win_error(GetLastError()));
  }

  return bl::FileSystem::file_info_from_win_file_attribute_data(*info_out, fa);
}

#else

// BLFileSystem - API - POSIX Implementation (Internal)
// ====================================================

namespace bl {
namespace FileSystem {

template<uint32_t kDst, uint32_t kSrc, uint32_t kMsk = 0x1>
static BL_INLINE uint32_t translate_flags(uint32_t src) noexcept {
  constexpr uint32_t kDstOffset = bl::IntOps::ctz_static(kDst);
  constexpr uint32_t kSrcOffset = bl::IntOps::ctz_static(kSrc);

  if constexpr (kDstOffset < kSrcOffset) {
    return (src >> (kSrcOffset - kDstOffset)) & (kMsk << kDstOffset);
  }
  else {
    return (src << (kDstOffset - kSrcOffset)) & (kMsk << kDstOffset);
  }
}

template<BLFileInfoFlags kDstX, uint32_t kSrcR, uint32_t kSrcW, uint32_t kSrcX>
static BL_INLINE uint32_t translate_rwx(uint32_t src) noexcept {
  if constexpr (kSrcW == (kSrcX << 1) && kSrcR == (kSrcX << 2)) {
    return translate_flags<kDstX, kSrcX, 0x7>(src);
  }
  else {
    return translate_flags<kDstX << 0, kSrcX>(src) |
           translate_flags<kDstX << 1, kSrcW>(src) |
           translate_flags<kDstX << 2, kSrcR>(src) ;
  }
}

template<typename T>
static BL_INLINE int64_t unix_micro_from_file_time(const T& t) noexcept {
  return int64_t(uint64_t(t) * 1000000);
}

#if defined(BL_FS_STAT_MTIMESPEC)
template<typename T>
static BL_INLINE int64_t unix_micro_from_time_spec(const T& ts) noexcept {
  return int64_t(uint64_t(ts.tv_sec) + (uint32_t(ts.tv_nsec) / 1000u));
}
#endif

static BLResult file_info_from_stat(BLFileInfo& info, struct BL_FILE64_API(stat)& s) noexcept {
  uint32_t flags = BL_FILE_INFO_VALID;

  // Translate file type to a portable representation:
  if (S_ISREG(s.st_mode)) flags |= BL_FILE_INFO_REGULAR;
  if (S_ISDIR(s.st_mode)) flags |= BL_FILE_INFO_DIRECTORY;

#if defined(S_ISLNK)
  if (S_ISLNK(s.st_mode)) flags |= BL_FILE_INFO_SYMLINK;
#endif // S_ISLNK

#if defined(S_ISCHR)
  if (S_ISCHR(s.st_mode)) flags |= BL_FILE_INFO_CHAR_DEVICE;
#endif // S_ISCHR

#if defined(S_ISBLK)
  if (S_ISBLK(s.st_mode)) flags |= BL_FILE_INFO_BLOCK_DEVICE;
#endif // S_ISBLK

#if defined(S_ISFIFO)
  if (S_ISFIFO(s.st_mode)) flags |= BL_FILE_INFO_FIFO;
#endif // S_ISFIFO

#if defined(S_ISSOCK)
  if (S_ISSOCK(s.st_mode)) flags |= BL_FILE_INFO_SOCKET;
#endif // S_ISSOCK

  // Translate file permissions to a portable representation:
  flags |= translate_rwx<BL_FILE_INFO_OWNER_X, S_IRUSR, S_IWUSR, S_IXUSR>(s.st_mode);
  flags |= translate_rwx<BL_FILE_INFO_GROUP_X, S_IRGRP, S_IWGRP, S_IXGRP>(s.st_mode);
  flags |= translate_rwx<BL_FILE_INFO_OTHER_X, S_IROTH, S_IWOTH, S_IXOTH>(s.st_mode);
  flags |= translate_flags<BL_FILE_INFO_SUID, S_ISUID>(s.st_mode);
  flags |= translate_flags<BL_FILE_INFO_SGID, S_ISGID>(s.st_mode);

  info = BLFileInfo{};

  if (flags & BL_FILE_INFO_REGULAR) {
    info.size = uint64_t(s.st_size);
  }

  info.flags = BLFileInfoFlags(flags);
  info.uid = uint32_t(s.st_uid);
  info.gid = uint32_t(s.st_gid);

#if defined(BL_FS_STAT_MTIMESPEC)
  info.modified_time = unix_micro_from_time_spec(BL_FS_STAT_MTIMESPEC(s));
#else
  info.modified_time = unix_micro_from_file_time(s.st_mtime);
#endif

  return BL_SUCCESS;
}

} // {FileSystem} namespace
} // {bl} namespace

// BLFile - API - POSIX Implementation
// ===================================

BL_API_IMPL BLResult bl_file_open(BLFileCore* self, const char* file_name, BLFileOpenFlags open_flags) noexcept {
  int of = 0;

  switch (open_flags & BL_FILE_OPEN_RW) {
    case BL_FILE_OPEN_READ : of |= O_RDONLY; break;
    case BL_FILE_OPEN_WRITE: of |= O_WRONLY; break;
    case BL_FILE_OPEN_RW   : of |= O_RDWR  ; break;

    default:
      return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  uint32_t kExtFlags = BL_FILE_OPEN_CREATE | BL_FILE_OPEN_CREATE_EXCLUSIVE | BL_FILE_OPEN_TRUNCATE;

  if ((open_flags & kExtFlags) && !(open_flags & BL_FILE_OPEN_WRITE)) {
    return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  if (open_flags & BL_FILE_OPEN_CREATE          ) of |= O_CREAT;
  if (open_flags & BL_FILE_OPEN_CREATE_EXCLUSIVE) of |= O_CREAT | O_EXCL;
  if (open_flags & BL_FILE_OPEN_TRUNCATE        ) of |= O_TRUNC;

  mode_t om = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH ;

  // NOTE: Do not close the file before calling `open()`. We should behave atomically, which means that we won't
  // close the existing file if `open()` fails...
  int fd = BL_FILE64_API(open)(file_name, of, om);
  if (fd < 0) {
    return bl_make_error(bl_result_from_posix_error(errno));
  }

  bl_file_close(self);
  self->handle = intptr_t(fd);

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_file_close(BLFileCore* self) noexcept {
  if (is_file_open(self)) {
    int fd = int(self->handle);
    int result = close(fd);

    // NOTE: Even when `close()` fails the handle cannot be used again as it could have already been reused. The
    // failure is just to inform the user that something failed and that there may be data-loss or handle leakage.
    self->handle = -1;

    if (BL_UNLIKELY(result != 0)) {
      return bl_make_error(bl_result_from_posix_error(errno));
    }
  }

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_file_seek(BLFileCore* self, int64_t offset, BLFileSeekType seek_type, int64_t* position_out) noexcept {
  *position_out = -1;

  int whence = 0;
  switch (seek_type) {
    case BL_FILE_SEEK_SET: whence = SEEK_SET; break;
    case BL_FILE_SEEK_CUR: whence = SEEK_CUR; break;
    case BL_FILE_SEEK_END: whence = SEEK_END; break;

    default:
      return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  if (!is_file_open(self)) {
    return bl_make_error(BL_ERROR_INVALID_HANDLE);
  }

  int fd = int(self->handle);
  int64_t result = BL_FILE64_API(lseek)(fd, offset, whence);

  if (result < 0) {
    int e = errno;

    // Returned when the file was not open for reading or writing.
    if (e == EBADF) {
      return bl_make_error(BL_ERROR_NOT_PERMITTED);
    }

    return bl_make_error(bl_result_from_posix_error(errno));
  }

  *position_out = result;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_file_read(BLFileCore* self, void* buffer, size_t n, size_t* bytes_read_out) noexcept {
  using SignedSizeT = std::make_signed_t<size_t>;

  if (!is_file_open(self)) {
    *bytes_read_out = 0;
    return bl_make_error(BL_ERROR_INVALID_HANDLE);
  }

  int fd = int(self->handle);
  size_t bytes_read = 0;

  for (;;) {
    SignedSizeT result = read(fd, buffer, n - bytes_read);
    if (result < 0) {
      int e = errno;
      *bytes_read_out = bytes_read;

      // Returned when the file was not open for reading.
      if (e == EBADF) {
        return bl_make_error(BL_ERROR_NOT_PERMITTED);
      }

      return bl_make_error(bl_result_from_posix_error(e));
    }
    else {
      bytes_read += size_t(result);
      if (bytes_read == n || result == 0) {
        break;
      }
    }
  }

  *bytes_read_out = bytes_read;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_file_write(BLFileCore* self, const void* buffer, size_t n, size_t* bytes_written_out) noexcept {
  using SignedSizeT = std::make_signed_t<size_t>;

  if (!is_file_open(self)) {
    *bytes_written_out = 0;
    return bl_make_error(BL_ERROR_INVALID_HANDLE);
  }

  int fd = int(self->handle);
  size_t bytes_written = 0;

  for (;;) {
    SignedSizeT result = write(fd, buffer, n - bytes_written);
    if (result < 0) {
      int e = errno;
      *bytes_written_out = bytes_written;

      // These are the two errors that would be returned if the file was open for read-only.
      if (e == EBADF || e == EINVAL) {
        return bl_make_error(BL_ERROR_NOT_PERMITTED);
      }

      return bl_make_error(bl_result_from_posix_error(e));
    }
    else {
      bytes_written += size_t(result);
      if (bytes_written == n || result == 0) {
        break;
      }
    }
  }

  *bytes_written_out = bytes_written;
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_file_truncate(BLFileCore* self, int64_t max_size) noexcept {
  if (!is_file_open(self)) {
    return bl_make_error(BL_ERROR_INVALID_HANDLE);
  }

  if (max_size < 0) {
    return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  int fd = int(self->handle);
  int result = BL_FILE64_API(ftruncate)(fd, max_size);

  if (result != 0) {
    int e = errno;

    // These are the two errors that would be returned if the file was open for read-only.
    if (e == EBADF || e == EINVAL) {
      return bl_make_error(BL_ERROR_NOT_PERMITTED);
    }

    // File was smaller than `max_size` - we don't consider this to be an error.
    if (e == EFBIG) {
      return BL_SUCCESS;
    }

    return bl_make_error(bl_result_from_posix_error(e));
  }
  else {
    return BL_SUCCESS;
  }
}

BL_API_IMPL BLResult bl_file_get_info(BLFileCore* self, BLFileInfo* info_out) noexcept {
  if (!is_file_open(self)) {
    *info_out = BLFileInfo{};
    return bl_make_error(BL_ERROR_INVALID_HANDLE);
  }

  int fd = int(self->handle);
  struct BL_FILE64_API(stat) s;

  if (BL_FILE64_API(fstat)(fd, &s) != 0) {
    *info_out = BLFileInfo{};
    return bl_make_error(bl_result_from_posix_error(errno));
  }

  return bl::FileSystem::file_info_from_stat(*info_out, s);
}

BL_API_IMPL BLResult bl_file_get_size(BLFileCore* self, uint64_t* file_size_out) noexcept {
  if (!is_file_open(self)) {
    *file_size_out = 0;
    return bl_make_error(BL_ERROR_INVALID_HANDLE);
  }

  int fd = int(self->handle);
  struct BL_FILE64_API(stat) s;

  if (BL_FILE64_API(fstat)(fd, &s) != 0) {
    *file_size_out = 0;
    return bl_make_error(bl_result_from_posix_error(errno));
  }

  *file_size_out = uint64_t(s.st_size);
  return BL_SUCCESS;
}

// BLFileSystem - API - POSIX Implementation
// =========================================

BL_API_IMPL BLResult bl_file_system_get_info(const char* file_name, BLFileInfo* info_out) noexcept {
  struct BL_FILE64_API(stat) s;

  if (BL_FILE64_API(stat)(file_name, &s) != 0) {
    *info_out = BLFileInfo{};
    return bl_make_error(bl_result_from_posix_error(errno));
  }

  return bl::FileSystem::file_info_from_stat(*info_out, s);
}
#endif

#if defined(_WIN32)

// BLFileMapping - Windows Implementation
// ======================================

BLResult BLFileMapping::map(BLFile& file, size_t size, uint32_t flags) noexcept {
  bl_unused(flags);

  if (!file.is_open())
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  DWORD map_protect = PAGE_READONLY;
  DWORD desired_access = FILE_MAP_READ;

  // Create a file mapping handle and map view of file into it.
#if defined(BL_PLATFORM_UWP)
  HANDLE file_mapping_handle = CreateFileMappingFromApp(
    (HANDLE)file.handle, // FileHandle
    nullptr,             // SecurityAttributes
    map_protect,         // PageProtection
    0,                   // MaximumSize
    nullptr              // Name
  );
#else
  HANDLE file_mapping_handle = CreateFileMappingW(
    (HANDLE)file.handle, // FileHandle
    nullptr,             // FileMappingAttributes
    map_protect,         // PageProtection
    0,                   // MaximumSizeHigh
    0,                   // MaximumSizeLow
    nullptr              // Name
  );
#endif

  if (file_mapping_handle == nullptr)
    return bl_make_error(bl_result_from_win_error(GetLastError()));

#if defined(BL_PLATFORM_UWP)
  void* data = MapViewOfFileFromApp(
    file_mapping_handle, // FileMappingHandle
    desired_access,      // DesiredAccess
    0,                   // FileOffset
    0                    // NumberOfBytesToMap
  );
#else
  void* data = MapViewOfFile(
    file_mapping_handle, // FileMappingHandle
    desired_access,      // DesiredAccess
    0,                   // FileOffsetHigh
    0,                   // FileOffsetLow
    0                    // NumberOfBytesToMap
  );
#endif

  if (!data) {
    BLResult result = bl_result_from_win_error(GetLastError());
    CloseHandle(file_mapping_handle);
    return bl_make_error(result);
  }

  // Succeeded, now is the time to change the content of `FileMapping`.
  unmap();

  _file_mapping_handle = file_mapping_handle;
  _data = data;
  _size = size;

  return BL_SUCCESS;
}

BLResult BLFileMapping::unmap() noexcept {
  if (is_empty())
    return BL_SUCCESS;

  BLResult result = BL_SUCCESS;
  DWORD err = 0;

  if (!UnmapViewOfFile(_data)) {
    err = GetLastError();
  }

  if (!CloseHandle(_file_mapping_handle) && !err) {
    err = GetLastError();
  }

  if (err) {
    result = bl_make_error(bl_result_from_win_error(err));
  }

  _file_mapping_handle = INVALID_HANDLE_VALUE;
  _data = nullptr;
  _size = 0;

  return result;
}

#else

// BLFileMapping - POSIX Implementation
// ====================================

BLResult BLFileMapping::map(BLFile& file, size_t size, uint32_t flags) noexcept {
  bl_unused(flags);

  if (!file.is_open())
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  int mmap_prot = PROT_READ;
  int mmap_flags = MAP_SHARED;

  // Create the mapping.
  void* data = mmap(nullptr, size, mmap_prot, mmap_flags, int(file.handle), 0);
  if (data == (void *)-1) {
    return bl_make_error(bl_result_from_posix_error(errno));
  }

  // Succeeded, now is the time to change the content of `BLFileMapping`.
  unmap();

  _data = data;
  _size = size;

  return BL_SUCCESS;
}

BLResult BLFileMapping::unmap() noexcept {
  if (is_empty()) {
    return BL_SUCCESS;
  }

  BLResult result = BL_SUCCESS;
  int unmap_status = munmap(_data, _size);

  // If error happened we must read `errno` now as a call to `close()` may
  // trash it. We prefer the first error instead of the last one.
  if (unmap_status != 0) {
    result = bl_make_error(bl_result_from_posix_error(errno));
  }

  _data = nullptr;
  _size = 0;

  return result;
}

#endif

// BLFileSystem - Memory Mapped File
// =================================

namespace bl {

static void BL_CDECL destroy_memory_mapped_file(void* impl, void* external_data, void* user_data) noexcept {
  bl_unused(external_data, user_data);

  BLFileMapping* impl_file_mapping = PtrOps::offset<BLFileMapping>(impl, sizeof(BLArrayImpl));
  bl_call_dtor(*impl_file_mapping);
}

static BLResult create_memory_mapped_file(BLArray<uint8_t>* dst, BLFile& file, size_t size) noexcept {
  // This condition must be handled before.
  BL_ASSERT(size != 0);

  BLFileMapping file_mapping;
  BL_PROPAGATE(file_mapping.map(file, size));

  BLObjectImplSize impl_size(sizeof(BLArrayImpl) + sizeof(BLFileMapping));
  uint32_t info = BLObjectInfo::pack_type_with_marker(BL_OBJECT_TYPE_ARRAY_UINT8);

  BLArrayCore newO;
  BL_PROPAGATE(ObjectInternal::alloc_impl_external_t<BLArrayImpl>(&newO, BLObjectInfo{info}, impl_size, true, destroy_memory_mapped_file, nullptr));

  BLArrayImpl* impl = bl::ArrayInternal::get_impl(&newO);
  impl->data = file_mapping.data<void>();
  impl->size = size;
  impl->capacity = size;

  BLFileMapping* impl_file_mapping = PtrOps::offset<BLFileMapping>(impl, sizeof(BLArrayImpl));
  bl_call_ctor(*impl_file_mapping, BLInternal::move(file_mapping));

  return bl::ArrayInternal::replace_instance(dst, &newO);
}

} // {bl}

// BLFileSystem - Read & Write File
// ================================

static constexpr uint32_t kSmallFileSizeThreshold = 16 * 1024;

BL_API_IMPL BLResult bl_file_system_read_file(const char* file_name, BLArrayCore* dst_, size_t max_size, BLFileReadFlags read_flags) noexcept {
  if (BL_UNLIKELY(dst_->_d.raw_type() != BL_OBJECT_TYPE_ARRAY_UINT8))
    return bl_make_error(BL_ERROR_INVALID_STATE);

  BLArray<uint8_t>& dst = dst_->dcast<BLArray<uint8_t>>();
  dst.clear();

  BLFile file;
  BL_PROPAGATE(file.open(file_name, BL_FILE_OPEN_READ));

  // TODO: This won't read `stat` files.
  uint64_t size64;
  BL_PROPAGATE(file.get_size(&size64));

  if (size64 == 0) {
    return BL_SUCCESS;
  }

  if (max_size) {
    size64 = bl_min<uint64_t>(size64, max_size);
  }

  if (bl_runtime_is_32bit() && BL_UNLIKELY(size64 >= uint64_t(SIZE_MAX))) {
    return bl_make_error(BL_ERROR_FILE_TOO_LARGE);
  }

  size_t size = size_t(size64);

  // Use memory mapped file IO if enabled.
  if (read_flags & BL_FILE_READ_MMAP_ENABLED) {
    bool is_small = size < kSmallFileSizeThreshold;
    if (!(read_flags & BL_FILE_READ_MMAP_AVOID_SMALL) || !is_small) {
      BLResult result = bl::create_memory_mapped_file(&dst, file, size);
      if (result == BL_SUCCESS)
        return result;

      if (read_flags & BL_FILE_READ_MMAP_NO_FALLBACK)
        return result;
    }
  }

  uint8_t* data;
  BL_PROPAGATE(dst.modify_op(BL_MODIFY_OP_ASSIGN_FIT, size, &data));

  size_t bytes_read;
  BLResult result = file.read(data, size, &bytes_read);
  dst.resize(bytes_read, 0);
  return result;
}

BL_API_IMPL BLResult bl_file_system_write_file(const char* file_name, const void* data, size_t size, size_t* bytes_written_out) noexcept {
  *bytes_written_out = 0;

  BLFile file;
  BL_PROPAGATE(file.open(file_name, BLFileOpenFlags(BL_FILE_OPEN_WRITE | BL_FILE_OPEN_CREATE | BL_FILE_OPEN_TRUNCATE)));
  return size ? file.write(data, size , bytes_written_out) : BL_SUCCESS;
}
