/***************************************************************************
    qgsrasterface.cpp - Internal raster processing modules interface
     --------------------------------------
    Date                 : Jun 21, 2012
    Copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QByteArray>
#include <QTime>

#include "qgslogger.h"
#include "qgsrasterinterface.h"

QgsRasterInterface::QgsRasterInterface( QgsRasterInterface * input )
    : mInput( input )
    , mTimeMinSize( 300 ) // dont forget resampling!
{
}

QgsRasterInterface::~QgsRasterInterface()
{
}

// To give to an image preallocated memory is the only way to avoid memcpy
// when we want to keep data but delete QImage
QImage * QgsRasterInterface::createImage( int width, int height, QImage::Format format )
{
  // Qt has its own internal function depthForFormat(), unfortunately it is not public

  QImage img( 1, 1, format );

  // We ignore QImage::Format_Mono and QImage::Format_MonoLSB ( depth 1)
  int size = width * height * img.bytesPerLine();
  uchar * data = ( uchar * ) malloc( size );
  return new QImage( data, width, height, format );
}

void * QgsRasterInterface::block( int bandNo, QgsRectangle  const & extent, int width, int height )
{
  QTime time;
  time.start();
  void * b =  readBlock( bandNo, extent, width, height );

  if ( width > mTimeMinSize && height > mTimeMinSize )
  {
    if ( mTime.size() <= bandNo )
    {
      mTime.resize( bandNo + 1 );
    }
    // QTime counts only in miliseconds
    mTime[bandNo] = time.elapsed();
    QgsDebugMsg( QString( "bandNo = %2 time = %3" ).arg( bandNo ).arg( mTime[bandNo] ) );
  }
  return b;
}

double QgsRasterInterface::time( )
{
  // We can only count give total time, because we have to subtract time of previous
  // interface(s) and we dont know how to assign bands to each other
  double t = 0;
  for ( int i = 1; i < mTime.size(); i++ )
  {
    t += mTime[i];
  }
  if ( mInput )
  {
    t -= mInput->time();
  }
  return t;
}
