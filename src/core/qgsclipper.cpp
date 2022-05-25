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
#include "qgslogger.h"

// Where has all the code gone?

// It's been inlined, so its in the qgsclipper.h file.

// But the static members must be initialized outside the class! (or GCC 4 dies)

// Qt also does clipping when the coordinates go over +/- 32767
// moreover from Qt 4.6, Qt clips also when the width/height of a painter path
// is more than 32767. Since we want to avoid clipping by Qt (because it is slow)
// we set coordinate limit to less than 32767 / 2
const double QgsClipper::MAX_X = 16000;
const double QgsClipper::MIN_X = -16000;
const double QgsClipper::MAX_Y = 16000;
const double QgsClipper::MIN_Y = -16000;

const double QgsClipper::SMALL_NUM = 1e-12;

void QgsClipper::clipped3dLine( const QVector< double > &xIn, const QVector< double > &yIn, const QVector<double> &zIn, QVector<double> &x, QVector<double> &y, QVector<double> &z, const QgsBox3d &clipExtent )
{
  double p0x, p0y, p0z, p1x = 0.0, p1y = 0.0, p1z = 0.0; //original coordinates
  double p1x_c, p1y_c, p1z_c; //clipped end coordinates
  double lastClipX = 0.0, lastClipY = 0.0, lastClipZ = 0.0; // last successfully clipped coordinates

  const int nPoints = xIn.size();

  x.reserve( nPoints );
  y.reserve( nPoints );
  z.reserve( nPoints );

  const double *sourceX = xIn.data();
  const double *sourceY = yIn.data();
  const double *sourceZ = zIn.data();

  for ( int i = 0; i < nPoints; ++i )
  {
    if ( i == 0 )
    {
      p1x = *sourceX++;
      p1y = *sourceY++;
      p1z = *sourceZ++;
    }
    else
    {
      p0x = p1x;
      p0y = p1y;
      p0z = p1z;

      p1x = *sourceX++;
      p1y = *sourceY++;
      p1z = *sourceZ++;

      p1x_c = p1x;
      p1y_c = p1y;
      p1z_c = p1z;

      // TODO: should be in 3D
      if ( clipLineSegment( clipExtent, p0x, p0y, p0z, p1x_c, p1y_c, p1z_c ) )
      {
        bool newLine = !x.isEmpty() && ( !qgsDoubleNear( p0x, lastClipX )
                                         || !qgsDoubleNear( p0y, lastClipY )
                                         || !qgsDoubleNear( p0z, lastClipZ ) );
        if ( newLine )
        {
          //add edge points to connect old and new line
          // TODO: should be (really) in 3D
          connectSeparatedLines( lastClipX, lastClipY, lastClipZ, p0x, p0y, p0z, clipExtent, x, y, z );
        }
        if ( x.isEmpty() || newLine )
        {
          //add first point
          x << p0x;
          y << p0y;
          z << p0z;
        }

        //add second point
        lastClipX = p1x_c;
        lastClipY = p1y_c;
        lastClipZ = p1z_c;
        x << p1x_c;
        y << p1y_c;
        z << p1z_c;
      }
    }
  }
}

QPolygonF QgsClipper::clippedLine( const QgsCurve &curve, const QgsRectangle &clipExtent )
{
  return clippedLine( curve.asQPolygonF(), clipExtent );
}

QPolygonF QgsClipper::clippedLine( const QPolygonF &curve, const QgsRectangle &clipExtent )
{
  const int nPoints = curve.size();

  double p0x, p0y, p1x = 0.0, p1y = 0.0; //original coordinates
  double p1x_c, p1y_c; //clipped end coordinates
  double lastClipX = 0.0, lastClipY = 0.0; //last successfully clipped coords

  QPolygonF line;
  line.reserve( nPoints + 1 );

  const QPointF *curveData = curve.data();

  for ( int i = 0; i < nPoints; ++i )
  {
    if ( i == 0 )
    {
      p1x = curveData->x();
      p1y = curveData->y();
    }
    else
    {
      p0x = p1x;
      p0y = p1y;

      p1x = curveData->x();
      p1y = curveData->y();

      p1x_c = p1x;
      p1y_c = p1y;
      if ( clipLineSegment( clipExtent.xMinimum(), clipExtent.xMaximum(), clipExtent.yMinimum(), clipExtent.yMaximum(),
                            p0x, p0y, p1x_c, p1y_c ) )
      {
        const bool newLine = !line.isEmpty() && ( !qgsDoubleNear( p0x, lastClipX ) || !qgsDoubleNear( p0y, lastClipY ) );
        if ( newLine )
        {
          //add edge points to connect old and new line
          connectSeparatedLines( lastClipX, lastClipY, p0x, p0y, clipExtent, line );
        }
        if ( line.empty() || newLine )
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
    curveData++;
  }
  return line;
}

void QgsClipper::connectSeparatedLines( double x0, double y0, double x1, double y1,
                                        const QgsRectangle &clipRect, QPolygonF &pts )
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

void QgsClipper::connectSeparatedLines( double x0, double y0, double z0, double x1, double y1, double z1,
                                        const QgsBox3d &clipRect, QVector< double > &ptsX, QVector< double > &ptsY, QVector<double> &ptsZ )
{
  // TODO: really relevant and sufficient?
  double meanZ = ( z0 + z1 ) / 2.0;

  //test the different edge combinations...
  if ( qgsDoubleNear( x0, clipRect.xMinimum() ) )
  {
    if ( qgsDoubleNear( x1, clipRect.xMinimum() ) )
    {
      return;
    }
    else if ( qgsDoubleNear( y1, clipRect.yMaximum() ) )
    {
      ptsX << clipRect.xMinimum();
      ptsY << clipRect.yMaximum();
      ptsZ << meanZ;
      return;
    }
    else if ( qgsDoubleNear( x1, clipRect.xMaximum() ) )
    {
      ptsX << clipRect.xMinimum();
      ptsY << clipRect.yMaximum();
      ptsZ << meanZ;
      ptsX << clipRect.xMaximum();
      ptsY << clipRect.yMaximum();
      ptsZ << meanZ;
      return;
    }
    else if ( qgsDoubleNear( y1, clipRect.yMinimum() ) )
    {
      ptsX << clipRect.xMinimum();
      ptsY << clipRect.yMinimum();
      ptsZ << meanZ;
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
      ptsX << clipRect.xMaximum();
      ptsY << clipRect.yMaximum();
      ptsZ << meanZ;
      return;
    }
    else if ( qgsDoubleNear( y1, clipRect.yMinimum() ) )
    {
      ptsX << clipRect.xMaximum();
      ptsY << clipRect.yMaximum();
      ptsZ << meanZ;
      ptsX << clipRect.xMaximum();
      ptsY << clipRect.yMinimum();
      ptsZ << meanZ;
      return;
    }
    else if ( qgsDoubleNear( x1, clipRect.xMinimum() ) )
    {
      ptsX << clipRect.xMinimum();
      ptsY << clipRect.yMaximum();
      ptsZ << meanZ;
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
      ptsX << clipRect.xMaximum();
      ptsY << clipRect.yMinimum();
      ptsZ << meanZ;
      return;
    }
    else if ( qgsDoubleNear( x1, clipRect.xMinimum() ) )
    {
      ptsX << clipRect.xMaximum();
      ptsY << clipRect.yMinimum();
      ptsZ << meanZ;
      ptsX << clipRect.xMinimum();
      ptsY << clipRect.yMinimum();
      ptsZ << meanZ;
      return;
    }
    else if ( qgsDoubleNear( y1, clipRect.yMaximum() ) )
    {
      ptsX << clipRect.xMaximum();
      ptsY << clipRect.yMaximum();
      ptsZ << meanZ;
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
      ptsX << clipRect.xMinimum();
      ptsY << clipRect.yMinimum();
      ptsZ << meanZ;
      return;
    }
    else if ( qgsDoubleNear( y1, clipRect.yMaximum() ) )
    {
      ptsX << clipRect.xMinimum();
      ptsY << clipRect.yMinimum();
      ptsZ << meanZ;
      ptsX << clipRect.xMinimum();
      ptsY << clipRect.yMaximum();
      ptsZ << meanZ;
      return;
    }
    else if ( qgsDoubleNear( x1, clipRect.xMaximum() ) )
    {
      ptsX << clipRect.xMaximum();
      ptsY << clipRect.yMinimum();
      ptsZ << meanZ;
      return;
    }
  }
}

