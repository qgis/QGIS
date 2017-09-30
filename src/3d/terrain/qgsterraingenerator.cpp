/***************************************************************************
  qgsterraingenerator.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsterraingenerator.h"

#include "qgsaabb.h"
#include "qgs3dmapsettings.h"


QgsAABB QgsTerrainGenerator::rootChunkBbox( const Qgs3DMapSettings &map ) const
{
  QgsRectangle te = extent();
  QgsCoordinateTransform terrainToMapTransform( crs(), map.crs() );
  te = terrainToMapTransform.transformBoundingBox( te );

  float hMin, hMax;
  rootChunkHeightRange( hMin, hMax );
  return QgsAABB( te.xMinimum() - map.originX(), hMin * map.terrainVerticalScale(), -te.yMaximum() + map.originY(),
                  te.xMaximum() - map.originX(), hMax * map.terrainVerticalScale(), -te.yMinimum() + map.originY() );
}

float QgsTerrainGenerator::rootChunkError( const Qgs3DMapSettings &map ) const
{
  QgsRectangle te = extent();
  QgsCoordinateTransform terrainToMapTransform( crs(), map.crs() );
  te = terrainToMapTransform.transformBoundingBox( te );

  // use texel size as the error
  return te.width() / map.mapTileResolution();
}

void QgsTerrainGenerator::rootChunkHeightRange( float &hMin, float &hMax ) const
{
  // TODO: makes sense to have kind of default implementation?
  hMin = 0;
  hMax = 400;
}

float QgsTerrainGenerator::heightAt( double x, double y, const Qgs3DMapSettings &map ) const
{
  Q_UNUSED( x );
  Q_UNUSED( y );
  Q_UNUSED( map );
  return 0.f;
}

QString QgsTerrainGenerator::typeToString( QgsTerrainGenerator::Type type )
{
  switch ( type )
  {
    case QgsTerrainGenerator::Flat:
      return "flat";
    case QgsTerrainGenerator::Dem:
      return "dem";
    case QgsTerrainGenerator::QuantizedMesh:
      return "quantized-mesh";
  }
  return QString();
}
