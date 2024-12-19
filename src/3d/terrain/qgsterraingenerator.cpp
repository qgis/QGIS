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
#include "qgs3dutils.h"
#include "qgscoordinatetransform.h"

QgsAABB QgsTerrainGenerator::rootChunkBbox( const Qgs3DMapSettings &map ) const
{
  QgsRectangle te = Qgs3DUtils::tryReprojectExtent2D( rootChunkExtent(), crs(), map.crs(), map.transformContext() );

  float hMin, hMax;
  rootChunkHeightRange( hMin, hMax );
  return Qgs3DUtils::mapToWorldExtent( te, hMin * map.terrainVerticalScale(), hMax * map.terrainVerticalScale(), map.origin() );
}

float QgsTerrainGenerator::rootChunkError( const Qgs3DMapSettings &map ) const
{
  QgsRectangle te = Qgs3DUtils::tryReprojectExtent2D( rootChunkExtent(), crs(), map.crs(), map.transformContext() );

  // use texel size as the error
  return te.width() / map.mapTileResolution();
}

void QgsTerrainGenerator::rootChunkHeightRange( float &hMin, float &hMax ) const
{
  // TODO: makes sense to have kind of default implementation?
  hMin = 0;
  hMax = 400;
}

float QgsTerrainGenerator::heightAt( double x, double y, const Qgs3DRenderContext &context ) const
{
  Q_UNUSED( x )
  Q_UNUSED( y )
  Q_UNUSED( context )
  return 0.f;
}

QString QgsTerrainGenerator::typeToString( QgsTerrainGenerator::Type type )
{
  switch ( type )
  {
    case QgsTerrainGenerator::Flat:
      return QStringLiteral( "flat" );
    case QgsTerrainGenerator::Dem:
      return QStringLiteral( "dem" );
    case QgsTerrainGenerator::Online:
      return QStringLiteral( "online" );
    case QgsTerrainGenerator::Mesh:
      return QStringLiteral( "mesh" );
    case QgsTerrainGenerator::QuantizedMesh:
      return QStringLiteral( "quantizedmesh" );
  }
  return QString();
}

bool QgsTerrainGenerator::isValid() const
{
  return mIsValid;
}
