/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_GLOBAL_H
#define QWT_GLOBAL_H

#include <qglobal.h>

// QWT_VERSION is (major << 16) + (minor << 8) + patch.

#define QWT_VERSION       0x060200
#define QWT_VERSION_STR   "6.2.0"

#if defined( _MSC_VER ) /* MSVC Compiler */
/* template-class specialization 'identifier' is already instantiated */
#pragma warning(disable: 4660)
/* inherits via dominance */
#pragma warning(disable: 4250)
#endif // _MSC_VER

#ifdef QWT_DLL

#if defined( QWT_MAKEDLL )     // create a Qwt DLL library
#define QWT_EXPORT Q_DECL_EXPORT
#else                        // use a Qwt DLL library
#define QWT_EXPORT Q_DECL_IMPORT
#endif

#endif // QWT_DLL

#ifndef QWT_EXPORT
#define QWT_EXPORT
#endif

#define QWT_CONSTEXPR Q_DECL_CONSTEXPR

#if QT_VERSION >= 0x050000
#define QWT_OVERRIDE Q_DECL_OVERRIDE
#define QWT_FINAL Q_DECL_FINAL
#endif

#ifndef QWT_CONSTEXPR
#define QWT_CONSTEXPR
#endif

#ifndef QWT_OVERRIDE
#define QWT_OVERRIDE
#endif

#ifndef QWT_FINAL
#define QWT_FINAL
#endif

#endif
