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

#include <QVector>
#include "qgsconfig.h"

#ifdef WIN32
#include <windows.h>
#include <dbghelp.h>
#endif

#include <QStringList>

///@cond PRIVATE


/**
* The QgsStacktrace class provides an interface to generate a stack trace for
* displaying additional debug information when things go wrong.
*
* \since QGIS 3.0
*/
class QgsStackTrace
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
      * \return TRUE if part of QGIS.
      */
      bool isQgisModule() const;

      /**
      * Check if this stack line is valid.  Considered valid when the filename and line
      * number are known.
      * \return TRUE of the line is valid.
      */
      bool isValid() const;
    };

    bool symbolsLoaded;
    QString fullStack;
    QVector<QgsStackTrace::StackLine> lines;

#ifdef _MSC_VER
    HANDLE process;
    HANDLE thread;
    std::vector<HANDLE> threads;

    /**
     * Returns a demangled stack backtrace of the caller function.
     *
     * \since QGIS 3.0
     */
    static QgsStackTrace *trace( DWORD processID, DWORD threadID, struct _EXCEPTION_POINTERS *ExceptionInfo, QString symbolPath );
#endif

#ifdef Q_OS_LINUX

    /**
    * Returns a demangled stack backtrace of the caller function.
     *
     * \since QGIS 3.0
     */
    static QVector<QgsStackTrace::StackLine> trace( unsigned int maxFrames = 63 );
#endif
};

typedef QVector<QgsStackTrace::StackLine> QgsStackLines;

///@endcond

#endif // QGSSTACKTRACE_H
