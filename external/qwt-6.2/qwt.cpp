/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt.h"
#include <qapplication.h>

#define QWT_GLOBAL_STRUT

#if QT_VERSION >= 0x050000
    #if QT_VERSION >= 0x060000 || !QT_DEPRECATED_SINCE(5, 15)
        #undef QWT_GLOBAL_STRUT
    #endif
#endif

QSize qwtExpandedToGlobalStrut( const QSize& size )
{
#ifdef QWT_GLOBAL_STRUT
    return size.expandedTo( QApplication::globalStrut() );
#else
    return size;
#endif
}
