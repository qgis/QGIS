/******************************************************************************
 * Project:  libsidx - A C API wrapper around libspatialindex
 * Purpose:  C API configuration
 * Author:   Howard Butler, hobu.inc@gmail.com
 ******************************************************************************
 * Copyright (c) 2009, Howard Butler
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

#ifndef SIDX_CONFIG_H_INCLUDED
#define SIDX_CONFIG_H_INCLUDED



#ifdef _MSC_VER

#if _MSC_VER <= 1500
  typedef __int8 int8_t;
  typedef __int16 int16_t;
  typedef __int32 int32_t;
  typedef __int64 int64_t;
  typedef unsigned __int8 uint8_t;
  typedef unsigned __int16 uint16_t;
  typedef unsigned __int32 uint32_t;
  typedef unsigned __int64 uint64_t;
#else
  #include <stdint.h>
#endif

   #include <windows.h>
   #define STRDUP _strdup
   #include <windows.h>

#else

   #include <stdint.h>
   #define SIDX_THREAD  __thread
   #define STRDUP strdup
#endif

#include <sys/stat.h>

#include "sidx_export.h"

typedef enum
{
   RT_None = 0,
   RT_Debug = 1,
   RT_Warning = 2,
   RT_Failure = 3,
   RT_Fatal = 4
} RTError;

typedef enum
{
   RT_RTree = 0,
   RT_MVRTree = 1,
   RT_TPRTree = 2,
   RT_InvalidIndexType = -99
} RTIndexType;

typedef enum
{
   RT_Memory = 0,
   RT_Disk = 1,
   RT_Custom = 2,
   RT_InvalidStorageType = -99
} RTStorageType;

typedef enum
{
   RT_Linear = 0,
   RT_Quadratic = 1,
   RT_Star = 2,
   RT_InvalidIndexVariant = -99
} RTIndexVariant;


#ifdef __cplusplus
#  define IDX_C_START           extern "C" {
#  define IDX_C_END             }
#else
#  define IDX_C_START
#  define IDX_C_END
#endif

typedef struct IndexS *IndexH;
typedef struct SpatialIndex_IData *IndexItemH;
typedef struct Tools_PropertySet *IndexPropertyH;



#endif
