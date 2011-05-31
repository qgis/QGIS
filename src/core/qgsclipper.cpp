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

// Where has all the code gone?

// It's been inlined, so its in the qgsclipper.h file.

// But the static members must be initialised outside the class! (or GCC 4 dies)

// Qt also does clipping when the coordinates go over +/- 32767
// moreover from Qt 4.6, Qt clips also when the width/height of a painter path
// is more than 32767. Since we want to avoid clipping by Qt (because it is slow)
// we set coordinate limit to less than 32767 / 2
const double QgsClipper::MAX_X =  16000;
const double QgsClipper::MIN_X = -16000;
const double QgsClipper::MAX_Y =  16000;
const double QgsClipper::MIN_Y = -16000;

const double QgsClipper::SMALL_NUM = 1e-12;

unsigned char* QgsClipper::clippedLineWKB( unsigned char* wkb, const QgsRectangle& clipExtent, QPolygonF& line )
{
  wkb++; // jump over endian info
  unsigned int wkbType = *(( int* ) wkb );
  wkb += sizeof( unsigned int );
  unsigned int nPoints = *(( int* ) wkb );
  wkb += sizeof( unsigned int );

  bool hasZValue = ( wkbType == QGis::WKBLineString25D );

  double p0x, p0y, p1x, p1y; //original coordinates
  double p1x_c, p1y_c; //clipped end coordinates
  double lastClipX, lastClipY; //last successfully clipped coords

  line.reserve( nPoints + 1 );
  line.clear();

  for ( unsigned int i = 0; i < nPoints; ++i )
  {
    if ( i == 0 )
    {
      memcpy( &p1x, wkb, sizeof( double ) );
      wkb += sizeof( double );
      memcpy( &p1y, wkb, sizeof( double ) );
      wkb += sizeof( double );
      if ( hasZValue ) // ignore Z value
      {
        wkb += sizeof( double );
      }
      continue;
    }
    else
    {
      p0x = p1x;
      p0y = p1y;

      memcpy( &p1x, wkb, sizeof( double ) );
      wkb += sizeof( double );
      memcpy( &p1y, wkb, sizeof( double ) );
      wkb += sizeof( double );
      if ( hasZValue ) // ignore Z value
      {
        wkb += sizeof( double );
      }

      p1x_c = p1x; p1y_c = p1y;
      if ( clipLineSegment( clipExtent.xMinimum(), clipExtent.xMaximum(), clipExtent.yMinimum(), clipExtent.yMaximum(),
                            p0x, p0y, p1x_c,  p1y_c ) )
      {
        bool newLine = line.size() > 0 && ( p0x != lastClipX || p0y != lastClipY );
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
        lastClipX = p1x_c; lastClipY = p1y_c;
        line << QPointF( p1x_c,  p1y_c );
      }
    }
  }
  return wkb;
}

void QgsClipper::connectSeparatedLines( double x0, double y0, double x1, double y1,
                                        const QgsRectangle& clipRect, QPolygonF& pts )
{
  //test the different edge combinations...
  if ( doubleNear( x0, clipRect.xMinimum() ) )
  {
    if ( doubleNear( x1, clipRect.xMinimum() ) )
    {
      return;
    }
    else if ( doubleNear( y1, clipRect.yMaximum() ) )
    {
      pts << QPointF( clipRect.xMinimum(), clipRect.yMaximum() );
      return;
    }
    else if ( doubleNear( x1, clipRect.xMaximum() ) )
    {
      pts << QPointF( clipRect.xMinimum(), clipRect.yMaximum() );
      pts << QPointF( clipRect.xMaximum(), clipRect.yMaximum() );
      return;
    }
    else if ( doubleNear( y1, clipRect.yMinimum() ) )
    {
      pts << QPointF( clipRect.xMinimum(), clipRect.yMinimum() );
      return;
    }
  }
  else if ( doubleNear( y0, clipRect.yMaximum() ) )
  {
    if ( doubleNear( y1, clipRect.yMaximum() ) )
    {
      return;
    }
    else if ( doubleNear( x1, clipRect.xMaximum() ) )
    {
      pts << QPointF( clipRect.xMaximum(), clipRect.yMaximum() );
      return;
    }
    else if ( doubleNear( y1, clipRect.yMinimum() ) )
    {
      pts << QPointF( clipRect.xMaximum(), clipRect.yMaximum() );
      pts << QPointF( clipRect.xMaximum(), clipRect.yMinimum() );
      return;
    }
    else if ( doubleNear( x1, clipRect.xMinimum() ) )
    {
      pts << QPointF( clipRect.xMinimum(), clipRect.yMaximum() );
      return;
    }
  }
  else if ( doubleNear( x0, clipRect.xMaximum() ) )
  {
    if ( doubleNear( x1, clipRect.xMaximum() ) )
    {
      return;
    }
    else if ( doubleNear( y1, clipRect.yMinimum() ) )
    {
      pts << QPointF( clipRect.xMaximum(), clipRect.yMinimum() );
      return;
    }
    else if ( doubleNear( x1, clipRect.xMinimum() ) )
    {
      pts << QPointF( clipRect.xMaximum(), clipRect.yMinimum() );
      pts << QPointF( clipRect.xMinimum(), clipRect.yMinimum() );
      return;
    }
    else if ( doubleNear( y1, clipRect.yMaximum() ) )
    {
      pts << QPointF( clipRect.xMaximum(), clipRect.yMaximum() );
      return;
    }
  }
  else if ( doubleNear( y0, clipRect.yMinimum() ) )
  {
    if ( doubleNear( y1, clipRect.yMinimum() ) )
    {
      return;
    }
    else if ( doubleNear( x1, clipRect.xMinimum() ) )
    {
      pts << QPointF( clipRect.xMinimum(), clipRect.yMinimum() );
      return;
    }
    else if ( doubleNear( y1, clipRect.yMaximum() ) )
    {
      pts << QPointF( clipRect.xMinimum(), clipRect.yMinimum() );
      pts << QPointF( clipRect.xMinimum(), clipRect.yMaximum() );
      return;
    }
    else if ( doubleNear( x1, clipRect.xMaximum() ) )
    {
      pts << QPointF( clipRect.xMaximum(), clipRect.yMinimum() );
      return;
    }
  }
}
