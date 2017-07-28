#include "utils.h"

#include "qgslinestring.h"
#include "qgspolygon.h"

#include "terraingenerator.h"



QString Utils::altClampingToString( AltitudeClamping altClamp )
{
  switch ( altClamp )
  {
    case AltClampAbsolute: return QStringLiteral( "absolute" );
    case AltClampRelative: return QStringLiteral( "relative" );
    case AltClampTerrain: return QStringLiteral( "terrain" );
    default: Q_ASSERT( false ); return QString();
  }
}


AltitudeClamping Utils::altClampingFromString( const QString &str )
{
  if ( str == "absolute" )
    return AltClampAbsolute;
  else if ( str == "terrain" )
    return AltClampTerrain;
  else   // "relative"  (default)
    return AltClampRelative;
}


QString Utils::altBindingToString( AltitudeBinding altBind )
{
  switch ( altBind )
  {
    case AltBindVertex: return QStringLiteral( "vertex" );
    case AltBindCentroid: return QStringLiteral( "centroid" );
    default: Q_ASSERT( false ); return QString();
  }
}


AltitudeBinding Utils::altBindingFromString( const QString &str )
{
  if ( str == "vertex" )
    return AltBindVertex;
  else  // "centroid"  (default)
    return AltBindCentroid;
}


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
      terrainZ = map.terrainGenerator()->heightAt( pt.x(), pt.y(), map );
    }

    float geomZ = 0;
    if ( altClamp == AltClampAbsolute || altClamp == AltClampRelative )
      geomZ = lineString->zAt( i );

    float z = ( terrainZ + geomZ ) * map.terrainVerticalScale() + height;
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


QString Utils::matrix4x4toString( const QMatrix4x4 &m )
{
  const float *d = m.constData();
  QStringList elems;
  for ( int i = 0; i < 16; ++i )
    elems << QString::number( d[i] );
  return elems.join( ' ' );
}

QMatrix4x4 Utils::stringToMatrix4x4( const QString &str )
{
  QMatrix4x4 m;
  float *d = m.data();
  QStringList elems = str.split( ' ' );
  for ( int i = 0; i < 16; ++i )
    d[i] = elems[i].toFloat();
  return m;
}
