/***************************************************************************
  qgsstacktrace.h - QgsStackTrace

 ---------------------
 begin                : 24.4.2017
 copyright            : (C) 2017 by Nathan Woodrow
 email                : woodrow.nathan@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSTACKTRACE_H
#define QGSSTACKTRACE_H

#include "qgis_core.h"

#include <QStringList>

///@cond PRIVATE


/**
* \ingroup core
* The QgsStacktrace class provides an interface to generate a stack trace for
* displaying additional debug information when things go wrong.
*
* \note Not available in python
* \note Added in QGIS 3.0
*/
class CORE_EXPORT QgsStackTrace
{
  public:

    /**
     * Represents a line from a stack trace.
     */
    struct StackLine
    {
      QString moduleName;
      QString symbolName;
      QString fileName;
      QString lineNumber;

      /**
       * Check if this stack line is part of QGIS.
       * \return True if part of QGIS.
       */
      bool isQgisModule() const;

      /**
       * Check if this stack line is valid.  Considered valid when the filename and line
       * number are known.
       * \return True of the line is valid.
       */
      bool isValid() const;
    };

#ifdef Q_OS_WIN

    /**
     * Return a demangled stack backtrace of the caller function.
     *
     * \note Added in QGIS 3.0
     */
    static QVector<QgsStackTrace::StackLine> trace( struct _EXCEPTION_POINTERS *ExceptionInfo );
#endif

#ifdef Q_OS_LINUX

    /**
    * Return a demangled stack backtrace of the caller function.
     *
     * \note Added in QGIS 3.0
     */
    static QVector<QgsStackTrace::StackLine> trace( unsigned int maxFrames = 63 );
#endif

  private:
    QgsStackTrace();

};

typedef QVector<QgsStackTrace::StackLine> QgsStackLines;

///@endcond

#endif // QGSSTACKTRACE_H
