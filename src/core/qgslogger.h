/***************************************************************************
                         qgslogger.h  -  description
                             -------------------
    begin                : April 2006
    copyright            : (C) 2006 by Marco Hugentobler
    email                : marco.hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLOGGER_H
#define QGSLOGGER_H

#include <iostream>
#include <sstream>
#include <QString>

#ifdef QGISDEBUG
#define QgsDebugMsg(str) QgsLogger::debug(QString(str), 1, __FILE__, __FUNCTION__, __LINE__)
#define QgsDebugMsgLevel(str, level) \
  { \
    if ( QgsLogger::debugLevel() >= (level) && (level) > 0 ) \
      QgsLogger::debug(QString(str), (level), __FILE__, __FUNCTION__, __LINE__); \
  }
#else
#define QgsDebugMsg(str)
#define QgsDebugMsgLevel(str, level)
#endif

/** \ingroup core
 * QgsLogger is a class to print debug/warning/error messages to the console.
 * The advantage of this class over iostream & co. is that the
 * output can be controlled with environment variables:
 * QGIS_DEBUG is an int describing what debug messages are written to the console.
 * If the debug level of a message is <= QGIS_DEBUG, the message is written to the
 * console. It the variable QGIS_DEBUG is not defined, it defaults to 1 for debug
 * mode and to 0 for release mode
 * QGIS_DEBUG_FILE may contain a file name. Only the messages from this file are
 * printed (provided they have the right debuglevel). If QGIS_DEBUG_FILE is not
 * set, messages from all files are printed
*/

class CORE_EXPORT QgsLogger
{
  public:

    /**Goes to qDebug.
    @param msg the message to be printed
    @param debuglevel
    @param file file name where the message comes from
    @param function function where the message comes from
    @param line place in file where the message comes from*/
    static void debug( const QString& msg, int debuglevel = 1, const char* file = NULL, const char* function = NULL, int line = -1 );

    /**Similar to the previous method, but prints a variable int-value pair*/
    static void debug( const QString& var, int val, int debuglevel = 1, const char* file = NULL, const char* function = NULL, int line = -1 );

    /**Similar to the previous method, but prints a variable double-value pair*/
    static void debug( const QString& var, double val, int debuglevel = 1, const char* file = NULL, const char* function = NULL, int line = -1 );

    /**Prints out a variable/value pair for types with overloaded operator<<*/
    template <typename T> static void debug( const QString& var, T val, const char* file = 0, const char* function = 0,
        int line = -1, int debuglevel = 1 )
    {
      const char* dfile = debugFile();
      if ( dfile ) //exit if QGIS_DEBUG_FILE is set and the message comes from the wrong file
      {
        if ( !file || strcmp( dfile, file ) != 0 )
        {
          return;
        }
      }
      std::ostringstream os;
      os << var.toLocal8Bit().data() << " = " << val;
      if ( line == -1 )
      {
        qDebug( "%s: (%s) %s", file, function, os.str().c_str() );
      }
      else
      {
#if defined(_MSC_VER)
        qDebug( "%s(%d): (%s) %s", file, line, function, os.str().c_str() );
#else
        qDebug( "%s: %d: (%s) %s", file, line, function, os.str().c_str() );
#endif
      }
    }

    /**Goes to qWarning*/
    static void warning( const QString& msg );

    /**Goes to qCritical*/
    static void critical( const QString& msg );

    /**Goes to qFatal*/
    static void fatal( const QString& msg );

    /**Reads the environment variable QGIS_DEBUG and converts it to int. If QGIS_DEBUG is not set,
     the function returns 1 if QGISDEBUG is defined and 0 if not*/
    static int debugLevel();

  private:

    /**Reads the environment variable QGIS_DEBUG_FILE. Returns NULL if the variable is not set*/
    static const char* debugFile();

    /** current debug level */
    static int mDebugLevel;
};

#endif
