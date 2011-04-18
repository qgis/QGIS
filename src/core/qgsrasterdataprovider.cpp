/***************************************************************************
    qgsrasterdataprovider.cpp - DataProvider Interface for raster layers
     --------------------------------------
    Date                 : Mar 11, 2005
    Copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsrasterdataprovider.h"
#include "qgsrasterprojector.h"
#include "qgslogger.h"

#include <QTime>
#include <QMap>
#include <QByteArray>

void QgsRasterDataProvider::readBlock( int bandNo, QgsRectangle  const & viewExtent, int width, int height, QgsCoordinateReferenceSystem theSrcCRS, QgsCoordinateReferenceSystem theDestCRS, void *data )
{
  QgsDebugMsg( "Entered" );
  QgsDebugMsg( "viewExtent = " + viewExtent.toString() );

  if ( ! theSrcCRS.isValid() || ! theDestCRS.isValid() || theSrcCRS == theDestCRS )
  {
    readBlock( bandNo, viewExtent, width, height, data );
    return;
  }

  QTime time;
  time.start();

  double mMaxSrcXRes = 0;
  double mMaxSrcYRes = 0;

  if ( capabilities() & QgsRasterDataProvider::ExactResolution )
  {
    mMaxSrcXRes = extent().width() / xSize();
    mMaxSrcYRes = extent().height() / ySize();
  }

  QgsRasterProjector myProjector( theSrcCRS, theDestCRS, viewExtent, height, width, mMaxSrcXRes, mMaxSrcYRes, extent() );

  QgsDebugMsg( QString( "create projector time  (ms): %1" ).arg( time.elapsed() ) );

  // TODO: init data by nulls

  // If we zoom out too much, projector srcRows / srcCols maybe 0, which can cause problems in providers
  if ( myProjector.srcRows() <= 0 || myProjector.srcCols() <= 0 ) return;

  // Allocate memory for not projected source data
  int mySize = dataTypeSize( bandNo ) / 8;
  void *mySrcData = malloc( mySize * myProjector.srcRows() * myProjector.srcCols() );

  time.restart();

  readBlock( bandNo, myProjector.srcExtent(), myProjector.srcCols(), myProjector.srcRows(), mySrcData );

  QgsDebugMsg( QString( "read not projected block time  (ms): %1" ).arg( time.elapsed() ) );
  time.restart();

  // Project data from source
  int mySrcRow;
  int mySrcCol;
  int mySrcOffset;
  int myDestOffset;
  for ( int r = 0; r < height; r++ )
  {
    for ( int c = 0; c < width; c++ )
    {
      myProjector.srcRowCol( r, c, &mySrcRow, &mySrcCol );
      mySrcOffset = mySize * ( mySrcRow * myProjector.srcCols() + mySrcCol );
      myDestOffset = mySize * ( r * width + c );
      // retype to char is just to avoid g++ warning
      memcpy(( char* ) data + myDestOffset, ( char* )mySrcData + mySrcOffset, mySize );
    }
  }
  QgsDebugMsg( QString( "reproject block time  (ms): %1" ).arg( time.elapsed() ) );

  free( mySrcData );
}


QgsRasterDataProvider::QgsRasterDataProvider(): mDpi( -1 )
{
}

QgsRasterDataProvider::QgsRasterDataProvider( QString const & uri )
    : QgsDataProvider( uri )
    , mDpi( -1 )
{
}

//
//Random Static convenience function
//
/////////////////////////////////////////////////////////
//TODO: Change these to private function or make seprate class
// convenience function for building metadata() HTML table cells
// convenience function for creating a string list from a C style string list
QStringList QgsRasterDataProvider::cStringList2Q_( char ** stringList )
{
  QStringList strings;

  // presume null terminated string list
  for ( size_t i = 0; stringList[i]; ++i )
  {
    strings.append( stringList[i] );
  }

  return strings;

} // cStringList2Q_


QString QgsRasterDataProvider::makeTableCell( QString const & value )
{
  return "<p>\n" + value + "</p>\n";
} // makeTableCell_



// convenience function for building metadata() HTML table cells
QString QgsRasterDataProvider::makeTableCells( QStringList const & values )
{
  QString s( "<tr>" );

  for ( QStringList::const_iterator i = values.begin();
        i != values.end();
        ++i )
  {
    s += QgsRasterDataProvider::makeTableCell( *i );
  }

  s += "</tr>";

  return s;
} // makeTableCell_

QString QgsRasterDataProvider::metadata()
{
  QString s;
  return s;
}

QString QgsRasterDataProvider::capabilitiesString() const
{
  QStringList abilitiesList;

  int abilities = capabilities();

  if ( abilities & QgsRasterDataProvider::Identify )
  {
    abilitiesList += tr( "Identify" );
  }

  if ( abilities & QgsRasterDataProvider::BuildPyramids )
  {
    abilitiesList += tr( "Build Pyramids" );
  }

  QgsDebugMsg( "Capability: " + abilitiesList.join( ", " ) );

  return abilitiesList.join( ", " );
}

bool QgsRasterDataProvider::identify( const QgsPoint& thePoint, QMap<QString, QString>& theResults )
{
  theResults.clear();
  return false;
}

QString QgsRasterDataProvider::lastErrorFormat()
{
  return "text/plain";
}

QByteArray QgsRasterDataProvider::noValueBytes( int theBandNo )
{
  int type = dataType( theBandNo );
  int size = dataTypeSize( theBandNo ) / 8;
  QByteArray ba;
  ba.resize( size );
  char * data = ba.data();
  double noval = mNoDataValue[theBandNo-1];
  unsigned char uc;
  unsigned short us;
  short s;
  unsigned int ui;
  int i;
  float f;
  double d;
  switch ( type )
  {
    case QgsRasterDataProvider::Byte:
      uc = ( unsigned char )noval;
      memcpy( data, &uc, size );
      break;
    case QgsRasterDataProvider::UInt16:
      us = ( unsigned short )noval;
      memcpy( data, &us, size );
      break;
    case QgsRasterDataProvider::Int16:
      s = ( short )noval;
      memcpy( data, &s, size );
      break;
    case QgsRasterDataProvider::UInt32:
      ui = ( unsigned int )noval;
      memcpy( data, &ui, size );
      break;
    case QgsRasterDataProvider::Int32:
      i = ( int )noval;
      memcpy( data, &i, size );
      break;
    case QgsRasterDataProvider::Float32:
      f = ( float )noval;
      memcpy( data, &f, size );
      break;
    case QgsRasterDataProvider::Float64:
      d = ( double )noval;
      memcpy( data, &d, size );
      break;
    default:
      QgsLogger::warning( "GDAL data type is not supported" );
  }
  return ba;
}

// ENDS
