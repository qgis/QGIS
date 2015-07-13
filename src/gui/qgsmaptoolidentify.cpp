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
{
  // set cursor
  QPixmap myIdentifyQPixmap = QPixmap(( const char ** ) identify_cursor );
  mCursor = QCursor( myIdentifyQPixmap, 1, 1 );
}

QgsMapToolIdentify::~QgsMapToolIdentify()
{
  delete mIdentifyMenu;
}

void QgsMapToolIdentify::canvasMoveEvent( QMouseEvent * e )
{
  Q_UNUSED( e );
}

void QgsMapToolIdentify::canvasPressEvent( QMouseEvent * e )
{
  Q_UNUSED( e );
}

void QgsMapToolIdentify::canvasReleaseEvent( QMouseEvent * e )
{
  Q_UNUSED( e );
}

QList<QgsMapToolIdentify::IdentifyResult> QgsMapToolIdentify::identify( int x, int y, QList<QgsMapLayer *> layerList, IdentifyMode mode )
{
  return identify( x, y, mode, layerList, AllLayers );
}

QList<QgsMapToolIdentify::IdentifyResult> QgsMapToolIdentify::identify( int x, int y, IdentifyMode mode, LayerType layerType )
{
  return identify( x, y, mode, QList<QgsMapLayer*>(), layerType );
}

QList<QgsMapToolIdentify::IdentifyResult> QgsMapToolIdentify::identify( int x, int y, IdentifyMode mode, QList<QgsMapLayer*> layerList, LayerType layerType )
{
  QList<IdentifyResult> results;

  mLastPoint = mCanvas->getCoordinateTransform()->toMapCoordinates( x, y );
  mLastExtent = mCanvas->extent();
  mLastMapUnitsPerPixel = mCanvas->mapUnitsPerPixel();

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

bool QgsMapToolIdentify::identifyLayer( QList<IdentifyResult> *results, QgsMapLayer *layer, QgsPoint point, QgsRectangle viewExtent, double mapUnitsPerPixel, LayerType layerType )
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

bool QgsMapToolIdentify::identifyVectorLayer( QList<IdentifyResult> *results, QgsVectorLayer *layer, QgsPoint point )
{
  if ( !layer || !layer->hasGeometryType() )
    return false;

  if ( layer->hasScaleBasedVisibility() &&
       ( layer->minimumScale() > mCanvas->mapSettings().scale() ||
         layer->maximumScale() <= mCanvas->mapSettings().scale() ) )
  {
    QgsDebugMsg( "Out of scale limits" );
    return false;
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QMap< QString, QString > commonDerivedAttributes;

  commonDerivedAttributes.insert( tr( "(clicked coordinate)" ), point.toString() );

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
  QgsFeatureRendererV2* renderer = layer->rendererV2();
  if ( renderer && renderer->capabilities() & QgsFeatureRendererV2::ScaleDependent )
  {
    // setup scale for scale dependent visibility (rule based)
    renderer->startRender( context, layer->pendingFields() );
    filter = renderer->capabilities() & QgsFeatureRendererV2::Filter;
  }

  for ( ; f_it != featureList.end(); ++f_it )
  {
    QMap< QString, QString > derivedAttributes = commonDerivedAttributes;

    QgsFeatureId fid = f_it->id();

    if ( filter && !renderer->willRenderFeature( *f_it ) )
      continue;

    featureCount++;

    derivedAttributes.unite( featureDerivedAttributes( &( *f_it ), layer ) );

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

QMap< QString, QString > QgsMapToolIdentify::featureDerivedAttributes( QgsFeature *feature, QgsMapLayer *layer )
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

  QGis::WkbType wkbType = QGis::WKBNoGeometry;
  QGis::GeometryType geometryType = QGis::NoGeometry;

  if ( feature->constGeometry() )
  {
    geometryType = feature->constGeometry()->type();
    wkbType = feature->constGeometry()->wkbType();
  }

  if ( geometryType == QGis::Line )
  {
    double dist = calc.measure( feature->constGeometry() );
    QGis::UnitType myDisplayUnits;
    convertMeasurement( calc, dist, myDisplayUnits, false );
    QString str = calc.textUnit( dist, 3, myDisplayUnits, false );  // dist and myDisplayUnits are out params
    derivedAttributes.insert( tr( "Length" ), str );
    if ( wkbType == QGis::WKBLineString || wkbType == QGis::WKBLineString25D )
    {
      // Add the start and end points in as derived attributes
      QgsPoint pnt = mCanvas->mapSettings().layerToMapCoordinates( layer, feature->constGeometry()->asPolyline().first() );
      str = QLocale::system().toString( pnt.x(), 'g', 10 );
      derivedAttributes.insert( tr( "firstX", "attributes get sorted; translation for lastX should be lexically larger than this one" ), str );
      str = QLocale::system().toString( pnt.y(), 'g', 10 );
      derivedAttributes.insert( tr( "firstY" ), str );
      pnt = mCanvas->mapSettings().layerToMapCoordinates( layer, feature->constGeometry()->asPolyline().last() );
      str = QLocale::system().toString( pnt.x(), 'g', 10 );
      derivedAttributes.insert( tr( "lastX", "attributes get sorted; translation for firstX should be lexically smaller than this one" ), str );
      str = QLocale::system().toString( pnt.y(), 'g', 10 );
      derivedAttributes.insert( tr( "lastY" ), str );
    }
  }
  else if ( geometryType == QGis::Polygon )
  {
    double area = calc.measure( feature->constGeometry() );
    double perimeter = calc.measurePerimeter( feature->constGeometry() );
    QGis::UnitType myDisplayUnits;
    convertMeasurement( calc, area, myDisplayUnits, true );  // area and myDisplayUnits are out params
    QString str = calc.textUnit( area, 3, myDisplayUnits, true );
    derivedAttributes.insert( tr( "Area" ), str );
    convertMeasurement( calc, perimeter, myDisplayUnits, false );  // perimeter and myDisplayUnits are out params
    str = calc.textUnit( perimeter, 3, myDisplayUnits, false );
    derivedAttributes.insert( tr( "Perimeter" ), str );
  }
  else if ( geometryType == QGis::Point &&
            ( wkbType == QGis::WKBPoint || wkbType == QGis::WKBPoint25D ) )
  {
    // Include the x and y coordinates of the point as a derived attribute
    QgsPoint pnt = mCanvas->mapSettings().layerToMapCoordinates( layer, feature->constGeometry()->asPoint() );
    QString str = QLocale::system().toString( pnt.x(), 'g', 10 );
    derivedAttributes.insert( "X", str );
    str = QLocale::system().toString( pnt.y(), 'g', 10 );
    derivedAttributes.insert( "Y", str );
  }

  return derivedAttributes;
}

bool QgsMapToolIdentify::identifyRasterLayer( QList<IdentifyResult> *results, QgsRasterLayer *layer, QgsPoint point, QgsRectangle viewExtent, double mapUnitsPerPixel )
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

  derivedAttributes.insert( tr( "(clicked coordinate)" ), point.toString() );

  if ( identifyResult.isValid() )
  {
    QMap<int, QVariant> values = identifyResult.results();
    QgsGeometry geometry;
    if ( format == QgsRaster::IdentifyFormatValue )
    {
      foreach ( int bandNo, values.keys() )
      {
        QString valueString;
        if ( values.value( bandNo ).isNull() )
        {
          valueString = tr( "no data" );
        }
        else
        {
          double value = values.value( bandNo ).toDouble();
          valueString = QgsRasterBlock::printValue( value );
        }
        attributes.insert( dprovider->generateBandName( bandNo ), valueString );
      }
      QString label = layer->name();
      results->append( IdentifyResult( qobject_cast<QgsMapLayer *>( layer ), label, attributes, derivedAttributes ) );
    }
    else if ( format == QgsRaster::IdentifyFormatFeature )
    {
      foreach ( int i, values.keys() )
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

        foreach ( QgsFeatureStore featureStore, featureStoreList )
        {
          foreach ( QgsFeature feature, featureStore.features() )
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
      foreach ( int bandNo, values.keys() )
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
  // Helper for converting between meters and feet
  // The parameter &u is out only...

  // Get the canvas units
  QGis::UnitType myUnits = mCanvas->mapUnits();

  calc.convertMeasurement( measure, myUnits, displayUnits(), isArea );
  u = myUnits;
}

QGis::UnitType QgsMapToolIdentify::displayUnits()
{
  return mCanvas->mapUnits();
}

void QgsMapToolIdentify::formatChanged( QgsRasterLayer *layer )
{
  QgsDebugMsg( "Entered" );
  QList<IdentifyResult> results;
  if ( identifyRasterLayer( &results, layer, mLastPoint, mLastExtent, mLastMapUnitsPerPixel ) )
  {
    emit changedRasterResults( results );
  }
}

