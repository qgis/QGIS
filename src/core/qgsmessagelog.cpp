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
#include <iostream>

class QgsMessageLogConsole;

void ( *QgsMessageLog::gmLogger )( QString message, QString tag, int level ) = 0;

void QgsMessageLog::setLogger( void ( *f )( QString message, QString tag, int level ) )
{
  gmLogger = f;
}

void QgsMessageLog::logMessage( QString message, QString tag, int level )
{
  QgsDebugMsg( QString( "%1 %2[%3] %4" ).arg( QDateTime::currentDateTime().toString( Qt::ISODate ) ).arg( tag ).arg( level ).arg( message ) );

  if ( !gmLogger )
    QgsMessageLogConsole::logger( message, tag, level );
  else
    gmLogger( message, tag, level );
}

void QgsMessageLogConsole::logger( QString message, QString tag, int level )
{
  std::cout << tag.toLocal8Bit().data() << "[" << level << "]: " << message.toLocal8Bit().data() << std::endl;
}

