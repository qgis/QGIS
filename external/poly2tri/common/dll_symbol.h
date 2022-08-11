/*
 * Poly2Tri Copyright (c) 2009-2022, Poly2Tri Contributors
 * https://github.com/jhasse/poly2tri
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * * Neither the name of Poly2Tri nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without specific
 *   prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#if defined(_WIN32)
#  define P2T_COMPILER_DLLEXPORT __declspec(dllexport)
#  define P2T_COMPILER_DLLIMPORT __declspec(dllimport)
#elif defined(__GNUC__)
#  define P2T_COMPILER_DLLEXPORT __attribute__ ((visibility ("default")))
#  define P2T_COMPILER_DLLIMPORT __attribute__ ((visibility ("default")))
#else
#  define P2T_COMPILER_DLLEXPORT
#  define P2T_COMPILER_DLLIMPORT
#endif

#ifndef P2T_DLL_SYMBOL
#  if defined(P2T_STATIC_EXPORTS)
#    define P2T_DLL_SYMBOL
#  elif defined(P2T_SHARED_EXPORTS)
#    define P2T_DLL_SYMBOL P2T_COMPILER_DLLEXPORT
#  else
#    define P2T_DLL_SYMBOL P2T_COMPILER_DLLIMPORT
#  endif
#endif
