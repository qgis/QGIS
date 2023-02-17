/***************************************************************************
  qgsvectortileutils.cpp
  --------------------------------------
  Date                 : March 2020
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

#include "qgsvectortileutils.h"

#include <math.h>

#include <QPolygon>

#include "qgscoordinatetransform.h"
#include "qgsgeometrycollection.h"
#include "qgsfields.h"
#include "qgslogger.h"
#include "qgsmaptopixel.h"
#include "qgsrectangle.h"
#include "qgsvectorlayer.h"

#include "qgsvectortilemvtdecoder.h"
#include "qgsvectortilelayer.h"
#include "qgsvectortilerenderer.h"



QPolygon QgsVectorTileUtils::tilePolygon( QgsTileXYZ id, const QgsCoordinateTransform &ct, const QgsTileMatrix &tm, const QgsMapToPixel &mtp )
{
  QgsRectangle r = tm.tileExtent( id );
  QgsPointXY p00a = mtp.transform( ct.transform( r.xMinimum(), r.yMinimum() ) );
  QgsPointXY p11a = mtp.transform( ct.transform( r.xMaximum(), r.yMaximum() ) );
  QgsPointXY p01a = mtp.transform( ct.transform( r.xMinimum(), r.yMaximum() ) );
  QgsPointXY p10a = mtp.transform( ct.transform( r.xMaximum(), r.yMinimum() ) );
  QPolygon path;
  path << p00a.toQPointF().toPoint();
  path << p01a.toQPointF().toPoint();
  path << p11a.toQPointF().toPoint();
  path << p10a.toQPointF().toPoint();
  return path;
}

QgsFields QgsVectorTileUtils::makeQgisFields( const QSet<QString> &flds )
{
  QgsFields fields;
  QStringList fieldsSorted = qgis::setToList( flds );
  std::sort( fieldsSorted.begin(), fieldsSorted.end() );
  for ( const QString &fieldName : std::as_const( fieldsSorted ) )
  {
    fields.append( QgsField( fieldName, QVariant::String ) );
  }
  return fields;
}

double QgsVectorTileUtils::scaleToZoom( double mapScale, double z0Scale )
{
  double s0 = z0Scale;
  double tileZoom2 = log( s0 / mapScale ) / log( 2 );
  tileZoom2 -= 1;   // TODO: it seems that map scale is double (is that because of high-dpi screen?)
  return tileZoom2;
}

int QgsVectorTileUtils::scaleToZoomLevel( double mapScale, int sourceMinZoom, int sourceMaxZoom, double z0Scale )
{
  int tileZoom = static_cast<int>( round( scaleToZoom( mapScale, z0Scale ) ) );

  if ( tileZoom < sourceMinZoom )
    tileZoom = sourceMinZoom;
  if ( tileZoom > sourceMaxZoom )
    tileZoom = sourceMaxZoom;

  return tileZoom;
}

QgsVectorLayer *QgsVectorTileUtils::makeVectorLayerForTile( QgsVectorTileLayer *mvt, QgsTileXYZ tileID, const QString &layerName )
{
  QgsVectorTileMVTDecoder decoder( mvt->tileMatrixSet() );
  decoder.decode( tileID, mvt->getRawTile( tileID ) );
  QSet<QString> fieldNames = qgis::listToSet( decoder.layerFieldNames( layerName ) );
  fieldNames << QStringLiteral( "_geom_type" );
  QMap<QString, QgsFields> perLayerFields;
  QgsFields fields = QgsVectorTileUtils::makeQgisFields( fieldNames );
  perLayerFields[layerName] = fields;
  QgsVectorTileFeatures data = decoder.layerFeatures( perLayerFields, QgsCoordinateTransform() );
  QgsFeatureList featuresList = data[layerName].toList();

  // turn all geometries to geom. collections (otherwise they won't be accepted by memory provider)
  for ( int i = 0; i < featuresList.count(); ++i )
  {
    QgsGeometry g = featuresList[i].geometry();
    QgsGeometryCollection *gc = new QgsGeometryCollection;
    const QgsAbstractGeometry *gg = g.constGet();
    if ( const QgsGeometryCollection *ggc = qgsgeometry_cast<const QgsGeometryCollection *>( gg ) )
    {
      for ( int k = 0; k < ggc->numGeometries(); ++k )
        gc->addGeometry( ggc->geometryN( k )->clone() );
    }
    else
      gc->addGeometry( gg->clone() );
    featuresList[i].setGeometry( QgsGeometry( gc ) );
  }

  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "GeometryCollection" ), layerName, QStringLiteral( "memory" ) );
  vl->dataProvider()->addAttributes( fields.toList() );
  vl->updateFields();
  bool res = vl->dataProvider()->addFeatures( featuresList );
  Q_UNUSED( res )
  Q_ASSERT( res );
  Q_ASSERT( featuresList.count() == vl->featureCount() );
  vl->updateExtents();
  QgsDebugMsgLevel( QStringLiteral( "Layer %1 features %2" ).arg( layerName ).arg( vl->featureCount() ), 2 );
  return vl;
}


QString QgsVectorTileUtils::formatXYZUrlTemplate( const QString &url, QgsTileXYZ tile, const QgsTileMatrix &tileMatrix )
{
  QString turl( url );

  turl.replace( QLatin1String( "{x}" ), QString::number( tile.column() ), Qt::CaseInsensitive );
  if ( turl.contains( QLatin1String( "{-y}" ) ) )
  {
    turl.replace( QLatin1String( "{-y}" ), QString::number( tileMatrix.matrixHeight() - tile.row() - 1 ), Qt::CaseInsensitive );
  }
  else
  {
    turl.replace( QLatin1String( "{y}" ), QString::number( tile.row() ), Qt::CaseInsensitive );
  }
  turl.replace( QLatin1String( "{z}" ), QString::number( tile.zoomLevel() ), Qt::CaseInsensitive );
  return turl;
}

bool QgsVectorTileUtils::checkXYZUrlTemplate( const QString &url )
{
  return url.contains( QStringLiteral( "{x}" ) ) &&
         ( url.contains( QStringLiteral( "{y}" ) ) || url.contains( QStringLiteral( "{-y}" ) ) ) &&
         url.contains( QStringLiteral( "{z}" ) );
}

//! a helper class for ordering tile requests according to the distance from view center
struct LessThanTileRequest
{
  QPointF center;  //!< Center in tile matrix (!) coordinates
  bool operator()( QgsTileXYZ req1, QgsTileXYZ req2 )
  {
    QPointF p1( req1.column() + 0.5, req1.row() + 0.5 );
    QPointF p2( req2.column() + 0.5, req2.row() + 0.5 );
    // using chessboard distance (loading order more natural than euclidean/manhattan distance)
    double d1 = std::max( std::fabs( center.x() - p1.x() ), std::fabs( center.y() - p1.y() ) );
    double d2 = std::max( std::fabs( center.x() - p2.x() ), std::fabs( center.y() - p2.y() ) );
    return d1 < d2;
  }
};

QVector<QgsTileXYZ> QgsVectorTileUtils::tilesInRange( QgsTileRange range, int zoomLevel )
{
  QVector<QgsTileXYZ> tiles;
  for ( int tileRow = range.startRow(); tileRow <= range.endRow(); ++tileRow )
  {
    for ( int tileColumn = range.startColumn(); tileColumn <= range.endColumn(); ++tileColumn )
    {
      tiles.append( QgsTileXYZ( tileColumn, tileRow, zoomLevel ) );
    }
  }
  return tiles;
}

void QgsVectorTileUtils::sortTilesByDistanceFromCenter( QVector<QgsTileXYZ> &tiles, QPointF center )
{
  LessThanTileRequest cmp;
  cmp.center = center;
  std::sort( tiles.begin(), tiles.end(), cmp );
}
