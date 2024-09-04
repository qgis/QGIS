/***************************************************************************
    qgsmaptoolidentify.cpp  -  map tool for identifying features
    ---------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsdistancearea.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturestore.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgsgeometryengine.h"
#include "qgsidentifymenu.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolidentify.h"
#include "qgsmeshlayer.h"
#include "qgsmaplayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsrasteridentifyresult.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayertemporalproperties.h"
#include "qgsvectortilelayer.h"
#include "qgsvectortileloader.h"
#include "qgsvectortilemvtdecoder.h"
#include "qgsvectortileutils.h"
#include "qgsproject.h"
#include "qgsrenderer.h"
#include "qgstiles.h"
#include "qgsgeometryutils.h"
#include "qgsgeometrycollection.h"
#include "qgscurve.h"
#include "qgscoordinateutils.h"
#include "qgsexception.h"
#include "qgssettings.h"
#include "qgsexpressioncontextutils.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudrenderer.h"
#include "qgspointcloudlayerelevationproperties.h"
#include "qgsrasterlayerelevationproperties.h"
#include "qgssymbol.h"
#include "qgsguiutils.h"
#include "qgsmessagelog.h"

#include <QMouseEvent>
#include <QCursor>
#include <QPixmap>
#include <QStatusBar>
#include <QVariant>

QgsMapToolIdentify::QgsMapToolIdentify( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
  , mIdentifyMenu( new QgsIdentifyMenu( mCanvas ) )
  , mLastMapUnitsPerPixel( -1.0 )
  , mCoordinatePrecision( 6 )
{
  setCursor( QgsApplication::getThemeCursor( QgsApplication::Cursor::Identify ) );
}

QgsMapToolIdentify::~QgsMapToolIdentify()
{
  delete mIdentifyMenu;
}

void QgsMapToolIdentify::canvasMoveEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
}

void QgsMapToolIdentify::canvasPressEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
}

void QgsMapToolIdentify::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
}

QList<QgsMapToolIdentify::IdentifyResult> QgsMapToolIdentify::identify( int x, int y, const QList<QgsMapLayer *> &layerList, IdentifyMode mode, const QgsIdentifyContext &identifyContext )
{
  return identify( x, y, mode, layerList, AllLayers, identifyContext );
}

QList<QgsMapToolIdentify::IdentifyResult> QgsMapToolIdentify::identify( int x, int y, IdentifyMode mode, LayerType layerType, const QgsIdentifyContext &identifyContext )
{
  return identify( x, y, mode, QList<QgsMapLayer *>(), layerType, identifyContext );
}

QList<QgsMapToolIdentify::IdentifyResult> QgsMapToolIdentify::identify( int x, int y, IdentifyMode mode, const QList<QgsMapLayer *> &layerList, LayerType layerType, const QgsIdentifyContext &identifyContext )
{
  return identify( QgsGeometry::fromPointXY( toMapCoordinates( QPoint( x, y ) ) ), mode, layerList, layerType, identifyContext );
}

QList<QgsMapToolIdentify::IdentifyResult> QgsMapToolIdentify::identify( const QgsGeometry &geometry, IdentifyMode mode, LayerType layerType, const QgsIdentifyContext &identifyContext )
{
  return identify( geometry, mode, QList<QgsMapLayer *>(), layerType, identifyContext );
}

QList<QgsMapToolIdentify::IdentifyResult> QgsMapToolIdentify::identify( const QgsGeometry &geometry, IdentifyMode mode, const QList<QgsMapLayer *> &layerList, LayerType layerType, const QgsIdentifyContext &identifyContext )
{
  QList<IdentifyResult> results;

  mLastGeometry = geometry;
  mLastExtent = mCanvas->extent();
  mLastMapUnitsPerPixel = mCanvas->mapUnitsPerPixel();

  mCoordinatePrecision = QgsCoordinateUtils::calculateCoordinatePrecision( mLastMapUnitsPerPixel, mCanvas->mapSettings().destinationCrs() );

  if ( mode == DefaultQgsSetting )
  {
    QgsSettings settings;
    mode = settings.enumValue( QStringLiteral( "Map/identifyMode" ), ActiveLayer );
  }

  if ( mode == LayerSelection )
  {
    QPoint canvasPt = toCanvasCoordinates( geometry.asPoint() );
    int x = canvasPt.x(), y = canvasPt.y();
    QList<IdentifyResult> results = identify( x, y, TopDownAll, layerList, layerType, identifyContext );
    QPoint globalPos = mCanvas->mapToGlobal( QPoint( x + 5, y + 5 ) );
    return mIdentifyMenu->exec( results, globalPos );
  }
  else if ( mode == ActiveLayer && layerList.isEmpty() )
  {
    QgsMapLayer *layer = mCanvas->currentLayer();

    if ( !layer )
    {
      emit identifyMessage( tr( "No active layer. To identify features, you must choose an active layer." ) );
      return results;
    }
    if ( !layer->flags().testFlag( QgsMapLayer::Identifiable ) )
      return results;

    QApplication::setOverrideCursor( Qt::WaitCursor );

    identifyLayer( &results, layer, mLastGeometry, mLastExtent, mLastMapUnitsPerPixel, layerType, identifyContext );
  }
  else
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );

    QList< QgsMapLayer * > targetLayers;
    if ( layerList.isEmpty() )
      targetLayers = mCanvas->layers( true );
    else
      targetLayers = layerList;

    const int layerCount = targetLayers.size();
    for ( int i = 0; i < layerCount; i++ )
    {
      QgsMapLayer *layer = targetLayers.value( i );

      emit identifyProgress( i, layerCount );
      emit identifyMessage( tr( "Identifying on %1…" ).arg( layer->name() ) );

      if ( !layer->flags().testFlag( QgsMapLayer::Identifiable ) )
        continue;

      if ( identifyLayer( &results, layer,  mLastGeometry, mLastExtent, mLastMapUnitsPerPixel, layerType, identifyContext ) )
      {
        if ( mode == TopDownStopAtFirst )
          break;
      }
    }

    emit identifyProgress( layerCount, layerCount );
    emit identifyMessage( tr( "Identifying done." ) );
  }

  QApplication::restoreOverrideCursor();

  return results;
}

void QgsMapToolIdentify::setCanvasPropertiesOverrides( double searchRadiusMapUnits )
{
  mOverrideCanvasSearchRadius = searchRadiusMapUnits;
}

void QgsMapToolIdentify::restoreCanvasPropertiesOverrides()
{
  mOverrideCanvasSearchRadius = -1;
}

void QgsMapToolIdentify::activate()
{
  QgsMapTool::activate();
}

void QgsMapToolIdentify::deactivate()
{
  QgsMapTool::deactivate();
}

bool QgsMapToolIdentify::identifyLayer( QList<IdentifyResult> *results, QgsMapLayer *layer, const QgsPointXY &point, const QgsRectangle &viewExtent, double mapUnitsPerPixel, QgsMapToolIdentify::LayerType layerType, const QgsIdentifyContext &identifyContext )
{
  return identifyLayer( results, layer, QgsGeometry::fromPointXY( point ), viewExtent, mapUnitsPerPixel, layerType, identifyContext );
}

bool QgsMapToolIdentify::identifyLayer( QList<IdentifyResult> *results, QgsMapLayer *layer, const QgsGeometry &geometry, const QgsRectangle &viewExtent, double mapUnitsPerPixel, QgsMapToolIdentify::LayerType layerType, const QgsIdentifyContext &identifyContext )
{
  switch ( layer->type() )
  {
    case Qgis::LayerType::Vector:
      if ( layerType.testFlag( VectorLayer ) )
      {
        return identifyVectorLayer( results, qobject_cast<QgsVectorLayer *>( layer ), geometry, identifyContext );
      }
      break;

    case Qgis::LayerType::Raster:
      if ( layerType.testFlag( RasterLayer ) )
      {
        return identifyRasterLayer( results, qobject_cast<QgsRasterLayer *>( layer ), geometry, viewExtent, mapUnitsPerPixel, identifyContext );
      }
      break;

    case Qgis::LayerType::Mesh:
      if ( layerType.testFlag( MeshLayer ) )
      {
        return identifyMeshLayer( results, qobject_cast<QgsMeshLayer *>( layer ), geometry, identifyContext );
      }
      break;

    case Qgis::LayerType::VectorTile:
      if ( layerType.testFlag( VectorTileLayer ) )
      {
        return identifyVectorTileLayer( results, qobject_cast<QgsVectorTileLayer *>( layer ), geometry, identifyContext );
      }
      break;

    case Qgis::LayerType::PointCloud:
      if ( layerType.testFlag( PointCloudLayer ) )
      {
        return identifyPointCloudLayer( results, qobject_cast<QgsPointCloudLayer *>( layer ), geometry, identifyContext );
      }
      break;

    // not supported
    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::Group:
    case Qgis::LayerType::TiledScene:
      break;
  }
  return false;
}

bool QgsMapToolIdentify::identifyVectorLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsVectorLayer *layer, const QgsPointXY &point, const QgsIdentifyContext &identifyContext )
{
  return identifyVectorLayer( results, layer, QgsGeometry::fromPointXY( point ), identifyContext );
}

bool QgsMapToolIdentify::identifyMeshLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsMeshLayer *layer, const QgsGeometry &geometry, const QgsIdentifyContext &identifyContext )
{
  const QgsPointXY point = geometry.asPoint();  // mesh layers currently only support identification by point
  return identifyMeshLayer( results, layer, point, identifyContext );
}

bool QgsMapToolIdentify::identifyMeshLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsMeshLayer *layer, const QgsPointXY &point, const QgsIdentifyContext &identifyContext )
{
  QgsDebugMsgLevel( "point = " + point.toString(), 4 );
  if ( !layer )
    return false;

  if ( !identifyContext.zRange().isInfinite() )
  {
    if ( !layer->elevationProperties()->isVisibleInZRange( identifyContext.zRange() ) )
      return false;
  }

  double searchRadius = mOverrideCanvasSearchRadius < 0 ? searchRadiusMU( mCanvas ) : mOverrideCanvasSearchRadius;
  bool isTemporal = identifyContext.isTemporal() && layer->temporalProperties()->isActive();

  QList<QgsMeshDatasetIndex> datasetIndexList;
  int activeScalarGroup = layer->rendererSettings().activeScalarDatasetGroup();
  int activeVectorGroup = layer->rendererSettings().activeVectorDatasetGroup();

  const QList<int> allGroup = layer->enabledDatasetGroupsIndexes();
  if ( isTemporal ) //non active dataset group value are only accessible if temporal is active
  {
    const QgsDateTimeRange &time = identifyContext.temporalRange();
    if ( activeScalarGroup >= 0 )
      datasetIndexList.append( layer->activeScalarDatasetAtTime( time ) );
    if ( activeVectorGroup >= 0 && activeVectorGroup != activeScalarGroup )
      datasetIndexList.append( layer->activeVectorDatasetAtTime( time ) );

    for ( int groupIndex : allGroup )
    {
      if ( groupIndex != activeScalarGroup && groupIndex != activeVectorGroup )
        datasetIndexList.append( layer->datasetIndexAtTime( time, groupIndex ) );
    }
  }
  else
  {
    // only active dataset group
    if ( activeScalarGroup >= 0 )
      datasetIndexList.append( layer->staticScalarDatasetIndex() );
    if ( activeVectorGroup >= 0 && activeVectorGroup != activeScalarGroup )
      datasetIndexList.append( layer->staticVectorDatasetIndex() );

    // ...and static dataset group
    for ( int groupIndex : allGroup )
    {
      if ( groupIndex != activeScalarGroup && groupIndex != activeVectorGroup )
      {
        if ( !layer->datasetGroupMetadata( groupIndex ).isTemporal() )
          datasetIndexList.append( groupIndex );
      }
    }
  }

  //create results
  for ( const QgsMeshDatasetIndex &index : datasetIndexList )
  {
    if ( !index.isValid() )
      continue;

    const QgsMeshDatasetGroupMetadata &groupMeta = layer->datasetGroupMetadata( index );
    QMap< QString, QString > derivedAttributes;

    QMap<QString, QString> attribute;
    if ( groupMeta.isScalar() )
    {
      const QgsMeshDatasetValue scalarValue = layer->datasetValue( index, point, searchRadius );
      const double scalar = scalarValue.scalar();
      attribute.insert( tr( "Scalar Value" ), std::isnan( scalar ) ? tr( "no data" ) : QLocale().toString( scalar ) );
    }

    if ( groupMeta.isVector() )
    {
      const QgsMeshDatasetValue vectorValue = layer->datasetValue( index, point, searchRadius );
      const double vectorX = vectorValue.x();
      const double vectorY = vectorValue.y();
      if ( std::isnan( vectorX ) || std::isnan( vectorY ) )
        attribute.insert( tr( "Vector Value" ), tr( "no data" ) );
      else
      {
        attribute.insert( tr( "Vector Magnitude" ), QLocale().toString( vectorValue.scalar() ) );
        derivedAttributes.insert( tr( "Vector x-component" ), QLocale().toString( vectorY ) );
        derivedAttributes.insert( tr( "Vector y-component" ), QLocale().toString( vectorX ) );
      }
    }

    const QgsMeshDatasetMetadata &meta = layer->datasetMetadata( index );

    if ( groupMeta.isTemporal() )
      derivedAttributes.insert( tr( "Time Step" ), layer->formatTime( meta.time() ) );
    derivedAttributes.insert( tr( "Source" ), groupMeta.uri() );

    QString resultName = groupMeta.name();
    if ( isTemporal && ( index.group() == activeScalarGroup  || index.group() == activeVectorGroup ) )
      resultName.append( tr( " (active)" ) );

    const IdentifyResult result( layer,
                                 resultName,
                                 attribute,
                                 derivedAttributes );

    results->append( result );
  }

  QMap<QString, QString> derivedGeometry;

  QgsPointXY vertexPoint = layer->snapOnElement( QgsMesh::Vertex, point, searchRadius );
  if ( !vertexPoint.isEmpty() )
  {
    derivedGeometry.insert( tr( "Snapped Vertex Position X" ), QLocale().toString( vertexPoint.x() ) );
    derivedGeometry.insert( tr( "Snapped Vertex Position Y" ), QLocale().toString( vertexPoint.y() ) );
  }

  QgsPointXY faceCentroid = layer->snapOnElement( QgsMesh::Face, point, searchRadius );
  if ( !faceCentroid.isEmpty() )
  {
    derivedGeometry.insert( tr( "Face Centroid X" ), QLocale().toString( faceCentroid.x() ) );
    derivedGeometry.insert( tr( "Face Centroid Y" ), QLocale().toString( faceCentroid.y() ) );
  }

  QgsPointXY pointOnEdge = layer->snapOnElement( QgsMesh::Edge, point, searchRadius );
  if ( !pointOnEdge.isEmpty() )
  {
    derivedGeometry.insert( tr( "Point on Edge X" ), QLocale().toString( pointOnEdge.x() ) );
    derivedGeometry.insert( tr( "Point on Edge Y" ), QLocale().toString( pointOnEdge.y() ) );
  }

  const IdentifyResult result( layer,
                               tr( "Geometry" ),
                               derivedAttributesForPoint( QgsPoint( point ) ),
                               derivedGeometry );

  results->append( result );

  return true;
}

bool QgsMapToolIdentify::identifyVectorTileLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsVectorTileLayer *layer, const QgsGeometry &geometry, const QgsIdentifyContext &identifyContext )
{
  Q_UNUSED( identifyContext )
  if ( !layer || !layer->isSpatial() )
    return false;

  if ( !layer->isInScaleRange( mCanvas->mapSettings().scale() ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "Out of scale limits" ), 2 );
    return false;
  }

  QgsTemporaryCursorOverride waitCursor( Qt::WaitCursor );

  QMap< QString, QString > commonDerivedAttributes;

  QgsGeometry selectionGeom = geometry;
  bool isPointOrRectangle;
  QgsPointXY point;
  bool isSingleClick = selectionGeom.type() == Qgis::GeometryType::Point;
  if ( isSingleClick )
  {
    isPointOrRectangle = true;
    point = selectionGeom.asPoint();

    commonDerivedAttributes = derivedAttributesForPoint( QgsPoint( point ) );
  }
  else
  {
    // we have a polygon - maybe it is a rectangle - in such case we can avoid costly insterestion tests later
    isPointOrRectangle = QgsGeometry::fromRect( selectionGeom.boundingBox() ).isGeosEqual( selectionGeom );
  }

  int featureCount = 0;

  std::unique_ptr<QgsGeometryEngine> selectionGeomPrepared;

  // toLayerCoordinates will throw an exception for an 'invalid' point.
  // For example, if you project a world map onto a globe using EPSG 2163
  // and then click somewhere off the globe, an exception will be thrown.
  try
  {
    QgsRectangle r;
    if ( isSingleClick )
    {
      double sr = mOverrideCanvasSearchRadius < 0 ? searchRadiusMU( mCanvas ) : mOverrideCanvasSearchRadius;
      r = toLayerCoordinates( layer, QgsRectangle( point.x() - sr, point.y() - sr, point.x() + sr, point.y() + sr ) );
    }
    else
    {
      r = toLayerCoordinates( layer, selectionGeom.boundingBox() );

      if ( !isPointOrRectangle )
      {
        QgsCoordinateTransform ct( mCanvas->mapSettings().destinationCrs(), layer->crs(), mCanvas->mapSettings().transformContext() );
        if ( ct.isValid() )
          selectionGeom.transform( ct );

        // use prepared geometry for faster intersection test
        selectionGeomPrepared.reset( QgsGeometry::createGeometryEngine( selectionGeom.constGet() ) );
      }
    }

    const double tileScale = layer->tileMatrixSet().calculateTileScaleForMap(
                               mCanvas->scale(),
                               mCanvas->mapSettings().destinationCrs(),
                               mCanvas->mapSettings().extent(),
                               mCanvas->size(),
                               mCanvas->logicalDpiX() );

    const int tileZoom = layer->tileMatrixSet().scaleToZoomLevel( tileScale );
    const QgsTileMatrix tileMatrix = layer->tileMatrixSet().tileMatrix( tileZoom );
    const QgsTileRange tileRange = tileMatrix.tileRangeFromExtent( r );

    const QVector< QgsTileXYZ> tiles = layer->tileMatrixSet().tilesInRange( tileRange, tileZoom );

    for ( const QgsTileXYZ &tileID : tiles )
    {
      const QgsVectorTileRawData data = layer->getRawTile( tileID );
      if ( data.data.isEmpty() )
        continue;  // failed to get data

      QgsVectorTileMVTDecoder decoder( layer->tileMatrixSet() );
      if ( !decoder.decode( data ) )
        continue;  // failed to decode

      QMap<QString, QgsFields> perLayerFields;
      const QStringList layerNames = decoder.layers();
      for ( const QString &layerName : layerNames )
      {
        QSet<QString> fieldNames = qgis::listToSet( decoder.layerFieldNames( layerName ) );
        perLayerFields[layerName] = QgsVectorTileUtils::makeQgisFields( fieldNames );
      }

      const QgsVectorTileFeatures features = decoder.layerFeatures( perLayerFields, QgsCoordinateTransform() );
      const QStringList featuresLayerNames = features.keys();
      for ( const QString &layerName : featuresLayerNames )
      {
        const QgsFields fFields = perLayerFields[layerName];
        const QVector<QgsFeature> &layerFeatures = features[layerName];
        for ( const QgsFeature &f : layerFeatures )
        {
          if ( f.geometry().intersects( r ) && ( !selectionGeomPrepared || selectionGeomPrepared->intersects( f.geometry().constGet() ) ) )
          {
            QMap< QString, QString > derivedAttributes = commonDerivedAttributes;
            derivedAttributes.insert( tr( "Feature ID" ), FID_TO_STRING( f.id() ) );
            derivedAttributes.insert( tr( "Tile column" ), QString::number( tileID.column() ) );
            derivedAttributes.insert( tr( "Tile row" ), QString::number( tileID.row() ) );
            derivedAttributes.insert( tr( "Tile zoom" ), QString::number( tileID.zoomLevel() ) );

            results->append( IdentifyResult( layer, layerName, fFields, f, derivedAttributes ) );

            featureCount++;
          }
        }
      }
    }
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse )
    // catch exception for 'invalid' point and proceed with no features found
    QgsDebugError( QStringLiteral( "Caught CRS exception %1" ).arg( cse.what() ) );
  }

  return featureCount > 0;
}

bool QgsMapToolIdentify::identifyPointCloudLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsPointCloudLayer *layer, const QgsGeometry &geometry, const QgsIdentifyContext &identifyContext )
{
  if ( !identifyContext.zRange().isInfinite() )
  {
    if ( !layer->elevationProperties()->isVisibleInZRange( identifyContext.zRange(), layer ) )
      return false;
  }

  QgsPointCloudRenderer *renderer = layer->renderer();

  QgsRenderContext context = QgsRenderContext::fromMapSettings( mCanvas->mapSettings() );
  context.setCoordinateTransform( QgsCoordinateTransform( layer->crs(), mCanvas->mapSettings().destinationCrs(), mCanvas->mapSettings().transformContext() ) );
  if ( !identifyContext.zRange().isInfinite() )
    context.setZRange( identifyContext.zRange() );

  const double searchRadiusMapUnits = mOverrideCanvasSearchRadius < 0 ? searchRadiusMU( mCanvas ) : mOverrideCanvasSearchRadius;

  const QVector<QVariantMap> points = renderer->identify( layer, context, geometry, searchRadiusMapUnits );

  fromPointCloudIdentificationToIdentifyResults( layer, points, *results );

  return true;
}

QMap<QString, QString> QgsMapToolIdentify::derivedAttributesForPoint( const QgsPoint &point )
{
  QMap< QString, QString > derivedAttributes;

  QString x;
  QString y;
  formatCoordinate( point, x, y );

  derivedAttributes.insert( tr( "(clicked coordinate X)" ), x );
  derivedAttributes.insert( tr( "(clicked coordinate Y)" ), y );
  if ( point.is3D() )
    derivedAttributes.insert( tr( "(clicked coordinate Z)" ), QLocale().toString( point.z(), 'f' ) );
  return derivedAttributes;
}

bool QgsMapToolIdentify::identifyVectorLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsVectorLayer *layer, const QgsGeometry &geometry, const QgsIdentifyContext &identifyContext )
{
  if ( !layer || !layer->isSpatial() || !layer->dataProvider() )
    return false;

  if ( !layer->isInScaleRange( mCanvas->mapSettings().scale() ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "Out of scale limits" ), 2 );
    return false;
  }

  QString temporalFilter;
  if ( identifyContext.isTemporal() )
  {
    if ( !layer->temporalProperties()->isVisibleInTemporalRange( identifyContext.temporalRange() ) )
      return false;

    QgsVectorLayerTemporalContext temporalContext;
    temporalContext.setLayer( layer );
    temporalFilter = qobject_cast< const QgsVectorLayerTemporalProperties * >( layer->temporalProperties() )->createFilterString( temporalContext, identifyContext.temporalRange() );
  }

  const bool fetchFeatureSymbols = layer->dataProvider()->capabilities() & Qgis::VectorProviderCapability::FeatureSymbology;

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QMap< QString, QString > commonDerivedAttributes;

  QgsGeometry selectionGeom = geometry;
  bool isPointOrRectangle;
  QgsPoint point;
  bool isSingleClick = selectionGeom.type() == Qgis::GeometryType::Point;
  if ( isSingleClick )
  {
    isPointOrRectangle = true;
    point = *qgsgeometry_cast< const QgsPoint *>( selectionGeom.constGet() );

    commonDerivedAttributes = derivedAttributesForPoint( point );
  }
  else
  {
    // we have a polygon - maybe it is a rectangle - in such case we can avoid costly insterestion tests later
    isPointOrRectangle = QgsGeometry::fromRect( selectionGeom.boundingBox() ).isGeosEqual( selectionGeom );
  }

  QgsFeatureList featureList;
  std::unique_ptr<QgsGeometryEngine> selectionGeomPrepared;

  // toLayerCoordinates will throw an exception for an 'invalid' point.
  // For example, if you project a world map onto a globe using EPSG 2163
  // and then click somewhere off the globe, an exception will be thrown.
  try
  {
    QgsRectangle r;
    if ( isSingleClick )
    {
      double sr = mOverrideCanvasSearchRadius < 0 ? searchRadiusMU( mCanvas ) : mOverrideCanvasSearchRadius;
      r = toLayerCoordinates( layer, QgsRectangle( point.x() - sr, point.y() - sr, point.x() + sr, point.y() + sr ) );
    }
    else
    {
      r = toLayerCoordinates( layer, selectionGeom.boundingBox() );

      if ( !isPointOrRectangle )
      {
        QgsCoordinateTransform ct( mCanvas->mapSettings().destinationCrs(), layer->crs(), mCanvas->mapSettings().transformContext() );
        if ( ct.isValid() )
          selectionGeom.transform( ct );

        // use prepared geometry for faster intersection test
        selectionGeomPrepared.reset( QgsGeometry::createGeometryEngine( selectionGeom.constGet() ) );
      }
    }

    QgsFeatureRequest featureRequest;
    featureRequest.setFilterRect( r );
    featureRequest.setFlags( Qgis::FeatureRequestFlag::ExactIntersect | ( fetchFeatureSymbols ? Qgis::FeatureRequestFlag::EmbeddedSymbols : Qgis::FeatureRequestFlags() ) );
    if ( !temporalFilter.isEmpty() )
      featureRequest.setFilterExpression( temporalFilter );

    QgsFeatureIterator fit = layer->getFeatures( featureRequest );
    QgsFeature f;
    while ( fit.nextFeature( f ) )
    {
      if ( !selectionGeomPrepared || selectionGeomPrepared->intersects( f.geometry().constGet() ) )
        featureList << QgsFeature( f );
    }
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse )
    // catch exception for 'invalid' point and proceed with no features found
    QgsDebugError( QStringLiteral( "Caught CRS exception %1" ).arg( cse.what() ) );
  }

  bool filter = false;

  QgsRenderContext context( QgsRenderContext::fromMapSettings( mCanvas->mapSettings() ) );
  context.setExpressionContext( mCanvas->createExpressionContext() );
  context.expressionContext() << QgsExpressionContextUtils::layerScope( layer );
  std::unique_ptr< QgsFeatureRenderer > renderer( layer->renderer() ? layer->renderer()->clone() : nullptr );
  if ( renderer )
  {
    // setup scale for scale dependent visibility (rule based)
    renderer->startRender( context, layer->fields() );
    filter = renderer->capabilities() & QgsFeatureRenderer::Filter;
  }

  // When not single click identify, pass an empty point so some derived attributes may still be computed
  if ( !isSingleClick )
    point = QgsPoint();

  const int featureCount = identifyVectorLayer( results, layer, featureList, filter ? renderer.get() : nullptr, commonDerivedAttributes,
                           [point, layer, this]( const QgsFeature & feature )->QMap< QString, QString >
  {
    return featureDerivedAttributes( feature, layer, toLayerCoordinates( layer, point ) );
  }, context );

  if ( renderer )
  {
    renderer->stopRender( context );
  }
  QApplication::restoreOverrideCursor();
  return featureCount > 0;
}

int QgsMapToolIdentify::identifyVectorLayer( QList<IdentifyResult> *results, QgsVectorLayer *layer, const QgsFeatureList &features, QgsFeatureRenderer *renderer, const QMap< QString, QString > &commonDerivedAttributes, const std::function< QMap< QString, QString > ( const QgsFeature & ) > &deriveAttributesForFeature, QgsRenderContext &context )
{
  int featureCount = 0;
  for ( const QgsFeature &feature : std::as_const( features ) )
  {
    QMap< QString, QString > derivedAttributes = commonDerivedAttributes;

    QgsFeatureId fid = feature.id();
    context.expressionContext().setFeature( feature );

    if ( renderer && !renderer->willRenderFeature( feature, context ) )
      continue;

    derivedAttributes.insert( deriveAttributesForFeature( feature ) );
    derivedAttributes.insert( tr( "Feature ID" ), fid < 0 ? tr( "new feature" ) : FID_TO_STRING( fid ) );

    results->append( IdentifyResult( qobject_cast<QgsMapLayer *>( layer ), feature, derivedAttributes ) );
    featureCount++;
  }
  return featureCount;
}

void QgsMapToolIdentify::closestVertexAttributes( const QgsCoordinateTransform layerToMapTransform,
    const QgsCoordinateReferenceSystem &layerVertCrs,
    const QgsCoordinateReferenceSystem &mapVertCrs,
    const QgsAbstractGeometry &geometry, QgsVertexId vId,
    bool showTransformedZ, QMap< QString, QString > &derivedAttributes )
{
  if ( ! vId.isValid( ) )
  {
    // We should not get here ...
    QgsDebugError( "Invalid vertex id!" );
    return;
  }

  QString str = QLocale().toString( vId.vertex + 1 );
  derivedAttributes.insert( tr( "Closest vertex number" ), str );

  QgsPoint closestPoint = geometry.vertexAt( vId );
  QgsPoint closestPointMapCoords = closestPoint;
  if ( layerToMapTransform.isValid() )
  {
    try
    {
      closestPointMapCoords.transform( layerToMapTransform, Qgis::TransformDirection::Forward, layerToMapTransform.hasVerticalComponent() );
    }
    catch ( QgsCsException &cse )
    {
      QgsMessageLog::logMessage( QObject::tr( "Transform error caught: %1" ).arg( cse.what() ), QObject::tr( "CRS" ) );
    }
  }

  QString x;
  QString y;
  formatCoordinate( closestPointMapCoords, x, y );
  derivedAttributes.insert( tr( "Closest vertex X" ), x );
  derivedAttributes.insert( tr( "Closest vertex Y" ), y );

  if ( closestPoint.is3D() )
  {
    str = QLocale().toString( closestPoint.z(), 'g', 10 );
    derivedAttributes.insert( showTransformedZ ? tr( "Closest vertex Z (%1)" ).arg( layerVertCrs.userFriendlyIdentifier( Qgis::CrsIdentifierType::MediumString ) )
                              : tr( "Closest vertex Z" ), str );
  }
  if ( showTransformedZ && !std::isnan( closestPointMapCoords.z() ) && !qgsDoubleNear( closestPoint.z(), closestPointMapCoords.z() ) )
  {
    const QString str = QLocale().toString( closestPointMapCoords.z(), 'g', 10 );
    derivedAttributes.insert( tr( "Closest vertex Z (%1)" ).arg( mapVertCrs.userFriendlyIdentifier( Qgis::CrsIdentifierType::MediumString ) ), str );
  }

  if ( closestPoint.isMeasure() )
  {
    str = QLocale().toString( closestPointMapCoords.m(), 'g', 10 );
    derivedAttributes.insert( tr( "Closest vertex M" ), str );
  }

  if ( vId.type == Qgis::VertexType::Curve )
  {
    double radius, centerX, centerY;
    QgsVertexId vIdBefore = vId;
    --vIdBefore.vertex;
    QgsVertexId vIdAfter = vId;
    ++vIdAfter.vertex;
    QgsGeometryUtils::circleCenterRadius( geometry.vertexAt( vIdBefore ), geometry.vertexAt( vId ),
                                          geometry.vertexAt( vIdAfter ), radius, centerX, centerY );
    derivedAttributes.insert( QStringLiteral( "Closest vertex radius" ), QLocale().toString( radius ) );
  }
}

void QgsMapToolIdentify::closestPointAttributes( const QgsCoordinateTransform layerToMapTransform,
    const QgsCoordinateReferenceSystem &layerVertCrs,
    const QgsCoordinateReferenceSystem &mapVertCrs,
    const QgsAbstractGeometry &geometry,
    const QgsPointXY &layerPoint,
    bool showTransformedZ,
    QMap<QString, QString> &derivedAttributes )
{
  QgsPoint closestPoint = QgsGeometryUtils::closestPoint( geometry, QgsPoint( layerPoint ) );
  QgsPoint closestPointMapCrs = closestPoint;
  if ( layerToMapTransform.isValid() )
  {
    try
    {
      closestPointMapCrs.transform( layerToMapTransform, Qgis::TransformDirection::Forward, layerToMapTransform.hasVerticalComponent() );
    }
    catch ( QgsCsException &cse )
    {
      QgsMessageLog::logMessage( QObject::tr( "Transform error caught: %1" ).arg( cse.what() ), QObject::tr( "CRS" ) );
    }
  }

  QString x;
  QString y;
  formatCoordinate( closestPoint, x, y );
  derivedAttributes.insert( tr( "Closest X" ), x );
  derivedAttributes.insert( tr( "Closest Y" ), y );

  if ( closestPoint.is3D() )
  {
    const QString str = QLocale().toString( closestPoint.z(), 'g', 10 );
    derivedAttributes.insert( showTransformedZ ? tr( "Interpolated Z (%1)" ).arg( layerVertCrs.userFriendlyIdentifier( Qgis::CrsIdentifierType::MediumString ) )
                              : tr( "Interpolated Z" ), str );
  }
  if ( showTransformedZ && !std::isnan( closestPointMapCrs.z() ) && !qgsDoubleNear( closestPoint.z(), closestPointMapCrs.z() ) )
  {
    const QString str = QLocale().toString( closestPointMapCrs.z(), 'g', 10 );
    derivedAttributes.insert( tr( "Interpolated Z (%1)" ).arg( mapVertCrs.userFriendlyIdentifier( Qgis::CrsIdentifierType::MediumString ) ), str );
  }

  if ( closestPoint.isMeasure() )
  {
    const QString str = QLocale().toString( closestPoint.m(), 'g', 10 );
    derivedAttributes.insert( tr( "Interpolated M" ), str );
  }
}

void QgsMapToolIdentify::formatCoordinate( const QgsPointXY &canvasPoint, QString &x, QString &y, const QgsCoordinateReferenceSystem &mapCrs, int coordinatePrecision )
{
  QgsCoordinateUtils::formatCoordinatePartsForProject( QgsProject::instance(), canvasPoint, mapCrs,
      coordinatePrecision, x, y );
}

void QgsMapToolIdentify::formatCoordinate( const QgsPointXY &canvasPoint, QString &x, QString &y ) const
{
  formatCoordinate( canvasPoint, x, y, mCanvas->mapSettings().destinationCrs(),
                    mCoordinatePrecision );
}

QMap< QString, QString > QgsMapToolIdentify::featureDerivedAttributes( const QgsFeature &feature, QgsMapLayer *layer, const QgsPointXY &layerPoint )
{
  // Calculate derived attributes and insert:
  // measure distance or area depending on geometry type
  QMap< QString, QString > derivedAttributes;

  // init distance/area calculator
  QString ellipsoid = QgsProject::instance()->ellipsoid();
  QgsDistanceArea calc;
  calc.setEllipsoid( ellipsoid );
  calc.setSourceCrs( layer->crs(), QgsProject::instance()->transformContext() );

  Qgis::WkbType wkbType = Qgis::WkbType::NoGeometry;
  Qgis::GeometryType geometryType = Qgis::GeometryType::Null;

  QgsVertexId vId;
  QgsPoint closestPoint;
  if ( feature.hasGeometry() )
  {
    geometryType = feature.geometry().type();
    wkbType = feature.geometry().wkbType();
    if ( !layerPoint.isEmpty() )
    {
      //find closest vertex to clicked point
      closestPoint = QgsGeometryUtils::closestVertex( *feature.geometry().constGet(), QgsPoint( layerPoint ), vId );
    }
  }

  if ( QgsWkbTypes::isMultiType( wkbType ) )
  {
    QString str = QLocale().toString( static_cast<const QgsGeometryCollection *>( feature.geometry().constGet() )->numGeometries() );
    derivedAttributes.insert( tr( "Parts" ), str );
    if ( !layerPoint.isEmpty() )
    {
      str = QLocale().toString( vId.part + 1 );
      derivedAttributes.insert( tr( "Part number" ), str );
    }
  }

  Qgis::DistanceUnit cartesianDistanceUnits = QgsUnitTypes::unitType( layer->crs().mapUnits() ) == QgsUnitTypes::unitType( displayDistanceUnits() )
      ? displayDistanceUnits() : layer->crs().mapUnits();
  Qgis::AreaUnit cartesianAreaUnits = QgsUnitTypes::unitType( QgsUnitTypes::distanceToAreaUnit( layer->crs().mapUnits() ) ) == QgsUnitTypes::unitType( displayAreaUnits() )
                                      ? displayAreaUnits() : QgsUnitTypes::distanceToAreaUnit( layer->crs().mapUnits() );

  const QgsCoordinateReferenceSystem mapVertCrs = QgsProject::instance()->crs3D().verticalCrs().isValid() ? QgsProject::instance()->crs3D().verticalCrs()
      : QgsProject::instance()->crs3D();
  const QgsCoordinateReferenceSystem layerVertCrs = layer->crs3D().verticalCrs().isValid() ? layer->crs3D().verticalCrs()
      : layer->crs3D();
  const bool showTransformedZ = QgsProject::instance()->crs3D() != layer->crs3D() && QgsProject::instance()->crs3D().hasVerticalAxis() && layer->crs3D().hasVerticalAxis();
  const QgsCoordinateTransform layerToMapTransform( layer->crs3D(), QgsProject::instance()->crs3D(), QgsProject::instance()->transformContext() );

  const QgsGeometry layerCrsGeometry = feature.geometry();
  QgsGeometry mapCrsGeometry = layerCrsGeometry;
  try
  {
    if ( layerToMapTransform.isValid() )
    {
      mapCrsGeometry.transform( layerToMapTransform, Qgis::TransformDirection::Forward, layerToMapTransform.hasVerticalComponent() );
    }
  }
  catch ( QgsCsException &cse )
  {
    QgsMessageLog::logMessage( QObject::tr( "Transform error caught: %1" ).arg( cse.what() ), QObject::tr( "CRS" ) );
  }

  if ( geometryType == Qgis::GeometryType::Line )
  {
    const QgsAbstractGeometry *layerCrsGeom = layerCrsGeometry.constGet();

    double dist = calc.measureLength( feature.geometry() );
    dist = calc.convertLengthMeasurement( dist, displayDistanceUnits() );
    QString str;
    if ( ellipsoid != geoNone() )
    {
      str = formatDistance( dist );
      derivedAttributes.insert( tr( "Length (Ellipsoidal — %1)" ).arg( ellipsoid ), str );
    }

    str = formatDistance( layerCrsGeom->length()
                          * QgsUnitTypes::fromUnitToUnitFactor( layer->crs().mapUnits(), cartesianDistanceUnits ), cartesianDistanceUnits );
    if ( QgsWkbTypes::hasZ( layerCrsGeom->wkbType() )
         && QgsWkbTypes::flatType( QgsWkbTypes::singleType( layerCrsGeom->wkbType() ) ) == Qgis::WkbType::LineString )
    {
      // 3d linestring (or multiline)
      derivedAttributes.insert( tr( "Length (Cartesian — 2D)" ), str );

      double totalLength3d = std::accumulate( layerCrsGeom->const_parts_begin(), layerCrsGeom->const_parts_end(), 0.0, []( double total, const QgsAbstractGeometry * part )
      {
        return total + qgsgeometry_cast< const QgsLineString * >( part )->length3D();
      } );

      str = formatDistance( totalLength3d, cartesianDistanceUnits );
      derivedAttributes.insert( tr( "Length (Cartesian — 3D)" ), str );
    }
    else
    {
      derivedAttributes.insert( tr( "Length (Cartesian)" ), str );
    }

    str = QLocale().toString( layerCrsGeom->nCoordinates() );
    derivedAttributes.insert( tr( "Vertices" ), str );
    if ( !layerPoint.isEmpty() )
    {
      //add details of closest vertex to identify point
      closestVertexAttributes( layerToMapTransform, layerVertCrs, mapVertCrs, *layerCrsGeom, vId, showTransformedZ, derivedAttributes );
      closestPointAttributes( layerToMapTransform, layerVertCrs, mapVertCrs, *layerCrsGeom, layerPoint, showTransformedZ, derivedAttributes );
    }

    if ( const QgsCurve *curve = qgsgeometry_cast< const QgsCurve * >( layerCrsGeom ) )
    {
      // Add the start and end points in as derived attributes
      QgsPointXY pnt = mCanvas->mapSettings().layerToMapCoordinates( layer, QgsPointXY( curve->startPoint().x(), curve->startPoint().y() ) );
      QString x;
      QString y;
      formatCoordinate( pnt, x, y );
      derivedAttributes.insert( tr( "firstX", "attributes get sorted; translation for lastX should be lexically larger than this one" ), x );
      derivedAttributes.insert( tr( "firstY" ), y );
      pnt = mCanvas->mapSettings().layerToMapCoordinates( layer, QgsPointXY( curve->endPoint().x(), curve->endPoint().y() ) );
      formatCoordinate( pnt, x, y );
      derivedAttributes.insert( tr( "lastX", "attributes get sorted; translation for firstX should be lexically smaller than this one" ), x );
      derivedAttributes.insert( tr( "lastY" ), y );
    }
  }
  else if ( geometryType == Qgis::GeometryType::Polygon )
  {
    double area = calc.measureArea( layerCrsGeometry );
    area = calc.convertAreaMeasurement( area, displayAreaUnits() );
    QString str;
    if ( ellipsoid != geoNone() )
    {
      str = formatArea( area );
      derivedAttributes.insert( tr( "Area (Ellipsoidal — %1)" ).arg( ellipsoid ), str );
    }
    str = formatArea( layerCrsGeometry.area()
                      * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::distanceToAreaUnit( layer->crs().mapUnits() ), cartesianAreaUnits ), cartesianAreaUnits );
    derivedAttributes.insert( tr( "Area (Cartesian)" ), str );

    if ( ellipsoid != geoNone() )
    {
      double perimeter = calc.measurePerimeter( layerCrsGeometry );
      perimeter = calc.convertLengthMeasurement( perimeter, displayDistanceUnits() );
      str = formatDistance( perimeter );
      derivedAttributes.insert( tr( "Perimeter (Ellipsoidal — %1)" ).arg( ellipsoid ), str );
    }
    str = formatDistance( layerCrsGeometry.constGet()->perimeter()
                          * QgsUnitTypes::fromUnitToUnitFactor( layer->crs().mapUnits(), cartesianDistanceUnits ), cartesianDistanceUnits );
    derivedAttributes.insert( tr( "Perimeter (Cartesian)" ), str );

    str = QLocale().toString( layerCrsGeometry.constGet()->nCoordinates() );
    derivedAttributes.insert( tr( "Vertices" ), str );

    if ( !layerPoint.isEmpty() )
    {
      //add details of closest vertex to identify point
      closestVertexAttributes( layerToMapTransform, layerVertCrs, mapVertCrs, *layerCrsGeometry.constGet(), vId, showTransformedZ, derivedAttributes );
      closestPointAttributes( layerToMapTransform, layerVertCrs, mapVertCrs, *layerCrsGeometry.constGet(), layerPoint, showTransformedZ, derivedAttributes );
    }
  }
  else if ( geometryType == Qgis::GeometryType::Point )
  {
    // Include the x, y, z coordinates of the point as a derived attribute
    if ( const QgsPoint *mapCrsPoint = qgsgeometry_cast< const QgsPoint * >( mapCrsGeometry.constGet() ) )
    {
      QString x;
      QString y;
      formatCoordinate( QgsPointXY( mapCrsPoint->x(), mapCrsPoint->y() ), x, y );
      derivedAttributes.insert( tr( "X" ), x );
      derivedAttributes.insert( tr( "Y" ), y );

      const double originalZ = QgsWkbTypes::hasZ( wkbType ) ? qgsgeometry_cast<const QgsPoint *>( layerCrsGeometry.constGet() )->z()
                               : std::numeric_limits< double >::quiet_NaN();
      const double mapCrsZ = mapCrsPoint->is3D() ? mapCrsPoint->z() : std::numeric_limits< double >::quiet_NaN();

      if ( !std::isnan( originalZ ) )
      {
        const QString str = QLocale().toString( originalZ, 'g', 10 );
        derivedAttributes.insert( showTransformedZ ? tr( "Z (%1)" ).arg( layerVertCrs.userFriendlyIdentifier( Qgis::CrsIdentifierType::MediumString ) )
                                  : tr( "Z" ), str );
      }
      if ( showTransformedZ && !std::isnan( mapCrsZ ) && !qgsDoubleNear( originalZ, mapCrsZ ) )
      {
        const QString str = QLocale().toString( mapCrsZ, 'g', 10 );
        derivedAttributes.insert( tr( "Z (%1)" ).arg( mapVertCrs.userFriendlyIdentifier( Qgis::CrsIdentifierType::MediumString ) ), str );
      }

      if ( QgsWkbTypes::hasM( wkbType ) )
      {
        const QString str = QLocale().toString( qgsgeometry_cast<const QgsPoint *>( layerCrsGeometry.constGet() )->m(), 'g', 10 );
        derivedAttributes.insert( tr( "M" ), str );
      }
    }
    else
    {
      //multipoint
      if ( !layerPoint.isEmpty() )
      {
        //add details of closest vertex to identify point
        const QgsAbstractGeometry *geom = layerCrsGeometry.constGet();
        closestVertexAttributes( layerToMapTransform, layerVertCrs, mapVertCrs, *geom, vId, showTransformedZ, derivedAttributes );
      }
    }
  }

  if ( feature.embeddedSymbol() )
  {
    derivedAttributes.insert( tr( "Embedded Symbol" ), tr( "%1 (%2)" ).arg( QgsSymbol::symbolTypeToString( feature.embeddedSymbol()->type() ), feature.embeddedSymbol()->color().name() ) );
  }

  return derivedAttributes;
}

bool QgsMapToolIdentify::identifyRasterLayer( QList<IdentifyResult> *results, QgsRasterLayer *layer, const QgsGeometry &geometry, const QgsRectangle &viewExtent, double mapUnitsPerPixel, const QgsIdentifyContext &identifyContext )
{
  QgsPointXY point = geometry.asPoint();  // raster layers currently only support identification by point
  return identifyRasterLayer( results, layer, point, viewExtent, mapUnitsPerPixel, identifyContext );
}

bool QgsMapToolIdentify::identifyRasterLayer( QList<IdentifyResult> *results, QgsRasterLayer *layer, QgsPointXY point, const QgsRectangle &viewExtent, double mapUnitsPerPixel, const QgsIdentifyContext &identifyContext )
{
  QgsDebugMsgLevel( "point = " + point.toString(), 2 );
  if ( !layer )
    return false;

  std::unique_ptr< QgsRasterDataProvider > dprovider( layer->dataProvider()->clone() );
  if ( !dprovider )
    return false;

  const Qgis::RasterInterfaceCapabilities capabilities = dprovider->capabilities();
  if ( !( capabilities & Qgis::RasterInterfaceCapability::Identify ) )
    return false;

  if ( identifyContext.isTemporal() )
  {
    if ( !layer->temporalProperties()->isVisibleInTemporalRange( identifyContext.temporalRange() ) )
      return false;

    dprovider->temporalCapabilities()->setRequestedTemporalRange( identifyContext.temporalRange() );
  }

  if ( !identifyContext.zRange().isInfinite() )
  {
    if ( !layer->elevationProperties()->isVisibleInZRange( identifyContext.zRange(), layer ) )
      return false;
  }

  QgsPointXY pointInCanvasCrs = point;
  try
  {
    point = toLayerCoordinates( layer, point );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse )
    QgsDebugError( QStringLiteral( "coordinate not reprojectable: %1" ).arg( cse.what() ) );
    return false;
  }
  QgsDebugMsgLevel( QStringLiteral( "point = %1 %2" ).arg( point.x() ).arg( point.y() ), 2 );

  if ( !layer->extent().contains( point ) )
    return false;

  QMap< QString, QString > attributes, derivedAttributes;

  Qgis::RasterIdentifyFormat format = QgsRasterDataProvider::identifyFormatFromName( layer->customProperty( QStringLiteral( "identify/format" ) ).toString() );

  // check if the format is really supported otherwise use first supported format
  if ( !( capabilities & QgsRasterDataProvider::identifyFormatToCapability( format ) ) )
  {
    if ( capabilities & Qgis::RasterInterfaceCapability::IdentifyFeature )
      format = Qgis::RasterIdentifyFormat::Feature;
    else if ( capabilities & Qgis::RasterInterfaceCapability::IdentifyValue )
      format = Qgis::RasterIdentifyFormat::Value;
    else if ( capabilities & Qgis::RasterInterfaceCapability::IdentifyHtml )
      format = Qgis::RasterIdentifyFormat::Html;
    else if ( capabilities & Qgis::RasterInterfaceCapability::IdentifyText )
      format = Qgis::RasterIdentifyFormat::Text;
    else return false;
  }

  QgsRasterIdentifyResult identifyResult;
  // We can only use current map canvas context (extent, width, height) if layer is not reprojected,
  if ( dprovider->crs() != mCanvas->mapSettings().destinationCrs() )
  {
    // To get some reasonable response for point/line WMS vector layers we must
    // use a context with approximately a resolution in layer CRS units
    // corresponding to current map canvas resolution (for examplei UMN Mapserver
    // in msWMSFeatureInfo() -> msQueryByRect() is using requested pixel
    // + TOLERANCE (layer param) for feature selection)
    //
    QgsRectangle r;
    r.setXMinimum( pointInCanvasCrs.x() - mapUnitsPerPixel / 2. );
    r.setXMaximum( pointInCanvasCrs.x() + mapUnitsPerPixel / 2. );
    r.setYMinimum( pointInCanvasCrs.y() - mapUnitsPerPixel / 2. );
    r.setYMaximum( pointInCanvasCrs.y() + mapUnitsPerPixel / 2. );
    r = toLayerCoordinates( layer, r ); // will be a bit larger
    // Mapserver (6.0.3, for example) does not work with 1x1 pixel box
    // but that is fixed (the rect is enlarged) in the WMS provider
    identifyResult = dprovider->identify( point, format, r, 1, 1 );
  }
  else
  {
    // It would be nice to use the same extent and size which was used for drawing,
    // so that WCS can use cache from last draw, unfortunately QgsRasterLayer::draw()
    // is doing some tricks with extent and size to align raster to output which
    // would be difficult to replicate here.
    // Note: cutting the extent may result in slightly different x and y resolutions
    // and thus shifted point calculated back in QGIS WMS (using average resolution)
    //viewExtent = dprovider->extent().intersect( &viewExtent );

    // Width and height are calculated from not projected extent and we hope that
    // are similar to source width and height used to reproject layer for drawing.
    // TODO: may be very dangerous, because it may result in different resolutions
    // in source CRS, and WMS server (QGIS server) calcs wrong coor using average resolution.
    int width = static_cast< int >( std::round( viewExtent.width() / mapUnitsPerPixel ) );
    int height = static_cast< int >( std::round( viewExtent.height() / mapUnitsPerPixel ) );

    QgsDebugMsgLevel( QStringLiteral( "viewExtent.width = %1 viewExtent.height = %2" ).arg( viewExtent.width() ).arg( viewExtent.height() ), 2 );
    QgsDebugMsgLevel( QStringLiteral( "width = %1 height = %2" ).arg( width ).arg( height ), 2 );
    QgsDebugMsgLevel( QStringLiteral( "xRes = %1 yRes = %2 mapUnitsPerPixel = %3" ).arg( viewExtent.width() / width ).arg( viewExtent.height() / height ).arg( mapUnitsPerPixel ), 2 );

    identifyResult = dprovider->identify( point, format, viewExtent, width, height );
  }

  QgsRasterLayerElevationProperties *elevationProperties = qobject_cast< QgsRasterLayerElevationProperties *>( layer->elevationProperties() );
  if ( identifyResult.isValid() && !identifyContext.zRange().isInfinite() && elevationProperties && elevationProperties->isEnabled() )
  {
    // filter results by z range
    switch ( format )
    {
      case Qgis::RasterIdentifyFormat::Value:
      {
        bool foundMatch = false;
        QMap<int, QVariant> values = identifyResult.results();
        QMap<int, QVariant> filteredValues;
        for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
        {
          if ( QgsVariantUtils::isNull( it.value() ) )
          {
            continue;
          }
          const double value = it.value().toDouble();
          const QgsDoubleRange elevationRange = elevationProperties->elevationRangeForPixelValue( layer, it.key(), value );
          if ( !elevationRange.isInfinite() && identifyContext.zRange().overlaps( elevationRange ) )
          {
            filteredValues.insert( it.key(), it.value() );
            foundMatch = true;
          }
        }

        if ( !foundMatch )
          return false;

        identifyResult = QgsRasterIdentifyResult( Qgis::RasterIdentifyFormat::Value, filteredValues );

        break;
      }

      // can't filter by z for these formats
      case Qgis::RasterIdentifyFormat::Undefined:
      case Qgis::RasterIdentifyFormat::Text:
      case Qgis::RasterIdentifyFormat::Html:
      case Qgis::RasterIdentifyFormat::Feature:
        break;
    }
  }

  derivedAttributes.insert( derivedAttributesForPoint( QgsPoint( pointInCanvasCrs ) ) );

  const double xres = layer->rasterUnitsPerPixelX();
  const double yres = layer->rasterUnitsPerPixelY();
  QgsRectangle pixelRect;
  // Don't derive clicked column/row for providers that serve dynamically rendered map images
  if ( ( dprovider->capabilities() & Qgis::RasterInterfaceCapability::Size ) && !qgsDoubleNear( xres, 0 ) && !qgsDoubleNear( yres, 0 ) )
  {
    // Try to determine the clicked column/row (0-based) in the raster
    const QgsRectangle extent = dprovider->extent();

    const int rasterCol = static_cast< int >( std::floor( ( point.x() - extent.xMinimum() ) / xres ) );
    const int rasterRow = static_cast< int >( std::floor( ( extent.yMaximum() - point.y() ) / yres ) );

    derivedAttributes.insert( tr( "Column (0-based)" ), QLocale().toString( rasterCol ) );
    derivedAttributes.insert( tr( "Row (0-based)" ), QLocale().toString( rasterRow ) );

    pixelRect = QgsRectangle( rasterCol * xres + extent.xMinimum(),
                              extent.yMaximum() - ( rasterRow + 1 ) * yres,
                              ( rasterCol + 1 ) * xres + extent.xMinimum(),
                              extent.yMaximum() - ( rasterRow * yres ) );
  }

  if ( identifyResult.isValid() )
  {
    QMap<int, QVariant> values = identifyResult.results();
    if ( format == Qgis::RasterIdentifyFormat::Value )
    {
      for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
      {
        QString valueString;
        if ( QgsVariantUtils::isNull( it.value() ) )
        {
          valueString = tr( "no data" );
        }
        else
        {
          QVariant value( it.value() );
          // The cast is legit. Quoting QT doc :
          // "Although this function is declared as returning QVariant::Type,
          // the return value should be interpreted as QMetaType::Type"
          if ( static_cast<QMetaType::Type>( value.userType() ) == QMetaType::Float )
          {
            valueString = QgsRasterBlock::printValue( value.toFloat(), true );
          }
          else
          {
            valueString = QgsRasterBlock::printValue( value.toDouble(), true );
          }
        }
        attributes.insert( dprovider->generateBandName( it.key() ), valueString );

        // Get raster attribute table attributes
        if ( const QgsRasterAttributeTable *rat = layer->attributeTable( it.key() ) )
        {
          bool ok;
          const double doubleValue { it.value().toDouble( &ok ) };
          if ( ok )
          {
            const QVariantList row = rat->row( doubleValue );
            if ( ! row.isEmpty() )
            {
              for ( int colIdx = 0; colIdx < std::min( rat->fields().count( ), row.count() ); ++colIdx )
              {
                const QgsRasterAttributeTable::Field ratField { rat->fields().at( colIdx ) };

                // Skip value and color fields
                if ( QgsRasterAttributeTable::valueAndColorFieldUsages().contains( ratField.usage ) )
                {
                  continue;
                }

                QString ratValue;
                switch ( ratField.type )
                {
                  case QMetaType::Type::QChar:
                  case QMetaType::Type::Int:
                  case QMetaType::Type::UInt:
                  case QMetaType::Type::LongLong:
                  case QMetaType::Type::ULongLong:
                    ratValue = QLocale().toString( row.at( colIdx ).toLongLong() );
                    break;
                  case QMetaType::Type::Double:
                    ratValue = QLocale().toString( row.at( colIdx ).toDouble( ) );
                    break;
                  default:
                    ratValue = row.at( colIdx ).toString();
                }
                attributes.insert( ratField.name, ratValue );
              }
            }
          }
        }  // end RAT

      }

      QString label = layer->name();
      QgsFeature feature;
      if ( !pixelRect.isNull() )
      {
        feature.setGeometry( QgsGeometry::fromRect( pixelRect ) );
      }

      IdentifyResult result( qobject_cast<QgsMapLayer *>( layer ), label, QgsFields(), feature, derivedAttributes );
      result.mAttributes = attributes;
      results->append( result );
    }
    else if ( format == Qgis::RasterIdentifyFormat::Feature )
    {
      for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
      {
        QVariant value = it.value();
        if ( value.userType() == QMetaType::Type::Bool && !value.toBool() )
        {
          // sublayer not visible or not queryable
          continue;
        }

        if ( value.userType() == QMetaType::Type::QString )
        {
          // error
          // TODO: better error reporting
          QString label = layer->subLayers().value( it.key() );
          attributes.clear();
          attributes.insert( tr( "Error" ), value.toString() );

          results->append( IdentifyResult( qobject_cast<QgsMapLayer *>( layer ), label, attributes, derivedAttributes ) );
          continue;
        }

        // list of feature stores for a single sublayer
        const QgsFeatureStoreList featureStoreList = value.value<QgsFeatureStoreList>();

        for ( const QgsFeatureStore &featureStore : featureStoreList )
        {
          const QgsFeatureList storeFeatures = featureStore.features();
          for ( const QgsFeature &feature : storeFeatures )
          {
            attributes.clear();
            // WMS sublayer and feature type, a sublayer may contain multiple feature types.
            // Sublayer name may be the same as layer name and feature type name
            // may be the same as sublayer. We try to avoid duplicities in label.
            QString sublayer = featureStore.params().value( QStringLiteral( "sublayer" ) ).toString();
            QString featureType = featureStore.params().value( QStringLiteral( "featureType" ) ).toString();
            // Strip UMN MapServer '_feature'
            featureType.remove( QStringLiteral( "_feature" ) );
            QStringList labels;
            if ( sublayer.compare( layer->name(), Qt::CaseInsensitive ) != 0 )
            {
              labels << sublayer;
            }
            if ( featureType.compare( sublayer, Qt::CaseInsensitive ) != 0 || labels.isEmpty() )
            {
              labels << featureType;
            }

            QMap< QString, QString > derAttributes = derivedAttributes;
            derAttributes.insert( featureDerivedAttributes( feature, layer, toLayerCoordinates( layer, point ) ) );

            IdentifyResult identifyResult( qobject_cast<QgsMapLayer *>( layer ), labels.join( QLatin1String( " / " ) ), featureStore.fields(), feature, derAttributes );

            identifyResult.mParams.insert( QStringLiteral( "getFeatureInfoUrl" ), featureStore.params().value( QStringLiteral( "getFeatureInfoUrl" ) ) );
            results->append( identifyResult );
          }
        }
      }
    }
    else // text or html
    {
      QgsDebugMsgLevel( QStringLiteral( "%1 HTML or text values" ).arg( values.size() ), 2 );
      for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
      {
        QString value = it.value().toString();
        attributes.clear();
        attributes.insert( QString(), value );

        QString label = layer->subLayers().value( it.key() );
        results->append( IdentifyResult( qobject_cast<QgsMapLayer *>( layer ), label, attributes, derivedAttributes ) );
      }
    }
  }
  else
  {
    attributes.clear();
    QString value = identifyResult.error().message( QgsErrorMessage::Text );
    attributes.insert( tr( "Error" ), value );
    QString label = tr( "Identify error" );
    results->append( IdentifyResult( qobject_cast<QgsMapLayer *>( layer ), label, attributes, derivedAttributes ) );
  }

  return true;
}

Qgis::DistanceUnit QgsMapToolIdentify::displayDistanceUnits() const
{
  return mCanvas->mapUnits();
}

Qgis::AreaUnit QgsMapToolIdentify::displayAreaUnits() const
{
  return QgsUnitTypes::distanceToAreaUnit( mCanvas->mapUnits() );
}

QString QgsMapToolIdentify::formatDistance( double distance ) const
{
  return formatDistance( distance, displayDistanceUnits() );
}

QString QgsMapToolIdentify::formatArea( double area ) const
{
  return formatArea( area, displayAreaUnits() );
}

QString QgsMapToolIdentify::formatDistance( double distance, Qgis::DistanceUnit unit ) const
{
  QgsSettings settings;
  bool baseUnit = settings.value( QStringLiteral( "qgis/measure/keepbaseunit" ), true ).toBool();

  return QgsDistanceArea::formatDistance( distance, mCoordinatePrecision, unit, baseUnit );
}

QString QgsMapToolIdentify::formatArea( double area, Qgis::AreaUnit unit ) const
{
  QgsSettings settings;
  bool baseUnit = settings.value( QStringLiteral( "qgis/measure/keepbaseunit" ), true ).toBool();

  return QgsDistanceArea::formatArea( area, mCoordinatePrecision, unit, baseUnit );
}

void QgsMapToolIdentify::formatChanged( QgsRasterLayer *layer )
{
  QList<IdentifyResult> results;
  if ( identifyRasterLayer( &results, layer, mLastGeometry, mLastExtent, mLastMapUnitsPerPixel ) )
  {
    emit changedRasterResults( results );
  }
}

void QgsMapToolIdentify::fromPointCloudIdentificationToIdentifyResults( QgsPointCloudLayer *layer, const QVector<QVariantMap> &identified, QList<QgsMapToolIdentify::IdentifyResult> &results )
{
  const QgsCoordinateReferenceSystem mapVertCrs = QgsProject::instance()->crs3D().verticalCrs().isValid() ? QgsProject::instance()->crs3D().verticalCrs()
      : QgsProject::instance()->crs3D();
  const QgsCoordinateReferenceSystem layerVertCrs = layer->crs3D().verticalCrs().isValid() ? layer->crs3D().verticalCrs()
      : layer->crs3D();
  const bool showTransformedZ = QgsProject::instance()->crs3D() != layer->crs3D() && QgsProject::instance()->crs3D().hasVerticalAxis() && layer->crs3D().hasVerticalAxis();
  const QgsCoordinateTransform layerToMapTransform( layer->crs3D(), QgsProject::instance()->crs3D(), QgsProject::instance()->transformContext() );

  int id = 1;
  const QgsPointCloudLayerElevationProperties *elevationProps = qobject_cast< const QgsPointCloudLayerElevationProperties *>( layer->elevationProperties() );
  for ( const QVariantMap &pt : identified )
  {
    QMap<QString, QString> ptStr;
    QString classification;
    for ( auto attrIt = pt.constBegin(); attrIt != pt.constEnd(); ++attrIt )
    {
      if ( attrIt.key().compare( QLatin1String( "Z" ), Qt::CaseInsensitive ) == 0
           && ( !qgsDoubleNear( elevationProps->zScale(), 1 ) || !qgsDoubleNear( elevationProps->zOffset(), 0 ) ) )
      {
        // Apply elevation properties
        ptStr[ tr( "Z (original)" ) ] = attrIt.value().toString();
        ptStr[ tr( "Z (adjusted)" ) ] = QString::number( attrIt.value().toDouble() * elevationProps->zScale() + elevationProps->zOffset() );
      }
      else if ( attrIt.key().compare( QLatin1String( "Classification" ), Qt::CaseInsensitive ) == 0 )
      {
        classification = QgsPointCloudDataProvider::translatedLasClassificationCodes().value( attrIt.value().toInt() );
        ptStr[ attrIt.key() ] = QStringLiteral( "%1 (%2)" ).arg( attrIt.value().toString(), classification );
      }
      else
      {
        ptStr[attrIt.key()] = attrIt.value().toString();
      }
    }

    QMap< QString, QString > derivedAttributes;
    QgsPoint layerPoint( pt.value( "X" ).toDouble(), pt.value( "Y" ).toDouble(), pt.value( "Z" ).toDouble() );

    QgsPoint mapCrsPoint = layerPoint;
    try
    {
      if ( layerToMapTransform.isValid() )
      {
        mapCrsPoint.transform( layerToMapTransform, Qgis::TransformDirection::Forward, layerToMapTransform.hasVerticalComponent() );
      }
    }
    catch ( QgsCsException &cse )
    {
      QgsMessageLog::logMessage( QObject::tr( "Transform error caught: %1" ).arg( cse.what() ), QObject::tr( "CRS" ) );
    }

    QString x;
    QString y;
    // BAD, we should not be using the hardcoded precision/crs values here, but this method is static and that's not trivial
    // to avoid...
    formatCoordinate( QgsPointXY( mapCrsPoint.x(), mapCrsPoint.y() ), x, y, QgsProject::instance()->crs(), 6 );
    derivedAttributes.insert( tr( "X" ), x );
    derivedAttributes.insert( tr( "Y" ), y );

    const double originalZ = layerPoint.z();
    const double mapCrsZ = mapCrsPoint.is3D() ? mapCrsPoint.z() : std::numeric_limits< double >::quiet_NaN();

    if ( !std::isnan( originalZ ) )
    {
      const QString str = QLocale().toString( originalZ, 'g', 10 );
      derivedAttributes.insert( showTransformedZ ? tr( "Z (%1)" ).arg( layerVertCrs.userFriendlyIdentifier( Qgis::CrsIdentifierType::MediumString ) )
                                : tr( "Z" ), str );
    }
    if ( showTransformedZ && !std::isnan( mapCrsZ ) && !qgsDoubleNear( originalZ, mapCrsZ ) )
    {
      const QString str = QLocale().toString( mapCrsZ, 'g', 10 );
      derivedAttributes.insert( tr( "Z (%1)" ).arg( mapVertCrs.userFriendlyIdentifier( Qgis::CrsIdentifierType::MediumString ) ), str );
    }

    QgsMapToolIdentify::IdentifyResult res( layer, classification.isEmpty() ? QString::number( id ) : QStringLiteral( "%1 (%2)" ).arg( id ).arg( classification ), ptStr, derivedAttributes );
    results.append( res );
    ++id;
  }
}

void QgsMapToolIdentify::fromElevationProfileLayerIdentificationToIdentifyResults( QgsMapLayer *layer, const QVector<QVariantMap> &identified, QList<IdentifyResult> &results )
{
  if ( !layer )
    return;

  if ( identified.empty() )
    return;

  switch ( layer->type() )
  {
    case Qgis::LayerType::Vector:
    {
      QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( layer );

      QgsFeatureList features;
      QHash< QgsFeatureId, QVariant > featureDistances;
      QHash< QgsFeatureId, QVariant > featureElevations;

      QgsFeatureIds filterIds;
      for ( const QVariantMap &map : identified )
      {
        if ( !map.contains( QStringLiteral( "id" ) ) )
        {
          QMap< QString, QString > attributes;
          if ( map.value( QStringLiteral( "distance" ) ).isValid() )
            attributes.insert( tr( "Distance along curve" ), QString::number( map.value( QStringLiteral( "distance" ) ).toDouble() ) );
          if ( map.value( QStringLiteral( "elevation" ) ).isValid() )
            attributes.insert( tr( "Elevation" ), QString::number( map.value( QStringLiteral( "elevation" ) ).toDouble() ) );

          results.append( IdentifyResult( layer, layer->name(), {}, attributes ) );
        }
        else
        {
          const QgsFeatureId id = map.value( QStringLiteral( "id" ) ).toLongLong();
          filterIds.insert( id );

          featureDistances.insert( id, map.value( QStringLiteral( "distance" ) ) );
          featureElevations.insert( id, map.value( QStringLiteral( "elevation" ) ) );
        }
      }

      QgsFeatureRequest request;
      request.setFilterFids( filterIds );
      QgsFeatureIterator it = vl->getFeatures( request );
      QgsFeature f;
      while ( it.nextFeature( f ) )
        features << f;

      QgsRenderContext context;
      identifyVectorLayer( &results, vl, features, nullptr, QMap< QString, QString >(), [this, vl, &featureDistances, &featureElevations]( const QgsFeature & feature )->QMap< QString, QString >
      {
        QMap< QString, QString > attributes = featureDerivedAttributes( feature, vl, QgsPointXY() );

        if ( featureDistances.value( feature.id() ).isValid() )
          attributes.insert( tr( "Distance along curve" ), QString::number( featureDistances.value( feature.id() ).toDouble() ) );
        if ( featureElevations.value( feature.id() ).isValid() )
          attributes.insert( tr( "Elevation" ), QString::number( featureElevations.value( feature.id() ).toDouble() ) );

        return attributes;
      }, context );
      break;
    }

    case Qgis::LayerType::Raster:
    case Qgis::LayerType::Mesh:
    {
      for ( const QVariantMap &map : identified )
      {
        QMap< QString, QString > attributes;
        if ( map.value( QStringLiteral( "distance" ) ).isValid() )
          attributes.insert( tr( "Distance along curve" ), QString::number( map.value( QStringLiteral( "distance" ) ).toDouble() ) );
        if ( map.value( QStringLiteral( "elevation" ) ).isValid() )
          attributes.insert( tr( "Elevation" ), QString::number( map.value( QStringLiteral( "elevation" ) ).toDouble() ) );

        results.append( IdentifyResult( layer, layer->name(), {}, attributes ) );
      }

      break;
    }

    case Qgis::LayerType::PointCloud:
    {
      QgsPointCloudLayer *pcLayer = qobject_cast< QgsPointCloudLayer * >( layer );
      fromPointCloudIdentificationToIdentifyResults( pcLayer, identified, results );
      break;
    }

    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::VectorTile:
    case Qgis::LayerType::TiledScene:
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::Group:
      break;
  }
}
