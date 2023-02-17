/***************************************************************************
  qgsvectortilemvtencoder.cpp
  --------------------------------------
  Date                 : April 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortilemvtencoder.h"

#include "qgsfeedback.h"
#include "qgslinestring.h"
#include "qgslogger.h"
#include "qgsmultilinestring.h"
#include "qgsmultipoint.h"
#include "qgsmultipolygon.h"
#include "qgspolygon.h"
#include "qgsvectorlayer.h"
#include "qgsvectortilemvtutils.h"


//! Helper class for writing of geometry commands
struct MVTGeometryWriter
{
  vector_tile::Tile_Feature *feature = nullptr;
  int resolution;
  double tileXMin, tileYMax, tileDX, tileDY;
  QPoint cursor;

  MVTGeometryWriter( vector_tile::Tile_Feature *f, int res, const QgsRectangle &tileExtent )
    : feature( f )
    , resolution( res )
    , tileXMin( tileExtent.xMinimum() )
    , tileYMax( tileExtent.yMaximum() )
    , tileDX( tileExtent.width() )
    , tileDY( tileExtent.height() )
  {
  }

  void addMoveTo( int count )
  {
    feature->add_geometry( 1 | ( count << 3 ) );
  }
  void addLineTo( int count )
  {
    feature->add_geometry( 2 | ( count << 3 ) );
  }
  void addClosePath()
  {
    feature->add_geometry( 7 | ( 1 << 3 ) );
  }

  void addPoint( const QgsPoint &pt )
  {
    addPoint( mapToTileCoordinates( pt.x(), pt.y() ) );
  }

  void addPoint( const QPoint &pt )
  {
    const qint32 vx = pt.x() - cursor.x();
    const qint32 vy = pt.y() - cursor.y();

    // (quint32)(-(qint32)((quint32)vx >> 31)) is a C/C++ compliant way
    // of doing vx >> 31, which is undefined behavior since vx is signed
    feature->add_geometry( ( ( quint32 )vx << 1 ) ^ ( ( quint32 )( -( qint32 )( ( quint32 )vx >> 31 ) ) ) );
    feature->add_geometry( ( ( quint32 )vy << 1 ) ^ ( ( quint32 )( -( qint32 )( ( quint32 )vy >> 31 ) ) ) );

    cursor = pt;
  }

  QPoint mapToTileCoordinates( double x, double y )
  {
    return QPoint( static_cast<int>( round( ( x - tileXMin ) * resolution / tileDX ) ),
                   static_cast<int>( round( ( tileYMax - y ) * resolution / tileDY ) ) );
  }
};


static void encodeLineString( const QgsLineString *lineString, bool isRing, bool reversed, MVTGeometryWriter &geomWriter )
{
  int count = lineString->numPoints();
  const double *xData = lineString->xData();
  const double *yData = lineString->yData();

  if ( isRing )
    count--;  // the last point in linear ring is repeated - but not in MVT

  // de-duplicate points
  QVector<QPoint> tilePoints;
  QPoint last( -9999, -9999 );
  tilePoints.reserve( count );
  for ( int i = 0; i < count; ++i )
  {
    const QPoint pt = geomWriter.mapToTileCoordinates( xData[i], yData[i] );
    if ( pt == last )
      continue;

    tilePoints << pt;
    last = pt;
  }
  count = tilePoints.count();

  geomWriter.addMoveTo( 1 );
  geomWriter.addPoint( tilePoints[0] );
  geomWriter.addLineTo( count - 1 );
  if ( reversed )
  {
    for ( int i = count - 1; i >= 1; --i )
      geomWriter.addPoint( tilePoints[i] );
  }
  else
  {
    for ( int i = 1; i < count; ++i )
      geomWriter.addPoint( tilePoints[i] );
  }
}

static void encodePolygon( const QgsPolygon *polygon, MVTGeometryWriter &geomWriter )
{
  const QgsLineString *exteriorRing = qgsgeometry_cast<const QgsLineString *>( polygon->exteriorRing() );
  encodeLineString( exteriorRing, true, !QgsVectorTileMVTUtils::isExteriorRing( exteriorRing ), geomWriter );
  geomWriter.addClosePath();

  for ( int i = 0; i < polygon->numInteriorRings(); ++i )
  {
    const QgsLineString *interiorRing = qgsgeometry_cast<const QgsLineString *>( polygon->interiorRing( i ) );
    encodeLineString( interiorRing, true, QgsVectorTileMVTUtils::isExteriorRing( interiorRing ), geomWriter );
    geomWriter.addClosePath();
  }
}


//


QgsVectorTileMVTEncoder::QgsVectorTileMVTEncoder( QgsTileXYZ tileID )
  : mTileID( tileID )
{
  const QgsTileMatrix tm = QgsTileMatrix::fromWebMercator( mTileID.zoomLevel() );
  mTileExtent = tm.tileExtent( mTileID );
  mCrs = tm.crs();
}

QgsVectorTileMVTEncoder::QgsVectorTileMVTEncoder( QgsTileXYZ tileID, const QgsTileMatrix &tileMatrix )
  : mTileID( tileID )
{
  mTileExtent = tileMatrix.tileExtent( mTileID );
  mCrs = tileMatrix.crs();
}

void QgsVectorTileMVTEncoder::addLayer( QgsVectorLayer *layer, QgsFeedback *feedback, QString filterExpression, QString layerName )
{
  if ( feedback && feedback->isCanceled() )
    return;

  const QgsCoordinateTransform ct( layer->crs(), mCrs, mTransformContext );

  QgsRectangle layerTileExtent = mTileExtent;
  try
  {
    QgsCoordinateTransform extentTransform = ct;
    extentTransform.setBallparkTransformsAreAppropriate( true );
    layerTileExtent = extentTransform.transformBoundingBox( layerTileExtent, Qgis::TransformDirection::Reverse );
    if ( !layerTileExtent.intersects( layer->extent() ) )
    {
      return;  // tile is completely outside of the layer'e extent
    }
  }
  catch ( const QgsCsException & )
  {
    QgsDebugMsg( "Failed to reproject tile extent to the layer" );
    return;
  }

  if ( layerName.isEmpty() )
    layerName = layer->name();

  // add buffer to both filter extent in layer CRS (for feature request) and tile extent in target CRS (for clipping)
  const double bufferRatio = static_cast<double>( mBuffer ) / mResolution;
  QgsRectangle tileExtent = mTileExtent;
  tileExtent.grow( bufferRatio * mTileExtent.width() );
  layerTileExtent.grow( bufferRatio * std::max( layerTileExtent.width(), layerTileExtent.height() ) );

  QgsFeatureRequest request;
  request.setFilterRect( layerTileExtent );
  if ( !filterExpression.isEmpty() )
    request.setFilterExpression( filterExpression );
  QgsFeatureIterator fit = layer->getFeatures( request );

  QgsFeature f;
  if ( !fit.nextFeature( f ) )
  {
    return;  // nothing to write - do not add the layer at all
  }

  vector_tile::Tile_Layer *tileLayer = tile.add_layers();
  tileLayer->set_name( layerName.toUtf8() );
  tileLayer->set_version( 2 );  // 2 means MVT spec version 2.1
  tileLayer->set_extent( static_cast<::google::protobuf::uint32>( mResolution ) );

  const QgsFields fields = layer->fields();
  for ( int i = 0; i < fields.count(); ++i )
  {
    tileLayer->add_keys( fields[i].name().toUtf8() );
  }

  do
  {
    if ( feedback && feedback->isCanceled() )
      break;

    QgsGeometry g = f.geometry();

    // reproject
    try
    {
      g.transform( ct );
    }
    catch ( const QgsCsException & )
    {
      QgsDebugMsg( "Failed to reproject geometry " + QString::number( f.id() ) );
      continue;
    }

    // clip
    g = g.clipped( tileExtent );

    f.setGeometry( g );

    addFeature( tileLayer, f );
  }
  while ( fit.nextFeature( f ) );

  mKnownValues.clear();
}

void QgsVectorTileMVTEncoder::addFeature( vector_tile::Tile_Layer *tileLayer, const QgsFeature &f )
{
  QgsGeometry g = f.geometry();
  const QgsWkbTypes::GeometryType geomType = g.type();
  const double onePixel = mTileExtent.width() / mResolution;

  if ( geomType == QgsWkbTypes::LineGeometry )
  {
    if ( g.length() < onePixel )
      return; // too short
  }
  else if ( geomType == QgsWkbTypes::PolygonGeometry )
  {
    if ( g.area() < onePixel * onePixel )
      return; // too small
  }

  vector_tile::Tile_Feature *feature = tileLayer->add_features();

  feature->set_id( static_cast<quint64>( f.id() ) );

  //
  // encode attributes
  //

  const QgsAttributes attrs = f.attributes();
  for ( int i = 0; i < attrs.count(); ++i )
  {
    const QVariant v = attrs.at( i );
    if ( QgsVariantUtils::isNull( v ) )
      continue;

    int valueIndex;
    if ( mKnownValues.contains( v ) )
    {
      valueIndex = mKnownValues[v];
    }
    else
    {
      vector_tile::Tile_Value *value = tileLayer->add_values();
      valueIndex = tileLayer->values_size() - 1;
      mKnownValues[v] = valueIndex;

      if ( v.type() == QVariant::Double )
        value->set_double_value( v.toDouble() );
      else if ( v.type() == QVariant::Int )
        value->set_int_value( v.toInt() );
      else if ( v.type() == QVariant::Bool )
        value->set_bool_value( v.toBool() );
      else
        value->set_string_value( v.toString().toUtf8().toStdString() );
    }

    feature->add_tags( static_cast<quint32>( i ) );
    feature->add_tags( static_cast<quint32>( valueIndex ) );
  }

  //
  // encode geometry
  //

  vector_tile::Tile_GeomType mvtGeomType = vector_tile::Tile_GeomType_UNKNOWN;
  if ( geomType == QgsWkbTypes::PointGeometry )
    mvtGeomType = vector_tile::Tile_GeomType_POINT;
  else if ( geomType == QgsWkbTypes::LineGeometry )
    mvtGeomType = vector_tile::Tile_GeomType_LINESTRING;
  else if ( geomType == QgsWkbTypes::PolygonGeometry )
    mvtGeomType = vector_tile::Tile_GeomType_POLYGON;
  feature->set_type( mvtGeomType );

  if ( QgsWkbTypes::isCurvedType( g.wkbType() ) )
  {
    g = QgsGeometry( g.get()->segmentize() );
  }

  MVTGeometryWriter geomWriter( feature, mResolution, mTileExtent );

  const QgsAbstractGeometry *geom = g.constGet();
  switch ( QgsWkbTypes::flatType( g.wkbType() ) )
  {
    case QgsWkbTypes::Point:
    {
      const QgsPoint *pt = static_cast<const QgsPoint *>( geom );
      geomWriter.addMoveTo( 1 );
      geomWriter.addPoint( *pt );
    }
    break;

    case QgsWkbTypes::LineString:
    {
      encodeLineString( qgsgeometry_cast<const QgsLineString *>( geom ), true, false, geomWriter );
    }
    break;

    case QgsWkbTypes::Polygon:
    {
      encodePolygon( static_cast<const QgsPolygon *>( geom ), geomWriter );
    }
    break;

    case QgsWkbTypes::MultiPoint:
    {
      const QgsMultiPoint *mpt = static_cast<const QgsMultiPoint *>( geom );
      geomWriter.addMoveTo( mpt->numGeometries() );
      for ( int i = 0; i < mpt->numGeometries(); ++i )
        geomWriter.addPoint( *mpt->pointN( i ) );
    }
    break;

    case QgsWkbTypes::MultiLineString:
    {
      const QgsMultiLineString *mls = qgsgeometry_cast<const QgsMultiLineString *>( geom );
      for ( int i = 0; i < mls->numGeometries(); ++i )
      {
        encodeLineString( mls->lineStringN( i ), true, false, geomWriter );
      }
    }
    break;

    case QgsWkbTypes::MultiPolygon:
    {
      const QgsMultiPolygon *mp = qgsgeometry_cast<const QgsMultiPolygon *>( geom );
      for ( int i = 0; i < mp->numGeometries(); ++i )
      {
        encodePolygon( mp->polygonN( i ), geomWriter );
      }
    }
    break;

    default:
      break;
  }
}


QByteArray QgsVectorTileMVTEncoder::encode() const
{
  return QByteArray::fromStdString( tile.SerializeAsString() );
}
