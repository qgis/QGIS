/***************************************************************************
    qgsmessagelog.h  -  interface for logging messages
    ----------------------
    begin                : October 2011
    copyright            : (C) 2011 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmessagelog.h"
#include <qgslogger.h>
#include <QDateTime>
#include <QMetaType>
#include <iostream>

class QgsMessageLogConsole;

QgsMessageLog *QgsMessageLog::sInstance = 0;

QgsMessageLog::QgsMessageLog()
    : QObject()
{
  sInstance = this;
}

QgsMessageLog *QgsMessageLog::instance()
{
  if ( !sInstance )
  {
    qRegisterMetaType<QgsMessageLog::MessageLevel>( "QgsMessageLog::MessageLevel" );
    sInstance = new QgsMessageLog();
  }

  return sInstance;
}

void QgsMessageLog::logMessage( QString message, QString tag, QgsMessageLog::MessageLevel level )
{
  QgsDebugMsg( QString( "%1 %2[%3] %4" ).arg( QDateTime::currentDateTime().toString( Qt::ISODate ) ).arg( tag ).arg( level ).arg( message ) );

  QgsMessageLog::instance()->emitMessage( message, tag, level );
}

void QgsMessageLog::emitMessage( QString message, QString tag, QgsMessageLog::MessageLevel level )
{
  emit messageReceived( message, tag, level );
}

QgsMessageLogConsole::QgsMessageLogConsole()
    : QObject( QgsMessageLog::instance() )
{
  connect( QgsMessageLog::instance(), SIGNAL( messageReceived( QString, QString, QgsMessageLog::MessageLevel ) ),
           this, SLOT( logMessage( QString, QString, QgsMessageLog::MessageLevel ) ) );
}

void QgsMessageLogConsole::logMessage( QString message, QString tag, QgsMessageLog::MessageLevel level )
{
  std::cout
    << tag.toLocal8Bit().data() << "[" <<
    ( level == QgsMessageLog::INFO ? "INFO"
      : level == QgsMessageLog::WARNING ? "WARNING"
      : "CRITICAL" )
    << "]: " << message.toLocal8Bit().data() << std::endl;
}

