/***************************************************************************
                         qgscentralpointpositionmanager.cpp  -  description
                         ----------------------------------
    begin                : January 2007
    copyright            : (C) 2007 by Marco Hugentobler
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

#include "qgscentralpointpositionmanager.h"
#include "qgsgeometry.h"
#include "qgsoverlayobject.h"
#include "qgsvectoroverlay.h"
#include <map>

QgsCentralPointPositionManager::QgsCentralPointPositionManager()
{

}

QgsCentralPointPositionManager::~QgsCentralPointPositionManager()
{

}

void QgsCentralPointPositionManager::addLayer( QgsVectorLayer* vl, QList<QgsVectorOverlay*>& overlays )
{
  Q_UNUSED( vl );
  mOverlays << overlays;
}

void QgsCentralPointPositionManager::removeLayers()
{
  mOverlays.clear();
}

void QgsCentralPointPositionManager::findObjectPositions( const QgsRenderContext& context, QGis::UnitType unitType )
{
  Q_UNUSED( context );
  Q_UNUSED( unitType );
  QList<QgsVectorOverlay*>::iterator overlay_it = mOverlays.begin();
  QgsVectorOverlay* currentOverlay = 0;
  QgsPoint currentPosition;

  for ( ; overlay_it != mOverlays.end(); ++overlay_it )
  {
    currentOverlay = *overlay_it;
    if ( !currentOverlay )
    {
      continue;
    }

    QMap<QgsFeatureId, QgsOverlayObject*>* objectMap = currentOverlay->overlayObjects();
    if ( !objectMap )
    {
      continue;
    }

    QMap<QgsFeatureId, QgsOverlayObject*>::iterator object_it = objectMap->begin();
    for ( ; object_it != objectMap->end(); ++object_it )
    {
      if ( findObjectPosition( object_it.value()->geometry()->asWkb(), currentPosition ) == 0 )
      {
        object_it.value()->addPosition( currentPosition );
      }
    }
  }
}

int QgsCentralPointPositionManager::findObjectPosition( const unsigned char* wkb, QgsPoint& position ) const
{
  QGis::WkbType type;
  int currentPosition = 0; //parsing position in the wkb binary
  double currentX, currentY;
  bool hasZValue = false;

  currentPosition += 1;
  memcpy( &type, &( wkb[currentPosition] ), sizeof( int ) );
  currentPosition += sizeof( int );

  switch ( type )
  {
    case QGis::WKBMultiPoint:
    case QGis::WKBMultiPoint25D:
      currentPosition += ( 2 * sizeof( int ) + 1 );
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
      memcpy( &currentX, &( wkb[currentPosition] ), sizeof( double ) );
      currentPosition += sizeof( double );
      memcpy( &currentY, &( wkb[currentPosition] ), sizeof( double ) );
      position.setX( currentX );
      position.setY( currentY );
      return 0;

    case QGis::WKBMultiLineString25D:
    case QGis::WKBMultiLineString:
    {
      int numberOfLines;
      memcpy( &numberOfLines, &( wkb[currentPosition] ), sizeof( int ) );
      if ( numberOfLines < 1 )
      {
        return 1;
      }
      currentPosition += ( 2 * sizeof( int ) + 1 );
    }
    case QGis::WKBLineString25D:
    case QGis::WKBLineString://get the middle point
    {
      if ( type == QGis::WKBLineString25D || type == QGis::WKBMultiLineString25D )
      {
        hasZValue = true;
      }

      int numberOfPoints;
      memcpy( &numberOfPoints, &( wkb[currentPosition] ), sizeof( int ) );
      currentPosition += sizeof( int );
      if ( numberOfPoints < 1 )
      {
        return 2;
      }
      if ( numberOfPoints > 2 )
      {
        int midpoint = ( numberOfPoints - 1 ) / 2    ;
        for ( int i = 0; i < midpoint; ++i )
        {
          currentPosition += 2 * sizeof( double );
          if ( hasZValue )
          {
            currentPosition += sizeof( double );
          }
        }
      }
      double xPos, yPos;
      memcpy( &xPos, &( wkb[currentPosition] ), sizeof( double ) );
      currentPosition += sizeof( double );
      memcpy( &yPos, &( wkb[currentPosition] ), sizeof( double ) );
      position.setX( xPos );
      position.setY( yPos );
      return 0;
    }

    case QGis::WKBMultiPolygon25D:
    case QGis::WKBMultiPolygon:
    {
      int numberOfPolygons;
      memcpy( &numberOfPolygons, &( wkb[currentPosition] ), sizeof( int ) );
      if ( numberOfPolygons < 1 )
      {
        return 3;
      }
      currentPosition += sizeof( int );
      currentPosition += ( 1 + sizeof( int ) );
    }
    case QGis::WKBPolygon25D:
    case QGis::WKBPolygon: //calculate the centroid of the first ring
    {
      //2.5D or 2D type?
      if ( type == QGis::WKBPolygon25D || type == QGis::WKBMultiPolygon25D )
      {
        hasZValue = true;
      }
      //number of rings
      int numberOfRings;
      memcpy( &numberOfRings, &( wkb[currentPosition] ), sizeof( int ) );
      if ( numberOfRings < 1 )
      {
        return 4;
      }
      currentPosition += sizeof( int );

      //number of points
      int numberOfPoints;
      memcpy( &numberOfPoints, &( wkb[currentPosition] ), sizeof( int ) );
      if ( numberOfPoints < 1 )
      {
        return 5;
      }
      currentPosition += sizeof( int );

      double *x = new double[numberOfPoints];
      double *y = new double[numberOfPoints];

      for ( int i = 0; i < numberOfPoints; ++i )
      {
        memcpy( &( x[i] ), &( wkb[currentPosition] ), sizeof( double ) );
        currentPosition += sizeof( double );
        memcpy( &( y[i] ), &( wkb[currentPosition] ), sizeof( double ) );
        currentPosition += sizeof( double );
        if ( hasZValue )
        {
          currentPosition += sizeof( double );
        }
      }
      double centroidX, centroidY;
      int res = calculatePolygonCentroid( x, y, numberOfPoints, centroidX, centroidY );
      delete [] x;
      delete [] y;

      if ( res != 0 )
      {
        return 1;
      }
      else
      {
        position.setX( centroidX );
        position.setY( centroidY );
        return 0;
      }
    }

    default:
      return 6;
  }
}

int QgsCentralPointPositionManager::calculatePolygonCentroid( double x[], double y[], int numberOfPoints, double& centroidX, double& centroidY ) const
{
  register int i, j;
  double ai, atmp = 0, xtmp = 0, ytmp = 0;
  if ( numberOfPoints < 3 )
  {
    return 1;
  }

  for ( i = numberOfPoints - 1, j = 0; j < numberOfPoints; i = j, j++ )
  {
    ai = x[i] * y[j] - x[j] * y[i];
    atmp += ai;
    xtmp += ( x[j] + x[i] ) * ai;
    ytmp += ( y[j] + y[i] ) * ai;
  }
  if ( atmp == 0 )
  {
    return 2;
  }
  centroidX = xtmp / ( 3 * atmp );
  centroidY = ytmp / ( 3 * atmp );
  return 0;
}

