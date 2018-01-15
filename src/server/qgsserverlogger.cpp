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
#include "qgsapplication.h"
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QTime>

#include <cstdlib>

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
  : mLogFile( nullptr )
{
  connect( QgsApplication::messageLog(), static_cast<void ( QgsMessageLog::* )( const QString &, const QString &, QgsMessageLog::MessageLevel )>( &QgsMessageLog::messageReceived ), this,
           &QgsServerLogger::logMessage );
}

void QgsServerLogger::setLogLevel( QgsMessageLog::MessageLevel level )
{
  mLogLevel = level;
}

void QgsServerLogger::setLogFile( const QString &f )
{
  if ( ! f.isEmpty() )
  {
    if ( mLogFile.exists() )
    {
      mTextStream.flush();
      mLogFile.close();
    }

    mLogFile.setFileName( f );
    if ( mLogFile.open( QIODevice::Append ) )
    {
      mTextStream.setDevice( &mLogFile );
    }
  }
}

void QgsServerLogger::logMessage( const QString &message, const QString &tag, QgsMessageLog::MessageLevel level )
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
