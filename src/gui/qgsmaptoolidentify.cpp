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
#include "qgscursors.h"
#include "qgsdistancearea.h"
#include "qgsfeature.h"
#include "qgsfeaturestore.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsidentifymenu.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolidentify.h"
#include "qgsmaptopixel.h"
#include "qgsmessageviewer.h"
#include "qgsmaplayer.h"
#include "qgsrasterlayer.h"
#include "qgsrasteridentifyresult.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsmaplayerregistry.h"
#include "qgsrendererv2.h"
#include "qgsgeometryutils.h"
#include "qgsgeometrycollectionv2.h"
#include "qgscurvev2.h"
#include "qgscoordinateutils.h"

#include <QSettings>
#include <QMouseEvent>
#include <QCursor>
#include <QPixmap>
#include <QStatusBar>
#include <QVariant>
#include <QMenu>

QgsMapToolIdentify::QgsMapToolIdentify( QgsMapCanvas* canvas )
    : QgsMapTool( canvas )
    , mIdentifyMenu( new QgsIdentifyMenu( mCanvas ) )
    , mLastMapUnitsPerPixel( -1.0 )
    , mCoordinatePrecision( 6 )
{
  // set cursor
  QPixmap myIdentifyQPixmap = QPixmap(( const char ** ) identify_cursor );
  mCursor = QCursor( myIdentifyQPixmap, 1, 1 );
}

QgsMapToolIdentify::~QgsMapToolIdentify()
{
  delete mIdentifyMenu;
}

void QgsMapToolIdentify::canvasMoveEvent( QgsMapMouseEvent* e )
{
  Q_UNUSED( e );
}

void QgsMapToolIdentify::canvasPressEvent( QgsMapMouseEvent* e )
{
  Q_UNUSED( e );
}

void QgsMapToolIdentify::canvasReleaseEvent( QgsMapMouseEvent* e )
{
  Q_UNUSED( e );
}

QList<QgsMapToolIdentify::IdentifyResult> QgsMapToolIdentify::identify( int x, int y, const QList<QgsMapLayer *>& layerList, IdentifyMode mode )
{
  return identify( x, y, mode, layerList, AllLayers );
}

QList<QgsMapToolIdentify::IdentifyResult> QgsMapToolIdentify::identify( int x, int y, IdentifyMode mode, const LayerType& layerType )
{
  return identify( x, y, mode, QList<QgsMapLayer*>(), layerType );
}

QList<QgsMapToolIdentify::IdentifyResult> QgsMapToolIdentify::identify( int x, int y, IdentifyMode mode, const QList<QgsMapLayer*>& layerList, const LayerType& layerType )
{
  QList<IdentifyResult> results;

  mLastPoint = mCanvas->getCoordinateTransform()->toMapCoordinates( x, y );
  mLastExtent = mCanvas->extent();
  mLastMapUnitsPerPixel = mCanvas->mapUnitsPerPixel();

  mCoordinatePrecision = QgsCoordinateUtils::calculateCoordinatePrecision( mLastMapUnitsPerPixel, mCanvas->mapSettings().destinationCrs() );

  if ( mode == DefaultQgsSetting )
  {
    QSettings settings;
    mode = static_cast<IdentifyMode>( settings.value( "/Map/identifyMode", 0 ).toInt() );
  }

  if ( mode == LayerSelection )
  {
    QList<IdentifyResult> results = identify( x, y, TopDownAll, layerList, layerType );
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

    QApplication::setOverrideCursor( Qt::WaitCursor );

    identifyLayer( &results, layer, mLastPoint, mLastExtent, mLastMapUnitsPerPixel, layerType );
  }
  else
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );

    QStringList noIdentifyLayerIdList = QgsProject::instance()->readListEntry( "Identify", "/disabledLayers" );

    int layerCount;
    if ( layerList.isEmpty() )
      layerCount = mCanvas->layerCount();
    else
      layerCount = layerList.count();


    for ( int i = 0; i < layerCount; i++ )
    {

      QgsMapLayer *layer;
      if ( layerList.isEmpty() )
        layer = mCanvas->layer( i );
      else
        layer = layerList.value( i );

      emit identifyProgress( i, mCanvas->layerCount() );
      emit identifyMessage( tr( "Identifying on %1..." ).arg( layer->name() ) );

      if ( noIdentifyLayerIdList.contains( layer->id() ) )
        continue;

      if ( identifyLayer( &results, layer,  mLastPoint, mLastExtent, mLastMapUnitsPerPixel, layerType ) )
      {
        if ( mode == TopDownStopAtFirst )
          break;
      }
    }

    emit identifyProgress( mCanvas->layerCount(), mCanvas->layerCount() );
    emit identifyMessage( tr( "Identifying done." ) );
  }

  QApplication::restoreOverrideCursor();

  return results;
}

void QgsMapToolIdentify::activate()
{
  QgsMapTool::activate();
}

void QgsMapToolIdentify::deactivate()
{
  QgsMapTool::deactivate();
}

bool QgsMapToolIdentify::identifyLayer( QList<IdentifyResult> *results, QgsMapLayer *layer, const QgsPoint& point, const QgsRectangle& viewExtent, double mapUnitsPerPixel, const LayerType& layerType )
{
  if ( layer->type() == QgsMapLayer::RasterLayer && layerType.testFlag( RasterLayer ) )
  {
    return identifyRasterLayer( results, qobject_cast<QgsRasterLayer *>( layer ), point, viewExtent, mapUnitsPerPixel );
  }
  else if ( layer->type() == QgsMapLayer::VectorLayer && layerType.testFlag( VectorLayer ) )
  {
    return identifyVectorLayer( results, qobject_cast<QgsVectorLayer *>( layer ), point );
  }
  else
  {
    return false;
  }
}

bool QgsMapToolIdentify::identifyVectorLayer( QList<IdentifyResult> *results, QgsVectorLayer *layer, const QgsPoint& point )
{
  if ( !layer || !layer->hasGeometryType() )
    return false;

  if ( !layer->isInScaleRange( mCanvas->mapSettings().scale() ) )
  {
    QgsDebugMsg( "Out of scale limits" );
    return false;
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QMap< QString, QString > commonDerivedAttributes;

  commonDerivedAttributes.insert( tr( "(clicked coordinate X)" ), formatXCoordinate( point ) );
  commonDerivedAttributes.insert( tr( "(clicked coordinate Y)" ), formatYCoordinate( point ) );

  int featureCount = 0;

  QgsFeatureList featureList;

  // toLayerCoordinates will throw an exception for an 'invalid' point.
  // For example, if you project a world map onto a globe using EPSG 2163
  // and then click somewhere off the globe, an exception will be thrown.
  try
  {
    // create the search rectangle
    double searchRadius = searchRadiusMU( mCanvas );

    QgsRectangle r;
    r.setXMinimum( point.x() - searchRadius );
    r.setXMaximum( point.x() + searchRadius );
    r.setYMinimum( point.y() - searchRadius );
    r.setYMaximum( point.y() + searchRadius );

    r = toLayerCoordinates( layer, r );

    QgsFeatureIterator fit = layer->getFeatures( QgsFeatureRequest().setFilterRect( r ).setFlags( QgsFeatureRequest::ExactIntersect ) );
    QgsFeature f;
    while ( fit.nextFeature( f ) )
      featureList << QgsFeature( f );
  }
  catch ( QgsCsException & cse )
  {
    Q_UNUSED( cse );
    // catch exception for 'invalid' point and proceed with no features found
    QgsDebugMsg( QString( "Caught CRS exception %1" ).arg( cse.what() ) );
  }

  QgsFeatureList::iterator f_it = featureList.begin();

  bool filter = false;

  QgsRenderContext context( QgsRenderContext::fromMapSettings( mCanvas->mapSettings() ) );
  context.expressionContext() << QgsExpressionContextUtils::layerScope( layer );
  QgsFeatureRendererV2* renderer = layer->rendererV2();
  if ( renderer && renderer->capabilities() & QgsFeatureRendererV2::ScaleDependent )
  {
    // setup scale for scale dependent visibility (rule based)
    renderer->startRender( context, layer->fields() );
    filter = renderer->capabilities() & QgsFeatureRendererV2::Filter;
  }

  for ( ; f_it != featureList.end(); ++f_it )
  {
    QMap< QString, QString > derivedAttributes = commonDerivedAttributes;

    QgsFeatureId fid = f_it->id();
    context.expressionContext().setFeature( *f_it );

    if ( filter && !renderer->willRenderFeature( *f_it, context ) )
      continue;

    featureCount++;

    derivedAttributes.unite( featureDerivedAttributes( &( *f_it ), layer, toLayerCoordinates( layer, point ) ) );

    derivedAttributes.insert( tr( "feature id" ), fid < 0 ? tr( "new feature" ) : FID_TO_STRING( fid ) );

    results->append( IdentifyResult( qobject_cast<QgsMapLayer *>( layer ), *f_it, derivedAttributes ) );
  }

  if ( renderer && renderer->capabilities() & QgsFeatureRendererV2::ScaleDependent )
  {
    renderer->stopRender( context );
  }

  QgsDebugMsg( "Feature count on identify: " + QString::number( featureCount ) );

  QApplication::restoreOverrideCursor();
  return featureCount > 0;
}

void QgsMapToolIdentify::closestVertexAttributes( const QgsAbstractGeometryV2& geometry, QgsVertexId vId, QgsMapLayer *layer, QMap< QString, QString >& derivedAttributes )
{
  QString str = QLocale::system().toString( vId.vertex + 1 );
  derivedAttributes.insert( tr( "Closest vertex number" ), str );

  QgsPointV2 closestPoint = geometry.vertexAt( vId );

  QgsPoint closestPointMapCoords = mCanvas->mapSettings().layerToMapCoordinates( layer, QgsPoint( closestPoint.x(), closestPoint.y() ) );
  derivedAttributes.insert( "Closest vertex X", formatXCoordinate( closestPointMapCoords ) );
  derivedAttributes.insert( "Closest vertex Y", formatYCoordinate( closestPointMapCoords ) );

  if ( closestPoint.is3D() )
  {
    str = QLocale::system().toString( closestPoint.z(), 'g', 10 );
    derivedAttributes.insert( "Closest vertex Z", str );
  }
  if ( closestPoint.isMeasure() )
  {
    str = QLocale::system().toString( closestPoint.m(), 'g', 10 );
    derivedAttributes.insert( "Closest vertex M", str );
  }

  if ( vId.type == QgsVertexId::CurveVertex )
  {
    double radius, centerX, centerY;
    QgsVertexId vIdBefore = vId;
    --vIdBefore.vertex;
    QgsVertexId vIdAfter = vId;
    ++vIdAfter.vertex;
    QgsGeometryUtils::circleCenterRadius( geometry.vertexAt( vIdBefore ), geometry.vertexAt( vId ),
                                          geometry.vertexAt( vIdAfter ), radius, centerX, centerY );
    derivedAttributes.insert( "Closest vertex radius", QLocale::system().toString( radius ) );
  }
}

QString QgsMapToolIdentify::formatCoordinate( const QgsPoint& canvasPoint ) const
{
  return QgsCoordinateUtils::formatCoordinateForProject( canvasPoint, mCanvas->mapSettings().destinationCrs(),
         mCoordinatePrecision );
}

QString QgsMapToolIdentify::formatXCoordinate( const QgsPoint& canvasPoint ) const
{
  QString coordinate = formatCoordinate( canvasPoint );
  return coordinate.split( ',' ).at( 0 );
}

QString QgsMapToolIdentify::formatYCoordinate( const QgsPoint& canvasPoint ) const
{
  QString coordinate = formatCoordinate( canvasPoint );
  return coordinate.split( ',' ).at( 1 );
}

QMap< QString, QString > QgsMapToolIdentify::featureDerivedAttributes( QgsFeature *feature, QgsMapLayer *layer, const QgsPoint& layerPoint )
{
  // Calculate derived attributes and insert:
  // measure distance or area depending on geometry type
  QMap< QString, QString > derivedAttributes;

  // init distance/area calculator
  QString ellipsoid = QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE );
  QgsDistanceArea calc;
  calc.setEllipsoidalMode( mCanvas->hasCrsTransformEnabled() );
  calc.setEllipsoid( ellipsoid );
  calc.setSourceCrs( layer->crs().srsid() );

  QgsWKBTypes::Type wkbType = QgsWKBTypes::NoGeometry;
  QGis::GeometryType geometryType = QGis::NoGeometry;

  QgsVertexId vId;
  QgsPointV2 closestPoint;
  if ( feature->constGeometry() )
  {
    geometryType = feature->constGeometry()->type();
    wkbType = feature->constGeometry()->geometry()->wkbType();
    //find closest vertex to clicked point
    closestPoint = QgsGeometryUtils::closestVertex( *feature->constGeometry()->geometry(), QgsPointV2( layerPoint.x(), layerPoint.y() ), vId );
  }

  if ( QgsWKBTypes::isMultiType( wkbType ) )
  {
    QString str = QLocale::system().toString( static_cast<QgsGeometryCollectionV2*>( feature->constGeometry()->geometry() )->numGeometries() );
    derivedAttributes.insert( tr( "Parts" ), str );
    str = QLocale::system().toString( vId.part + 1 );
    derivedAttributes.insert( tr( "Part number" ), str );
  }

  if ( geometryType == QGis::Line )
  {
    double dist = calc.measureLength( feature->constGeometry() );
    dist = calc.convertLengthMeasurement( dist, displayDistanceUnits() );
    QString str = formatDistance( dist );
    derivedAttributes.insert( tr( "Length" ), str );

    const QgsCurveV2* curve = dynamic_cast< const QgsCurveV2* >( feature->constGeometry()->geometry() );
    if ( curve )
    {
      str = QLocale::system().toString( curve->nCoordinates() );
      derivedAttributes.insert( tr( "Vertices" ), str );

      //add details of closest vertex to identify point
      closestVertexAttributes( *curve, vId, layer, derivedAttributes );

      // Add the start and end points in as derived attributes
      QgsPoint pnt = mCanvas->mapSettings().layerToMapCoordinates( layer, QgsPoint( curve->startPoint().x(), curve->startPoint().y() ) );
      str = formatXCoordinate( pnt );
      derivedAttributes.insert( tr( "firstX", "attributes get sorted; translation for lastX should be lexically larger than this one" ), str );
      str = formatYCoordinate( pnt );
      derivedAttributes.insert( tr( "firstY" ), str );
      pnt = mCanvas->mapSettings().layerToMapCoordinates( layer, QgsPoint( curve->endPoint().x(), curve->endPoint().y() ) );
      str = formatXCoordinate( pnt );
      derivedAttributes.insert( tr( "lastX", "attributes get sorted; translation for firstX should be lexically smaller than this one" ), str );
      str = formatYCoordinate( pnt );
      derivedAttributes.insert( tr( "lastY" ), str );
    }
  }
  else if ( geometryType == QGis::Polygon )
  {
    double area = calc.measureArea( feature->constGeometry() );
    area = calc.convertAreaMeasurement( area, displayAreaUnits() );
    QString str = formatArea( area );
    derivedAttributes.insert( tr( "Area" ), str );

    double perimeter = calc.measurePerimeter( feature->constGeometry() );
    perimeter = calc.convertLengthMeasurement( perimeter, displayDistanceUnits() );
    str = formatDistance( perimeter );
    derivedAttributes.insert( tr( "Perimeter" ), str );

    str = QLocale::system().toString( feature->constGeometry()->geometry()->nCoordinates() );
    derivedAttributes.insert( tr( "Vertices" ), str );

    //add details of closest vertex to identify point
    closestVertexAttributes( *feature->constGeometry()->geometry(), vId, layer, derivedAttributes );
  }
  else if ( geometryType == QGis::Point &&
            QgsWKBTypes::flatType( wkbType ) == QgsWKBTypes::Point )
  {
    // Include the x and y coordinates of the point as a derived attribute
    QgsPoint pnt = mCanvas->mapSettings().layerToMapCoordinates( layer, feature->constGeometry()->asPoint() );
    QString str = formatXCoordinate( pnt );
    derivedAttributes.insert( "X", str );
    str = formatYCoordinate( pnt );
    derivedAttributes.insert( "Y", str );

    if ( QgsWKBTypes::hasZ( wkbType ) )
    {
      str = QLocale::system().toString( static_cast<QgsPointV2*>( feature->constGeometry()->geometry() )->z(), 'g', 10 );
      derivedAttributes.insert( "Z", str );
    }
    if ( QgsWKBTypes::hasM( wkbType ) )
    {
      str = QLocale::system().toString( static_cast<QgsPointV2*>( feature->constGeometry()->geometry() )->m(), 'g', 10 );
      derivedAttributes.insert( "M", str );
    }
  }

  return derivedAttributes;
}

bool QgsMapToolIdentify::identifyRasterLayer( QList<IdentifyResult> *results, QgsRasterLayer *layer, QgsPoint point, const QgsRectangle& viewExtent, double mapUnitsPerPixel )
{
  QgsDebugMsg( "point = " + point.toString() );
  if ( !layer )
    return false;

  QgsRasterDataProvider *dprovider = layer->dataProvider();
  if ( !dprovider )
    return false;

  int capabilities = dprovider->capabilities();
  if ( !( capabilities & QgsRasterDataProvider::Identify ) )
    return false;

  QgsPoint pointInCanvasCrs = point;
  try
  {
    point = toLayerCoordinates( layer, point );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    QgsDebugMsg( QString( "coordinate not reprojectable: %1" ).arg( cse.what() ) );
    return false;
  }
  QgsDebugMsg( QString( "point = %1 %2" ).arg( point.x() ).arg( point.y() ) );

  if ( !layer->extent().contains( point ) )
    return false;

  QMap< QString, QString > attributes, derivedAttributes;

  QgsRaster::IdentifyFormat format = QgsRasterDataProvider::identifyFormatFromName( layer->customProperty( "identify/format" ).toString() );

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
  if ( mCanvas->hasCrsTransformEnabled() && dprovider->crs() != mCanvas->mapSettings().destinationCrs() )
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
    // is doing some tricks with extent and size to allign raster to output which
    // would be difficult to replicate here.
    // Note: cutting the extent may result in slightly different x and y resolutions
    // and thus shifted point calculated back in QGIS WMS (using average resolution)
    //viewExtent = dprovider->extent().intersect( &viewExtent );

    // Width and height are calculated from not projected extent and we hope that
    // are similar to source width and height used to reproject layer for drawing.
    // TODO: may be very dangerous, because it may result in different resolutions
    // in source CRS, and WMS server (QGIS server) calcs wrong coor using average resolution.
    int width = qRound( viewExtent.width() / mapUnitsPerPixel );
    int height = qRound( viewExtent.height() / mapUnitsPerPixel );

    QgsDebugMsg( QString( "viewExtent.width = %1 viewExtent.height = %2" ).arg( viewExtent.width() ).arg( viewExtent.height() ) );
    QgsDebugMsg( QString( "width = %1 height = %2" ).arg( width ).arg( height ) );
    QgsDebugMsg( QString( "xRes = %1 yRes = %2 mapUnitsPerPixel = %3" ).arg( viewExtent.width() / width ).arg( viewExtent.height() / height ).arg( mapUnitsPerPixel ) );

    identifyResult = dprovider->identify( point, format, viewExtent, width, height );
  }

  derivedAttributes.insert( tr( "(clicked coordinate X)" ), formatXCoordinate( pointInCanvasCrs ) );
  derivedAttributes.insert( tr( "(clicked coordinate Y)" ), formatYCoordinate( pointInCanvasCrs ) );

  if ( identifyResult.isValid() )
  {
    QMap<int, QVariant> values = identifyResult.results();
    QgsGeometry geometry;
    if ( format == QgsRaster::IdentifyFormatValue )
    {
      Q_FOREACH ( int bandNo, values.keys() )
      {
        QString valueString;
        if ( values.value( bandNo ).isNull() )
        {
          valueString = tr( "no data" );
        }
        else
        {
          QVariant value( values.value( bandNo ) );
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
        attributes.insert( dprovider->generateBandName( bandNo ), valueString );
      }
      QString label = layer->name();
      results->append( IdentifyResult( qobject_cast<QgsMapLayer *>( layer ), label, attributes, derivedAttributes ) );
    }
    else if ( format == QgsRaster::IdentifyFormatFeature )
    {
      Q_FOREACH ( int i, values.keys() )
      {
        QVariant value = values.value( i );
        if ( value.type() == QVariant::Bool && !value.toBool() )
        {
          // sublayer not visible or not queryable
          continue;
        }

        if ( value.type() == QVariant::String )
        {
          // error
          // TODO: better error reporting
          QString label = layer->subLayers().value( i );
          attributes.clear();
          attributes.insert( tr( "Error" ), value.toString() );

          results->append( IdentifyResult( qobject_cast<QgsMapLayer *>( layer ), label, attributes, derivedAttributes ) );
          continue;
        }

        // list of feature stores for a single sublayer
        QgsFeatureStoreList featureStoreList = values.value( i ).value<QgsFeatureStoreList>();

        Q_FOREACH ( QgsFeatureStore featureStore, featureStoreList )
        {
          Q_FOREACH ( QgsFeature feature, featureStore.features() )
          {
            attributes.clear();
            // WMS sublayer and feature type, a sublayer may contain multiple feature types.
            // Sublayer name may be the same as layer name and feature type name
            // may be the same as sublayer. We try to avoid duplicities in label.
            QString sublayer = featureStore.params().value( "sublayer" ).toString();
            QString featureType = featureStore.params().value( "featureType" ).toString();
            // Strip UMN MapServer '_feature'
            featureType.remove( "_feature" );
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
            derAttributes.unite( featureDerivedAttributes( &feature, layer ) );

            IdentifyResult identifyResult( qobject_cast<QgsMapLayer *>( layer ), labels.join( " / " ), featureStore.fields(), feature, derAttributes );

            identifyResult.mParams.insert( "getFeatureInfoUrl", featureStore.params().value( "getFeatureInfoUrl" ) );
            results->append( identifyResult );
          }
        }
      }
    }
    else // text or html
    {
      QgsDebugMsg( QString( "%1 HTML or text values" ).arg( values.size() ) );
      Q_FOREACH ( int bandNo, values.keys() )
      {
        QString value = values.value( bandNo ).toString();
        attributes.clear();
        attributes.insert( "", value );

        QString label = layer->subLayers().value( bandNo );
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

void QgsMapToolIdentify::convertMeasurement( QgsDistanceArea &calc, double &measure, QGis::UnitType &u, bool isArea )
{
  // Helper for converting between units
  // The parameter &u is out only...

  // Get the canvas units
  QGis::UnitType myUnits = mCanvas->mapUnits();

  Q_NOWARN_DEPRECATED_PUSH
  calc.convertMeasurement( measure, myUnits, displayUnits(), isArea );
  u = displayUnits();
  Q_NOWARN_DEPRECATED_POP
}

QGis::UnitType QgsMapToolIdentify::displayUnits()
{
  return mCanvas->mapUnits();
}

QGis::UnitType QgsMapToolIdentify::displayDistanceUnits() const
{
  return mCanvas->mapUnits();
}

QgsUnitTypes::AreaUnit QgsMapToolIdentify::displayAreaUnits() const
{
  return QgsUnitTypes::distanceToAreaUnit( mCanvas->mapUnits() );
}

QString QgsMapToolIdentify::formatDistance( double distance ) const
{
  QSettings settings;
  bool baseUnit = settings.value( "/qgis/measure/keepbaseunit", false ).toBool();

  return QgsDistanceArea::formatDistance( distance, 3, displayDistanceUnits(), baseUnit );
}

QString QgsMapToolIdentify::formatArea( double area ) const
{
  QSettings settings;
  bool baseUnit = settings.value( "/qgis/measure/keepbaseunit", false ).toBool();

  return QgsDistanceArea::formatArea( area, 3, displayAreaUnits(), baseUnit );
}

void QgsMapToolIdentify::formatChanged( QgsRasterLayer *layer )
{
  QList<IdentifyResult> results;
  if ( identifyRasterLayer( &results, layer, mLastPoint, mLastExtent, mLastMapUnitsPerPixel ) )
  {
    emit changedRasterResults( results );
  }
}

