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

int QgsLogger::mDebugLevel = -999; // undefined value

void QgsLogger::debug( const QString& msg, int debuglevel, const char* file, const char* function, int line )
{
  const char* dfile = debugFile();
  if ( dfile ) //exit if QGIS_DEBUG_FILE is set and the message comes from the wrong file
  {
    if ( !file || strcmp( dfile, file ) != 0 )
    {
      return;
    }
  }

  int dlevel = debugLevel();
  if ( dlevel >= debuglevel && debuglevel > 0 )
  {
    if ( file == NULL )
    {
      qDebug( "%s", msg.toLocal8Bit().constData() );
      logMessageToFile( msg );
    }
    else if ( function == NULL )
    {
      qDebug( "%s: %s", file, msg.toLocal8Bit().constData() );
      logMessageToFile( msg );
    }
    else if ( line == -1 )
    {
      qDebug( "%s: (%s) %s", file, function, msg.toLocal8Bit().constData() );
      logMessageToFile( msg );
    }
    else
    {
#ifndef _MSC_VER
      qDebug( "%s: %d: (%s) %s", file, line, function, msg.toLocal8Bit().constData() );
#else
      qDebug( "%s(%d) : (%s) %s", file, line, function, msg.toLocal8Bit().constData() );
#endif
      logMessageToFile( msg );
    }
  }
}

void QgsLogger::debug( const QString& var, int val, int debuglevel, const char* file, const char* function, int line )
{
  const char* dfile = debugFile();
  if ( dfile ) //exit if QGIS_DEBUG_FILE is set and the message comes from the wrong file
  {
    if ( !file || strcmp( dfile, file ) != 0 )
    {
      return;
    }
  }

  int dlevel = debugLevel();
  if ( dlevel >= debuglevel && debuglevel > 0 )
  {
    if ( file == NULL )
    {
      qDebug( "%s: %d", var.toLocal8Bit().constData(), val );
      logMessageToFile( QString( "%s: %d" ).arg( var.toLocal8Bit().constData() ).arg( val ) );
    }
    else if ( function == NULL )
    {
      qDebug( "%s: %s: %d", file, var.toLocal8Bit().constData(), val );
      logMessageToFile( QString( "%s: %s: %d" ).arg( file ).arg( var.toLocal8Bit().constData() ).arg( val ) );
    }
    else if ( line == -1 )
    {
      qDebug( "%s: (%s): %s: %d", file, function, var.toLocal8Bit().constData(), val );
      logMessageToFile( QString( "%s: (%s): %s: %d" ).arg( file ).arg( function ).arg( var.toLocal8Bit().constData() ).arg( val ) );
    }
    else
    {
#ifdef _MSC_VER
      qDebug( "%s(%d): (%s), %s: %d", file, line, function, var.toLocal8Bit().constData(), val );
#else
      qDebug( "%s: %d: (%s), %s: %d", file, line, function, var.toLocal8Bit().constData(), val );
#endif
      logMessageToFile( QString(  "%s: %d: (%s), %s: %d" ).arg( file ).arg( line ).arg( function ).arg( var.toLocal8Bit().constData() ).arg( val ) );
    }
  }
}

void QgsLogger::debug( const QString& var, double val, int debuglevel, const char* file, const char* function, int line )
{
  const char* dfile = debugFile();
  if ( dfile ) //exit if QGIS_DEBUG_FILE is set and the message comes from the wrong file
  {
    if ( !file || strcmp( dfile, file ) != 0 )
    {
      return;
    }
  }

  int dlevel = debugLevel();
  if ( dlevel >= debuglevel && debuglevel > 0 )
  {
    if ( file == NULL )
    {
      qDebug( "%s: %f", var.toLocal8Bit().constData(), val );
      logMessageToFile( QString( "%s: %f" ).arg( var.toLocal8Bit().constData() ).arg( val ) ); 
    }
    else if ( function == NULL )
    {
      qDebug( "%s: %s: %f", file, var.toLocal8Bit().constData(), val );
      logMessageToFile( QString( "%s: %s: %f" ).arg( file ).arg( var.toLocal8Bit().constData() ).arg( val ) );
    }
    else if ( line == -1 )
    {
      qDebug( "%s: (%s): %s: %f", file, function, var.toLocal8Bit().constData(), val );
      logMessageToFile( QString(  "%s: (%s): %s: %f" ).arg( file ).arg( function ).arg( var.toLocal8Bit().constData() ).arg( val ) );
    }
    else
    {
#ifdef _MSC_VER
      qDebug( "%s(%d): (%s), %s: %f", file, line, function, var.toLocal8Bit().constData(), val );
#else
      qDebug( "%s: %d: (%s), %s: %f", file, line, function, var.toLocal8Bit().constData(), val );
#endif
      logMessageToFile( QString( "%s: %d: (%s), %s: %f").arg( file ).arg( line ).arg( function ).arg( var.toLocal8Bit().constData() ).arg( val ) );
    }
  }
}

void QgsLogger::warning( const QString& msg )
{
  qWarning( "%s", msg.toLocal8Bit().constData() );
  logMessageToFile( msg );
}

void QgsLogger::critical( const QString& msg )
{
  qCritical( "%s", msg.toLocal8Bit().constData() );
  logMessageToFile( msg );
}

void QgsLogger::fatal( const QString& msg )
{
  qFatal( "%s", msg.toLocal8Bit().constData() );
  logMessageToFile( msg );
}

int QgsLogger::debugLevel()
{
  if ( mDebugLevel == -999 )
  {
    // read the environment variable QGIS_DEBUG just once,
    // then reuse the value

    const char* dlevel = getenv( "QGIS_DEBUG" );
    if ( dlevel == NULL ) //environment variable not set
    {
#ifdef QGISDEBUG
      mDebugLevel = 1; //1 is default value in debug mode
#else
      mDebugLevel = 0;
#endif
    }
    else
    {
      mDebugLevel = atoi( dlevel );
#ifdef QGISDEBUG
      if ( mDebugLevel == 0 )
      {
        mDebugLevel = 1;
      }
#endif
    }
  }

  return mDebugLevel;
}

const QString QgsLogger::logFile()
{
  const QString logFile = getenv( "QGIS_LOG_FILE" );
  return logFile;
}

const void QgsLogger::logMessageToFile( QString theMessage )
{
  if ( ! logFile().isEmpty() )
  {
    //Maybe more efficient to keep the file open for the life of qgis...
    QFile file( logFile() );
    file.open(QIODevice::Append);
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
