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

QgsPythonRunner* QgsPythonRunner::mInstance = nullptr;

///////////////////////////
// static methods

bool QgsPythonRunner::isValid()
{
  return nullptr != mInstance;
}

bool QgsPythonRunner::run( const QString& command, const QString& messageOnError )
{
  if ( mInstance )
  {
    QgsDebugMsg( "Running " + command );
    return mInstance->runCommand( command, messageOnError );
  }
  else
  {
    QgsDebugMsg( "Unable to run Python command: runner not available!" );
    return false;
  }
}

bool QgsPythonRunner::eval( const QString& command, QString& result )
{
  if ( mInstance )
  {
    return mInstance->evalCommand( command, result );
  }
  else
  {
    QgsDebugMsg( "Unable to run Python command: runner not available!" );
    return false;
  }
}

void QgsPythonRunner::setInstance( QgsPythonRunner* runner )
{
  delete mInstance;
  mInstance = runner;
}

///////////////////////////
// non-static methods

QgsPythonRunner::QgsPythonRunner()
{
}

QgsPythonRunner::~QgsPythonRunner()
{

}
