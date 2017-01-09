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

QgsServerLogger* QgsServerLogger::mInstance = nullptr;

QgsServerLogger* QgsServerLogger::instance()
{
  if ( !mInstance )
  {
    mInstance = new QgsServerLogger();
  }
  return mInstance;
}

QgsServerLogger::QgsServerLogger()
    : mLogFile( nullptr )
    , mLogLevel( QgsMessageLog::NONE )
{
  connect( QgsApplication::messageLog(), SIGNAL( messageReceived( QString, QString, QgsMessageLog::MessageLevel ) ), this,
           SLOT( logMessage( QString, QString, QgsMessageLog::MessageLevel ) ) );
}

void QgsServerLogger::setLogLevel( QgsMessageLog::MessageLevel level )
{
  mLogLevel = level;
}

void QgsServerLogger::setLogFile( const QString& f )
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

void QgsServerLogger::logMessage( const QString& message, const QString& tag, QgsMessageLog::MessageLevel level )
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
