/***************************************************************************
    qgsmsutils.cpp
    ---------------------
    begin                : August 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmsutils.h"
#include "qgslogger.h"
#include <stdlib.h>
#include <time.h>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>

QString QgsMSUtils::createTempFilePath()
{


  //save the content of the file into a temporary location
  //generate a name considering the current time
  //struct timeval currentTime;
  //gettimeofday(&currentTime, nullptr);

  time_t seconds;
  time( &seconds );
  srand( seconds );

  int randomNumber = rand();
  QString tempFileName = QString::number( randomNumber );
  QString tempFilePath;
  //on windows, store the temporary file in current_path/tmp directory,
  //on unix, store it in /tmp/qgis_wms_serv
#ifndef Q_OS_WIN
  QDir tempFileDir( "/tmp/qgis_map_serv" );
  if ( !tempFileDir.exists() ) //make sure the directory exists
  {
    QDir tmpDir( "/tmp" );
    tmpDir.mkdir( "qgis_map_serv" );
  }
  tempFilePath = "/tmp/qgis_map_serv/" + tempFileName;
#else
  QDir tempFileDir( QDir::currentPath() + "/tmp" );
  if ( !tempFileDir.exists() )
  {
    QDir currentDir( QDir::currentPath() );
    currentDir.mkdir( "tmp" );
  }
  tempFilePath = QDir::currentPath() + "/tmp" + "/" + tempFileName;
#endif // Q_OS_WIN

  QFileInfo testFile( tempFilePath );
  while ( testFile.exists() )
  {
    //change the name
    tempFilePath += "1";
    testFile.setFile( tempFilePath );
  }
  QgsDebugMsg( tempFilePath );
  return tempFilePath;
}

int QgsMSUtils::createTextFile( const QString& filePath, const QString& text )
{
  QFile file( filePath );
  if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
  {
    QTextStream fileStream( &file );
    fileStream << text;
    file.close();
    return 0;
  }
  else
  {
    return 1;
  }
}
