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
#include "qgsapplication.h"
#include "qgslogger.h"
#include <QDateTime>
#include <QMetaType>
#include <QTextStream>
#include <iostream>
#include <stdio.h>

class QgsMessageLogConsole;

void QgsMessageLog::logMessage( const QString &message, const QString &tag, Qgis::MessageLevel level, bool notifyUser )
{
  QgsDebugMsg( QStringLiteral( "%1 %2[%3] %4" ).arg( QDateTime::currentDateTime().toString( Qt::ISODate ), tag ).arg( static_cast< int >( level ) ).arg( message ) );

  QgsApplication::messageLog()->emitMessage( message, tag, level, notifyUser );
}

void QgsMessageLog::emitMessage( const QString &message, const QString &tag, Qgis::MessageLevel level, bool notifyUser )
{
  emit messageReceived( message, tag, level );
  if ( level != Qgis::MessageLevel::Info && notifyUser && mAdviseBlockCount == 0 )
  {
    emit messageReceived( true );
  }
}

QgsMessageLogConsole::QgsMessageLogConsole()
  : QObject( QgsApplication::messageLog() )
{
  connect( QgsApplication::messageLog(), static_cast < void ( QgsMessageLog::* )( const QString &, const QString &, Qgis::MessageLevel ) >( &QgsMessageLog::messageReceived ),
           this, &QgsMessageLogConsole::logMessage );
}

void QgsMessageLogConsole::logMessage( const QString &message, const QString &tag, Qgis::MessageLevel level )
{
  const QString formattedMessage = formatLogMessage( message, tag, level );
  QTextStream cerr( stderr );
  cerr << formattedMessage;
}

QString QgsMessageLogConsole::formatLogMessage( const QString &message, const QString &tag, Qgis::MessageLevel level ) const
{
  const QString time = QTime::currentTime().toString();
  const QString levelStr = level == Qgis::MessageLevel::Info ? QStringLiteral( "INFO" ) :
                           level == Qgis::MessageLevel::Warning ? QStringLiteral( "WARNING" ) :
                           QStringLiteral( "CRITICAL" );
  const QString pid = QString::number( QCoreApplication::applicationPid() );
  return QStringLiteral( "%1 %2 %3[%4]: %5\n" ).arg( time, levelStr, tag, pid, message );
}

//
// QgsMessageLogNotifyBlocker
//

QgsMessageLogNotifyBlocker::QgsMessageLogNotifyBlocker()
{
  QgsApplication::messageLog()->mAdviseBlockCount++;
}

QgsMessageLogNotifyBlocker::~QgsMessageLogNotifyBlocker()
{
  QgsApplication::messageLog()->mAdviseBlockCount--;
}
