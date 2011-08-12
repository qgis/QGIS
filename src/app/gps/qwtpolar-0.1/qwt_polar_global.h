/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_GLOBAL_H
#define QWT_POLAR_GLOBAL_H

#include <qglobal.h>
#if QT_VERSION < 0x040000
#include <qmodules.h>
#endif

// QWT_POLAR_VERSION is (major << 16) + (minor << 8) + patch.

#define QWT_POLAR_VERSION       0x000100
#define QWT_POLAR_VERSION_STR   "0.1.0"

#if defined(Q_WS_WIN) || defined(Q_WS_S60)

#if defined(_MSC_VER) /* MSVC Compiler */
/* template-class specialization 'identifier' is already instantiated */
#pragma warning(disable: 4660)
#endif // _MSC_VER

#ifdef QWT_POLAR_DLL

#if defined(QWT_POLAR_MAKEDLL)     // create a Qwt DLL library 
#define QWT_POLAR_EXPORT  __declspec(dllexport)
#define QWT_POLAR_TEMPLATEDLL
#else                        // use a Qwt DLL library
#define QWT_POLAR_EXPORT  __declspec(dllimport)
#endif

#endif // QWT_POLAR_MAKEDLL

#endif // Q_WS_WIN

#ifndef QWT_POLAR_EXPORT
#define QWT_POLAR_EXPORT
#endif

#endif // QWT_POLAR_GLOBAL_H
