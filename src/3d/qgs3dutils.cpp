/***************************************************************************
  qgs3dutils.cpp
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

#include "qgs3dutils.h"

#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgsfeaturerequest.h"
#include "qgsfeatureiterator.h"
#include "qgsfeature.h"
#include "qgsabstractgeometry.h"
#include "qgsvectorlayer.h"

#include "qgsterraingenerator.h"



int Qgs3DUtils::maxZoomLevel( double tile0width, double tileResolution, double maxError )
{
  if ( maxError <= 0 || tileResolution <= 0 || tile0width <= 0 )
    return 0;  // invalid input

  // derived from:
  // tile width [map units] = tile0width / 2^zoomlevel
  // tile error [map units] = tile width / tile resolution
  // + re-arranging to get zoom level if we know tile error we want to get
  double zoomLevel = -log( tileResolution * maxError / tile0width ) / log( 2 );
  return round( zoomLevel );  // we could use ceil() here if we wanted to always get to the desired error
}

QString Qgs3DUtils::altClampingToString( AltitudeClamping altClamp )
{
  switch ( altClamp )
  {
    case AltClampAbsolute: return QStringLiteral( "absolute" );
    case AltClampRelative: return QStringLiteral( "relative" );
    case AltClampTerrain: return QStringLiteral( "terrain" );
    default: Q_ASSERT( false ); return QString();
  }
}


AltitudeClamping Qgs3DUtils::altClampingFromString( const QString &str )
{
  if ( str == "absolute" )
    return AltClampAbsolute;
  else if ( str == "terrain" )
    return AltClampTerrain;
  else   // "relative"  (default)
    return AltClampRelative;
}


QString Qgs3DUtils::altBindingToString( AltitudeBinding altBind )
{
  switch ( altBind )
  {
    case AltBindVertex: return QStringLiteral( "vertex" );
    case AltBindCentroid: return QStringLiteral( "centroid" );
    default: Q_ASSERT( false ); return QString();
  }
}


AltitudeBinding Qgs3DUtils::altBindingFromString( const QString &str )
{
  if ( str == "vertex" )
    return AltBindVertex;
  else  // "centroid"  (default)
    return AltBindCentroid;
}


void Qgs3DUtils::clampAltitudes( QgsLineString *lineString, AltitudeClamping altClamp, AltitudeBinding altBind, const QgsPoint &centroid, float height, const Qgs3DMapSettings &map )
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


bool Qgs3DUtils::clampAltitudes( QgsPolygon *polygon, AltitudeClamping altClamp, AltitudeBinding altBind, float height, const Qgs3DMapSettings &map )
{
  if ( !polygon->is3D() )
    polygon->addZValue( 0 );

  QgsPoint centroid;
  if ( altBind == AltBindCentroid )
    centroid = polygon->centroid();

  QgsCurve *curve = const_cast<QgsCurve *>( polygon->exteriorRing() );
  QgsLineString *lineString = qgsgeometry_cast<QgsLineString *>( curve );
  if ( !lineString )
    return false;

  clampAltitudes( lineString, altClamp, altBind, centroid, height, map );

  for ( int i = 0; i < polygon->numInteriorRings(); ++i )
  {
    QgsCurve *curve = const_cast<QgsCurve *>( polygon->interiorRing( i ) );
    QgsLineString *lineString = qgsgeometry_cast<QgsLineString *>( curve );
    if ( !lineString )
      return false;

    clampAltitudes( lineString, altClamp, altBind, centroid, height, map );
  }
  return true;
}


QString Qgs3DUtils::matrix4x4toString( const QMatrix4x4 &m )
{
  const float *d = m.constData();
  QStringList elems;
  for ( int i = 0; i < 16; ++i )
    elems << QString::number( d[i] );
  return elems.join( ' ' );
}

QMatrix4x4 Qgs3DUtils::stringToMatrix4x4( const QString &str )
{
  QMatrix4x4 m;
  float *d = m.data();
  QStringList elems = str.split( ' ' );
  for ( int i = 0; i < 16; ++i )
    d[i] = elems[i].toFloat();
  return m;
}

QList<QVector3D> Qgs3DUtils::positions( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsFeatureRequest &request, AltitudeClamping altClamp )
{
  QList<QVector3D> positions;
  QgsFeature f;
  QgsFeatureIterator fi = layer->getFeatures( request );
  while ( fi.nextFeature( f ) )
  {
    if ( f.geometry().isNull() )
      continue;

    const QgsAbstractGeometry *g = f.geometry().constGet();
    for ( auto it = g->vertices_begin(); it != g->vertices_end(); ++it )
    {
      QgsPoint pt = *it;
      float geomZ = 0;
      if ( pt.is3D() )
      {
        geomZ = pt.z();
      }
      float terrainZ = map.terrainGenerator()->heightAt( pt.x(), pt.y(), map ) * map.terrainVerticalScale();
      float h;
      switch ( altClamp )
      {
        case AltClampAbsolute:
          h = geomZ;
          break;
        case AltClampTerrain:
          h = terrainZ;
          break;
        case AltClampRelative:
          h = terrainZ + geomZ;
          break;
      }
      positions.append( QVector3D( pt.x() - map.originX(), h, -( pt.y() - map.originY() ) ) );
      //qDebug() << positions.last();
    }
  }

  return positions;
}

/**
 * copied from https://searchcode.com/codesearch/view/35195518/
 * qt3d /src/threed/painting/qglpainter.cpp
 * no changes in the code
 */
static inline uint outcode( const QVector4D &v )
{
  // For a discussion of outcodes see pg 388 Dunn & Parberry.
  // For why you can't just test if the point is in a bounding box
  // consider the case where a view frustum with view-size 1.5 x 1.5
  // is tested against a 2x2 box which encloses the near-plane, while
  // all the points in the box are outside the frustum.
  // TODO: optimise this with assembler - according to D&P this can
  // be done in one line of assembler on some platforms
  uint code = 0;
  if ( v.x() < -v.w() ) code |= 0x01;
  if ( v.x() > v.w() )  code |= 0x02;
  if ( v.y() < -v.w() ) code |= 0x04;
  if ( v.y() > v.w() )  code |= 0x08;
  if ( v.z() < -v.w() ) code |= 0x10;
  if ( v.z() > v.w() )  code |= 0x20;
  return code;
}


/**
 * coarse box vs frustum test for culling.
 * corners of oriented box are transformed to clip space
 * and there is a test that all points are on the wrong side of the same plane
 * see http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-testing-boxes/
 *
 * should be equivalent to https://searchcode.com/codesearch/view/35195518/
 * qt3d /src/threed/painting/qglpainter.cpp
 * bool QGLPainter::isCullable(const QBox3D& box) const
 */
bool Qgs3DUtils::isCullable( const QgsAABB &bbox, const QMatrix4x4 &viewProjectionMatrix )
{
  uint out = 0xff;

  for ( int i = 0; i < 8; ++i )
  {
    QVector4D p( ( ( i >> 0 ) & 1 ) ? bbox.xMin : bbox.xMax,
                 ( ( i >> 1 ) & 1 ) ? bbox.yMin : bbox.yMax,
                 ( ( i >> 2 ) & 1 ) ? bbox.zMin : bbox.zMax, 1 );
    QVector4D pc = viewProjectionMatrix * p;

    // if the logical AND of all the outcodes is non-zero then the BB is
    // definitely outside the view frustum.
    out = out & outcode( pc );
  }
  return out;
}

