/*
===============================================================================

  FILE:  util.hpp
  
  CONTENTS:
    Utility classes

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
    uday.karan@gmail.com - Hobu, Inc.
  
  COPYRIGHT:
  
    (c) 2007-2014, martin isenburg, rapidlasso - tools to catch reality
    (c) 2014, Uday Verma, Hobu, Inc.

    This is free software; you can redistribute and/or modify it under the
    terms of the Apache Public License 2.0 published by the Apache Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
===============================================================================
*/

#ifndef __util_hpp__
#define __util_hpp__

#include <string.h>

#include <array>
#include <cstdint>
#include <cstdlib>
#include <limits>

#ifdef NDEBUG
#define LAZDEBUG(e) ((void)0)
#else
#define LAZDEBUG(e) (void)(e)
#endif

namespace lazperf
{
namespace utils
{

template<int BIT, typename T>
T clearBit(T t)
{
    return t & ~(1 << BIT);
}

// Clamp the input value to the range of the output type.
template<typename T, typename U>
T clamp(U u)
{
    constexpr T mn = (std::numeric_limits<T>::min)();
    constexpr T mx = (std::numeric_limits<T>::max)();
    if (u <= mn)
        return mn;
    else if (u >= mx)
        return mx;
    return u;
}

inline double u2d(uint64_t u)
{
    double d;
    memcpy(&d, &u, sizeof(d));
    return d;
}

inline double i2d(int64_t i)
{
    double d;
    memcpy(&d, &i, sizeof(d));
    return d;
}

inline uint64_t d2u(double d)
{
    uint64_t u;
    memcpy(&u, &d, sizeof(u));
    return u;
}

inline int64_t d2i(double d)
{
    int64_t i;
    memcpy(&i, &d, sizeof(i));
    return i;
}

template<typename T>
T unpack(const char *)
{
    static_assert(sizeof(T) != 0, "Only specialized instances of packers should be used");
};

//ABELL - All this junk should be replaced with (no-op) endian changes followed by a copy, which
//  probably means one instruction in the end.
template<>
inline uint64_t unpack(const char *in)
{
    uint64_t b1 = in[0],
        b2 = in[1],
        b3 = in[2],
        b4 = in[3],
        b5 = in[4],
        b6 = in[5],
        b7 = in[6],
        b8 = in[7];

    return (b8 << 56) | (b7 << 48) | (b6 << 40) | (b5 << 32) |
        (b4 << 24) | (b3 << 16) | (b2 << 8) | b1;
}

inline void pack(uint64_t v, char *out)
{
    out[7] = (char)(v >> 56);
    out[6] = (char)(v >> 48);
    out[5] = (char)(v >> 40);
    out[4] = (char)(v >> 32);
    out[3] = (char)(v >> 24);
    out[2] = (char)(v >> 16);
    out[1] = (char)(v >> 8);
    out[0] = (char)v;
}

template<>
inline uint32_t unpack(const char *in)
{
    uint32_t b1 = in[0],
        b2 = in[1],
        b3 = in[2],
        b4 = in[3];

    return ((b4 << 24) |
        ((b3 & 0xFF) << 16) |
        ((b2 & 0xFF) << 8) |
        (b1 & 0xFF));
}

inline void pack(const uint32_t v, char *out)
{
    out[3] = (v >> 24) & 0xFF;
    out[2] = (v >> 16) & 0xFF;
    out[1] = (v >> 8) & 0xFF;
    out[0] = v & 0xFF;
}

template<>
inline double unpack(const char *in)
{
    uint64_t lower = unpack<uint32_t>(in);
    uint64_t upper = unpack<uint32_t>(in + 4);
    return u2d((upper << 32) | lower);
}

inline void pack(const double& d, char *buf)
{
    uint64_t val = d2u(d);
    pack(uint32_t(val & 0xFFFFFFFF), buf);
    pack(uint32_t(val >> 32), buf + 4);
}

template<>
inline uint16_t unpack(const char *in)
{
    uint16_t b1 = in[0],
        b2 = in[1];

    return (((b2 & 0xFF) << 8) | (b1 & 0xFF));
}

inline void pack(const uint16_t v, char *out)
{
    out[1] = (v >> 8) & 0xFF;
    out[0] = v & 0xFF;
}

template<>
inline int64_t unpack(const char *in)
{
    return static_cast<int64_t>(unpack<uint64_t>(in));
}

inline void pack(int64_t t, char *out)
{
    pack(static_cast<uint64_t>(t), out);
}

template<>
inline int32_t unpack(const char *in)
{
    return static_cast<uint32_t>(unpack<uint32_t>(in));
}

inline void pack(int32_t t, char *out)
{
    pack(static_cast<uint32_t>(t), out);
}

template<>
inline int16_t unpack(const char *in)
{
    return static_cast<uint16_t>(unpack<uint16_t>(in));
}

inline void pack(int16_t t, char *out)
{
    pack(static_cast<uint16_t>(t), out);
}

inline int32_t sum(const uint8_t *buf, uint32_t size)
{
    int32_t total = 0;
    while (size--)
        total += *buf++;
    return total;
}

struct Summer
{
    Summer() : sum(0), cnt(0)
    {}

    template <typename T>
    void add(const T& t)
    {
        const uint8_t *b = reinterpret_cast<const uint8_t *>(&t);
        sum += utils::sum(b, sizeof(T));
        cnt++;
    }

    void add(uint8_t *b, size_t size)
    {
        sum += utils::sum(b, size);
    }

    uint32_t value()
    {
        uint32_t v = sum;
        sum = 0;
        return v;
    }

    uint32_t count()
    {
        uint32_t c = cnt;
        cnt = 0;
        return c;
    }

    uint32_t sum;
    uint32_t cnt;
};

#define ALIGN 64

inline void *aligned_malloc(int size)
{
    void *mem = malloc(size+ALIGN+sizeof(void*));
    void **ptr = (void**)(( ((uintptr_t)mem)+ALIGN+sizeof(void*) ) & ~(uintptr_t)(ALIGN-1) );
    ptr[-1] = mem;
    return ptr;
}

inline void aligned_free(void *ptr)
{
    free(((void**)ptr)[-1]);
}

template<typename T>
class streaming_median
{
public:
    std::array<T, 5> values;
    bool high;

    void init() {
        values.fill(T(0));
        high = true;
    }

    inline void add(const T& v)
    {
        if (high) {
            if (v < values[2]) {
                values[4] = values[3];
                values[3] = values[2];
                if (v < values[0]) {
                    values[2] = values[1];
                    values[1] = values[0];
                    values[0] = v;
                }
                else if (v < values[1]) {
                    values[2] = values[1];
                    values[1] = v;
                }
                else {
                    values[2] = v;
                }
            }
            else {
                if (v < values[3]) {
                    values[4] = values[3];
                    values[3] = v;
                }
                else {
                    values[4] = v;
                }
                high = false;
            }
        }
        else {
            if (values[2] < v) {
                values[0] = values[1];
                values[1] = values[2];
                if (values[4] < v) {
                    values[2] = values[3];
                    values[3] = values[4];
                    values[4] = v;
                }
                else if (values[3] < v) {
                    values[2] = values[3];
                    values[3] = v;
                }
                else {
                    values[2] = v;
                }
            }
            else {
                if (values[1] < v) {
                    values[0] = values[1];
                    values[1] = v;
                }
                else {
                    values[0] = v;
                }
                high = true;
            }
        }
    }

    inline T get() const {
        return values[2];
    }

    streaming_median() {
        init();
    }
};

} // namespace utils
} // namespace lazperf

#endif // __util_hpp__
