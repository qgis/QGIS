// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/core/misc_p.h>
#include <asmjit/core/type.h>

ASMJIT_BEGIN_NAMESPACE

namespace TypeUtils {

template<uint32_t Index>
struct ScalarOfTypeId {
  static inline constexpr uint32_t kTypeId = uint32_t(
    is_scalar(TypeId(Index)) ? TypeId(Index) :
    is_mask8 (TypeId(Index)) ? TypeId::kUInt8 :
    is_mask16(TypeId(Index)) ? TypeId::kUInt16 :
    is_mask32(TypeId(Index)) ? TypeId::kUInt32 :
    is_mask64(TypeId(Index)) ? TypeId::kUInt64 :
    is_mmx32 (TypeId(Index)) ? TypeId::kUInt32 :
    is_mmx64 (TypeId(Index)) ? TypeId::kUInt64 :
    is_vec32 (TypeId(Index)) ? TypeId((Index - uint32_t(TypeId::_kVec32Start ) + uint32_t(TypeId::kInt8)) & 0xFF) :
    is_vec64 (TypeId(Index)) ? TypeId((Index - uint32_t(TypeId::_kVec64Start ) + uint32_t(TypeId::kInt8)) & 0xFF) :
    is_vec128(TypeId(Index)) ? TypeId((Index - uint32_t(TypeId::_kVec128Start) + uint32_t(TypeId::kInt8)) & 0xFF) :
    is_vec256(TypeId(Index)) ? TypeId((Index - uint32_t(TypeId::_kVec256Start) + uint32_t(TypeId::kInt8)) & 0xFF) :
    is_vec512(TypeId(Index)) ? TypeId((Index - uint32_t(TypeId::_kVec512Start) + uint32_t(TypeId::kInt8)) & 0xFF) : TypeId::kVoid);
};

template<uint32_t Index>
struct SizeOfTypeId {
  static inline constexpr uint32_t kTypeSize =
    is_int8   (TypeId(Index)) ?  1 :
    is_uint8  (TypeId(Index)) ?  1 :
    is_int16  (TypeId(Index)) ?  2 :
    is_uint16 (TypeId(Index)) ?  2 :
    is_int32  (TypeId(Index)) ?  4 :
    is_uint32 (TypeId(Index)) ?  4 :
    is_int64  (TypeId(Index)) ?  8 :
    is_uint64 (TypeId(Index)) ?  8 :
    is_float32(TypeId(Index)) ?  4 :
    is_float64(TypeId(Index)) ?  8 :
    is_float80(TypeId(Index)) ? 10 :
    is_mask8  (TypeId(Index)) ?  1 :
    is_mask16 (TypeId(Index)) ?  2 :
    is_mask32 (TypeId(Index)) ?  4 :
    is_mask64 (TypeId(Index)) ?  8 :
    is_mmx32  (TypeId(Index)) ?  4 :
    is_mmx64  (TypeId(Index)) ?  8 :
    is_vec32  (TypeId(Index)) ?  4 :
    is_vec64  (TypeId(Index)) ?  8 :
    is_vec128 (TypeId(Index)) ? 16 :
    is_vec256 (TypeId(Index)) ? 32 :
    is_vec512 (TypeId(Index)) ? 64 : 0;
};

const TypeData _type_data = {
  #define VALUE(x) TypeId(ScalarOfTypeId<x>::kTypeId)
  { ASMJIT_LOOKUP_TABLE_256(VALUE, 0) },
  #undef VALUE

  #define VALUE(x) SizeOfTypeId<x>::kTypeSize
  { ASMJIT_LOOKUP_TABLE_256(VALUE, 0) }
  #undef VALUE
};

} // {TypeUtils}

ASMJIT_END_NAMESPACE
