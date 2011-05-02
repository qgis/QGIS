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
    }
    else if ( function == NULL )
    {
      qDebug( "%s: %s", file, msg.toLocal8Bit().constData() );
    }
    else if ( line == -1 )
    {
      qDebug( "%s: (%s) %s", file, function, msg.toLocal8Bit().constData() );
    }
    else
    {
#ifndef _MSC_VER
      qDebug( "%s: %d: (%s) %s", file, line, function, msg.toLocal8Bit().constData() );
#else
      qDebug( "%s(%d) : (%s) %s", file, line, function, msg.toLocal8Bit().constData() );
#endif
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
    }
    else if ( function == NULL )
    {
      qDebug( "%s: %s: %d", file, var.toLocal8Bit().constData(), val );
    }
    else if ( line == -1 )
    {
      qDebug( "%s: (%s): %s: %d", file, function, var.toLocal8Bit().constData(), val );
    }
    else
    {
#ifdef _MSC_VER
      qDebug( "%s(%d): (%s), %s: %d", file, line, function, var.toLocal8Bit().constData(), val );
#else
      qDebug( "%s: %d: (%s), %s: %d", file, line, function, var.toLocal8Bit().constData(), val );
#endif
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
    }
    else if ( function == NULL )
    {
      qDebug( "%s: %s: %f", file, var.toLocal8Bit().constData(), val );
    }
    else if ( line == -1 )
    {
      qDebug( "%s: (%s): %s: %f", file, function, var.toLocal8Bit().constData(), val );
    }
    else
    {
#ifdef _MSC_VER
      qDebug( "%s(%d): (%s), %s: %f", file, line, function, var.toLocal8Bit().constData(), val );
#else
      qDebug( "%s: %d: (%s), %s: %f", file, line, function, var.toLocal8Bit().constData(), val );
#endif
    }
  }
}

void QgsLogger::warning( const QString& msg )
{
  qWarning( "%s", msg.toLocal8Bit().constData() );
}

void QgsLogger::critical( const QString& msg )
{
  qCritical( "%s", msg.toLocal8Bit().constData() );
}

void QgsLogger::fatal( const QString& msg )
{
  qFatal( "%s", msg.toLocal8Bit().constData() );
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

const char* QgsLogger::debugFile()
{
  const char* dfile = getenv( "QGIS_DEBUG_FILE" );
  return dfile;
}
