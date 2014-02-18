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

#include <QtDebug>
#include <QFile>

#include "qgsconfig.h"

#ifndef CMAKE_SOURCE_DIR
#error CMAKE_SOURCE_DIR undefinied
#endif // CMAKE_SOURCE_DIR

int QgsLogger::sDebugLevel = -999; // undefined value
int QgsLogger::sPrefixLength = -1;

void QgsLogger::debug( const QString& msg, int debuglevel, const char* file, const char* function, int line )
{
  const char* dfile = debugFile();
  if ( dfile ) //exit if QGIS_DEBUG_FILE is set and the message comes from the wrong file
  {
    if ( !file || strncmp( dfile, file, strlen( dfile ) ) != 0 )
    {
      return;
    }
  }

  int dlevel = debugLevel();
  if ( dlevel >= debuglevel && debuglevel > 0 )
  {
    QString m;

    if ( !file )
    {
      m =  msg;
    }
    else if ( !function )
    {
      m = QString( "%1: %2" ).arg( file + sPrefixLength ).arg( msg );
    }
    else if ( line == -1 )
    {
      m = QString( "%1: (%2) %3" ).arg( file + sPrefixLength ).arg( function ).arg( msg );
    }
    else
    {
#ifndef _MSC_VER
      m = QString( "%1: %2: (%3) %4" ).arg( file + sPrefixLength ).arg( line ).arg( function ).arg( msg );
#else
      m = QString( "%1(%2) : (%3) %4" ).arg( file ).arg( line ).arg( function ).arg( msg );
#endif
    }
    if ( logFile().isEmpty() )
    {
      qDebug( "%s", m.toLocal8Bit().constData() );
    }
    else
    {
      logMessageToFile( m );
    }
  }
}

void QgsLogger::debug( const QString& var, int val, int debuglevel, const char* file, const char* function, int line )
{
  const char* dfile = debugFile();
  if ( dfile ) //exit if QGIS_DEBUG_FILE is set and the message comes from the wrong file
  {
    if ( !file || strncmp( dfile, file, strlen( dfile ) ) != 0 )
    {
      return;
    }
  }

  int dlevel = debugLevel();
  if ( dlevel >= debuglevel && debuglevel > 0 )
  {
    if ( !file )
    {
      qDebug( "%s: %d", var.toLocal8Bit().constData(), val );
      logMessageToFile( QString( "%s: %d" ).arg( var.toLocal8Bit().constData() ).arg( val ) );
    }
    else if ( !function )
    {
      qDebug( "%s: %s: %d", file + sPrefixLength, var.toLocal8Bit().constData(), val );
      logMessageToFile( QString( "%s: %s: %d" ).arg( file + sPrefixLength ).arg( var.toLocal8Bit().constData() ).arg( val ) );
    }
    else if ( line == -1 )
    {
      qDebug( "%s: (%s): %s: %d", file + sPrefixLength, function, var.toLocal8Bit().constData(), val );
      logMessageToFile( QString( "%s: (%s): %s: %d" ).arg( file + sPrefixLength ).arg( function ).arg( var.toLocal8Bit().constData() ).arg( val ) );
    }
    else
    {
#ifdef _MSC_VER
      qDebug( "%s(%d): (%s), %s: %d", file + sPrefixLength, line, function, var.toLocal8Bit().constData(), val );
#else
      qDebug( "%s: %d: (%s), %s: %d", file + sPrefixLength, line, function, var.toLocal8Bit().constData(), val );
#endif
      logMessageToFile( QString( "%s: %d: (%s), %s: %d" ).arg( file + sPrefixLength ).arg( line ).arg( function ).arg( var.toLocal8Bit().constData() ).arg( val ) );
    }
  }
}

void QgsLogger::debug( const QString& var, double val, int debuglevel, const char* file, const char* function, int line )
{
  const char* dfile = debugFile();
  if ( dfile ) //exit if QGIS_DEBUG_FILE is set and the message comes from the wrong file
  {
    if ( !file || strncmp( dfile, file, strlen( dfile ) ) != 0 )
    {
      return;
    }
  }

  int dlevel = debugLevel();
  if ( dlevel >= debuglevel && debuglevel > 0 )
  {
    if ( !file )
    {
      qDebug( "%s: %f", var.toLocal8Bit().constData(), val );
      logMessageToFile( QString( "%s: %f" ).arg( var.toLocal8Bit().constData() ).arg( val ) );
    }
    else if ( !function )
    {
      qDebug( "%s: %s: %f", file + sPrefixLength, var.toLocal8Bit().constData(), val );
      logMessageToFile( QString( "%s: %s: %f" ).arg( file + sPrefixLength ).arg( var.toLocal8Bit().constData() ).arg( val ) );
    }
    else if ( line == -1 )
    {
      qDebug( "%s: (%s): %s: %f", file + sPrefixLength, function, var.toLocal8Bit().constData(), val );
      logMessageToFile( QString( "%s: (%s): %s: %f" ).arg( file + sPrefixLength ).arg( function ).arg( var.toLocal8Bit().constData() ).arg( val ) );
    }
    else
    {
#ifdef _MSC_VER
      qDebug( "%s(%d): (%s), %s: %f", file + sPrefixLength, line, function, var.toLocal8Bit().constData(), val );
#else
      qDebug( "%s: %d: (%s), %s: %f", file + sPrefixLength, line, function, var.toLocal8Bit().constData(), val );
#endif
      logMessageToFile( QString( "%s: %d: (%s), %s: %f" ).arg( file  + sPrefixLength ).arg( line ).arg( function ).arg( var.toLocal8Bit().constData() ).arg( val ) );
    }
  }
}

void QgsLogger::warning( const QString& msg )
{
  logMessageToFile( msg );
  qWarning( "%s", msg.toLocal8Bit().constData() );
}

void QgsLogger::critical( const QString& msg )
{
  logMessageToFile( msg );
  qCritical( "%s", msg.toLocal8Bit().constData() );
}

void QgsLogger::fatal( const QString& msg )
{
  logMessageToFile( msg );
  qFatal( "%s", msg.toLocal8Bit().constData() );
}

int QgsLogger::debugLevel()
{
  if ( sPrefixLength == -1 )
  {
    sPrefixLength = sizeof( CMAKE_SOURCE_DIR );
    if ( CMAKE_SOURCE_DIR[sPrefixLength-1] == '/' )
      sPrefixLength++;
  }

  if ( sDebugLevel == -999 )
  {
    // read the environment variable QGIS_DEBUG just once,
    // then reuse the value

    const char* dlevel = getenv( "QGIS_DEBUG" );
    if ( dlevel == NULL ) //environment variable not set
    {
#ifdef QGISDEBUG
      sDebugLevel = 1; //1 is default value in debug mode
#else
      sDebugLevel = 0;
#endif
    }
    else
    {
      sDebugLevel = atoi( dlevel );
#ifdef QGISDEBUG
      if ( sDebugLevel == 0 )
      {
        sDebugLevel = 1;
      }
#endif
    }
  }

  return sDebugLevel;
}

const QString QgsLogger::logFile()
{
  const QString logFile = getenv( "QGIS_LOG_FILE" );
  return logFile;
}

void QgsLogger::logMessageToFile( QString theMessage )
{
  if ( ! logFile().isEmpty() )
  {
    //Maybe more efficient to keep the file open for the life of qgis...
    QFile file( logFile() );
    file.open( QIODevice::Append );
    file.write( theMessage.toStdString().c_str() );
    file.write( "\n" );
    file.close();
  }
  return;
}

const char* QgsLogger::debugFile()
{
  const char* dfile = getenv( "QGIS_DEBUG_FILE" );
  return dfile;
}
