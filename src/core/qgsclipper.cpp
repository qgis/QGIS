/***************************************************************************
                          qgsclipper.cpp  -  a class that clips line
                          segments and polygons
                             -------------------
    begin                : March 2004
    copyright            : (C) 2005 by Gavin Macaulay
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsclipper.h"
#include "qgsgeometry.h"
#include "qgswkbptr.h"
#include "qgslogger.h"

// Where has all the code gone?

// It's been inlined, so its in the qgsclipper.h file.

// But the static members must be initialized outside the class! (or GCC 4 dies)

// Qt also does clipping when the coordinates go over +/- 32767
// moreover from Qt 4.6, Qt clips also when the width/height of a painter path
// is more than 32767. Since we want to avoid clipping by Qt (because it is slow)
// we set coordinate limit to less than 32767 / 2
const double QgsClipper::MAX_X =  16000;
const double QgsClipper::MIN_X = -16000;
const double QgsClipper::MAX_Y =  16000;
const double QgsClipper::MIN_Y = -16000;

const double QgsClipper::SMALL_NUM = 1e-12;

QgsConstWkbPtr QgsClipper::clippedLineWKB( QgsConstWkbPtr& wkbPtr, const QgsRectangle& clipExtent, QPolygonF& line )
{
  QgsWKBTypes::Type wkbType = wkbPtr.readHeader();

  int nPoints;
  wkbPtr >> nPoints;

  int skipZM = ( QgsWKBTypes::coordDimensions( wkbType ) - 2 ) * sizeof( double );

  if ( static_cast<int>( nPoints * ( 2 * sizeof( double ) + skipZM ) ) > wkbPtr.remaining() )
  {
    QgsDebugMsg( QString( "%1 points exceed wkb length (%2>%3)" ).arg( nPoints ).arg( nPoints * ( 2 * sizeof( double ) + skipZM ) ).arg( wkbPtr.remaining() ) );
    return QgsConstWkbPtr( nullptr, 0 );
  }

  double p0x, p0y, p1x = 0.0, p1y = 0.0; //original coordinates
  double p1x_c, p1y_c; //clipped end coordinates
  double lastClipX = 0.0, lastClipY = 0.0; //last successfully clipped coords

  QPolygonF pts;
  wkbPtr -= sizeof( unsigned int );
  wkbPtr >> pts;
  nPoints = pts.size();

  line.clear();
  line.reserve( nPoints + 1 );

  QPointF *ptr = pts.data();

  for ( int i = 0; i < nPoints; ++i, ++ptr )
  {
    if ( i == 0 )
    {
      p1x = ptr->rx();
      p1y = ptr->ry();
      continue;
    }
    else
    {
      p0x = p1x;
      p0y = p1y;

      p1x = ptr->rx();
      p1y = ptr->ry();

      p1x_c = p1x;
      p1y_c = p1y;
      if ( clipLineSegment( clipExtent.xMinimum(), clipExtent.xMaximum(), clipExtent.yMinimum(), clipExtent.yMaximum(),
                            p0x, p0y, p1x_c,  p1y_c ) )
      {
        bool newLine = !line.isEmpty() && ( !qgsDoubleNear( p0x, lastClipX ) || !qgsDoubleNear( p0y, lastClipY ) );
        if ( newLine )
        {
          //add edge points to connect old and new line
          connectSeparatedLines( lastClipX, lastClipY, p0x, p0y, clipExtent, line );
        }
        if ( line.size() < 1 || newLine )
        {
          //add first point
          line << QPointF( p0x, p0y );
        }

        //add second point
        lastClipX = p1x_c;
        lastClipY = p1y_c;
        line << QPointF( p1x_c,  p1y_c );
      }
    }
  }
  return wkbPtr;
}

void QgsClipper::connectSeparatedLines( double x0, double y0, double x1, double y1,
                                        const QgsRectangle& clipRect, QPolygonF& pts )
{
  //test the different edge combinations...
  if ( qgsDoubleNear( x0, clipRect.xMinimum() ) )
  {
    if ( qgsDoubleNear( x1, clipRect.xMinimum() ) )
    {
      return;
    }
    else if ( qgsDoubleNear( y1, clipRect.yMaximum() ) )
    {
      pts << QPointF( clipRect.xMinimum(), clipRect.yMaximum() );
      return;
    }
    else if ( qgsDoubleNear( x1, clipRect.xMaximum() ) )
    {
      pts << QPointF( clipRect.xMinimum(), clipRect.yMaximum() );
      pts << QPointF( clipRect.xMaximum(), clipRect.yMaximum() );
      return;
    }
    else if ( qgsDoubleNear( y1, clipRect.yMinimum() ) )
    {
      pts << QPointF( clipRect.xMinimum(), clipRect.yMinimum() );
      return;
    }
  }
  else if ( qgsDoubleNear( y0, clipRect.yMaximum() ) )
  {
    if ( qgsDoubleNear( y1, clipRect.yMaximum() ) )
    {
      return;
    }
    else if ( qgsDoubleNear( x1, clipRect.xMaximum() ) )
    {
      pts << QPointF( clipRect.xMaximum(), clipRect.yMaximum() );
      return;
    }
    else if ( qgsDoubleNear( y1, clipRect.yMinimum() ) )
    {
      pts << QPointF( clipRect.xMaximum(), clipRect.yMaximum() );
      pts << QPointF( clipRect.xMaximum(), clipRect.yMinimum() );
      return;
    }
    else if ( qgsDoubleNear( x1, clipRect.xMinimum() ) )
    {
      pts << QPointF( clipRect.xMinimum(), clipRect.yMaximum() );
      return;
    }
  }
  else if ( qgsDoubleNear( x0, clipRect.xMaximum() ) )
  {
    if ( qgsDoubleNear( x1, clipRect.xMaximum() ) )
    {
      return;
    }
    else if ( qgsDoubleNear( y1, clipRect.yMinimum() ) )
    {
      pts << QPointF( clipRect.xMaximum(), clipRect.yMinimum() );
      return;
    }
    else if ( qgsDoubleNear( x1, clipRect.xMinimum() ) )
    {
      pts << QPointF( clipRect.xMaximum(), clipRect.yMinimum() );
      pts << QPointF( clipRect.xMinimum(), clipRect.yMinimum() );
      return;
    }
    else if ( qgsDoubleNear( y1, clipRect.yMaximum() ) )
    {
      pts << QPointF( clipRect.xMaximum(), clipRect.yMaximum() );
      return;
    }
  }
  else if ( qgsDoubleNear( y0, clipRect.yMinimum() ) )
  {
    if ( qgsDoubleNear( y1, clipRect.yMinimum() ) )
    {
      return;
    }
    else if ( qgsDoubleNear( x1, clipRect.xMinimum() ) )
    {
      pts << QPointF( clipRect.xMinimum(), clipRect.yMinimum() );
      return;
    }
    else if ( qgsDoubleNear( y1, clipRect.yMaximum() ) )
    {
      pts << QPointF( clipRect.xMinimum(), clipRect.yMinimum() );
      pts << QPointF( clipRect.xMinimum(), clipRect.yMaximum() );
      return;
    }
    else if ( qgsDoubleNear( x1, clipRect.xMaximum() ) )
    {
      pts << QPointF( clipRect.xMaximum(), clipRect.yMinimum() );
      return;
    }
  }
}
