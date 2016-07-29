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
#include <QTime>
class QFile;

#ifdef QGISDEBUG
#define QgsDebugMsg(str) QgsLogger::debug(QString(str), 1, __FILE__, __FUNCTION__, __LINE__)
#define QgsDebugMsgLevel(str, level) QgsLogger::debug(QString(str), (level), __FILE__, __FUNCTION__, __LINE__)
#define QgsDebugCall QgsScopeLogger _qgsScopeLogger(__FILE__, __FUNCTION__, __LINE__)
#else
#define QgsDebugCall
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
 *
 * QGIS_LOG_FILE may contain a file name. If set, all messages will be appended
 * to this file rather than to stdout.
*/

class CORE_EXPORT QgsLogger
{
  public:

    /** Goes to qDebug.
    @param msg the message to be printed
    @param debuglevel
    @param file file name where the message comes from
    @param function function where the message comes from
    @param line place in file where the message comes from*/
    static void debug( const QString& msg, int debuglevel = 1, const char* file = nullptr, const char* function = nullptr, int line = -1 );

    /** Similar to the previous method, but prints a variable int-value pair*/
    static void debug( const QString& var, int val, int debuglevel = 1, const char* file = nullptr, const char* function = nullptr, int line = -1 );

    /** Similar to the previous method, but prints a variable double-value pair*/
    // @note not available in python bindings
    static void debug( const QString& var, double val, int debuglevel = 1, const char* file = nullptr, const char* function = nullptr, int line = -1 );

    /** Prints out a variable/value pair for types with overloaded operator<<*/
    // @note not available in python bindings
    template <typename T> static void debug( const QString& var, T val, const char* file = nullptr, const char* function = nullptr,
        int line = -1, int debuglevel = 1 )
    {
      std::ostringstream os;
      os << var.toLocal8Bit().data() << " = " << val;
      debug( var, os.str().c_str(), file, function, line, debuglevel );
    }

    /** Goes to qWarning*/
    static void warning( const QString& msg );

    /** Goes to qCritical*/
    static void critical( const QString& msg );

    /** Goes to qFatal*/
    static void fatal( const QString& msg );

    /** Reads the environment variable QGIS_DEBUG and converts it to int. If QGIS_DEBUG is not set,
     the function returns 1 if QGISDEBUG is defined and 0 if not*/
    static int debugLevel() { init(); return sDebugLevel; }

    /** Logs the message passed in to the logfile defined in QGIS_LOG_FILE if any. **/
    static void logMessageToFile( const QString& theMessage );

    /** Reads the environment variable QGIS_LOG_FILE. Returns NULL if the variable is not set,
     * otherwise returns a file name for writing log messages to.*/
    static const QString logFile() { init(); return sLogFile; }

  private:
    static void init();

    /** Current debug level */
    static int sDebugLevel;
    static int sPrefixLength;
    static QString sLogFile;
    static QString sFileFilter;
    static QTime sTime;
};

/** \ingroup core
 */
class QgsScopeLogger
{
  public:
    QgsScopeLogger( const char* file, const char* func, int line )
        : _file( file )
        , _func( func )
        , _line( line )
    {
      QgsLogger::debug( "Entering.", 1, _file, _func, _line );
    }
    ~QgsScopeLogger()
    {
      QgsLogger::debug( "Leaving.", 1, _file, _func, _line );
    }
  private:
    const char *_file;
    const char *_func;
    int _line;
};

#endif
