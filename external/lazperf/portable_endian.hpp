// (c) Mathias Panzenböck
// http://github.com/panzi/mathfun/blob/master/examples/portable_endian.h
//

#pragma once

#if (defined(_WIN16) || defined(_WIN32) || defined(_WIN64)) && !defined(__WINDOWS__)

#   define __WINDOWS__

#endif

// use standard posix style headers for apple emscripten builds as well since emscripten sdk now ships its own
// libc headers
#if defined(__linux__) || defined(__CYGWIN__) || defined(__EMSCRIPTEN__)

#   include <endian.h>

#elif defined(__APPLE__)

#   include <machine/endian.h>
#   include <libkern/OSByteOrder.h>

#   define htobe16 OSSwapHostToBigInt16
#   define htole16 OSSwapHostToLittleInt16
#   define be16toh OSSwapBigToHostInt16
#   define le16toh OSSwapLittleToHostInt16

#   define htobe32 OSSwapHostToBigInt32
#   define htole32 OSSwapHostToLittleInt32
#   define be32toh OSSwapBigToHostInt32
#   define le32toh OSSwapLittleToHostInt32

#   define htobe64 OSSwapHostToBigInt64
#   define htole64 OSSwapHostToLittleInt64
#   define be64toh OSSwapBigToHostInt64
#   define le64toh OSSwapLittleToHostInt64

/**
#   define __BYTE_ORDER    BYTE_ORDER
#   define __BIG_ENDIAN    BIG_ENDIAN
#   define __LITTLE_ENDIAN LITTLE_ENDIAN
#   define __PDP_ENDIAN    PDP_ENDIAN
**/

#elif defined(__OpenBSD__)|| defined(__FreeBSD__) 

#   include <sys/endian.h>

#elif defined(__NetBSD__) || defined(__DragonFly__)

#   define be16toh betoh16
#   define le16toh letoh16

#   define be32toh betoh32
#   define le32toh letoh32

#   define be64toh betoh64
#   define le64toh letoh64

#elif defined(__WINDOWS__)

#   include <winsock2.h>

#   if BYTE_ORDER == LITTLE_ENDIAN

#       define htobe16 htons
#       define htole16(x) (x)
#       define be16toh ntohs
#       define le16toh(x) (x)

#       define htobe32 htonl
#       define htole32(x) (x)
#       define be32toh ntohl
#       define le32toh(x) (x)

#       define htobe64 htonll
#       define htole64(x) (x)
#       define be64toh ntohll
#       define le64toh(x) (x)

#   elif BYTE_ORDER == BIG_ENDIAN

                                /* that would be xbox 360 */
#       define htobe16(x) (x)
#       define htole16(x) __builtin_bswap16(x)
#       define be16toh(x) (x)
#       define le16toh(x) __builtin_bswap16(x)

#       define htobe32(x) (x)
#       define htole32(x) __builtin_bswap32(x)
#       define be32toh(x) (x)
#       define le32toh(x) __builtin_bswap32(x)

#       define htobe64(x) (x)
#       define htole64(x) __builtin_bswap64(x)
#       define be64toh(x) (x)
#       define le64toh(x) __builtin_bswap64(x)

#   else

#       error byte order not supported

#   endif

#   define __BYTE_ORDER    BYTE_ORDER
#   define __BIG_ENDIAN    BIG_ENDIAN
#   define __LITTLE_ENDIAN LITTLE_ENDIAN
#   define __PDP_ENDIAN    PDP_ENDIAN

#else

#   error platform not supported

#endif
