/***************************************************************************
    qgsmaptoolfeatureaction.h  -  map tool for running feature actions
    ---------------------
    begin                : January 2012
    copyright            : (C) 2012 by Giuseppe Sucameli
    email                : brush.tyler at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolfeatureaction.h"

#include "qgsdistancearea.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgsmessageviewer.h"
#include "qgsattributeaction.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsmaplayerregistry.h"
#include "qgisapp.h"

#include <QSettings>
#include <QMessageBox>
#include <QMouseEvent>
#include <QStatusBar>

QgsMapToolFeatureAction::QgsMapToolFeatureAction( QgsMapCanvas* canvas )
    : QgsMapTool( canvas )
{
}

QgsMapToolFeatureAction::~QgsMapToolFeatureAction()
{
}

void QgsMapToolFeatureAction::canvasMoveEvent( QMouseEvent *e )
{
  Q_UNUSED( e );
}

void QgsMapToolFeatureAction::canvasPressEvent( QMouseEvent *e )
{
  Q_UNUSED( e );
}

void QgsMapToolFeatureAction::canvasReleaseEvent( QMouseEvent *e )
{
  if ( !mCanvas || mCanvas->isDrawing() )
  {
    return;
  }

  QgsMapLayer *layer = mCanvas->currentLayer();

  if ( !layer || layer->type() != QgsMapLayer::VectorLayer )
  {
    QMessageBox::warning( mCanvas,
                          tr( "No active vector layer" ),
                          tr( "To run an action, you must choose a vector layer by clicking on its name in the legend" ) );
    return;
  }

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( vlayer->actions()->size() == 0 )
  {
    QMessageBox::warning( mCanvas,
                          tr( "No actions available" ),
                          tr( "The active vector layer has no defined actions" ) );
    return;
  }

  if ( !doAction( vlayer, e->x(), e->y() ) )
    QgisApp::instance()->statusBar()->showMessage( tr( "No features at this position found." ) );
}

void QgsMapToolFeatureAction::activate()
{
  QgsMapTool::activate();
}

void QgsMapToolFeatureAction::deactivate()
{
  QgsMapTool::deactivate();
}

bool QgsMapToolFeatureAction::doAction( QgsVectorLayer *layer, int x, int y )
{
  if ( !layer )
    return false;

  QgsPoint point = mCanvas->getCoordinateTransform()->toMapCoordinates( x, y );

  // load identify radius from settings
  QSettings settings;
  double identifyValue = settings.value( "/Map/identifyRadius", QGis::DEFAULT_IDENTIFY_RADIUS ).toDouble();
  QString ellipsoid = settings.value( "/qgis/measure/ellipsoid", "WGS84" ).toString();

  if ( identifyValue <= 0.0 )
    identifyValue = QGis::DEFAULT_IDENTIFY_RADIUS;

  QgsFeature feat;

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
    if ( layer->nextFeature( f ) )
      feat = QgsFeature( f );
    else
      return false;
  }
  catch ( QgsCsException & cse )
  {
    Q_UNUSED( cse );
    // catch exception for 'invalid' point and proceed with no features found
    QgsDebugMsg( QString( "Caught CRS exception %1" ).arg( cse.what() ) );
  }

  int action = layer->actions()->defaultAction();

  // define custom substitutions: layer id and clicked coords
  QMap<QString, QVariant> substitutionMap;
  substitutionMap.insert( "$layerid", layer->id() );
  point = toLayerCoordinates( layer, point );
  substitutionMap.insert( "$clickx", point.x() );
  substitutionMap.insert( "$clicky", point.y() );

  layer->actions()->doAction( action, feat, &substitutionMap );
  return true;
}
