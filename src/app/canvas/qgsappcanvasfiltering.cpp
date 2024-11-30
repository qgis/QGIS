/***************************************************************************
    qgsappcanvasfiltering.cpp
    -------------------------
    begin                : March 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsappcanvasfiltering.h"
#include "moc_qgsappcanvasfiltering.cpp"
#include "qgselevationcontrollerwidget.h"
#include "qgsprojectelevationproperties.h"
#include "qgsmapcanvas.h"
#include "qgisapp.h"
#include "qgselevationutils.h"
#include "qgsmaplayerelevationproperties.h"

#include <QInputDialog>

QgsAppCanvasFiltering::QgsAppCanvasFiltering( QObject *parent )
  : QObject( parent )
{
}

void QgsAppCanvasFiltering::setupElevationControllerAction( QAction *action, QgsMapCanvas *canvas )
{
  action->setCheckable( true );
  connect( action, &QAction::toggled, canvas, [canvas, action, this]( bool checked ) {
    if ( checked )
    {
      createElevationController( action, canvas );
    }
    else
    {
      canvas->setZRange( QgsDoubleRange() );
      if ( QgsElevationControllerWidget *controller = mCanvasElevationControllerMap.value( canvas ) )
      {
        controller->deleteLater();
      }
    }
  } );
}

void QgsAppCanvasFiltering::createElevationController( QAction *senderAction, QgsMapCanvas *canvas )
{
  QgsElevationControllerWidget *controller = new QgsElevationControllerWidget();

  QAction *setProjectLimitsAction = new QAction( tr( "Set Elevation Range…" ), controller );
  controller->menu()->addAction( setProjectLimitsAction );
  connect( setProjectLimitsAction, &QAction::triggered, QgisApp::instance(), [] {
    QgisApp::instance()->showProjectProperties( tr( "Elevation" ) );
  } );
  QAction *disableAction = new QAction( tr( "Disable Elevation Filter" ), controller );
  controller->menu()->addAction( disableAction );
  connect( disableAction, &QAction::triggered, senderAction, [senderAction] {
    senderAction->setChecked( false );
  } );

  mCanvasElevationControllerMap.insert( canvas, controller );
  connect( canvas, &QObject::destroyed, this, [canvas, this] {
    mCanvasElevationControllerMap.remove( canvas );
  } );
  connect( controller, &QObject::destroyed, this, [canvas, this] {
    mCanvasElevationControllerMap.remove( canvas );
  } );

  // bridge is parented to controller
  QgsCanvasElevationControllerBridge *bridge = new QgsCanvasElevationControllerBridge( controller, canvas );
  ( void ) bridge;
}

QgsCanvasElevationControllerBridge::QgsCanvasElevationControllerBridge( QgsElevationControllerWidget *controller, QgsMapCanvas *canvas )
  : QObject( controller )
  , mController( controller )
  , mCanvas( canvas )
{
  // canvas updates are applied after a short timeout, to avoid sending too many rapid redraw requests
  // while the controller slider is being dragged
  mUpdateCanvasTimer = new QTimer( this );
  mUpdateCanvasTimer->setSingleShot( true );
  connect( mController, &QgsElevationControllerWidget::rangeChanged, this, &QgsCanvasElevationControllerBridge::controllerZRangeChanged );
  connect( mUpdateCanvasTimer, &QTimer::timeout, this, &QgsCanvasElevationControllerBridge::setCanvasZRange );

  mCanvas->addOverlayWidget( mController, Qt::Edge::LeftEdge );

  if ( mCanvas == QgisApp::instance()->mapCanvas() )
  {
    // for main canvas, attach settings to project settings
    mController->setFixedRangeSize( QgsProject::instance()->elevationProperties()->elevationFilterRangeSize() );
    connect( mController, &QgsElevationControllerWidget::fixedRangeSizeChanged, this, []( double size ) {
      QgsProject::instance()->elevationProperties()->setElevationFilterRangeSize( size );
    } );
    mController->setInverted( QgsProject::instance()->elevationProperties()->invertElevationFilter() );
    connect( mController, &QgsElevationControllerWidget::invertedChanged, this, []( bool inverted ) {
      QgsProject::instance()->elevationProperties()->setInvertElevationFilter( inverted );
    } );
  }

  connect( mCanvas, &QgsMapCanvas::layersChanged, this, &QgsCanvasElevationControllerBridge::canvasLayersChanged );

  canvasLayersChanged();
}

void QgsCanvasElevationControllerBridge::canvasLayersChanged()
{
  if ( !mCanvas )
    return;

  // disconnect from old layers
  for ( QgsMapLayer *layer : std::as_const( mCanvasLayers ) )
  {
    if ( layer )
    {
      disconnect( layer->elevationProperties(), &QgsMapLayerElevationProperties::changed, this, &QgsCanvasElevationControllerBridge::updateSignificantElevations );
    }
  }

  // and connect to new
  const QList<QgsMapLayer *> layers = mCanvas->layers( true );
  for ( QgsMapLayer *layer : layers )
  {
    connect( layer->elevationProperties(), &QgsMapLayerElevationProperties::changed, this, &QgsCanvasElevationControllerBridge::updateSignificantElevations );
  }
  mCanvasLayers = _qgis_listRawToQPointer( layers );

  updateSignificantElevations();
}

void QgsCanvasElevationControllerBridge::updateSignificantElevations()
{
  if ( !mCanvas )
    return;

  mController->setSignificantElevations( QgsElevationUtils::significantZValuesForLayers( _qgis_listQPointerToRaw( mCanvasLayers ) ) );
}

void QgsCanvasElevationControllerBridge::controllerZRangeChanged( const QgsDoubleRange & )
{
  mUpdateCanvasTimer->start( 100 );
}

void QgsCanvasElevationControllerBridge::setCanvasZRange()
{
  if ( !mCanvas )
    return;

  mCanvas->setZRange( mController->range() );
}
