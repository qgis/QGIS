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

#include "qgs3dmapscene.h"
#include "qgsabstract3dengine.h"
#include "qgsterraingenerator.h"


QImage Qgs3DUtils::captureSceneImage( QgsAbstract3DEngine &engine, Qgs3DMapScene *scene )
{
  QImage resImage;
  QEventLoop evLoop;

  auto requestImageFcn = [&engine, scene]
  {
    if ( scene->sceneState() == Qgs3DMapScene::Ready )
    {
      engine.requestCaptureImage();
    }
  };

  auto saveImageFcn = [&evLoop, &resImage]( const QImage & img )
  {
    resImage = img;
    evLoop.quit();
  };

  QMetaObject::Connection conn1 = QObject::connect( &engine, &QgsAbstract3DEngine::imageCaptured, saveImageFcn );
  QMetaObject::Connection conn2;

  if ( scene->sceneState() == Qgs3DMapScene::Ready )
  {
    requestImageFcn();
  }
  else
  {
    // first wait until scene is loaded
    conn2 = QObject::connect( scene, &Qgs3DMapScene::sceneStateChanged, requestImageFcn );
  }

  evLoop.exec();

  QObject::disconnect( conn1 );
  if ( conn2 )
    QObject::disconnect( conn2 );

  return resImage;
}


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

QString Qgs3DUtils::altClampingToString( Qgs3DTypes::AltitudeClamping altClamp )
{
  switch ( altClamp )
  {
    case Qgs3DTypes::AltClampAbsolute: return QStringLiteral( "absolute" );
    case Qgs3DTypes::AltClampRelative: return QStringLiteral( "relative" );
    case Qgs3DTypes::AltClampTerrain: return QStringLiteral( "terrain" );
    default: Q_ASSERT( false ); return QString();
  }
}


Qgs3DTypes::AltitudeClamping Qgs3DUtils::altClampingFromString( const QString &str )
{
  if ( str == QLatin1String( "absolute" ) )
    return Qgs3DTypes::AltClampAbsolute;
  else if ( str == QLatin1String( "terrain" ) )
    return Qgs3DTypes::AltClampTerrain;
  else   // "relative"  (default)
    return Qgs3DTypes::AltClampRelative;
}


QString Qgs3DUtils::altBindingToString( Qgs3DTypes::AltitudeBinding altBind )
{
  switch ( altBind )
  {
    case Qgs3DTypes::AltBindVertex: return QStringLiteral( "vertex" );
    case Qgs3DTypes::AltBindCentroid: return QStringLiteral( "centroid" );
    default: Q_ASSERT( false ); return QString();
  }
}


Qgs3DTypes::AltitudeBinding Qgs3DUtils::altBindingFromString( const QString &str )
{
  if ( str == QLatin1String( "vertex" ) )
    return Qgs3DTypes::AltBindVertex;
  else  // "centroid"  (default)
    return Qgs3DTypes::AltBindCentroid;
}

QString Qgs3DUtils::cullingModeToString( Qgs3DTypes::CullingMode mode )
{
  switch ( mode )
  {
    case Qgs3DTypes::NoCulling: return QStringLiteral( "no-culling" );
    case Qgs3DTypes::Front: return QStringLiteral( "front" );
    case Qgs3DTypes::Back: return QStringLiteral( "back" );
    case Qgs3DTypes::FrontAndBack: return QStringLiteral( "front-and-back" );
  }
  return QString();
}

Qgs3DTypes::CullingMode Qgs3DUtils::cullingModeFromString( const QString &str )
{
  if ( str == QStringLiteral( "front" ) )
    return Qgs3DTypes::Front;
  else if ( str == QStringLiteral( "back" ) )
    return Qgs3DTypes::Back;
  else if ( str == QStringLiteral( "front-and-back" ) )
    return Qgs3DTypes::FrontAndBack;
  else
    return Qgs3DTypes::NoCulling;
}

float Qgs3DUtils::clampAltitude( const QgsPoint &p, Qgs3DTypes::AltitudeClamping altClamp, Qgs3DTypes::AltitudeBinding altBind, float height, const QgsPoint &centroid, const Qgs3DMapSettings &map )
{
  float terrainZ = 0;
  if ( altClamp == Qgs3DTypes::AltClampRelative || altClamp == Qgs3DTypes::AltClampTerrain )
  {
    QgsPointXY pt = altBind == Qgs3DTypes::AltBindVertex ? p : centroid;
    terrainZ = map.terrainGenerator()->heightAt( pt.x(), pt.y(), map );
  }

  float geomZ = altClamp == Qgs3DTypes::AltClampAbsolute || altClamp == Qgs3DTypes::AltClampRelative ? p.z() : 0;

  float z = ( terrainZ + geomZ ) * map.terrainVerticalScale() + height;
  return z;
}

void Qgs3DUtils::clampAltitudes( QgsLineString *lineString, Qgs3DTypes::AltitudeClamping altClamp, Qgs3DTypes::AltitudeBinding altBind, const QgsPoint &centroid, float height, const Qgs3DMapSettings &map )
{
  for ( int i = 0; i < lineString->nCoordinates(); ++i )
  {
    float terrainZ = 0;
    if ( altClamp == Qgs3DTypes::AltClampRelative || altClamp == Qgs3DTypes::AltClampTerrain )
    {
      QgsPointXY pt;
      if ( altBind == Qgs3DTypes::AltBindVertex )
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
    if ( altClamp == Qgs3DTypes::AltClampAbsolute || altClamp == Qgs3DTypes::AltClampRelative )
      geomZ = lineString->zAt( i );

    float z = ( terrainZ + geomZ ) * map.terrainVerticalScale() + height;
    lineString->setZAt( i, z );
  }
}


bool Qgs3DUtils::clampAltitudes( QgsPolygon *polygon, Qgs3DTypes::AltitudeClamping altClamp, Qgs3DTypes::AltitudeBinding altBind, float height, const Qgs3DMapSettings &map )
{
  if ( !polygon->is3D() )
    polygon->addZValue( 0 );

  QgsPoint centroid;
  if ( altBind == Qgs3DTypes::AltBindCentroid )
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
  elems.reserve( 16 );
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

QList<QVector3D> Qgs3DUtils::positions( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsFeatureRequest &request, Qgs3DTypes::AltitudeClamping altClamp )
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
        case Qgs3DTypes::AltClampAbsolute:
        default:
          h = geomZ;
          break;
        case Qgs3DTypes::AltClampTerrain:
          h = terrainZ;
          break;
        case Qgs3DTypes::AltClampRelative:
          h = terrainZ + geomZ;
          break;
      }
      positions.append( QVector3D( pt.x() - map.origin().x(), h, -( pt.y() - map.origin().y() ) ) );
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
static inline uint outcode( QVector4D v )
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

QgsVector3D Qgs3DUtils::mapToWorldCoordinates( const QgsVector3D &mapCoords, const QgsVector3D &origin )
{
  return QgsVector3D( mapCoords.x() - origin.x(),
                      mapCoords.z() - origin.z(),
                      -( mapCoords.y() - origin.y() ) );

}

QgsVector3D Qgs3DUtils::worldToMapCoordinates( const QgsVector3D &worldCoords, const QgsVector3D &origin )
{
  return QgsVector3D( worldCoords.x() + origin.x(),
                      -worldCoords.z() + origin.y(),
                      worldCoords.y() + origin.z() );
}

QgsVector3D Qgs3DUtils::transformWorldCoordinates( const QgsVector3D &worldPoint1, const QgsVector3D &origin1, const QgsCoordinateReferenceSystem &crs1, const QgsVector3D &origin2, const QgsCoordinateReferenceSystem &crs2, const QgsCoordinateTransformContext &context )
{
  QgsVector3D mapPoint1 = worldToMapCoordinates( worldPoint1, origin1 );
  QgsVector3D mapPoint2 = mapPoint1;
  if ( crs1 != crs2 )
  {
    // reproject if necessary
    QgsCoordinateTransform ct( crs1, crs2, context );
    try
    {
      QgsPointXY pt = ct.transform( QgsPointXY( mapPoint1.x(), mapPoint1.y() ) );
      mapPoint2.set( pt.x(), pt.y(), mapPoint1.z() );
    }
    catch ( const QgsCsException & )
    {
      // bad luck, can't reproject for some reason
    }
  }
  return mapToWorldCoordinates( mapPoint2, origin2 );
}

