#include "utils.h"

#include "qgslinestring.h"
#include "qgspolygon.h"

#include "terraingenerator.h"


void Utils::clampAltitudes( QgsLineString *lineString, AltitudeClamping altClamp, AltitudeBinding altBind, const QgsPoint &centroid, float height, const Map3D &map )
{
  for ( int i = 0; i < lineString->nCoordinates(); ++i )
  {
    float terrainZ = 0;
    if ( altClamp == AltClampRelative || altClamp == AltClampTerrain )
    {
      QgsPointXY pt;
      if ( altBind == AltBindVertex )
      {
        pt.setX( lineString->xAt( i ) );
        pt.setY( lineString->yAt( i ) );
      }
      else
      {
        pt.set( centroid.x(), centroid.y() );
      }
      terrainZ = map.terrainGenerator->heightAt( pt.x(), pt.y(), map );
    }

    float geomZ = 0;
    if ( altClamp == AltClampAbsolute || altClamp == AltClampRelative )
      geomZ = lineString->zAt( i );

    float z = ( terrainZ + geomZ ) * map.zExaggeration + height;
    lineString->setZAt( i, z );
  }
}


bool Utils::clampAltitudes( QgsPolygonV2 *polygon, AltitudeClamping altClamp, AltitudeBinding altBind, float height, const Map3D &map )
{
  if ( !polygon->is3D() )
    polygon->addZValue( 0 );

  QgsPoint centroid;
  if ( altBind == AltBindCentroid )
    centroid = polygon->centroid();

  QgsCurve *curve = const_cast<QgsCurve *>( polygon->exteriorRing() );
  QgsLineString *lineString = dynamic_cast<QgsLineString *>( curve );
  if ( !lineString )
    return false;

  clampAltitudes( lineString, altClamp, altBind, centroid, height, map );

  for ( int i = 0; i < polygon->numInteriorRings(); ++i )
  {
    QgsCurve *curve = const_cast<QgsCurve *>( polygon->interiorRing( i ) );
    QgsLineString *lineString = dynamic_cast<QgsLineString *>( curve );
    if ( !lineString )
      return false;

    clampAltitudes( lineString, altClamp, altBind, centroid, height, map );
  }
  return true;
}
