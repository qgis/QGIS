/******************************************************************************
 * Project:  libspatialindex - A C++ library for spatial indexing
 * Author:   Howard Butler, hobu.inc@gmail.com
 ******************************************************************************
 * Copyright (c) 2011, Howard Butler
 *
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
******************************************************************************/


#pragma once

/* Only define this stuff if we're not ANDROID */
#ifndef ANDROID

#ifndef HAVE_SRAND48

#if HAVE_FEATURES_H
#include <features.h>
#ifndef __THROW
/* copy-pasted from sys/cdefs.h */
/* GCC can always grok prototypes.  For C++ programs we add throw()
to help it optimize the function calls.  But this works only with
gcc 2.8.x and egcs.  For gcc 3.2 and up we even mark C functions
as non-throwing using a function attribute since programs can use
the -fexceptions options for C code as well.  */
# if !defined __cplusplus && __GNUC_PREREQ (3, 3)
#  define __THROW      __attribute__ ((__nothrow__))
#  define __NTH(fct)   __attribute__ ((__nothrow__)) fct
# else
#  if defined __cplusplus && __GNUC_PREREQ (2,8)
#   define __THROW     throw ()
#   define __NTH(fct)  fct throw ()
#  else
#   define __THROW
#   define __NTH(fct)  fct
#  endif
# endif
#endif
#endif

#ifndef __THROW
#   define __THROW
#endif

#ifndef __NTH
#   define __NTH(fct)  fct
#endif

extern void srand48(long int seed) __THROW;

extern unsigned short *seed48(unsigned short xseed[3]) __THROW;

extern long nrand48(unsigned short xseed[3]) __THROW;

extern long mrand48(void) __THROW;

extern long lrand48(void) __THROW;

extern void lcong48(unsigned short p[7]) __THROW;

extern long jrand48(unsigned short xseed[3]) __THROW;

extern double erand48(unsigned short xseed[3]) __THROW;

extern double drand48(void) __THROW;

#endif

/* Only define this stuff if we're not ANDROID */
#endif
