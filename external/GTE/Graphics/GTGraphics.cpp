// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>

// The preprocessor symbol GTE_USE_MSWINDOWS is added to all Visual Studio project
// configurations.
#if defined(GTE_USE_MSWINDOWS)

#if !defined(_MSC_VER)
#error Microsoft Visual Studio 2015 or later is required.
#endif

//  MSVC  6   is version 12.0
//  MSVC  7.0 is version 13.0 (MSVS 2002)
//  MSVC  7.1 is version 13.1 (MSVS 2003)
//  MSVC  8.0 is version 14.0 (MSVS 2005)
//  MSVC  9.0 is version 15.0 (MSVS 2008)
//  MSVC 10.0 is version 16.0 (MSVS 2010)
//  MSVC 11.0 is version 17.0 (MSVS 2012)
//  MSVC 12.0 is version 18.0 (MSVS 2013)
//  MSVC 14.0 is version 19.0 (MSVS 2015)
//  MSVC 15.0 is version 19.1 (MSVS 2017) [latest is 19.16 for 15.9.1]
//  MSVC 16.0 is version 19.2 (MSVS 2019)
//  Currently, projects are provided only for MSVS 2013, 2015, 2017, 2019
#if _MSC_VER < 1900
#error Microsoft Visual Studio 2015 or later is required.
#endif

// You can enable iterator debugging by adding to the preprocessor
// symbols:
//   _ITERATOR_DEBUG_LEVEL=value
//
// Debug build values (choose_your_value is 0, 1, or 2)
// 0:  Disables checked iterators and disables iterator debugging.
// 1:  Enables checked iterators and disables iterator debugging.
// 2:  (default) Enables iterator debugging; checked iterators are not relevant.
//
// Release build values (choose_your_value is 0 or 1)
// 0:  (default) Disables checked iterators.
// 1:  Enables checked iterators; iterator debugging is not relevant.

// The preprocessor symbols include GTE_USE_ROW_MAJOR; matrices are stored
// in row-major order.  If you want column-major-order matrices, remove this
// preprocessor symbol.

// The preprocessor symbols include GTE_USE_MAT_VEC; the action of a matrix M
// on a vector V is M*V.  If you want the action to be V*M, remove this
// preprocessor symbol.

#endif
