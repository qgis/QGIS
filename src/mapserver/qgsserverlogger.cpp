/***************************************************************************
                              qgsserverlogger.cpp
                              -------------------
  begin                : May 5, 2014
  copyright            : (C) 2014 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsserverlogger.h"
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QTime>

#include <cstdlib>

QgsServerLogger* QgsServerLogger::mInstance = 0;

QgsServerLogger* QgsServerLogger::instance()
{
  if ( mInstance == 0 )
  {
    mInstance = new QgsServerLogger();
  }
  return mInstance;
}

QgsServerLogger::QgsServerLogger(): mLogFile( 0 )
{
  //logfile
  QString filePath = getenv( "QGIS_SERVER_LOG_FILE" );
  mLogFile.setFileName( filePath );
  if ( mLogFile.open( QIODevice::Append ) )
  {
    mTextStream.setDevice( &mLogFile );
  }

  //log level
  char* logLevelChar = getenv( "QGIS_SERVER_LOG_LEVEL" );
  if ( logLevelChar )
  {
    mLogLevel = atoi( logLevelChar );
  }
  else
  {
    mLogLevel = 3;
  }

  connect( QgsMessageLog::instance(), SIGNAL( messageReceived( QString, QString, QgsMessageLog::MessageLevel ) ), this,
           SLOT( logMessage( QString, QString, QgsMessageLog::MessageLevel ) ) );
}

void QgsServerLogger::logMessage( QString message, QString tag, QgsMessageLog::MessageLevel level )
{
  Q_UNUSED( tag );
  if ( !mLogFile.isOpen() || mLogLevel > level )
  {
    return;
  }

  mTextStream << ( "[" + QString::number( qlonglong( QCoreApplication::applicationPid() ) ) + "]["
                   + QTime::currentTime().toString() + "] " + message + "\n" );
  mTextStream.flush();
}
