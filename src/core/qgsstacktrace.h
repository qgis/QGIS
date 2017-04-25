/***************************************************************************
  qgsbacktrace - %{Cpp:License:ClassName}

 ---------------------
 begin                : 30.3.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSBACKTRACE_H
#define QGSBACKTRACE_H

#ifndef QGISDEBUG
#define QgsStacktrace #error "QgsStacktrace is only available in bug mode"
#else

#include <QStringList>
#include "qgis_core.h"

/*
 * Based on stacktrace.h (c) 2008, Timo Bingmann from http://idlebox.net
 * originally published under the WTFPL v2.0
 */

/**
 * \ingroup core
 * The QgsStacktrace class provides an interface to generate a stack trace for
 * displaying additional debug information when things go wrong.
 *
 * \note Not available in python
 * \note Added in QGIS 3.0
 */
class CORE_EXPORT QgsStacktrace
{
  public:

    /**
     * Return a demangled stack backtrace of the caller function.
     *
     * \note Not available in python
     * \note Added in QGIS 3.0
     */
    static QStringList trace( unsigned int maxFrames = 63 );
};

#endif // QGISDEBUG

#endif // QGSBACKTRACE_H
