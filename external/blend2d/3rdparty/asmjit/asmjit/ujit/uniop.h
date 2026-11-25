// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_UJIT_UNIOP_H_INCLUDED
#define ASMJIT_UJIT_UNIOP_H_INCLUDED

#include <asmjit/ujit/ujitbase.h>

#if !defined(ASMJIT_NO_UJIT)

ASMJIT_BEGIN_SUB_NAMESPACE(ujit)

//! \addtogroup asmjit_ujit
//! \{

//! Instruction that can be used by \ref UniCondition.
enum class UniOpCond : uint32_t {
  kAssignAnd,                 //!< Assign-and `a &= b`.
  kAssignOr,                  //!< Assign-or  `a |= b`.
  kAssignXor,                 //!< Assign-xor `a ^= b`.
  kAssignAdd,                 //!< Assign-add `a += b`.
  kAssignSub,                 //!< Assign-sub `a -= b`.
  kAssignShr,                 //!< Assign-shr `a >>= b`.
  kTest,                      //!< Test       `a & b`.
  kBitTest,                   //!< Bit-test   `a & (1 << b)`.
  kCompare,                   //!< Compare    `a <=> b`.

  kMaxValue = kCompare
};

//! Instruction with a single memory operand.
enum class UniOpM : uint32_t {
  kPrefetch,                  //!< Explicitly prefetch memory for reading (can be implemented as NOP).
  kStoreZeroReg,              //!< Store zero (data-width depends on register size).
  kStoreZeroU8,               //!< Store zero (8-bit).
  kStoreZeroU16,              //!< Store zero (16-bit).
  kStoreZeroU32,              //!< Store zero (32-bit).
  kStoreZeroU64               //!< Store zero (64-bit).
};

//! Instruction with `[reg, mem]` operands.
enum class UniOpRM : uint32_t {
  kLoadReg,                   //!< N-bit load (the size depends on the register size).
  kLoadI8,                    //!< 8-bit load, sign extended.
  kLoadU8,                    //!< 8-bit load, zero extended.
  kLoadI16,                   //!< 16-bit load, sign extended.
  kLoadU16,                   //!< 16-bit load, zero extended.
  kLoadI32,                   //!< 32-bit load, sign extended.
  kLoadU32,                   //!< 32-bit load, zero extended.
  kLoadI64,                   //!< 64-bit load.
  kLoadU64,                   //!< 64-bit load.
  kLoadMergeU8,               //!< 8-bit load and merge.
  kLoadShiftU8,               //!< 8-bit load, shift, and merge.
  kLoadMergeU16,              //!< 16-bit load and merge.
  kLoadShiftU16               //!< 16-bit load, shift, and merge.
};

//! Instruction with `[mem, reg]` operands.
enum class UniOpMR : uint32_t {
  kStoreReg,                  //!< N-bit store (the size depends on the register size).
  kStoreU8,                   //!< 8-bit store.
  kStoreU16,                  //!< 16-bit store.
  kStoreU32,                  //!< 32-bit store.
  kStoreU64,                  //!< 64-bit store.
  kAddReg,                    //!< N-bit load+add+store (the size depends on the register size).
  kAddU8,                     //!< 8-bit load+add+store.
  kAddU16,                    //!< 16-bit load+add+store.
  kAddU32,                    //!< 32-bit load+add+store.
  kAddU64                     //!< 64-bit load+add+store.
};

//! Instruction with `[reg, reg]` operands.
//!
//! Arithmetic operations having 2 operands (dst, src).
//!
//! \note For convenience, the second operand can be register, memory, or immediate value.
enum class UniOpRR : uint32_t {
  kAbs,                       //!< Absolute value of a signed integer - `dst = abs(src)`.
  kNeg,                       //!< Arithmetic negation - `dst = -src` (`dst = ~src + 1`).
  kNot,                       //!< Bitwise-not - `dst = ~src`.
  kBSwap,                     //!< Byteswap - `dst = bswap(src)`.
  kCLZ,                       //!< Count leading zeros - `dst = clz(src)`.
  kCTZ,                       //!< Count trailing zeros - `dst = ctz(src)`.
  kReflect,                   //!< Integer reflection.

  kMaxValue = kReflect
};

//! Instruction with `[reg, reg, reg]` operands.
//!
//! Arithmetic operation having 3 operands (dst, src1, src2).
//!
//! \note For convenience, the third operand can be register, memory, or immediate value.
enum class UniOpRRR : uint32_t {
  kAnd,                       //!< Bitwise AND `dst = src1 & src2`.
  kOr,                        //!< Bitwise OR  `dst = src1 | src2`.
  kXor,                       //!< Bitwise XOR `dst = src1 ^ src2`.
  kBic,                       //!< Bitwise BIC `dst = src1 & ~src2`.
  kAdd,                       //!< Add `dst = src1 + src2`.
  kSub,                       //!< Subtract `dst = src1 - src2`.
  kMul,                       //!< Multiply `dst = src1 * src2`.
  kUDiv,                      //!< Unsigned divide `dst = src1 / src2`.
  kUMod,                      //!< Unsigned modulo `dst = src1 & src2`.
  kSMin,                      //!< Signed minimum `dst = smin(src1, src2)`.
  kSMax,                      //!< Signed maximum `dst = smax(src1, src2)`.
  kUMin,                      //!< Unsigned minimum `dst = umin(src1, src2)`.
  kUMax,                      //!< Unsigned maximum `dst = umax(src1, src2)`.
  kSll,                       //!< Shift left logical `dst = src1 << src2`.
  kSrl,                       //!< Shift left logical `dst = src1 >> src2`.
  kSra,                       //!< Shift left logical `dst = sra(src1, src2)`.
  kRol,                       //!< Rotate left `dst = (src1 << src2) | (src1 >> (N_BITS - src2))`.
  kRor,                       //!< Rotate right `dst = (src1 >> src2) | (src1 << (N_BITS - src2))`.
  kSBound,                    //!< Signed bounds.

  kMaxValue = kSBound
};

//! Instruction with `[vec, reg]` operands.
//!
//! Describes instructions where general-purpose is either moved, converted, or inserted to
//! a vector register.
enum class UniOpVR : uint32_t {
  kMov,                       //!< N-bit move into a vector register (the size depends on source register width).
  kMovU32,                    //!< 32-bit move into a vector register.
  kMovU64,                    //!< 64-bit move into a vector register.
  kInsertU8,                  //!< 8-bit insertion into a vector register.
  kInsertU16,                 //!< 16-bit insertion into a vector register.
  kInsertU32,                 //!< 32-bit insertion into a vector register.
  kInsertU64,                 //!< 64-bit insertion into a vector register.
  kExtractU8,                 //!< 8-bit extraction from a vector register.
  kExtractU16,                //!< 16-bit extraction from a vector register.
  kExtractU32,                //!< 32-bit extraction from a vector register.
  kExtractU64,                //!< 64-bit extraction from a vector register.
  kCvtIntToF32,               //!< Int to float32 conversion.
  kCvtIntToF64,               //!< Int to float64 conversion.
  kCvtTruncF32ToInt,          //!< Float32 to int conversion with truncation semantics.
  kCvtRoundF32ToInt,          //!< Float64 to int conversion with round-to-even semantics.
  kCvtTruncF64ToInt,          //!< Float32 to int conversion with truncation semantics.
  kCvtRoundF64ToInt,          //!< Float64 to int conversion with round-to-even semantics.

  kMaxValue = kCvtRoundF64ToInt
};

//! Instruction with `[vec, mem]` operands.
//!
//! Describes load, convert, and insert instructions.
enum class UniOpVM : uint32_t {
  kLoad8,                     //!< 8-bit load into a vector register (the rest is cleared).
  kLoad16_U16,                //!< 16-bit load into a vector register (the rest is cleared).
  kLoad32_U32,                //!< 32-bit load (int) into a vector register (the rest is cleared).
  kLoad32_F32,                //!< 32-bit load (f32) into a vector register (the rest is cleared).

  kLoad64_U32,                //!< 32-bit load (int) into a vector register (the rest is cleared).
  kLoad64_U64,                //!< 64-bit load (int) into a vector register (the rest is cleared).
  kLoad64_F32,                //!< 32-bit load (f32) into a vector register (the rest is cleared).
  kLoad64_F64,                //!< 64-bit load (f64) into a vector register (the rest is cleared).

  kLoad128_U32,               //!< 128-bit load (int) into a vector register (the rest is cleared).
  kLoad128_U64,               //!< 128-bit load (int) into a vector register (the rest is cleared).
  kLoad128_F32,               //!< 128-bit load (f32) into a vector register (the rest is cleared).
  kLoad128_F64,               //!< 128-bit load (f64) into a vector register (the rest is cleared).

  kLoad256_U32,               //!< 256-bit load (int) into a vector register (the rest is cleared).
  kLoad256_U64,               //!< 256-bit load (int) into a vector register (the rest is cleared).
  kLoad256_F32,               //!< 256-bit load (f32) into a vector register (the rest is cleared).
  kLoad256_F64,               //!< 256-bit load (f64) into a vector register (the rest is cleared).

  kLoad512_U32,               //!< 512-bit load (int) into a vector register (the rest is cleared).
  kLoad512_U64,               //!< 512-bit load (int) into a vector register (the rest is cleared).
  kLoad512_F32,               //!< 512-bit load (f32) into a vector register (the rest is cleared).
  kLoad512_F64,               //!< 512-bit load (f64) into a vector register (the rest is cleared).

  kLoadN_U32,                 //!< N-bit load (int) into a vector register (the size depends on the vector width).
  kLoadN_U64,                 //!< N-bit load (int) into a vector register (the size depends on the vector width).
  kLoadN_F32,                 //!< N-bit load (f32) into a vector register (the size depends on the vector width).
  kLoadN_F64,                 //!< N-bit load (f64) into a vector register (the size depends on the vector width).

  kLoadCvt16_U8ToU64,         //!< 16-bit load into a vector register with 8-bit to 64-bit zero extension (128-bit result).
  kLoadCvt32_U8ToU64,         //!< 32-bit load into a vector register with 8-bit to 64-bit zero extension (256-bit result).
  kLoadCvt64_U8ToU64,         //!< 64-bit load into a vector register with 8-bit to 64-bit zero extension (512-bit result).

  kLoadCvt32_I8ToI16,         //!< 32-bit load into a vector register with 8-bit to 16-bit sign extension (64-bit result).
  kLoadCvt32_U8ToU16,         //!< 32-bit load into a vector register with 8-bit to 16-bit zero extension (64-bit result).
  kLoadCvt32_I8ToI32,         //!< 32-bit load into a vector register with 8-bit to 32-bit sign extension (128-bit result).
  kLoadCvt32_U8ToU32,         //!< 32-bit load into a vector register with 8-bit to 32-bit zero extension (128-bit result).
  kLoadCvt32_I16ToI32,        //!< 32-bit load into a vector register with 16-bit to 32-bit sign extension (64-bit result).
  kLoadCvt32_U16ToU32,        //!< 32-bit load into a vector register with 16-bit to 32-bit zero extension (64-bit result).
  kLoadCvt32_I32ToI64,        //!< 32-bit load into a vector register with 32-bit to 64-bit sign extension (64-bit result).
  kLoadCvt32_U32ToU64,        //!< 32-bit load into a vector register with 32-bit to 64-bit zero extension (64-bit result).

  kLoadCvt64_I8ToI16,         //!< 64-bit load into a vector register with 8-bit to 16-bit sign extension (128-bit result).
  kLoadCvt64_U8ToU16,         //!< 64-bit load into a vector register with 8-bit to 16-bit zero extension (128-bit result).
  kLoadCvt64_I8ToI32,         //!< 64-bit load into a vector register with 8-bit to 32-bit sign extension (256-bit result).
  kLoadCvt64_U8ToU32,         //!< 64-bit load into a vector register with 8-bit to 32-bit zero extension (256-bit result).
  kLoadCvt64_I16ToI32,        //!< 64-bit load into a vector register with 16-bit to 32-bit sign extension (128-bit result).
  kLoadCvt64_U16ToU32,        //!< 64-bit load into a vector register with 16-bit to 32-bit zero extension (128-bit result).
  kLoadCvt64_I32ToI64,        //!< 64-bit load into a vector register with 32-bit to 64-bit sign extension (128-bit result).
  kLoadCvt64_U32ToU64,        //!< 64-bit load into a vector register with 32-bit to 64-bit zero extension (128-bit result).

  kLoadCvt128_I8ToI16,        //!< 128-bit load into a vector register with 8-bit to 16-bit sign extension (256-bit result).
  kLoadCvt128_U8ToU16,        //!< 128-bit load into a vector register with 8-bit to 16-bit zero extension (256-bit result).
  kLoadCvt128_I8ToI32,        //!< 128-bit load into a vector register with 8-bit to 32-bit sign extension (512-bit result).
  kLoadCvt128_U8ToU32,        //!< 128-bit load into a vector register with 8-bit to 32-bit zero extension (512-bit result).
  kLoadCvt128_I16ToI32,       //!< 128-bit load into a vector register with 16-bit to 32-bit sign extension (256-bit result).
  kLoadCvt128_U16ToU32,       //!< 128-bit load into a vector register with 16-bit to 32-bit zero extension (256-bit result).
  kLoadCvt128_I32ToI64,       //!< 128-bit load into a vector register with 32-bit to 64-bit sign extension (256-bit result).
  kLoadCvt128_U32ToU64,       //!< 128-bit load into a vector register with 32-bit to 64-bit zero extension (256-bit result).

  kLoadCvt256_I8ToI16,        //!< 256-bit load into a vector register with 8-bit to 16-bit sign extension (512-bit result).
  kLoadCvt256_U8ToU16,        //!< 256-bit load into a vector register with 8-bit to 16-bit zero extension (512-bit result).
  kLoadCvt256_I16ToI32,       //!< 256-bit load into a vector register with 16-bit to 32-bit sign extension (512-bit result).
  kLoadCvt256_U16ToU32,       //!< 256-bit load into a vector register with 16-bit to 32-bit zero extension (512-bit result).
  kLoadCvt256_I32ToI64,       //!< 256-bit load into a vector register with 32-bit to 64-bit sign extension (512-bit result).
  kLoadCvt256_U32ToU64,       //!< 256-bit load into a vector register with 32-bit to 64-bit zero extension (512-bit result).

  kLoadCvtN_U8ToU64,          //!< N-bit load with 8-bit to 64-bit zero extension (the size depends on the vector width).

  kLoadCvtN_I8ToI16,          //!< N-bit load with 8-bit to 16-bit sign extension (the size depends on the vector width).
  kLoadCvtN_U8ToU16,          //!< N-bit load with 8-bit to 16-bit zero extension (the size depends on the vector width).
  kLoadCvtN_I8ToI32,          //!< N-bit load with 8-bit to 32-bit sign extension (the size depends on the vector width).
  kLoadCvtN_U8ToU32,          //!< N-bit load with 8-bit to 32-bit zero extension (the size depends on the vector width).
  kLoadCvtN_I16ToI32,         //!< N-bit load with 16-bit to 32-bit sign extension (the size depends on the vector width).
  kLoadCvtN_U16ToU32,         //!< N-bit load with 16-bit to 32-bit zero extension (the size depends on the vector width).
  kLoadCvtN_I32ToI64,         //!< N-bit load with 32-bit to 64-bit sign extension (the size depends on the vector width).
  kLoadCvtN_U32ToU64,         //!< N-bit load with 32-bit to 64-bit zero extension (the size depends on the vector width).

  kLoadInsertU8,              //!< 8-bit insert (int) into a vector register from memory.
  kLoadInsertU16,             //!< 16-bit insert (int) into a vector register from memory.
  kLoadInsertU32,             //!< 32-bit insert (int) into a vector register from memory.
  kLoadInsertU64,             //!< 64-bit insert (int) into a vector register from memory.
  kLoadInsertF32,             //!< 32-bit insert (f32) into a vector register from memory.
  kLoadInsertF32x2,           //!< 64-bit insert (f32x2) into a vector register from memory.
  kLoadInsertF64,             //!< 64-bit insert (f64) into a vector register from memory.

  kMaxValue = kLoadInsertF64
};

//! Instruction with `[mem, vec]` operands.
//!
//! Describes store and extract instructions.
enum class UniOpMV : uint32_t {
  kStore8,                    //!< 8-bit store (int) of a vector register.
  kStore16_U16,               //!< 16-bit store (int) of a vector register.
  kStore32_U32,               //!< 16-bit store (int) of a vector register.
  kStore32_F32,               //!< 16-bit store (f32) of a vector register.

  kStore64_U32,               //!< 64-bit store (int) of a vector register.
  kStore64_U64,               //!< 64-bit store (int) of a vector register.
  kStore64_F32,               //!< 64-bit store (f32) of a vector register.
  kStore64_F64,               //!< 64-bit store (f64) of a vector register.

  kStore128_U32,              //!< 128-bit store (int) of a vector register.
  kStore128_U64,              //!< 128-bit store (int) of a vector register.
  kStore128_F32,              //!< 128-bit store (f32) of a vector register.
  kStore128_F64,              //!< 128-bit store (f64) of a vector register.

  kStore256_U32,              //!< 256-bit store (int) of a vector register.
  kStore256_U64,              //!< 256-bit store (int) of a vector register.
  kStore256_F32,              //!< 256-bit store (f32) of a vector register.
  kStore256_F64,              //!< 256-bit store (f64) of a vector register.

  kStore512_U32,              //!< 512-bit store (int) of a vector register.
  kStore512_U64,              //!< 512-bit store (int) of a vector register.
  kStore512_F32,              //!< 512-bit store (f32) of a vector register.
  kStore512_F64,              //!< 512-bit store (f64) of a vector register.

  kStoreN_U32,                //!< N-bit store (int) of a vector register (the size depends on the vector width).
  kStoreN_U64,                //!< N-bit store (int) of a vector register (the size depends on the vector width).
  kStoreN_F32,                //!< N-bit store (f32) of a vector register (the size depends on the vector width).
  kStoreN_F64,                //!< N-bit store (f64) of a vector register (the size depends on the vector width).

  kStoreExtractU16,           //!< 16-bit extract from lane and store.
  kStoreExtractU32,           //!< 32-bit extract from lane and store.
  kStoreExtractU64,           //!< 64-bit extract from lane and store.

  /*
  kStoreCvtz64_U16ToU8,
  kStoreCvtz64_U32ToU16,
  kStoreCvtz64_U64ToU32,
  kStoreCvts64_I16ToI8,
  kStoreCvts64_I16ToU8,
  kStoreCvts64_U16ToU8,
  kStoreCvts64_I32ToI16,
  kStoreCvts64_U32ToU16,
  kStoreCvts64_I64ToI32,
  kStoreCvts64_U64ToU32,

  kStoreCvtz128_U16ToU8,
  kStoreCvtz128_U32ToU16,
  kStoreCvtz128_U64ToU32,
  kStoreCvts128_I16ToI8,
  kStoreCvts128_I16ToU8,
  kStoreCvts128_U16ToU8,
  kStoreCvts128_I32ToI16,
  kStoreCvts128_U32ToU16,
  kStoreCvts128_I64ToI32,
  kStoreCvts128_U64ToU32,

  kStoreCvtz256_U16ToU8,
  kStoreCvtz256_U32ToU16,
  kStoreCvtz256_U64ToU32,
  kStoreCvts256_I16ToI8,
  kStoreCvts256_I16ToU8,
  kStoreCvts256_U16ToU8,
  kStoreCvts256_I32ToI16,
  kStoreCvts256_U32ToU16,
  kStoreCvts256_I64ToI32,
  kStoreCvts256_U64ToU32,

  kStoreCvtzN_U16ToU8,
  kStoreCvtzN_U32ToU16,
  kStoreCvtzN_U64ToU32,
  kStoreCvtsN_I16ToI8,
  kStoreCvtsN_I16ToU8,
  kStoreCvtsN_U16ToU8,
  kStoreCvtsN_I32ToI16,
  kStoreCvtsN_U32ToU16,
  kStoreCvtsN_I64ToI32,
  kStoreCvtsN_U64ToU32,
  */

  kMaxValue = kStoreExtractU64
};

//! Instruction with `[vec, vec]` operands.
//!
//! Describes vector arithmetic that has one destination and one source.
//!
//! \note For convenience, the second operand can be register, memory, or immediate value.
enum class UniOpVV : uint32_t {
  kMov,                       //!< Vector move.
  kMovU64,                    //!< Vector move of the low 64-bit data, the rest is set to zero.

  kBroadcastU8Z,              //!< Vector u8  broadcast with an assumption that the rest of the source vector is zero.
  kBroadcastU16Z,             //!< Vector u16 broadcast with an assumption that the rest of the source vector is zero.
  kBroadcastU8,               //!< Vector u8  broadcast to all lanes.
  kBroadcastU16,              //!< Vector u16 broadcast to all lanes.
  kBroadcastU32,              //!< Vector u32 broadcast to all lanes.
  kBroadcastU64,              //!< Vector u64 broadcast to all lanes.
  kBroadcastF32,              //!< Vector f32 broadcast to all lanes.
  kBroadcastF64,              //!< Vector f64 broadcast to all lanes.
  kBroadcastV128_U32,         //!< Vector broadcast of 128-bit lanes.
  kBroadcastV128_U64,         //!< Vector broadcast of 128-bit lanes.
  kBroadcastV128_F32,         //!< Vector broadcast of 128-bit lanes.
  kBroadcastV128_F64,         //!< Vector broadcast of 128-bit lanes.
  kBroadcastV256_U32,         //!< Vector broadcast of 256-bit lanes.
  kBroadcastV256_U64,         //!< Vector broadcast of 256-bit lanes.
  kBroadcastV256_F32,         //!< Vector broadcast of 256-bit lanes.
  kBroadcastV256_F64,         //!< Vector broadcast of 256-bit lanes.

  kAbsI8,                     //!< Vector i8  absolute value - `dst = abs(src)`.
  kAbsI16,                    //!< Vector i16 absolute value - `dst = abs(src)`.
  kAbsI32,                    //!< Vector i32 absolute value - `dst = abs(src)`.
  kAbsI64,                    //!< Vector i64 absolute value - `dst = abs(src)`.

  kNotU32,                    //!< Vector u32 bitwise NOT - `dst = ~src`.
  kNotU64,                    //!< Vector u64 bitwise NOT - `dst = ~src`.

  kCvtI8LoToI16,              //!< Vector sign extend low  i8  to i16.
  kCvtI8HiToI16,              //!< Vector sign extend high i8  to i16.
  kCvtU8LoToU16,              //!< Vector zero extend low  u8  to u16.
  kCvtU8HiToU16,              //!< Vector zero extend high u8  to u16.
  kCvtI8ToI32,                //!< Vector zero extend low  i8  to i32.
  kCvtU8ToU32,                //!< Vector zero extend high u8  to u32.
  kCvtI16LoToI32,             //!< Vector sign extend low  i16 to i32.
  kCvtI16HiToI32,             //!< Vector sign extend high i16 to i32.
  kCvtU16LoToU32,             //!< Vector zero extend low  u16 to u32.
  kCvtU16HiToU32,             //!< Vector zero extend high u16 to u32.
  kCvtI32LoToI64,             //!< Vector sign extend low  i32 to i64.
  kCvtI32HiToI64,             //!< Vector sign extend high i32 to i64.
  kCvtU32LoToU64,             //!< Vector zero extend low  u32 to u64.
  kCvtU32HiToU64,             //!< Vector zero extend high u32 to u64.

  kAbsF32S,                   //!< Scalar f32 absolute value.
  kAbsF64S,                   //!< Scalar f64 absolute value.
  kAbsF32,                    //!< Vector f32 absolute value.
  kAbsF64,                    //!< Vector f64 absolute value.

  kNegF32S,                   //!< Scalar f32 negate.
  kNegF64S,                   //!< Scalar f64 negate.
  kNegF32,                    //!< Vector f32 negate.
  kNegF64,                    //!< Vector f64 negate.

  kNotF32,                    //!< Vector f32 bitwise NOT.
  kNotF64,                    //!< Vector f64 bitwise NOT.

  kTruncF32S,                 //!< Scalar f32 truncate.
  kTruncF64S,                 //!< Scalar f64 truncate.
  kTruncF32,                  //!< Vector f32 truncate.
  kTruncF64,                  //!< Vector f64 truncate.

  kFloorF32S,                 //!< Scalar f32 floor.
  kFloorF64S,                 //!< Scalar f64 floor.
  kFloorF32,                  //!< Vector f32 floor.
  kFloorF64,                  //!< Vector f64 floor.

  kCeilF32S,                  //!< Scalar f32 ceil.
  kCeilF64S,                  //!< Scalar f64 ceil.
  kCeilF32,                   //!< Vector f32 ceil.
  kCeilF64,                   //!< Vector f64 ceil.

  kRoundEvenF32S,             //!< Scalar f32 round-even.
  kRoundEvenF64S,             //!< Scalar f64 round-even.
  kRoundEvenF32,              //!< Vector f32 round-even.
  kRoundEvenF64,              //!< Vector f64 round-even.

  kRoundHalfAwayF32S,         //!< Scalar f32 round-half-away (0.5 and greater fraction rounds away from zero).
  kRoundHalfAwayF64S,         //!< Scalar f64 round-half-away (0.5 and greater fraction rounds away from zero).
  kRoundHalfAwayF32,          //!< Vector f32 round-half-away (0.5 and greater fraction rounds away from zero).
  kRoundHalfAwayF64,          //!< Vector f64 round-half-away (0.5 and greater fraction rounds away from zero).

  kRoundHalfUpF32S,           //!< Scalar f32 round-half-up (0.5 and greater fraction rounds up).
  kRoundHalfUpF64S,           //!< Scalar f64 round-half-up (0.5 and greater fraction rounds up).
  kRoundHalfUpF32,            //!< Vector f32 round-half-up (0.5 and greater fraction rounds up).
  kRoundHalfUpF64,            //!< Vector f64 round-half-up (0.5 and greater fraction rounds up).

  kRcpF32,                    //!< Vector f32 reciprocal - `dst = 1.0 / src`.
  kRcpF64,                    //!< Vector f64 reciprocal - `dst = 1.0 / src`.

  kSqrtF32S,                  //!< Scalar f32 square root.
  kSqrtF64S,                  //!< Scalar f64 square root.
  kSqrtF32,                   //!< Vector f32 square root.
  kSqrtF64,                   //!< Vector f64 square root.

  kCvtF32ToF64S,
  kCvtF64ToF32S,
  kCvtI32ToF32,
  kCvtF32LoToF64,
  kCvtF32HiToF64,
  kCvtF64ToF32Lo,
  kCvtF64ToF32Hi,
  kCvtI32LoToF64,
  kCvtI32HiToF64,
  kCvtTruncF32ToI32,
  kCvtTruncF64ToI32Lo,
  kCvtTruncF64ToI32Hi,
  kCvtRoundF32ToI32,
  kCvtRoundF64ToI32Lo,
  kCvtRoundF64ToI32Hi,

  kMaxValue = kCvtRoundF64ToI32Hi
};

//! Instruction with `[vec, vec, imm]` operands.
//!
//! Describes vector arithmetic that has one destination, one source, and one immediate.
//!
//! \note For convenience, the second operand can be register, memory, or immediate value.
enum class UniOpVVI : uint32_t {
  kSllU16,                    //!< Vector u16 shift left logical.
  kSllU32,                    //!< Vector u32 shift left logical.
  kSllU64,                    //!< Vector u64 shift left logical.
  kSrlU16,                    //!< Vector u16 shift right logical.
  kSrlU32,                    //!< Vector u32 shift right logical.
  kSrlU64,                    //!< Vector u64 shift right logical.
  kSraI16,                    //!< Vector u16 shift right arithmetic.
  kSraI32,                    //!< Vector u32 shift right arithmetic.
  kSraI64,                    //!< Vector u64 shift right arithmetic.
  kSllbU128,                  //!< Vector shift bytes (128-bit lanes).
  kSrlbU128,                  //!< Vector shift bytes (128-bit lanes).
  kSwizzleU16x4,              //!< Vector swizzle u16x4 (128-bit lanes).
  kSwizzleLoU16x4,            //!< Vector swizzle u16x4 (low  64-bit lanes).
  kSwizzleHiU16x4,            //!< Vector swizzle u16x4 (high 64-bit lanes)
  kSwizzleU32x4,              //!< Vector swizzle u32x4 (128-bit lanes).
  kSwizzleU64x2,              //!< Vector swizzle u64x2 (128-bit lanes).
  kSwizzleF32x4,              //!< Vector swizzle f32x4 (128-bit lanes).
  kSwizzleF64x2,              //!< Vector swizzle f64x2 (128-bit lanes).
  kSwizzleU64x4,              //!< Vector swizzle u64x4 (256-bit lanes).
  kSwizzleF64x4,              //!< Vector swizzle f64x4 (256-bit lanes).
  kExtractV128_I32,           //!< Vector extract 128-bit lane from 256-bit or 512-bit vector.
  kExtractV128_I64,           //!< Vector extract 128-bit lane from 256-bit or 512-bit vector.
  kExtractV128_F32,           //!< Vector extract 128-bit lane from 256-bit or 512-bit vector.
  kExtractV128_F64,           //!< Vector extract 128-bit lane from 256-bit or 512-bit vector.
  kExtractV256_I32,           //!< Vector extract 256-bit lane from 512-bit vector.
  kExtractV256_I64,           //!< Vector extract 256-bit lane from 512-bit vector.
  kExtractV256_F32,           //!< Vector extract 256-bit lane from 512-bit vector.
  kExtractV256_F64,           //!< Vector extract 256-bit lane from 512-bit vector.

#if defined(ASMJIT_UJIT_AARCH64)
  kSrlRndU16,
  kSrlRndU32,
  kSrlRndU64,
  kSrlAccU16,
  kSrlAccU32,
  kSrlAccU64,
  kSrlRndAccU16,
  kSrlRndAccU32,
  kSrlRndAccU64,
  kSrlnLoU16,
  kSrlnHiU16,
  kSrlnLoU32,
  kSrlnHiU32,
  kSrlnLoU64,
  kSrlnHiU64,
  kSrlnRndLoU16,
  kSrlnRndHiU16,
  kSrlnRndLoU32,
  kSrlnRndHiU32,
  kSrlnRndLoU64,
  kSrlnRndHiU64,

  kMaxValue = kSrlnRndHiU64

#elif defined(ASMJIT_UJIT_X86)

  kMaxValue = kExtractV256_F64

#else

  kMaxValue = kExtractV256_F64

#endif // ASMJIT_UJIT_AARCH64
};

//! Instruction with `[vec, vec, vec]` operands.
//!
//! Describes vector arithmetic that has one destination and two sources.
//!
//! \note For convenience, the third operand can be register, memory, or immediate value.
enum class UniOpVVV : uint32_t {
  kAndU32,                    //!< Vector u32 bitwise AND  - `dst = src1 & src2`.
  kAndU64,                    //!< Vector u64 bitwise AND  - `dst = src1 & src2`.
  kOrU32,                     //!< Vector u32 bitwise OR   - `dst = src1 | src2`.
  kOrU64,                     //!< Vector u64 bitwise OR   - `dst = src1 | src2`.
  kXorU32,                    //!< Vector u32 bitwise XOR  - `dst = src1 ^ src2`.
  kXorU64,                    //!< Vector u64 bitwise XOR  - `dst = src1 ^ src2`.
  kAndnU32,                   //!< Vector u32 bitwise ANDN - `dst = ~src1 & src2`.
  kAndnU64,                   //!< Vector u64 bitwise ANDN - `dst = ~src1 & src2`.
  kBicU32,                    //!< Vector u32 bitwise BIC  - `dst = src1 & ~src2`.
  kBicU64,                    //!< Vector u64 bitwise BIC  - `dst = src1 & ~src2`.
  kAvgrU8,                    //!< Vector u8  average rounded half up `dst = (src1 + src2 + 1) >> 1`.
  kAvgrU16,                   //!< Vector u16 average rounded half up `dst = (src1 + src2 + 1) >> 1`.
  kAddU8,                     //!< Vector u8  add.
  kAddU16,                    //!< Vector u16 add.
  kAddU32,                    //!< Vector u32 add.
  kAddU64,                    //!< Vector u64 add.
  kSubU8,                     //!< Vector u8  sub.
  kSubU16,                    //!< Vector u16 sub.
  kSubU32,                    //!< Vector u32 sub.
  kSubU64,                    //!< Vector u64 sub.
  kAddsI8,                    //!< Vector i8  add with saturation (signed).
  kAddsU8,                    //!< Vector u8  add with saturation (unsigned).
  kAddsI16,                   //!< Vector i16 add with saturation (signed).
  kAddsU16,                   //!< Vector u16 add with saturation (unsigned).
  kSubsI8,                    //!< Vector i8  sub with saturation (signed).
  kSubsU8,                    //!< Vector u8  sub with saturation (unsigned).
  kSubsI16,                   //!< Vector i16 sub with saturation (signed).
  kSubsU16,                   //!< Vector u16 sub with saturation (unsigned).
  kMulU16,                    //!< Vector u16 multiply.
  kMulU32,                    //!< Vector u32 multiply.
  kMulU64,                    //!< Vector u64 multiply.
  kMulhI16,                   //!< Vector i16 multiply high - `dst = (src1 * src2) >> 16`.
  kMulhU16,                   //!< Vector u16 multiply high - `dst = (src1 * src2) >> 16`.
  kMulU64_LoU32,              //!< Vector u64xu32 multiply.
  kMHAddI16_I32,              //!< Vector i16 multiply with horizontal widening add to form a 32-bit result.
  kMinI8,                     //!< Vector i8  minimum.
  kMinU8,                     //!< Vector u8  minimum.
  kMinI16,                    //!< Vector i16 minimum.
  kMinU16,                    //!< Vector u16 minimum.
  kMinI32,                    //!< Vector i32 minimum.
  kMinU32,                    //!< Vector u32 minimum.
  kMinI64,                    //!< Vector i64 minimum.
  kMinU64,                    //!< Vector u64 minimum.
  kMaxI8,                     //!< Vector i8  maximum.
  kMaxU8,                     //!< Vector u8  maximum.
  kMaxI16,                    //!< Vector i16 maximum.
  kMaxU16,                    //!< Vector u16 maximum.
  kMaxI32,                    //!< Vector i32 maximum.
  kMaxU32,                    //!< Vector u32 maximum.
  kMaxI64,                    //!< Vector i64 maximum.
  kMaxU64,                    //!< Vector u64 maximum.
  kCmpEqU8,                   //!< Vector u8  compare equal.
  kCmpEqU16,                  //!< Vector u16 compare equal.
  kCmpEqU32,                  //!< Vector u32 compare equal.
  kCmpEqU64,                  //!< Vector u64 compare equal.
  kCmpGtI8,                   //!< Vector i8  compare greater-than.
  kCmpGtU8,                   //!< Vector u8  compare greater-than.
  kCmpGtI16,                  //!< Vector i16 compare greater-than.
  kCmpGtU16,                  //!< Vector u16 compare greater-than.
  kCmpGtI32,                  //!< Vector i32 compare greater-than.
  kCmpGtU32,                  //!< Vector u32 compare greater-than.
  kCmpGtI64,                  //!< Vector i64 compare greater-than.
  kCmpGtU64,                  //!< Vector u64 compare greater-than.
  kCmpGeI8,                   //!< Vector i8  compare greater-or-equal.
  kCmpGeU8,                   //!< Vector u8  compare greater-or-equal.
  kCmpGeI16,                  //!< Vector i16 compare greater-or-equal.
  kCmpGeU16,                  //!< Vector u16 compare greater-or-equal.
  kCmpGeI32,                  //!< Vector i32 compare greater-or-equal.
  kCmpGeU32,                  //!< Vector u32 compare greater-or-equal.
  kCmpGeI64,                  //!< Vector i64 compare greater-or-equal.
  kCmpGeU64,                  //!< Vector u64 compare greater-or-equal.
  kCmpLtI8,                   //!< Vector i8  compare lesser-than.
  kCmpLtU8,                   //!< Vector u8  compare lesser-than.
  kCmpLtI16,                  //!< Vector i16 compare lesser-than.
  kCmpLtU16,                  //!< Vector u16 compare lesser-than.
  kCmpLtI32,                  //!< Vector i32 compare lesser-than.
  kCmpLtU32,                  //!< Vector u32 compare lesser-than.
  kCmpLtI64,                  //!< Vector i64 compare lesser-than.
  kCmpLtU64,                  //!< Vector u64 compare lesser-than.
  kCmpLeI8,                   //!< Vector i8  compare lesser-or-equal.
  kCmpLeU8,                   //!< Vector u8  compare lesser-or-equal.
  kCmpLeI16,                  //!< Vector i16 compare lesser-or-equal.
  kCmpLeU16,                  //!< Vector u16 compare lesser-or-equal.
  kCmpLeI32,                  //!< Vector i32 compare lesser-or-equal.
  kCmpLeU32,                  //!< Vector u32 compare lesser-or-equal.
  kCmpLeI64,                  //!< Vector i64 compare lesser-or-equal.
  kCmpLeU64,                  //!< Vector u64 compare lesser-or-equal.

  kAndF32,                    //!< Vector f32 bitwise AND  - `dst = src1 & src2`.
  kAndF64,                    //!< Vector f64 bitwise AND  - `dst = src1 & src2`.
  kOrF32,                     //!< Vector f32 bitwise OR   - `dst = src1 | src2`.
  kOrF64,                     //!< Vector f64 bitwise OR   - `dst = src1 | src2`.
  kXorF32,                    //!< Vector f32 bitwise XOR  - `dst = src1 ^ src2`.
  kXorF64,                    //!< Vector f64 bitwise XOR  - `dst = src1 ^ src2`.
  kAndnF32,                   //!< Vector f32 bitwise ANDN - `dst = ~src1 & src2`.
  kAndnF64,                   //!< Vector f64 bitwise ANDN - `dst = ~src1 & src2`.
  kBicF32,                    //!< Vector f32 bitwise BIC  - `dst = src1 & ~src2`.
  kBicF64,                    //!< Vector f64 bitwise BIC  - `dst = src1 & ~src2`.
  kAddF32S,                   //!< Scalar f32 add.
  kAddF64S,                   //!< Scalar f64 add.
  kAddF32,                    //!< Vector f32 add.
  kAddF64,                    //!< Vector f64 add.
  kSubF32S,                   //!< Scalar f32 sub.
  kSubF64S,                   //!< Scalar f64 sub.
  kSubF32,                    //!< Vector f32 sub.
  kSubF64,                    //!< Vector f64 sub.
  kMulF32S,                   //!< Scalar f32 mul.
  kMulF64S,                   //!< Scalar f64 mul.
  kMulF32,                    //!< Vector f32 mul.
  kMulF64,                    //!< Vector f64 mul.
  kDivF32S,                   //!< Scalar f32 div.
  kDivF64S,                   //!< Scalar f64 div.
  kDivF32,                    //!< Vector f32 div.
  kDivF64,                    //!< Vector f64 div.
  kModF32S,                   //!< Scalar f32 modulo.
  kModF64S,                   //!< Scalar f64 modulo.
  kModF32,                    //!< Vector f32 modulo.
  kModF64,                    //!< Vector f64 modulo.
  kMinF32S,                   //!< Scalar f32 minimum.
  kMinF64S,                   //!< Scalar f64 minimum.
  kMinF32,                    //!< Vector f32 minimum.
  kMinF64,                    //!< Vector f64 minimum.
  kMaxF32S,                   //!< Scalar f32 maximum.
  kMaxF64S,                   //!< Scalar f64 maximum.
  kMaxF32,                    //!< Vector f32 maximum.
  kMaxF64,                    //!< Vector f64 maximum.
  kCmpEqF32S,                 //!< Scalar f32 compare equal (ordered).
  kCmpEqF64S,                 //!< Scalar f64 compare equal (ordered).
  kCmpEqF32,                  //!< Vector f32 compare equal (ordered).
  kCmpEqF64,                  //!< Vector f64 compare equal (ordered).
  kCmpNeF32S,                 //!< Scalar f32 compare not-equal (ordered)
  kCmpNeF64S,                 //!< Scalar f64 compare not-equal (ordered)
  kCmpNeF32,                  //!< Vector f32 compare not-equal (ordered)
  kCmpNeF64,                  //!< Vector f64 compare not-equal (ordered)
  kCmpGtF32S,                 //!< Scalar f32 compare greater-than (ordered)
  kCmpGtF64S,                 //!< Scalar f64 compare greater-than (ordered)
  kCmpGtF32,                  //!< Vector f32 compare greater-than (ordered)
  kCmpGtF64,                  //!< Vector f64 compare greater-than (ordered)
  kCmpGeF32S,                 //!< Scalar f32 compare greater-or-equal (ordered)
  kCmpGeF64S,                 //!< Scalar f64 compare greater-or-equal (ordered)
  kCmpGeF32,                  //!< Vector f32 compare greater-or-equal (ordered)
  kCmpGeF64,                  //!< Vector f64 compare greater-or-equal (ordered)
  kCmpLtF32S,                 //!< Scalar f32 compare lesser-than (ordered)
  kCmpLtF64S,                 //!< Scalar f64 compare lesser-than (ordered)
  kCmpLtF32,                  //!< Vector f32 compare lesser-than (ordered)
  kCmpLtF64,                  //!< Vector f64 compare lesser-than (ordered)
  kCmpLeF32S,                 //!< Scalar f32 compare lesser-or-equal (ordered)
  kCmpLeF64S,                 //!< Scalar f64 compare lesser-or-equal (ordered)
  kCmpLeF32,                  //!< Vector f32 compare lesser-or-equal (ordered)
  kCmpLeF64,                  //!< Vector f64 compare lesser-or-equal (ordered)
  kCmpOrdF32S,                //!< Scalar f32 compare ordered.
  kCmpOrdF64S,                //!< Scalar f64 compare ordered.
  kCmpOrdF32,                 //!< Vector f32 compare ordered.
  kCmpOrdF64,                 //!< Vector f64 compare ordered.
  kCmpUnordF32S,              //!< Scalar f32 compare unordered.
  kCmpUnordF64S,              //!< Scalar f64 compare unordered.
  kCmpUnordF32,               //!< Vector f32 compare unordered.
  kCmpUnordF64,               //!< Vector f64 compare unordered.

  kHAddF64,                   //!< Vector f64 horizontal-add.

  kCombineLoHiU64,            //!< Combine low and high u64 lanes.
  kCombineLoHiF64,            //!< Combine low and high f64 lanes.
  kCombineHiLoU64,            //!< Combine low and high u64 lanes.
  kCombineHiLoF64,            //!< Combine low and high f64 lanes.

  kInterleaveLoU8,            //!< Interleave low  u8  lanes.
  kInterleaveHiU8,            //!< Interleave high u8  lanes.
  kInterleaveLoU16,           //!< Interleave low  u16 lanes.
  kInterleaveHiU16,           //!< Interleave high u16 lanes.
  kInterleaveLoU32,           //!< Interleave low  u32 lanes.
  kInterleaveHiU32,           //!< Interleave high u32 lanes.
  kInterleaveLoU64,           //!< Interleave low  u64 lanes.
  kInterleaveHiU64,           //!< Interleave high u64 lanes.
  kInterleaveLoF32,           //!< Interleave low  f32 lanes.
  kInterleaveHiF32,           //!< Interleave high f32 lanes.
  kInterleaveLoF64,           //!< Interleave low  f64 lanes.
  kInterleaveHiF64,           //!< Interleave high f64 lanes.

  kPacksI16_I8,               //!< Pack i16 to i8 with saturation.
  kPacksI16_U8,               //!< Pack i16 to u8 with saturation.
  kPacksI32_I16,              //!< Pack i32 to i16 with saturation.
  kPacksI32_U16,              //!< Pack i32 to u16 with saturation.

  kSwizzlev_U8,               //!< Swizzle 16xu8 elements in each 128-bit lane.

#if defined(ASMJIT_UJIT_AARCH64)

  kMulwLoI8,
  kMulwLoU8,
  kMulwHiI8,
  kMulwHiU8,
  kMulwLoI16,
  kMulwLoU16,
  kMulwHiI16,
  kMulwHiU16,
  kMulwLoI32,
  kMulwLoU32,
  kMulwHiI32,
  kMulwHiU32,

  kMAddwLoI8,
  kMAddwLoU8,
  kMAddwHiI8,
  kMAddwHiU8,
  kMAddwLoI16,
  kMAddwLoU16,
  kMAddwHiI16,
  kMAddwHiU16,
  kMAddwLoI32,
  kMAddwLoU32,
  kMAddwHiI32,
  kMAddwHiU32,

  kMaxValue = kMAddwHiU32

#elif defined(ASMJIT_UJIT_X86)

  kPermuteU8,                 //!< Permute u8 elements  across the vector.
  kPermuteU16,                //!< Permute u16 elements across the vector.
  kPermuteU32,                //!< Permute u32 elements across the vector.
  kPermuteU64,                //!< Permute u64 elements across the vector.

  kMaxValue = kPermuteU64

#else

  kMaxValue = kSwizzlev_U8

#endif // ASMJIT_UJIT_AARCH64
};

//! Instruction with `[vec, vec, vec, imm]` operands.
//!
//! Describes vector arithmetic that has one destination, two sources, and immediate.
//!
//! \note For convenience, the third operand can be register, memory, or immediate value.
enum class UniOpVVVI : uint32_t {
  kAlignr_U128,               //!< Align-right 8-bit elements in 128-bit.
  kInterleaveShuffleU32x4,    //!< Interleaved u32x4 shuffle.
  kInterleaveShuffleU64x2,    //!< Interleaved u64x2 shuffle.
  kInterleaveShuffleF32x4,    //!< Interleaved f32x4 shuffle.
  kInterleaveShuffleF64x2,    //!< Interleaved f64x2 shuffle.
  kInsertV128_U32,            //!< Insert a 128-bit lane (u32) into 256-bit or 512-bit vector.
  kInsertV128_F32,            //!< Insert a 128-bit lane (f32) into 256-bit or 512-bit vector.
  kInsertV128_U64,            //!< Insert a 128-bit lane (u64) into 256-bit or 512-bit vector.
  kInsertV128_F64,            //!< Insert a 128-bit lane (f64) into 256-bit or 512-bit vector.
  kInsertV256_U32,            //!< Insert a 256-bit lane (u32) into 512-bit vector.
  kInsertV256_F32,            //!< Insert a 256-bit lane (f32) into 512-bit vector.
  kInsertV256_U64,            //!< Insert a 256-bit lane (u64) into 512-bit vector.
  kInsertV256_F64,            //!< Insert a 256-bit lane (f64) into 512-bit vector.

  kMaxValue = kInsertV256_F64
};

//! Instruction with `[vec, vec, vec, vec]` operands.
//!
//! Describes vector arithmetic that has one destination and three sources.
//!
//! \note For convenience, the fourth operand can be register, memory, or immediate value.
//!
//! \remarks For FMA functionality, check also \ref FMAddOpBehavior.
enum class UniOpVVVV : uint32_t {
  kBlendV_U8,

  kMAddU16,                   //!< Vector u16 multiply-add.
  kMAddU32,                   //!< Vector u32 multiply-add.

  kMAddF32S,                  //!< Scalar f32 multiply-add (FMA if available, or separate MUL+ADD if not).
  kMAddF64S,                  //!< Scalar f64 multiply-add (FMA if available, or separate MUL+ADD if not).
  kMAddF32,                   //!< Vector f32 multiply-add (FMA if available, or separate MUL+ADD if not).
  kMAddF64,                   //!< Vector f64 multiply-add (FMA if available, or separate MUL+ADD if not).

  kMSubF32S,                  //!< Scalar f32 multiply-sub (FMA if available, or separate MUL+ADD if not).
  kMSubF64S,                  //!< Scalar f64 multiply-sub (FMA if available, or separate MUL+ADD if not).
  kMSubF32,                   //!< Vector f32 multiply-sub (FMA if available, or separate MUL+ADD if not).
  kMSubF64,                   //!< Vector f64 multiply-sub (FMA if available, or separate MUL+ADD if not).

  kNMAddF32S,                 //!< Scalar f32 negated-multiply-add (FMA if available, or separate MUL+ADD if not)
  kNMAddF64S,                 //!< Scalar f64 negated-multiply-add (FMA if available, or separate MUL+ADD if not)
  kNMAddF32,                  //!< Vector f32 negated-multiply-add (FMA if available, or separate MUL+ADD if not).
  kNMAddF64,                  //!< Vector f64 negated-multiply-add (FMA if available, or separate MUL+ADD if not).

  kNMSubF32S,                 //!< Scalar f32 negated-multiply-sub (FMA if available, or separate MUL+ADD if not).
  kNMSubF64S,                 //!< Scalar f64 negated-multiply-sub (FMA if available, or separate MUL+ADD if not).
  kNMSubF32,                  //!< Vector f32 negated-multiply-sub (FMA if available, or separate MUL+ADD if not).
  kNMSubF64,                  //!< Vector f64 negated-multiply-sub (FMA if available, or separate MUL+ADD if not).

  kMaxValue = kNMSubF64
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_UJIT
#endif // ASMJIT_UJIT_UNIOP_H_INCLUDED
