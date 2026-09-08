/***************************************************************************
  qgsvectorlayerchunkloader_p.h
  --------------------------------------
  Date                 : September 2026
  Copyright            : (C) 2026 by Dominik Cindric
  Email                : viper dot miniq at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsglobeutils_p.h"

#include "qgscoordinatetransform.h"
#include "qgsexception.h"
#include "qgslogger.h"
#include "qgsorientedbox3d.h"
#include "qgsvector3d.h"

#include <QString>

using namespace Qt::StringLiterals;

QgsRectangle QgsGlobeUtils::nodeIdToLonLatRect( QgsChunkNodeId id )
{
  if ( id.d == 0 )
    return QgsRectangle( -180, -90, 180, 90 );

  const double tileSize = 180.0 / std::pow( 2.0, id.d - 1 );
  const double lonMin = id.x * tileSize - 180.0;
  const double latMin = id.y * tileSize - 90.0;
  return QgsRectangle( lonMin, latMin, lonMin + tileSize, latMin + tileSize );
}

QgsChunkNodeId QgsGlobeUtils::tileIdForExtent( const QgsRectangle &lonLatExtent )
{
  QgsChunkNodeId id( 0, 0, 0 );
  if ( !lonLatExtent.isValid() )
    return id;

  bool descended = true;
  while ( descended )
  {
    descended = false;
    const int childCount = id.d == 0 ? 2 : 4;
    for ( int i = 0; i < childCount; ++i )
    {
      const int dx = i & 1, dy = id.d == 0 ? 0 : !!( i & 2 );
      const QgsChunkNodeId childId( id.d + 1, id.x * 2 + dx, id.y * 2 + dy );
      if ( nodeIdToLonLatRect( childId ).contains( lonLatExtent ) )
      {
        id = childId;
        descended = true;
        break;
      }
    }
  }
  return id;
}

QgsBox3D QgsGlobeUtils::nodeIdToBox3D( QgsChunkNodeId id, const QgsCoordinateTransform &crsToLatLon, double radiusX, double radiusY, double radiusZ )
{
  if ( id.d == 0 )
    return QgsBox3D( -radiusX, -radiusY, -radiusZ, radiusX, radiusY, radiusZ );

  if ( id.d == 1 )
    return id.x == 0 ? QgsBox3D( -radiusX, -radiusY, -radiusZ, radiusX, 0, radiusZ ) : QgsBox3D( -radiusX, 0, -radiusZ, radiusX, radiusY, radiusZ );

  const QgsRectangle rect = nodeIdToLonLatRect( id );
  const QVector<QgsVector3D> corners = {
    QgsVector3D( rect.xMinimum(), rect.yMinimum(), 0.0 ),
    QgsVector3D( rect.xMinimum(), rect.yMaximum(), 0.0 ),
    QgsVector3D( rect.xMaximum(), rect.yMinimum(), 0.0 ),
    QgsVector3D( rect.xMaximum(), rect.yMaximum(), 0.0 ),
  };

  QgsBox3D box3D;
  for ( const QgsVector3D &corner : corners )
  {
    try
    {
      const QgsVector3D transformed = crsToLatLon.transform( corner, Qgis::TransformDirection::Reverse );
      box3D.combineWith( transformed.x(), transformed.y(), transformed.z() );
    }
    catch ( const QgsCsException & )
    {
      QgsDebugError( u"Failed to transform tile corner to globe CRS"_s );
    }
  }

  return box3D;
}

QgsRectangle QgsGlobeUtils::box3DTransformedExtent( const QgsBox3D &box3D, const QgsCoordinateTransform &transform, Qgis::TransformDirection direction )
{
  QgsRectangle rect;
  const QVector<QgsVector3D> corners = QgsOrientedBox3D::fromBox3D( box3D ).corners();
  for ( const QgsVector3D &corner : corners )
  {
    try
    {
      const QgsVector3D transformed = transform.transform( corner, direction );
      if ( rect.isNull() )
        rect = QgsRectangle( transformed.x(), transformed.y(), transformed.x(), transformed.y() );
      else
        rect.combineExtentWith( transformed.x(), transformed.y() );
    }
    catch ( const QgsCsException & )
    {
      QgsDebugError( u"Failed to transform box3D corner while computing extent"_s );
    }
  }

  return rect;
}
