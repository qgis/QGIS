#include <cstdint>

#pragma once

typedef union U32I32F32
{ 
    uint32_t u32;
    int32_t i32;
    float f32;
} U32I32F32;

typedef union U64I64F64
{
    uint64_t u64;
    int64_t i64;
    double f64;
} U64I64F64;

/* this header byte needs to change in case incompatible change happen */
const int AC_HEADER_BYTE = 2;
const int AC_BUFFER_SIZE = 1024;

const uint32_t AC__MinLength = 0x01000000U;   // threshold for renormalization
const uint32_t AC__MaxLength = 0xFFFFFFFFU;      // maximum AC interval length

                                           // Maximum values for binary models
const uint32_t BM__LengthShift = 13;     // length bits discarded before mult.
const uint32_t BM__MaxCount    = 1 << BM__LengthShift;  // for adaptive models

                                          // Maximum values for general models
const uint32_t DM__LengthShift = 15;     // length bits discarded before mult.
const uint32_t DM__MaxCount    = 1 << DM__LengthShift;  // for adaptive models
