/***************************************************************************
                            qgsgrassdatafile.cpp
                             -------------------
    begin                : June, 2015
    copyright            : (C) 2015 Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgrassdatafile.h"

QgsGrassDataFile::QgsGrassDataFile( QObject *parent ) :
    QFile( parent )
{
}

qint64 QgsGrassDataFile::readData( char * data, qint64 len )
{
  qint64 readSoFar = 0;
  forever
  {
    qint64 read = QFile::readData( data + readSoFar, len - readSoFar );
    if ( read == -1 )
    {
      return -1;
    }
    readSoFar += read;
    if ( readSoFar == len )
    {
      break;
    }
    // Should we sleep or select()? QFile has no waitForReadyRead() implementation.
    // Even without sleep there was not observed CPU consuming looping on Linux.

    //G_warning("len = %d readSoFar = %d", (int)len, (int)readSoFar);
  }
  return readSoFar;
}
