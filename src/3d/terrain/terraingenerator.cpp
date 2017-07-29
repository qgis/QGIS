#include "terraingenerator.h"

#include "aabb.h"
#include "map3d.h"


AABB TerrainGenerator::rootChunkBbox( const Map3D &map ) const
{
  QgsRectangle te = extent();
  QgsCoordinateTransform terrainToMapTransform( crs(), map.crs );
  te = terrainToMapTransform.transformBoundingBox( te );

  float hMin, hMax;
  rootChunkHeightRange( hMin, hMax );
  return AABB( te.xMinimum() - map.originX, hMin * map.terrainVerticalScale(), -te.yMaximum() + map.originY,
               te.xMaximum() - map.originX, hMax * map.terrainVerticalScale(), -te.yMinimum() + map.originY );
}

float TerrainGenerator::rootChunkError( const Map3D &map ) const
{
  QgsRectangle te = extent();
  QgsCoordinateTransform terrainToMapTransform( crs(), map.crs );
  te = terrainToMapTransform.transformBoundingBox( te );

  // use texel size as the error
  return te.width() / map.mapTileResolution();
}

void TerrainGenerator::rootChunkHeightRange( float &hMin, float &hMax ) const
{
  // TODO: makes sense to have kind of default implementation?
  hMin = 0;
  hMax = 400;
}

float TerrainGenerator::heightAt( double x, double y, const Map3D &map ) const
{
  Q_UNUSED( x );
  Q_UNUSED( y );
  Q_UNUSED( map );
  return 0.f;
}

QString TerrainGenerator::typeToString( TerrainGenerator::Type type )
{
  switch ( type )
  {
    case TerrainGenerator::Flat:
      return "flat";
    case TerrainGenerator::Dem:
      return "dem";
    case TerrainGenerator::QuantizedMesh:
      return "quantized-mesh";
  }
  return QString();
}
