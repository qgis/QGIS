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

#include "qgscursors.h"
#include "qgsdistancearea.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsidentifyresults.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgsmessageviewer.h"
#include "qgsmaptoolidentify.h"
#include "qgsrasterlayer.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsmaplayerregistry.h"
#include "qgisapp.h"

#include <QSettings>
#include <QMessageBox>
#include <QMouseEvent>
#include <QCursor>
#include <QPixmap>
#include <QStatusBar>

QgsMapToolIdentify::QgsMapToolIdentify( QgsMapCanvas* canvas )
    : QgsMapTool( canvas )
{
  // set cursor
  QPixmap myIdentifyQPixmap = QPixmap(( const char ** ) identify_cursor );
  mCursor = QCursor( myIdentifyQPixmap, 1, 1 );
}

QgsMapToolIdentify::~QgsMapToolIdentify()
{
  if ( mResults )
  {
    mResults->done( 0 );
  }
}

QgsIdentifyResults *QgsMapToolIdentify::results()
{
  if ( !mResults )
    mResults = new QgsIdentifyResults( mCanvas, mCanvas->window() );

  return mResults;
}

void QgsMapToolIdentify::canvasMoveEvent( QMouseEvent *e )
{
  Q_UNUSED( e );
}

void QgsMapToolIdentify::canvasPressEvent( QMouseEvent *e )
{
  Q_UNUSED( e );
}

void QgsMapToolIdentify::canvasReleaseEvent( QMouseEvent *e )
{
  if ( !mCanvas || mCanvas->isDrawing() )
  {
    return;
  }

  results()->clear();

  QSettings settings;
  int identifyMode = settings.value( "/Map/identifyMode", 0 ).toInt();

  bool res = false;

  if ( identifyMode == 0 )
  {
    QgsMapLayer *layer = mCanvas->currentLayer();

    if ( !layer )
    {
      QMessageBox::warning( mCanvas,
                            tr( "No active layer" ),
                            tr( "To identify features, you must choose an active layer by clicking on its name in the legend" ) );
      return;
    }

    QApplication::setOverrideCursor( Qt::WaitCursor );

    res = identifyLayer( layer, e->x(), e->y() );

    QApplication::restoreOverrideCursor();
  }
  else
  {
    connect( this, SIGNAL( identifyProgress( int, int ) ), QgisApp::instance(), SLOT( showProgress( int, int ) ) );
    connect( this, SIGNAL( identifyMessage( QString ) ), QgisApp::instance(), SLOT( showStatusMessage( QString ) ) );

    QApplication::setOverrideCursor( Qt::WaitCursor );

    QStringList noIdentifyLayerIdList = QgsProject::instance()->readListEntry( "Identify", "/disabledLayers" );

    for ( int i = 0; i < mCanvas->layerCount(); i++ )
    {
      QgsMapLayer *layer = mCanvas->layer( i );

      emit identifyProgress( i, mCanvas->layerCount() );
      emit identifyMessage( tr( "Identifying on %1..." ).arg( layer->name() ) );

      if ( noIdentifyLayerIdList.contains( layer->id() ) )
        continue;

      if ( identifyLayer( layer, e->x(), e->y() ) )
      {
        res = true;
        if ( identifyMode == 1 )
          break;
      }
    }

    emit identifyProgress( mCanvas->layerCount(), mCanvas->layerCount() );
    emit identifyMessage( tr( "Identifying done." ) );

    disconnect( this, SIGNAL( identifyProgress( int, int ) ), QgisApp::instance(), SLOT( showProgress( int, int ) ) );
    disconnect( this, SIGNAL( identifyMessage( QString ) ), QgisApp::instance(), SLOT( showStatusMessage( QString ) ) );

    QApplication::restoreOverrideCursor();
  }

  if ( res )
  {
    results()->show();
  }
  else
  {
    QSettings mySettings;
    bool myDockFlag = mySettings.value( "/qgis/dockIdentifyResults", false ).toBool();
    if ( !myDockFlag )
    {
      results()->hide();
    }
    else
    {
      results()->clear();
    }
    QgisApp::instance()->statusBar()->showMessage( tr( "No features at this position found." ) );
  }
}

void QgsMapToolIdentify::activate()
{
  results()->activate();
  QgsMapTool::activate();
}

void QgsMapToolIdentify::deactivate()
{
  results()->deactivate();
  QgsMapTool::deactivate();
}

bool QgsMapToolIdentify::identifyLayer( QgsMapLayer *layer, int x, int y )
{
  bool res = false;

  if ( layer->type() == QgsMapLayer::RasterLayer )
  {
    res = identifyRasterLayer( qobject_cast<QgsRasterLayer *>( layer ), x, y );
  }
  else
  {
    res = identifyVectorLayer( qobject_cast<QgsVectorLayer *>( layer ), x, y );
  }

  return res;
}


bool QgsMapToolIdentify::identifyVectorLayer( QgsVectorLayer *layer, int x, int y )
{
  if ( !layer )
    return false;

  QMap< QString, QString > attributes, derivedAttributes;

  QgsPoint point = mCanvas->getCoordinateTransform()->toMapCoordinates( x, y );

  derivedAttributes.insert( tr( "(clicked coordinate)" ), point.toString() );

  // load identify radius from settings
  QSettings settings;
  double identifyValue = settings.value( "/Map/identifyRadius", QGis::DEFAULT_IDENTIFY_RADIUS ).toDouble();
  QString ellipsoid = settings.value( "/qgis/measure/ellipsoid", "WGS84" ).toString();

  if ( identifyValue <= 0.0 )
    identifyValue = QGis::DEFAULT_IDENTIFY_RADIUS;

  int featureCount = 0;

  QgsFeatureList featureList;

  // toLayerCoordinates will throw an exception for an 'invalid' point.
  // For example, if you project a world map onto a globe using EPSG 2163
  // and then click somewhere off the globe, an exception will be thrown.
  try
  {
    // create the search rectangle
    double searchRadius = mCanvas->extent().width() * ( identifyValue / 100.0 );

    QgsRectangle r;
    r.setXMinimum( point.x() - searchRadius );
    r.setXMaximum( point.x() + searchRadius );
    r.setYMinimum( point.y() - searchRadius );
    r.setYMaximum( point.y() + searchRadius );

    r = toLayerCoordinates( layer, r );

    layer->select( layer->pendingAllAttributesList(), r, true, true );
    QgsFeature f;
    while ( layer->nextFeature( f ) )
      featureList << QgsFeature( f );
  }
  catch ( QgsCsException & cse )
  {
    Q_UNUSED( cse );
    // catch exception for 'invalid' point and proceed with no features found
    QgsDebugMsg( QString( "Caught CRS exception %1" ).arg( cse.what() ) );
  }

  // init distance/area calculator
  QgsDistanceArea calc;
  if ( !featureList.count() == 0 )
  {
    calc.setEllipsoidalMode( mCanvas->hasCrsTransformEnabled() );
    calc.setEllipsoid( ellipsoid );
    calc.setSourceCrs( layer->crs().srsid() );
  }
  QgsFeatureList::iterator f_it = featureList.begin();

  for ( ; f_it != featureList.end(); ++f_it )
  {
    featureCount++;

    QgsFeatureId fid = f_it->id();
    QMap<QString, QString> derivedAttributes;

    // Calculate derived attributes and insert:
    // measure distance or area depending on geometry type
    if ( layer->geometryType() == QGis::Line )
    {
      double dist = calc.measure( f_it->geometry() );
      QGis::UnitType myDisplayUnits;
      convertMeasurement( calc, dist, myDisplayUnits, false );
      QString str = calc.textUnit( dist, 3, myDisplayUnits, false );  // dist and myDisplayUnits are out params
      derivedAttributes.insert( tr( "Length" ), str );
      if ( f_it->geometry()->wkbType() == QGis::WKBLineString ||
           f_it->geometry()->wkbType() == QGis::WKBLineString25D )
      {
        // Add the start and end points in as derived attributes
        str = QLocale::system().toString( f_it->geometry()->asPolyline().first().x(), 'g', 10 );
        derivedAttributes.insert( tr( "firstX", "attributes get sorted; translation for lastX should be lexically larger than this one" ), str );
        str = QLocale::system().toString( f_it->geometry()->asPolyline().first().y(), 'g', 10 );
        derivedAttributes.insert( tr( "firstY" ), str );
        str = QLocale::system().toString( f_it->geometry()->asPolyline().last().x(), 'g', 10 );
        derivedAttributes.insert( tr( "lastX", "attributes get sorted; translation for firstX should be lexically smaller than this one" ), str );
        str = QLocale::system().toString( f_it->geometry()->asPolyline().last().y(), 'g', 10 );
        derivedAttributes.insert( tr( "lastY" ), str );
      }
    }
    else if ( layer->geometryType() == QGis::Polygon )
    {
      double area = calc.measure( f_it->geometry() );
      QGis::UnitType myDisplayUnits;
      convertMeasurement( calc, area, myDisplayUnits, true );  // area and myDisplayUnits are out params
      QString str = calc.textUnit( area, 3, myDisplayUnits, true );
      derivedAttributes.insert( tr( "Area" ), str );
    }
    else if ( layer->geometryType() == QGis::Point &&
              ( f_it->geometry()->wkbType() == QGis::WKBPoint ||
                f_it->geometry()->wkbType() == QGis::WKBPoint25D ) )
    {
      // Include the x and y coordinates of the point as a derived attribute
      QString str;
      str = QLocale::system().toString( f_it->geometry()->asPoint().x(), 'g', 10 );
      derivedAttributes.insert( "X", str );
      str = QLocale::system().toString( f_it->geometry()->asPoint().y(), 'g', 10 );
      derivedAttributes.insert( "Y", str );
    }

    derivedAttributes.insert( tr( "feature id" ), fid < 0 ? tr( "new feature" ) : FID_TO_STRING( fid ) );

    results()->addFeature( layer, *f_it, derivedAttributes );
  }

  QgsDebugMsg( "Feature count on identify: " + QString::number( featureCount ) );

  return featureCount > 0;
}

bool QgsMapToolIdentify::identifyRasterLayer( QgsRasterLayer *layer, int x, int y )
{
  bool res = true;

  if ( !layer )
    return false;

  QgsRasterDataProvider *dprovider = layer->dataProvider();
  if ( dprovider && ( dprovider->capabilities() & QgsRasterDataProvider::Identify ) == 0 )
    return false;

  QMap< QString, QString > attributes, derivedAttributes;
  QgsPoint idPoint = mCanvas->getCoordinateTransform()->toMapCoordinates( x, y );
  try
  {
    idPoint = toLayerCoordinates( layer, idPoint );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    QgsDebugMsg( QString( "coordinate not reprojectable: %1" ).arg( cse.what() ) );
    return false;
  }

  QString type;

  if ( layer->providerType() == "wms" )
  {
    type = tr( "WMS layer" );

    //if WMS layer does not cover the view origin,
    //we need to map the view pixel coordinates
    //to WMS layer pixel coordinates
    QgsRectangle viewExtent = toLayerCoordinates( layer, mCanvas->extent() );
    QgsRectangle layerExtent = layer->extent();
    double mapUnitsPerPixel = mCanvas->mapUnitsPerPixel();
    if ( mapUnitsPerPixel > 0 && viewExtent.intersects( layerExtent ) )
    {
      double xMinView = viewExtent.xMinimum();
      double yMaxView = viewExtent.yMaximum();
      double xMinLayer = layerExtent.xMinimum();
      double yMaxLayer = layerExtent.yMaximum();

      idPoint.set(
        xMinView < xMinLayer ? floor( x - ( xMinLayer - xMinView ) / mapUnitsPerPixel ) : x,
        yMaxView > yMaxLayer ? floor( y - ( yMaxView - yMaxLayer ) / mapUnitsPerPixel ) : y
      );

      attributes.insert( tr( "Feature info" ), layer->identifyAsHtml( idPoint ) );
    }
    else
    {
      res = false;
    }
  }
  else
  {
    type = tr( "Raster" );
    res = layer->extent().contains( idPoint ) && layer->identify( idPoint, attributes );
  }

  if ( res )
  {
    derivedAttributes.insert( tr( "(clicked coordinate)" ), idPoint.toString() );
    results()->addFeature( layer, type, attributes, derivedAttributes );
  }

  return res;
}


void QgsMapToolIdentify::convertMeasurement( QgsDistanceArea &calc, double &measure, QGis::UnitType &u, bool isArea )
{
  // Helper for converting between meters and feet
  // The parameter &u is out only...

  // Get the canvas units
  QGis::UnitType myUnits = mCanvas->mapUnits();

  // Get the units for display
  QSettings settings;
  QString myDisplayUnitsTxt = settings.value( "/qgis/measure/displayunits", "meters" ).toString();

  QGis::UnitType displayUnits;
  if ( myDisplayUnitsTxt == "feet" )
  {
    displayUnits = QGis::Feet;
  }
  else
  {
    displayUnits = QGis::Meters;
  }

  calc.convertMeasurement( measure, myUnits, displayUnits, isArea );
  u = myUnits;
}
