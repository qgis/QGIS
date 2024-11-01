/***************************************************************************
                         qgslogger.cpp  -  description
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

#include "qgslogger.h"

#include <QApplication>
#include <QtDebug>
#include <QFile>
#include <QElapsedTimer>
#include <QThread>

#ifndef CMAKE_SOURCE_DIR
#error CMAKE_SOURCE_DIR undefined
#endif // CMAKE_SOURCE_DIR

int QgsLogger::sDebugLevel = -999; // undefined value
int QgsLogger::sPrefixLength = -1;
Q_GLOBAL_STATIC( QString, sFileFilter )
Q_GLOBAL_STATIC( QString, sLogFile )
Q_GLOBAL_STATIC( QElapsedTimer, sTime )

void QgsLogger::init()
{
  if ( sDebugLevel != -999 )
    return;

  sTime()->start();

  *sLogFile() = getenv( "QGIS_LOG_FILE" ) ? getenv( "QGIS_LOG_FILE" ) : "";
  *sFileFilter() = getenv( "QGIS_DEBUG_FILE" ) ? getenv( "QGIS_DEBUG_FILE" ) : "";
  sDebugLevel = getenv( "QGIS_DEBUG" ) ? atoi( getenv( "QGIS_DEBUG" ) ) :
#ifdef QGISDEBUG
                                       1
#else
                                       0
#endif
    ;

  sPrefixLength = sizeof( CMAKE_SOURCE_DIR );
  // cppcheck-suppress internalAstError
  if ( CMAKE_SOURCE_DIR[sPrefixLength - 1] == '/' )
    sPrefixLength++;
}

void QgsLogger::debug( const QString &msg, int debuglevel, const char *file, const char *function, int line )
{
  init();

  if ( !file && !sFileFilter()->isEmpty() && !sFileFilter()->endsWith( file ) )
    return;

  if ( sDebugLevel == 0 || debuglevel > sDebugLevel )
    return;


  QString m = msg;

  if ( file )
  {
    if ( qApp && qApp->thread() != QThread::currentThread() )
    {
      m.prepend( QStringLiteral( "[thread:0x%1] " ).arg( reinterpret_cast<qint64>( QThread::currentThread() ), 0, 16 ) );
    }

    m.prepend( QStringLiteral( "[%1ms] " ).arg( sTime()->elapsed() ) );
    sTime()->restart();

    if ( function )
    {
      m.prepend( QStringLiteral( " (%1) " ).arg( function ) );
    }

    if ( line != -1 )
    {
#ifndef _MSC_VER
      m.prepend( QStringLiteral( ":%1 :" ).arg( line ) );
#else
      m.prepend( QString( "(%1) :" ).arg( line ) );
#endif
    }

#ifndef _MSC_VER
    m.prepend( file + ( file[0] == '/' ? sPrefixLength : 0 ) );
#else
    m.prepend( file );
#endif
  }

  if ( sLogFile()->isEmpty() )
  {
    if ( debuglevel == 0 )
    {
      // debug level 0 is for errors only, so highlight these by dumping them to stderr
      std::cerr << m.toUtf8().constData() << std::endl;
    }
    else
    {
      qDebug( "%s", m.toUtf8().constData() );
    }
  }
  else
  {
    logMessageToFile( m );
  }
}

void QgsLogger::debug( const QString &var, int val, int debuglevel, const char *file, const char *function, int line )
{
  debug( QStringLiteral( "%1: %2" ).arg( var ).arg( val ), debuglevel, file, function, line );
}

void QgsLogger::debug( const QString &var, double val, int debuglevel, const char *file, const char *function, int line )
{
  debug( QStringLiteral( "%1: %2" ).arg( var ).arg( val ), debuglevel, file, function, line );
}

void QgsLogger::warning( const QString &msg )
{
  logMessageToFile( msg );
  qWarning( "Logged warning: %s", msg.toLocal8Bit().constData() );
}

void QgsLogger::critical( const QString &msg )
{
  logMessageToFile( msg );
  qCritical( "Logged critical: %s", msg.toLocal8Bit().constData() );
}

void QgsLogger::fatal( const QString &msg )
{
  logMessageToFile( msg );
  qFatal( "Logged fatal: %s", msg.toLocal8Bit().constData() );
}

void QgsLogger::logMessageToFile( const QString &message )
{
  if ( sLogFile()->isEmpty() )
    return;

  //Maybe more efficient to keep the file open for the life of qgis...
  QFile file( *sLogFile() );
  if ( !file.open( QIODevice::Append ) )
    return;
  file.write( message.toLocal8Bit().constData() );
  file.write( "\n" );
  file.close();
}

QString QgsLogger::logFile()
{
  init();
  return *sLogFile();
}
