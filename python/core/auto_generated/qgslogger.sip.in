/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgslogger.h                                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








class QgsLogger
{
%Docstring
QgsLogger is a class to print debug/warning/error messages to the console.
The advantage of this class over iostream & co. is that the
output can be controlled with environment variables:
QGIS_DEBUG is an int describing what debug messages are written to the console.
If the debug level of a message is <= QGIS_DEBUG, the message is written to the
console. It the variable QGIS_DEBUG is not defined, it defaults to 1 for debug
mode and to 0 for release mode
QGIS_DEBUG_FILE may contain a file name. Only the messages from this file are
printed (provided they have the right debuglevel). If QGIS_DEBUG_FILE is not
set, messages from all files are printed

QGIS_LOG_FILE may contain a file name. If set, all messages will be appended
to this file rather than to stdout.
%End

%TypeHeaderCode
#include "qgslogger.h"
%End
  public:

    static void debug( const QString &msg, int debuglevel = 1, const char *file = 0, const char *function = 0, int line = -1 );
%Docstring
Goes to qDebug.

:param msg: the message to be printed
:param debuglevel:
:param file: file name where the message comes from
:param function: function where the message comes from
:param line: place in file where the message comes from*
%End

    static void debug( const QString &var, int val, int debuglevel = 1, const char *file = 0, const char *function = 0, int line = -1 );
%Docstring
Similar to the previous method, but prints a variable int-value pair
%End



    static void warning( const QString &msg );
%Docstring
Goes to qWarning
%End

    static void critical( const QString &msg );
%Docstring
Goes to qCritical
%End

    static void fatal( const QString &msg );
%Docstring
Goes to qFatal
%End

    static int debugLevel();
%Docstring
Reads the environment variable QGIS_DEBUG and converts it to int. If QGIS_DEBUG is not set,
the function returns 1 if QGISDEBUG is defined and 0 if not*
%End

    static void logMessageToFile( const QString &message );
%Docstring
Logs the message passed in to the logfile defined in QGIS_LOG_FILE if any. *
%End

    static const QString logFile();
%Docstring
Reads the environment variable QGIS_LOG_FILE. Returns NULL if the variable is not set,
otherwise returns a file name for writing log messages to.*
%End

};

class QgsScopeLogger
{

%TypeHeaderCode
#include "qgslogger.h"
%End
  public:
    QgsScopeLogger( const char *file, const char *func, int line );
    ~QgsScopeLogger();
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgslogger.h                                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
