// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/core/globals.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

// DebugUtils - Error As String
// ============================

ASMJIT_FAVOR_SIZE const char* DebugUtils::error_as_string(Error err) noexcept {
#ifndef ASMJIT_NO_TEXT
  // @EnumStringBegin{"enum": "Error", "output": "error_string", "strip": "k"}@
  static const char error_string_data[] =
    "Ok\0"
    "OutOfMemory\0"
    "InvalidArgument\0"
    "InvalidState\0"
    "InvalidArch\0"
    "NotInitialized\0"
    "AlreadyInitialized\0"
    "FeatureNotEnabled\0"
    "TooManyHandles\0"
    "TooLarge\0"
    "NoCodeGenerated\0"
    "InvalidDirective\0"
    "InvalidLabel\0"
    "TooManyLabels\0"
    "LabelAlreadyBound\0"
    "LabelAlreadyDefined\0"
    "LabelNameTooLong\0"
    "InvalidLabelName\0"
    "InvalidParentLabel\0"
    "InvalidSection\0"
    "TooManySections\0"
    "InvalidSectionName\0"
    "TooManyRelocations\0"
    "InvalidRelocEntry\0"
    "RelocOffsetOutOfRange\0"
    "InvalidAssignment\0"
    "InvalidInstruction\0"
    "InvalidRegType\0"
    "InvalidRegGroup\0"
    "InvalidPhysId\0"
    "InvalidVirtId\0"
    "InvalidElementIndex\0"
    "InvalidPrefixCombination\0"
    "InvalidLockPrefix\0"
    "InvalidXAcquirePrefix\0"
    "InvalidXReleasePrefix\0"
    "InvalidRepPrefix\0"
    "InvalidRexPrefix\0"
    "InvalidExtraReg\0"
    "InvalidKMaskUse\0"
    "InvalidKZeroUse\0"
    "InvalidBroadcast\0"
    "InvalidEROrSAE\0"
    "InvalidAddress\0"
    "InvalidAddressIndex\0"
    "InvalidAddressScale\0"
    "InvalidAddress64Bit\0"
    "InvalidAddress64BitZeroExtension\0"
    "InvalidDisplacement\0"
    "InvalidSegment\0"
    "InvalidImmediate\0"
    "InvalidOperandSize\0"
    "AmbiguousOperandSize\0"
    "OperandSizeMismatch\0"
    "InvalidOption\0"
    "OptionAlreadyDefined\0"
    "InvalidTypeId\0"
    "InvalidUseOfGpbHi\0"
    "InvalidUseOfGpq\0"
    "InvalidUseOfF80\0"
    "NotConsecutiveRegs\0"
    "ConsecutiveRegsAllocation\0"
    "IllegalVirtReg\0"
    "TooManyVirtRegs\0"
    "NoMorePhysRegs\0"
    "OverlappedRegs\0"
    "OverlappingStackRegWithRegArg\0"
    "ExpressionLabelNotBound\0"
    "ExpressionOverflow\0"
    "FailedToOpenAnonymousMemory\0"
    "FailedToOpenFile\0"
    "ProtectionFailure\0"
    "<Unknown>\0";

  static const uint16_t error_string_index[] = {
    0, 3, 15, 31, 44, 56, 71, 90, 108, 123, 132, 148, 165, 178, 192, 210, 230,
    247, 264, 283, 298, 314, 333, 352, 370, 392, 410, 429, 444, 460, 474, 488,
    508, 533, 551, 573, 595, 612, 629, 645, 661, 677, 694, 709, 724, 744, 764,
    784, 817, 837, 852, 869, 888, 909, 929, 943, 964, 978, 996, 1012, 1028, 1047,
    1073, 1088, 1104, 1119, 1134, 1164, 1188, 1207, 1235, 1252, 1270
  };
  // @EnumStringEnd@

  return error_string_data + error_string_index[Support::min(uint32_t(err), uint32_t(Error::kMaxValue) + 1u)];
#else
  Support::maybe_unused(err);
  static const char no_message[] = "";
  return no_message;
#endif
}

// DebugUtils - Debug Output
// =========================

ASMJIT_FAVOR_SIZE void DebugUtils::debug_output(const char* str) noexcept {
#if defined(_WIN32)
  ::OutputDebugStringA(str);
#else
  ::fputs(str, stderr);
#endif
}

// DebugUtils - Fatal Errors
// =========================

ASMJIT_FAVOR_SIZE void DebugUtils::assertion_failure(const char* file, int line, const char* msg) noexcept {
  char str[1024];

  snprintf(str, 1024,
    "[asmjit] Assertion failed at %s (line %d):\n"
    "[asmjit] %s\n", file, line, msg);

  debug_output(str);
  ::abort();
}

ASMJIT_END_NAMESPACE
