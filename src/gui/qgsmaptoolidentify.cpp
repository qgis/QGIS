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
#include "qgscoordinateformatter.h"
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
#include "qgsmaptopixel.h"
#include "qgsmessageviewer.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayertemporalproperties.h"
#include "qgsmaplayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsrasteridentifyresult.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayertemporalproperties.h"
#include "qgsvectortilelayer.h"
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
#include "qgspointcloudlayerrenderer.h"
#include "qgspointcloudlayerelevationproperties.h"
#include "qgssymbol.h"
#include "qgsmultilinestring.h"

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
  if ( layer->type() == QgsMapLayerType::RasterLayer && layerType.testFlag( RasterLayer ) )
  {
    return identifyRasterLayer( results, qobject_cast<QgsRasterLayer *>( layer ), geometry, viewExtent, mapUnitsPerPixel, identifyContext );
  }
  else if ( layer->type() == QgsMapLayerType::VectorLayer && layerType.testFlag( VectorLayer ) )
  {
    return identifyVectorLayer( results, qobject_cast<QgsVectorLayer *>( layer ), geometry, identifyContext );
  }
  else if ( layer->type() == QgsMapLayerType::MeshLayer && layerType.testFlag( MeshLayer ) )
  {
    return identifyMeshLayer( results, qobject_cast<QgsMeshLayer *>( layer ), geometry, identifyContext );
  }
  else if ( layer->type() == QgsMapLayerType::VectorTileLayer && layerType.testFlag( VectorTileLayer ) )
  {
    return identifyVectorTileLayer( results, qobject_cast<QgsVectorTileLayer *>( layer ), geometry, identifyContext );
  }
  else if ( layer->type() == QgsMapLayerType::PointCloudLayer && layerType.testFlag( PointCloudLayer ) )
  {
    return identifyPointCloudLayer( results, qobject_cast<QgsPointCloudLayer *>( layer ), geometry, identifyContext );
  }
  else
  {
    return false;
  }
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

  double searchRadius = mOverrideCanvasSearchRadius < 0 ? searchRadiusMU( mCanvas ) : mOverrideCanvasSearchRadius;
  bool isTemporal = identifyContext.isTemporal() && layer->temporalProperties()->isActive();

  QList<QgsMeshDatasetIndex> datasetIndexList;
  int activeScalarGroup = layer->rendererSettings().activeScalarDatasetGroup();
  int activeVectorGroup = layer->rendererSettings().activeVectorDatasetGroup();

  const QList<int> allGroup = layer->enabledDatasetGroupsIndexes();
  if ( isTemporal ) //non active dataset group value are only accesible if temporal is active
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
  bool isSingleClick = selectionGeom.type() == QgsWkbTypes::PointGeometry;
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

    const int tileZoom = layer->tileMatrixSet().scaleToZoomLevel( mCanvas->scale() );
    const QgsTileMatrix tileMatrix = layer->tileMatrixSet().tileMatrix( tileZoom );
    const QgsTileRange tileRange = tileMatrix.tileRangeFromExtent( r );

    for ( int row = tileRange.startRow(); row <= tileRange.endRow(); ++row )
    {
      for ( int col = tileRange.startColumn(); col <= tileRange.endColumn(); ++col )
      {
        QgsTileXYZ tileID( col, row, tileZoom );
        QByteArray data = layer->getRawTile( tileID );
        if ( data.isEmpty() )
          continue;  // failed to get data

        QgsVectorTileMVTDecoder decoder( layer->tileMatrixSet() );
        if ( !decoder.decode( tileID, data ) )
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

              results->append( IdentifyResult( layer, layerName, fFields, f, derivedAttributes ) );

              featureCount++;
            }
          }
        }
      }
    }

  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse )
    // catch exception for 'invalid' point and proceed with no features found
    QgsDebugMsg( QStringLiteral( "Caught CRS exception %1" ).arg( cse.what() ) );
  }

  return featureCount > 0;
}

bool QgsMapToolIdentify::identifyPointCloudLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsPointCloudLayer *layer, const QgsGeometry &geometry, const QgsIdentifyContext &identifyContext )
{
  Q_UNUSED( identifyContext )
  QgsPointCloudRenderer *renderer = layer->renderer();

  QgsRenderContext context = QgsRenderContext::fromMapSettings( mCanvas->mapSettings() );
  context.setCoordinateTransform( QgsCoordinateTransform( layer->crs(), mCanvas->mapSettings().destinationCrs(), mCanvas->mapSettings().transformContext() ) );

  const double searchRadiusMapUnits = mOverrideCanvasSearchRadius < 0 ? searchRadiusMU( mCanvas ) : mOverrideCanvasSearchRadius;

  const QVector<QVariantMap> points = renderer->identify( layer, context, geometry, searchRadiusMapUnits );

  fromPointCloudIdentificationToIdentifyResults( layer, points, *results );

  return true;
}

QMap<QString, QString> QgsMapToolIdentify::derivedAttributesForPoint( const QgsPoint &point )
{
  QMap< QString, QString > derivedAttributes;
  derivedAttributes.insert( tr( "(clicked coordinate X)" ), formatXCoordinate( point ) );
  derivedAttributes.insert( tr( "(clicked coordinate Y)" ), formatYCoordinate( point ) );
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
    QgsDebugMsg( QStringLiteral( "Out of scale limits" ) );
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

  const bool fetchFeatureSymbols = layer->dataProvider()->capabilities() & QgsVectorDataProvider::FeatureSymbology;

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QMap< QString, QString > commonDerivedAttributes;

  QgsGeometry selectionGeom = geometry;
  bool isPointOrRectangle;
  QgsPointXY point;
  bool isSingleClick = selectionGeom.type() == QgsWkbTypes::PointGeometry;
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
    featureRequest.setFlags( QgsFeatureRequest::ExactIntersect | ( fetchFeatureSymbols ? QgsFeatureRequest::EmbeddedSymbols : QgsFeatureRequest::Flags() ) );
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
    QgsDebugMsg( QStringLiteral( "Caught CRS exception %1" ).arg( cse.what() ) );
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

  for ( const QgsFeature &feature : std::as_const( featureList ) )
  {
    QMap< QString, QString > derivedAttributes = commonDerivedAttributes;

    QgsFeatureId fid = feature.id();
    context.expressionContext().setFeature( feature );

    if ( filter && !renderer->willRenderFeature( feature, context ) )
      continue;

    featureCount++;

    // When not single click identify, pass an empty point so some derived attributes may still be computed
    if ( !isSingleClick )
      point = QgsPointXY();
    derivedAttributes.unite( featureDerivedAttributes( feature, layer, toLayerCoordinates( layer, point ) ) );

    derivedAttributes.insert( tr( "Feature ID" ), fid < 0 ? tr( "new feature" ) : FID_TO_STRING( fid ) );

    results->append( IdentifyResult( qobject_cast<QgsMapLayer *>( layer ), feature, derivedAttributes ) );
  }

  if ( renderer )
  {
    renderer->stopRender( context );
  }

  QgsDebugMsgLevel( "Feature count on identify: " + QString::number( featureCount ), 2 );

  QApplication::restoreOverrideCursor();
  return featureCount > 0;
}

void QgsMapToolIdentify::closestVertexAttributes( const QgsAbstractGeometry &geometry, QgsVertexId vId, QgsMapLayer *layer, QMap< QString, QString > &derivedAttributes )
{
  if ( ! vId.isValid( ) )
  {
    // We should not get here ...
    QgsDebugMsg( "Invalid vertex id!" );
    return;
  }

  QString str = QLocale().toString( vId.vertex + 1 );
  derivedAttributes.insert( tr( "Closest vertex number" ), str );

  QgsPoint closestPoint = geometry.vertexAt( vId );

  QgsPointXY closestPointMapCoords = mCanvas->mapSettings().layerToMapCoordinates( layer, QgsPointXY( closestPoint.x(), closestPoint.y() ) );
  derivedAttributes.insert( tr( "Closest vertex X" ), formatXCoordinate( closestPointMapCoords ) );
  derivedAttributes.insert( tr( "Closest vertex Y" ), formatYCoordinate( closestPointMapCoords ) );

  if ( closestPoint.is3D() )
  {
    str = QLocale().toString( closestPoint.z(), 'g', 10 );
    derivedAttributes.insert( tr( "Closest vertex Z" ), str );
  }
  if ( closestPoint.isMeasure() )
  {
    str = QLocale().toString( closestPoint.m(), 'g', 10 );
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

void QgsMapToolIdentify::closestPointAttributes( const QgsAbstractGeometry &geometry, const QgsPointXY &layerPoint, QMap<QString, QString> &derivedAttributes )
{
  QgsPoint closestPoint = QgsGeometryUtils::closestPoint( geometry, QgsPoint( layerPoint ) );

  derivedAttributes.insert( tr( "Closest X" ), formatXCoordinate( closestPoint ) );
  derivedAttributes.insert( tr( "Closest Y" ), formatYCoordinate( closestPoint ) );

  if ( closestPoint.is3D() )
  {
    const QString str = QLocale().toString( closestPoint.z(), 'g', 10 );
    derivedAttributes.insert( tr( "Interpolated Z" ), str );
  }
  if ( closestPoint.isMeasure() )
  {
    const QString str = QLocale().toString( closestPoint.m(), 'g', 10 );
    derivedAttributes.insert( tr( "Interpolated M" ), str );
  }
}

QString QgsMapToolIdentify::formatCoordinate( const QgsPointXY &canvasPoint ) const
{
  return QgsCoordinateUtils::formatCoordinateForProject( QgsProject::instance(), canvasPoint, mCanvas->mapSettings().destinationCrs(),
         mCoordinatePrecision );
}

QString QgsMapToolIdentify::formatXCoordinate( const QgsPointXY &canvasPoint ) const
{
  QString coordinate = formatCoordinate( canvasPoint );
  return coordinate.split( QgsCoordinateFormatter::separator() ).at( 0 );
}

QString QgsMapToolIdentify::formatYCoordinate( const QgsPointXY &canvasPoint ) const
{
  QString coordinate = formatCoordinate( canvasPoint );
  return coordinate.split( QgsCoordinateFormatter::separator() ).at( 1 );
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

  QgsWkbTypes::Type wkbType = QgsWkbTypes::NoGeometry;
  QgsWkbTypes::GeometryType geometryType = QgsWkbTypes::NullGeometry;

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

  QgsUnitTypes::DistanceUnit cartesianDistanceUnits = QgsUnitTypes::unitType( layer->crs().mapUnits() ) == QgsUnitTypes::unitType( displayDistanceUnits() )
      ? displayDistanceUnits() : layer->crs().mapUnits();
  QgsUnitTypes::AreaUnit cartesianAreaUnits = QgsUnitTypes::unitType( QgsUnitTypes::distanceToAreaUnit( layer->crs().mapUnits() ) ) == QgsUnitTypes::unitType( displayAreaUnits() )
      ? displayAreaUnits() : QgsUnitTypes::distanceToAreaUnit( layer->crs().mapUnits() );

  if ( geometryType == QgsWkbTypes::LineGeometry )
  {
    const QgsAbstractGeometry *geom = feature.geometry().constGet();

    double dist = calc.measureLength( feature.geometry() );
    dist = calc.convertLengthMeasurement( dist, displayDistanceUnits() );
    QString str;
    if ( ellipsoid != geoNone() )
    {
      str = formatDistance( dist );
      derivedAttributes.insert( tr( "Length (Ellipsoidal — %1)" ).arg( ellipsoid ), str );
    }

    str = formatDistance( geom->length()
                          * QgsUnitTypes::fromUnitToUnitFactor( layer->crs().mapUnits(), cartesianDistanceUnits ), cartesianDistanceUnits );
    if ( QgsWkbTypes::hasZ( geom->wkbType() )
         && QgsWkbTypes::flatType( QgsWkbTypes::singleType( geom->wkbType() ) ) == QgsWkbTypes::LineString )
    {
      // 3d linestring (or multiline)
      derivedAttributes.insert( tr( "Length (Cartesian — 2D)" ), str );

      double totalLength3d = std::accumulate( geom->const_parts_begin(), geom->const_parts_end(), 0.0, []( double total, const QgsAbstractGeometry * part )
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

    str = QLocale().toString( geom->nCoordinates() );
    derivedAttributes.insert( tr( "Vertices" ), str );
    if ( !layerPoint.isEmpty() )
    {
      //add details of closest vertex to identify point
      closestVertexAttributes( *geom, vId, layer, derivedAttributes );
      closestPointAttributes( *geom, layerPoint, derivedAttributes );
    }

    if ( const QgsCurve *curve = qgsgeometry_cast< const QgsCurve * >( geom ) )
    {
      // Add the start and end points in as derived attributes
      QgsPointXY pnt = mCanvas->mapSettings().layerToMapCoordinates( layer, QgsPointXY( curve->startPoint().x(), curve->startPoint().y() ) );
      str = formatXCoordinate( pnt );
      derivedAttributes.insert( tr( "firstX", "attributes get sorted; translation for lastX should be lexically larger than this one" ), str );
      str = formatYCoordinate( pnt );
      derivedAttributes.insert( tr( "firstY" ), str );
      pnt = mCanvas->mapSettings().layerToMapCoordinates( layer, QgsPointXY( curve->endPoint().x(), curve->endPoint().y() ) );
      str = formatXCoordinate( pnt );
      derivedAttributes.insert( tr( "lastX", "attributes get sorted; translation for firstX should be lexically smaller than this one" ), str );
      str = formatYCoordinate( pnt );
      derivedAttributes.insert( tr( "lastY" ), str );
    }
  }
  else if ( geometryType == QgsWkbTypes::PolygonGeometry )
  {
    double area = calc.measureArea( feature.geometry() );
    area = calc.convertAreaMeasurement( area, displayAreaUnits() );
    QString str;
    if ( ellipsoid != geoNone() )
    {
      str = formatArea( area );
      derivedAttributes.insert( tr( "Area (Ellipsoidal — %1)" ).arg( ellipsoid ), str );
    }
    str = formatArea( feature.geometry().area()
                      * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::distanceToAreaUnit( layer->crs().mapUnits() ), cartesianAreaUnits ), cartesianAreaUnits );
    derivedAttributes.insert( tr( "Area (Cartesian)" ), str );

    if ( ellipsoid != geoNone() )
    {
      double perimeter = calc.measurePerimeter( feature.geometry() );
      perimeter = calc.convertLengthMeasurement( perimeter, displayDistanceUnits() );
      str = formatDistance( perimeter );
      derivedAttributes.insert( tr( "Perimeter (Ellipsoidal — %1)" ).arg( ellipsoid ), str );
    }
    str = formatDistance( feature.geometry().constGet()->perimeter()
                          * QgsUnitTypes::fromUnitToUnitFactor( layer->crs().mapUnits(), cartesianDistanceUnits ), cartesianDistanceUnits );
    derivedAttributes.insert( tr( "Perimeter (Cartesian)" ), str );

    str = QLocale().toString( feature.geometry().constGet()->nCoordinates() );
    derivedAttributes.insert( tr( "Vertices" ), str );

    if ( !layerPoint.isEmpty() )
    {
      //add details of closest vertex to identify point
      closestVertexAttributes( *feature.geometry().constGet(), vId, layer, derivedAttributes );
      closestPointAttributes( *feature.geometry().constGet(), layerPoint, derivedAttributes );
    }
  }
  else if ( geometryType == QgsWkbTypes::PointGeometry )
  {
    if ( QgsWkbTypes::flatType( wkbType ) == QgsWkbTypes::Point )
    {
      // Include the x and y coordinates of the point as a derived attribute
      QgsPointXY pnt = mCanvas->mapSettings().layerToMapCoordinates( layer, feature.geometry().asPoint() );
      QString str = formatXCoordinate( pnt );
      derivedAttributes.insert( tr( "X" ), str );
      str = formatYCoordinate( pnt );
      derivedAttributes.insert( tr( "Y" ), str );

      if ( QgsWkbTypes::hasZ( wkbType ) )
      {
        str = QLocale().toString( static_cast<const QgsPoint *>( feature.geometry().constGet() )->z(), 'g', 10 );
        derivedAttributes.insert( tr( "Z" ), str );
      }
      if ( QgsWkbTypes::hasM( wkbType ) )
      {
        str = QLocale().toString( static_cast<const QgsPoint *>( feature.geometry().constGet() )->m(), 'g', 10 );
        derivedAttributes.insert( tr( "M" ), str );
      }
    }
    else
    {
      //multipart

      if ( !layerPoint.isEmpty() )
      {
        //add details of closest vertex to identify point
        const QgsAbstractGeometry *geom = feature.geometry().constGet();
        closestVertexAttributes( *geom, vId, layer, derivedAttributes );
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
  QgsDebugMsg( "point = " + point.toString() );
  if ( !layer )
    return false;

  std::unique_ptr< QgsRasterDataProvider > dprovider( layer->dataProvider()->clone() );
  if ( !dprovider )
    return false;

  int capabilities = dprovider->capabilities();
  if ( !( capabilities & QgsRasterDataProvider::Identify ) )
    return false;

  if ( identifyContext.isTemporal() )
  {
    if ( !layer->temporalProperties()->isVisibleInTemporalRange( identifyContext.temporalRange() ) )
      return false;

    dprovider->temporalCapabilities()->setRequestedTemporalRange( identifyContext.temporalRange() );
  }

  QgsPointXY pointInCanvasCrs = point;
  try
  {
    point = toLayerCoordinates( layer, point );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse )
    QgsDebugMsg( QStringLiteral( "coordinate not reprojectable: %1" ).arg( cse.what() ) );
    return false;
  }
  QgsDebugMsg( QStringLiteral( "point = %1 %2" ).arg( point.x() ).arg( point.y() ) );

  if ( !layer->extent().contains( point ) )
    return false;

  QMap< QString, QString > attributes, derivedAttributes;

  QgsRaster::IdentifyFormat format = QgsRasterDataProvider::identifyFormatFromName( layer->customProperty( QStringLiteral( "identify/format" ) ).toString() );

  // check if the format is really supported otherwise use first supported format
  if ( !( QgsRasterDataProvider::identifyFormatToCapability( format ) & capabilities ) )
  {
    if ( capabilities & QgsRasterInterface::IdentifyFeature ) format = QgsRaster::IdentifyFormatFeature;
    else if ( capabilities & QgsRasterInterface::IdentifyValue ) format = QgsRaster::IdentifyFormatValue;
    else if ( capabilities & QgsRasterInterface::IdentifyHtml ) format = QgsRaster::IdentifyFormatHtml;
    else if ( capabilities & QgsRasterInterface::IdentifyText ) format = QgsRaster::IdentifyFormatText;
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

    QgsDebugMsg( QStringLiteral( "viewExtent.width = %1 viewExtent.height = %2" ).arg( viewExtent.width() ).arg( viewExtent.height() ) );
    QgsDebugMsg( QStringLiteral( "width = %1 height = %2" ).arg( width ).arg( height ) );
    QgsDebugMsg( QStringLiteral( "xRes = %1 yRes = %2 mapUnitsPerPixel = %3" ).arg( viewExtent.width() / width ).arg( viewExtent.height() / height ).arg( mapUnitsPerPixel ) );

    identifyResult = dprovider->identify( point, format, viewExtent, width, height );
  }

  derivedAttributes.unite( derivedAttributesForPoint( QgsPoint( pointInCanvasCrs ) ) );

  if ( identifyResult.isValid() )
  {
    QMap<int, QVariant> values = identifyResult.results();
    QgsGeometry geometry;
    if ( format == QgsRaster::IdentifyFormatValue )
    {
      for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
      {
        QString valueString;
        if ( it.value().isNull() )
        {
          valueString = tr( "no data" );
        }
        else
        {
          QVariant value( it.value() );
          // The cast is legit. Quoting QT doc :
          // "Although this function is declared as returning QVariant::Type,
          // the return value should be interpreted as QMetaType::Type"
          if ( static_cast<QMetaType::Type>( value.type() ) == QMetaType::Float )
          {
            valueString = QgsRasterBlock::printValue( value.toFloat() );
          }
          else
          {
            valueString = QgsRasterBlock::printValue( value.toDouble() );
          }
        }
        attributes.insert( dprovider->generateBandName( it.key() ), valueString );
      }
      QString label = layer->name();
      results->append( IdentifyResult( qobject_cast<QgsMapLayer *>( layer ), label, attributes, derivedAttributes ) );
    }
    else if ( format == QgsRaster::IdentifyFormatFeature )
    {
      for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
      {
        QVariant value = it.value();
        if ( value.type() == QVariant::Bool && !value.toBool() )
        {
          // sublayer not visible or not queryable
          continue;
        }

        if ( value.type() == QVariant::String )
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
            derAttributes.unite( featureDerivedAttributes( feature, layer, toLayerCoordinates( layer, point ) ) );

            IdentifyResult identifyResult( qobject_cast<QgsMapLayer *>( layer ), labels.join( QLatin1String( " / " ) ), featureStore.fields(), feature, derAttributes );

            identifyResult.mParams.insert( QStringLiteral( "getFeatureInfoUrl" ), featureStore.params().value( QStringLiteral( "getFeatureInfoUrl" ) ) );
            results->append( identifyResult );
          }
        }
      }
    }
    else // text or html
    {
      QgsDebugMsg( QStringLiteral( "%1 HTML or text values" ).arg( values.size() ) );
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

QgsUnitTypes::DistanceUnit QgsMapToolIdentify::displayDistanceUnits() const
{
  return mCanvas->mapUnits();
}

QgsUnitTypes::AreaUnit QgsMapToolIdentify::displayAreaUnits() const
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

QString QgsMapToolIdentify::formatDistance( double distance, QgsUnitTypes::DistanceUnit unit ) const
{
  QgsSettings settings;
  bool baseUnit = settings.value( QStringLiteral( "qgis/measure/keepbaseunit" ), true ).toBool();

  return QgsDistanceArea::formatDistance( distance, mCoordinatePrecision, unit, baseUnit );
}

QString QgsMapToolIdentify::formatArea( double area, QgsUnitTypes::AreaUnit unit ) const
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
    QgsMapToolIdentify::IdentifyResult res( layer, classification.isEmpty() ? QString::number( id ) : QStringLiteral( "%1 (%2)" ).arg( id ).arg( classification ), ptStr, QMap<QString, QString>() );
    results.append( res );
    ++id;
  }
}
