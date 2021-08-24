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

QgsServerLogger *QgsServerLogger::sInstance = nullptr;

QgsServerLogger *QgsServerLogger::instance()
{
  if ( !sInstance )
  {
    sInstance = new QgsServerLogger();
  }
  return sInstance;
}

QgsServerLogger::QgsServerLogger()
  : QgsMessageLogConsole()
{
}

void QgsServerLogger::logMessage( const QString &message, const QString &tag, Qgis::MessageLevel level )
{
  if ( mLogLevel > level )
  {
    return;
  }
  if ( mLogFile.isOpen() )
  {
    const QString formattedMessage = formatLogMessage( message, tag, level );
    mTextStream << formattedMessage;
    mTextStream.flush();
  }
  else if ( mLogStderr )
  {
    QgsMessageLogConsole::logMessage( message, tag, level );
  }
}

void QgsServerLogger::setLogLevel( const Qgis::MessageLevel level )
{
  mLogLevel = level;
}

void QgsServerLogger::setLogFile( const QString &filename )
{
  mTextStream.flush();
  mLogFile.close();
  mLogFile.setFileName( filename );

  if ( ( ! filename.isEmpty() ) && mLogFile.open( QIODevice::Append ) )
  {
    mTextStream.setDevice( &mLogFile );
  }
}

void QgsServerLogger::setLogStderr()
{
  setLogFile();
  mLogStderr = true;
}
