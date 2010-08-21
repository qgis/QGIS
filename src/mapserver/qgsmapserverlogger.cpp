/***************************************************************************
                              qgsmapserverlogger.cpp
                              ----------------------
  begin                : July 04, 2006
  copyright            : (C) 2006 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapserverlogger.h"

QgsMapServerLogger* QgsMapServerLogger::mInstance = 0;

QgsMapServerLogger::QgsMapServerLogger()
{

}
QgsMapServerLogger::~QgsMapServerLogger()
{
  delete mInstance;
}

QgsMapServerLogger* QgsMapServerLogger::instance()
{
  if ( mInstance == 0 )
  {
    mInstance = new QgsMapServerLogger();
  }
  return mInstance;
}

int QgsMapServerLogger::setLogFilePath( const QString& path )
{
  mLogFile.setFileName( path );
  if ( mLogFile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append ) )
  {
    mTextStream.setDevice( &mLogFile );
    return 0;
  }
  return 1;
}

void QgsMapServerLogger::printMessage( const QString& message )
{
  mTextStream << message << endl;
}

void QgsMapServerLogger::printChar( QChar c )
{
  mTextStream << c;
}
