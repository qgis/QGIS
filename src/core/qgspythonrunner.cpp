/***************************************************************************
    qgspythonrunner.cpp
    ---------------------
    begin                : May 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgspythonrunner.h"

#include "qgslogger.h"

QgsPythonRunner *QgsPythonRunner::sInstance = nullptr;

///////////////////////////
// static methods

bool QgsPythonRunner::isValid()
{
  return nullptr != sInstance;
}

bool QgsPythonRunner::run( const QString &command, const QString &messageOnError )
{
  if ( sInstance )
  {
    QgsDebugMsgLevel( "Running " + command, 3 );
    return sInstance->runCommand( command, messageOnError );
  }
  else
  {
    QgsDebugError( u"Unable to run Python command: runner not available!"_s );
    return false;
  }
}

bool QgsPythonRunner::runFile( const QString &filename, const QString &messageOnError )
{
  if ( sInstance )
  {
    QgsDebugMsgLevel( "Running " + filename, 3 );
    return sInstance->runFileCommand( filename, messageOnError );
  }
  else
  {
    QgsDebugError( u"Unable to run Python command: runner not available!"_s );
    return false;
  }
}

bool QgsPythonRunner::eval( const QString &command, QString &result )
{
  if ( sInstance )
  {
    return sInstance->evalCommand( command, result );
  }
  else
  {
    QgsDebugError( u"Unable to run Python command: runner not available!"_s );
    return false;
  }
}

bool QgsPythonRunner::setArgv( const QStringList &arguments, const QString &messageOnError )
{
  if ( sInstance )
  {
    return sInstance->setArgvCommand( arguments, messageOnError );
  }
  else
  {
    QgsDebugError( u"Unable to run Python command: runner not available!"_s );
    return false;
  }
}

void QgsPythonRunner::setInstance( QgsPythonRunner *runner )
{
  delete sInstance;
  sInstance = runner;
}

