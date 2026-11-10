// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_ENVIRONMENT_H_INCLUDED
#define ASMJIT_CORE_ENVIRONMENT_H_INCLUDED

#include <asmjit/core/archtraits.h>

#if defined(__APPLE__)
  #include <TargetConditionals.h>
#endif

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

//! Vendor.
//!
//! \note AsmJit doesn't use vendor information at the moment. It's provided for future use, if required.
enum class Vendor : uint8_t {
  //! Unknown or uninitialized platform vendor.
  kUnknown = 0,

  //! Maximum value of `Vendor`.
  kMaxValue = kUnknown,

  //! Platform vendor detected at compile-time.
  kHost =
#if defined(_DOXYGEN)
    DETECTED_AT_COMPILE_TIME
#else
    kUnknown
#endif
};

//! Platform - runtime environment or operating system.
enum class Platform : uint8_t {
  //! Unknown or uninitialized platform.
  kUnknown = 0,

  //! Windows OS.
  kWindows,

  //! Other platform that is not Windows, most likely POSIX based.
  kOther,

  //! Linux OS.
  kLinux,
  //! GNU/Hurd OS.
  kHurd,

  //! FreeBSD OS.
  kFreeBSD,
  //! OpenBSD OS.
  kOpenBSD,
  //! NetBSD OS.
  kNetBSD,
  //! DragonFly BSD OS.
  kDragonFlyBSD,

  //! Haiku OS.
  kHaiku,

  //! Apple OSX.
  kOSX,
  //! Apple iOS.
  kIOS,
  //! Apple TVOS.
  kTVOS,
  //! Apple WatchOS.
  kWatchOS,

  //! Emscripten platform.
  kEmscripten,

  //! Maximum value of `Platform`.
  kMaxValue = kEmscripten,

  //! Platform detected at compile-time (platform of the host).
  kHost =
#if defined(_DOXYGEN)
    DETECTED_AT_COMPILE_TIME
#elif defined(__EMSCRIPTEN__)
    kEmscripten
#elif defined(_WIN32)
    kWindows
#elif defined(__linux__)
    kLinux
#elif defined(__gnu_hurd__)
    kHurd
#elif defined(__FreeBSD__)
    kFreeBSD
#elif defined(__OpenBSD__)
    kOpenBSD
#elif defined(__NetBSD__)
    kNetBSD
#elif defined(__DragonFly__)
    kDragonFlyBSD
#elif defined(__HAIKU__)
    kHaiku
#elif defined(__APPLE__) && TARGET_OS_OSX
    kOSX
#elif defined(__APPLE__) && TARGET_OS_TV
    kTVOS
#elif defined(__APPLE__) && TARGET_OS_WATCH
    kWatchOS
#elif defined(__APPLE__) && TARGET_OS_IPHONE
    kIOS
#else
    kOther
#endif
};

//! Platform ABI (application binary interface).
enum class PlatformABI : uint8_t {
  //! Unknown or uninitialized environment.
  kUnknown = 0,
  //! Microsoft ABI.
  kMSVC,
  //! GNU ABI.
  kGNU,
  //! Android Environment / ABI.
  kAndroid,
  //! Cygwin ABI.
  kCygwin,
  //! Darwin ABI.
  kDarwin,

  //! Maximum value of `PlatformABI`.
  kMaxValue,

  //! Host ABI detected at compile-time.
  kHost =
#if defined(_DOXYGEN)
    DETECTED_AT_COMPILE_TIME
#elif defined(_MSC_VER)
    kMSVC
#elif defined(__CYGWIN__)
    kCygwin
#elif defined(__MINGW32__) || defined(__GLIBC__)
    kGNU
#elif defined(__ANDROID__)
    kAndroid
#elif defined(__APPLE__)
    kDarwin
#else
    kUnknown
#endif
};

//! Floating point ABI (ARM).
enum class FloatABI : uint8_t {
  kHardFloat = 0,
  kSoftFloat,

  kHost =
#if ASMJIT_ARCH_ARM == 32 && defined(__SOFTFP__)
  kSoftFloat
#else
  kHardFloat
#endif
};

//! Object format.
//!
//! \note AsmJit doesn't really use anything except \ref ObjectFormat::kUnknown and \ref ObjectFormat::kJIT at
//! the moment. Object file formats are provided for future extensibility and a possibility to generate object
//! files at some point.
enum class ObjectFormat : uint8_t {
  //! Unknown or uninitialized object format.
  kUnknown = 0,

  //! JIT code generation object, most likely \ref JitRuntime or a custom
  //! \ref Target implementation.
  kJIT,

  //! Executable and linkable format (ELF).
  kELF,
  //! Common object file format.
  kCOFF,
  //! Extended COFF object format.
  kXCOFF,
  //! Mach object file format.
  kMachO,

  //! Maximum value of `ObjectFormat`.
  kMaxValue
};

//! Represents an environment, which is usually related to a \ref Target.
//!
//! Environment has usually an 'arch-subarch-vendor-os-abi' format, which is sometimes called "Triple" (historically
//! it used to be 3 only parts) or "Tuple", which is a convention used by Debian Linux.
//!
//! AsmJit doesn't support all possible combinations or architectures and ABIs, however, it models the environment
//! similarly to other compilers for future extensibility.
class Environment {
public:
  //! \name Members
  //! \{

  //! Architecture.
  Arch _arch = Arch::kUnknown;
  //! Sub-architecture type.
  SubArch _sub_arch = SubArch::kUnknown;
  //! Vendor type.
  Vendor _vendor = Vendor::kUnknown;
  //! Platform.
  Platform _platform = Platform::kUnknown;
  //! Platform ABI.
  PlatformABI _platform_abi = PlatformABI::kUnknown;
  //! Object format.
  ObjectFormat _object_format = ObjectFormat::kUnknown;
  //! Floating point ABI.
  FloatABI _float_abi = FloatABI::kHardFloat;
  //! Reserved for future use, must be zero.
  uint8_t _reserved = 0;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a default initialized environment (all values either unknown or set to safe defaults).
  ASMJIT_INLINE_CONSTEXPR Environment() noexcept = default;
  //! Creates a copy of `other` instance.
  ASMJIT_INLINE_CONSTEXPR Environment(const Environment& other) noexcept = default;

  //! Creates \ref Environment initialized to `arch`, `sub_arch`, `vendor`, `platform`, `platform_abi`, `object_format`,
  //! and `float_abi`.
  ASMJIT_INLINE_CONSTEXPR explicit Environment(
    Arch arch,
    SubArch sub_arch = SubArch::kUnknown,
    Vendor vendor = Vendor::kUnknown,
    Platform platform = Platform::kUnknown,
    PlatformABI platform_abi = PlatformABI::kUnknown,
    ObjectFormat object_format = ObjectFormat::kUnknown,
    FloatABI float_abi = FloatABI::kHardFloat) noexcept
    : _arch(arch),
      _sub_arch(sub_arch),
      _vendor(vendor),
      _platform(platform),
      _platform_abi(platform_abi),
      _object_format(object_format),
      _float_abi(float_abi) {}

  //! Returns the host environment constructed from preprocessor macros defined by the compiler.
  //!
  //! The returned environment should precisely match the target host architecture, sub-architecture, platform,
  //! and ABI.
  static ASMJIT_INLINE_CONSTEXPR Environment host() noexcept {
    return Environment(Arch::kHost, SubArch::kHost, Vendor::kHost, Platform::kHost, PlatformABI::kHost, ObjectFormat::kUnknown, FloatABI::kHost);
  }

  //! \}

  //! \name Overloaded Operators
  //! \{

  ASMJIT_INLINE_NODEBUG Environment& operator=(const Environment& other) noexcept = default;

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator==(const Environment& other) const noexcept { return equals(other); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator!=(const Environment& other) const noexcept { return !equals(other); }

  //! \}

  //! \name Accessors
  //! \{

  //! Tests whether the environment is not set up.
  //!
  //! Returns true if all members are zero, and thus unknown.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept {
    // Unfortunately compilers won't optimize fields are checked one by one...
    return _packed() == 0;
  }

  //! Tests whether the environment is initialized, which means it must have
  //! a valid architecture.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_initialized() const noexcept {
    return _arch != Arch::kUnknown;
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t _packed() const noexcept {
    uint64_t x;
    memcpy(&x, this, 8);
    return x;
  }

  //! Resets all members of the environment to zero / unknown.
  ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = Environment{}; }

  //! Tests whether this environment is equal to `other`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool equals(const Environment& other) const noexcept { return _packed() == other._packed(); }

  //! Returns the architecture.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arch arch() const noexcept { return _arch; }

  //! Returns the sub-architecture.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG SubArch sub_arch() const noexcept { return _sub_arch; }

  //! Returns vendor.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Vendor vendor() const noexcept { return _vendor; }

  //! Returns target's platform or operating system.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Platform platform() const noexcept { return _platform; }

  //! Returns target's ABI.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG PlatformABI platform_abi() const noexcept { return _platform_abi; }

  //! Returns target's object format.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ObjectFormat object_format() const noexcept { return _object_format; }

  //! Returns floating point ABI.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FloatABI float_abi() const noexcept { return _float_abi; }

  //! Initializes \ref Environment to `arch`, `sub_arch`, `vendor`, `platform`, `platform_abi`, `object_format`,
  //! and `float_abi`.
  inline void init(
    Arch arch,
    SubArch sub_arch = SubArch::kUnknown,
    Vendor vendor = Vendor::kUnknown,
    Platform platform = Platform::kUnknown,
    PlatformABI platform_abi = PlatformABI::kUnknown,
    ObjectFormat object_format = ObjectFormat::kUnknown,
    FloatABI float_abi = FloatABI::kHardFloat) noexcept {

    _arch = arch;
    _sub_arch = sub_arch;
    _vendor = vendor;
    _platform = platform;
    _platform_abi = platform_abi;
    _object_format = object_format;
    _float_abi = float_abi;
    _reserved = 0;
  }

  //! Tests whether this environment describes a 32-bit X86.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_arch_x86() const noexcept { return _arch == Arch::kX86; }

  //! Tests whether this environment describes a 64-bit X86.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_arch_x64() const noexcept { return _arch == Arch::kX64; }

  //! Tests whether this environment describes a 32-bit ARM.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_arch_arm() const noexcept { return is_arch_arm(_arch); }

  //! Tests whether this environment describes a 32-bit ARM in THUMB mode.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_arch_thumb() const noexcept { return is_arch_thumb(_arch); }

  //! Tests whether this environment describes a 64-bit X86.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_arch_aarch64() const noexcept { return is_arch_aarch64(_arch); }

  //! Tests whether this environment describes a 32-bit MIPS.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_arch_mips32() const noexcept { return is_arch_mips32(_arch); }

  //! Tests whether this environment describes a 64-bit MIPS.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_arch_mips64() const noexcept { return is_arch_mips64(_arch); }

  //! Tests whether this environment describes a 32-bit RISC-V.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_arch_riscv32() const noexcept { return _arch == Arch::kRISCV32; }

  //! Tests whether this environment describes a 64-bit RISC-V.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_arch_riscv64() const noexcept { return _arch == Arch::kRISCV64; }

  //! Tests whether the architecture is 32-bit.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_32bit() const noexcept { return is_32bit(_arch); }

  //! Tests whether the architecture is 64-bit.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_64bit() const noexcept { return is_64bit(_arch); }

  //! Tests whether the architecture is little endian.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_little_endian() const noexcept { return is_little_endian(_arch); }

  //! Tests whether the architecture is big endian.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_big_endian() const noexcept { return is_big_endian(_arch); }

  //! Tests whether this architecture is of X86 family.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_family_x86() const noexcept { return is_family_x86(_arch); }

  //! Tests whether this architecture family is ARM, THUMB, or AArch64.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_family_arm() const noexcept { return is_family_arm(_arch); }

  //! Tests whether this architecture family is AArch32 (ARM or THUMB).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_family_aarch32() const noexcept { return is_family_aarch32(_arch); }

  //! Tests whether this architecture family is AArch64.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_family_aarch64() const noexcept { return is_family_aarch64(_arch); }

  //! Tests whether this architecture family is MISP or MIPS64.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_family_mips() const noexcept { return is_family_mips(_arch); }

  //! Tests whether this architecture family is RISC-V (both 32-bit and 64-bit).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_family_riscv() const noexcept { return is_family_riscv(_arch); }

  //! Tests whether the environment platform is Windows.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_platform_windows() const noexcept { return _platform == Platform::kWindows; }

  //! Tests whether the environment platform is Linux.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_platform_linux() const noexcept { return _platform == Platform::kLinux; }

  //! Tests whether the environment platform is Hurd.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_platform_hurd() const noexcept { return _platform == Platform::kHurd; }

  //! Tests whether the environment platform is Haiku.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_platform_haiku() const noexcept { return _platform == Platform::kHaiku; }

  //! Tests whether the environment platform is any BSD.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_platform_bsd() const noexcept {
    return _platform == Platform::kFreeBSD ||
           _platform == Platform::kOpenBSD ||
           _platform == Platform::kNetBSD ||
           _platform == Platform::kDragonFlyBSD;
  }

  //! Tests whether the environment platform is any Apple platform (OSX, iOS, TVOS, WatchOS).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_platform_apple() const noexcept {
    return _platform == Platform::kOSX ||
           _platform == Platform::kIOS ||
           _platform == Platform::kTVOS ||
           _platform == Platform::kWatchOS;
  }

  //! Tests whether the ABI is MSVC.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_msvc_abi() const noexcept { return _platform_abi == PlatformABI::kMSVC; }

  //! Tests whether the ABI is GNU.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_gnu_abi() const noexcept { return _platform_abi == PlatformABI::kGNU; }

  //! Tests whether the ABI is GNU.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_darwin_abi() const noexcept { return _platform_abi == PlatformABI::kDarwin; }

  //! Returns a calculated stack alignment for this environment.
  [[nodiscard]]
  ASMJIT_API uint32_t stack_alignment() const noexcept;

  //! Returns a native register size of this architecture.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t register_size() const noexcept { return reg_size_of_arch(_arch); }

  //! Sets the architecture to `arch`.
  ASMJIT_INLINE_NODEBUG void set_arch(Arch arch) noexcept { _arch = arch; }
  //! Sets the sub-architecture to `sub_arch`.
  ASMJIT_INLINE_NODEBUG void set_sub_arch(SubArch sub_arch) noexcept { _sub_arch = sub_arch; }
  //! Sets the vendor to `vendor`.
  ASMJIT_INLINE_NODEBUG void set_vendor(Vendor vendor) noexcept { _vendor = vendor; }
  //! Sets the platform to `platform`.
  ASMJIT_INLINE_NODEBUG void set_platform(Platform platform) noexcept { _platform = platform; }
  //! Sets the ABI to `platform_abi`.
  ASMJIT_INLINE_NODEBUG void set_platform_abi(PlatformABI platform_abi) noexcept { _platform_abi = platform_abi; }
  //! Sets the object format to `object_format`.
  ASMJIT_INLINE_NODEBUG void set_object_format(ObjectFormat object_format) noexcept { _object_format = object_format; }

  //! Sets floating point ABI to `float_abi`.
  ASMJIT_INLINE_NODEBUG void set_float_abi(FloatABI float_abi) noexcept { _float_abi = float_abi; }

  //! \}

  //! \name Static Utilities
  //! \{

  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_defined_arch(Arch arch) noexcept {
    return uint32_t(arch) <= uint32_t(Arch::kMaxValue);
  }

  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_valid_arch(Arch arch) noexcept {
    return arch != Arch::kUnknown && uint32_t(arch) <= uint32_t(Arch::kMaxValue);
  }

  //! Tests whether the given architecture `arch` is 32-bit.
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_32bit(Arch arch) noexcept {
    return (uint32_t(arch) & uint32_t(Arch::k32BitMask)) == uint32_t(Arch::k32BitMask);
  }

  //! Tests whether the given architecture `arch` is 64-bit.
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_64bit(Arch arch) noexcept {
    return (uint32_t(arch) & uint32_t(Arch::k32BitMask)) == 0;
  }

  //! Tests whether the given architecture `arch` is little endian.
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_little_endian(Arch arch) noexcept {
    return uint32_t(arch) < uint32_t(Arch::kBigEndian);
  }

  //! Tests whether the given architecture `arch` is big endian.
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_big_endian(Arch arch) noexcept {
    return uint32_t(arch) >= uint32_t(Arch::kBigEndian);
  }

  //! Tests whether the given architecture is Thumb or Thumb_BE.
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_arch_thumb(Arch arch) noexcept {
    return arch == Arch::kThumb || arch == Arch::kThumb_BE;
  }

  //! Tests whether the given architecture is ARM or ARM_BE.
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_arch_arm(Arch arch) noexcept {
    return arch == Arch::kARM || arch == Arch::kARM_BE;
  }

  //! Tests whether the given architecture is AArch64 or AArch64_BE.
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_arch_aarch64(Arch arch) noexcept {
    return arch == Arch::kAArch64 || arch == Arch::kAArch64_BE;
  }

  //! Tests whether the given architecture is MIPS32_LE or MIPS32_BE.
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_arch_mips32(Arch arch) noexcept {
    return arch == Arch::kMIPS32_LE || arch == Arch::kMIPS32_BE;
  }

  //! Tests whether the given architecture is MIPS64_LE or MIPS64_BE.
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_arch_mips64(Arch arch) noexcept {
    return arch == Arch::kMIPS64_LE || arch == Arch::kMIPS64_BE;
  }

  //! Tests whether the given architecture family is X86 or X64.
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_family_x86(Arch arch) noexcept {
    return arch == Arch::kX86 || arch == Arch::kX64;
  }

  //! Tests whether the given architecture family is AArch32 (ARM or THUMB).
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_family_aarch32(Arch arch) noexcept {
    return is_arch_arm(arch) || is_arch_thumb(arch);
  }

  //! Tests whether the given architecture family is AArch64.
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_family_aarch64(Arch arch) noexcept {
    return is_arch_aarch64(arch);
  }

  //! Tests whether the given architecture family is ARM, THUMB, or AArch64.
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_family_arm(Arch arch) noexcept {
    return is_family_aarch32(arch) || is_family_aarch64(arch);
  }

  //! Tests whether the given architecture family is MIPS or MIPS64.
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_family_mips(Arch arch) noexcept {
    return is_arch_mips32(arch) || is_arch_mips64(arch);
  }

  //! Tests whether the given architecture family is RISC-V (both 32-bit and 64-bit).
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_family_riscv(Arch arch) noexcept {
    return arch == Arch::kRISCV32 || arch == Arch::kRISCV64;
  }

  //! Returns a native general purpose register size from the given architecture.
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG uint32_t reg_size_of_arch(Arch arch) noexcept {
    return is_32bit(arch) ? 4u : 8u;
  }

  //! \}
};

static_assert(sizeof(Environment) == 8,
              "Environment must occupy exactly 8 bytes.");

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_ENVIRONMENT_H_INCLUDED
