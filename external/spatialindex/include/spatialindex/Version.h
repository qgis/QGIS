/******************************************************************************
 * Project:  libspatialindex - A C++ library for spatial indexing
 * Author:   Marios Hadjieleftheriou, mhadji@gmail.com
 ******************************************************************************
 * Copyright (c) 2003, Marios Hadjieleftheriou
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

#ifndef SIDX_VERSION_MAJOR
#define SIDX_VERSION_MAJOR    2
#define SIDX_VERSION_MINOR    0
#define SIDX_VERSION_REV      0
#define SIDX_VERSION_BUILD    0
#endif

#ifndef SIDX_VERSION_NUM
#define SIDX_VERSION_NUM      (SIDX_VERSION_MAJOR*1000+SIDX_VERSION_MINOR*100+SIDX_VERSION_REV*10+SIDX_VERSION_BUILD)
#endif

#ifndef SIDX_RELEASE_DATE
#define SIDX_RELEASE_DATE     20240517
#endif

#ifndef SIDX_RELEASE_NAME
#define SIDX_RELEASE_NAME     "2.0.0"
#endif

