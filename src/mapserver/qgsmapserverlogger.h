/***************************************************************************
                              qgsmapserverlogger.h
                              --------------------
  begin                : July 04, 2006
  copyright            : (C) 2006 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPSERVERLOGGER_H
#define QGSMAPSERVERLOGGER_H

#include <QFile>
#include <QTextStream>

#ifdef QGSMSDEBUG
#define QgsMSDebugMsg(str) QgsMapServerLogger::instance()->printMessage(str);
#else
#define QgsMSDebugMsg(str)
#endif

class QString;

/**A singleton class that supports writing server log messages into a log file*/
class QgsMapServerLogger
{
  public:
    ~QgsMapServerLogger();
    static QgsMapServerLogger* instance();
    /**Sets a new log file. */
    int setLogFilePath( const QString& path );
    /**Print a message to the Logfile*/
    void printMessage( const QString& message );
    /**Prints only one char*/
    void printChar( QChar c );
  private:
    static QgsMapServerLogger* mInstance;
    QgsMapServerLogger();
    QFile mLogFile;
    QTextStream mTextStream;
};

#endif
