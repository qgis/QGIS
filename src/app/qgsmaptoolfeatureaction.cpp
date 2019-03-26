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

#include "qgsfeatureiterator.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgsmessageviewer.h"
#include "qgsactionmanager.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsexception.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsmaplayeractionregistry.h"
#include "qgisapp.h"
#include "qgsgui.h"
#include "qgsstatusbar.h"
#include "qgsmapmouseevent.h"
#include "qgsexpressioncontextutils.h"

#include <QSettings>
#include <QStatusBar>

QgsMapToolFeatureAction::QgsMapToolFeatureAction( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
}

void QgsMapToolFeatureAction::canvasMoveEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e );
}

void QgsMapToolFeatureAction::canvasPressEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e );
}

void QgsMapToolFeatureAction::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  QgsMapLayer *layer = mCanvas->currentLayer();

  if ( !layer || layer->type() != QgsMapLayerType::VectorLayer )
  {
    emit messageEmitted( tr( "To run an action, you must choose an active vector layer." ), Qgis::Info );
    return;
  }

  if ( !mCanvas->layers().contains( layer ) )
  {
    // do not run actions on hidden layers
    return;
  }

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( vlayer->actions()->actions( QStringLiteral( "Canvas" ) ).isEmpty() && QgsGui::mapLayerActionRegistry()->mapLayerActions( vlayer ).isEmpty() )
  {
    emit messageEmitted( tr( "The active vector layer has no defined actions" ), Qgis::Info );
    return;
  }

  if ( !doAction( vlayer, e->x(), e->y() ) )
    QgisApp::instance()->statusBarIface()->showMessage( tr( "No features at this position found." ) );
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

  QgsPointXY point = mCanvas->getCoordinateTransform()->toMapCoordinates( x, y );
  QPoint position = mCanvas->mapToGlobal( QPoint( x + 5, y + 5 ) );

  QgsRectangle r;

  // create the search rectangle
  double searchRadius = searchRadiusMU( mCanvas );

  r.setXMinimum( point.x() - searchRadius );
  r.setXMaximum( point.x() + searchRadius );
  r.setYMinimum( point.y() - searchRadius );
  r.setYMaximum( point.y() + searchRadius );

  // toLayerCoordinates will throw an exception for an 'invalid' point.
  // For example, if you project a world map onto a globe using EPSG 2163
  // and then click somewhere off the globe, an exception will be thrown.
  try
  {
    r = toLayerCoordinates( layer, r );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    // catch exception for 'invalid' point and proceed with no features found
    QgsDebugMsg( QStringLiteral( "Caught CRS exception %1" ).arg( cse.what() ) );
  }

  QgsFeature f;
  QgsFeatureList features;
  QgsFeatureIterator fit = layer->getFeatures( QgsFeatureRequest().setFilterRect( r ).setFlags( QgsFeatureRequest::ExactIntersect ) );
  while ( fit.nextFeature( f ) )
  {
    features.append( f );
  }

  if ( !features.isEmpty() )
  {
    if ( features.count() == 1 )
    {
      doActionForFeature( layer, features.first(), point );
    }
    else
    {
      QMenu *featureMenu = new QMenu();
      for ( const QgsFeature &feature : qgis::as_const( features ) )
      {
        QAction *featureAction = featureMenu->addAction( FID_TO_STRING( feature.id() ) );
        connect( featureAction, &QAction::triggered, this, [ = ] { doActionForFeature( layer, feature, point );} );
      }
      QAction *allFeatureAction = featureMenu->addAction( tr( "All Features" ) );
      connect( allFeatureAction, &QAction::triggered, this, [ = ]
      {
        for ( const QgsFeature &feature : qgis::as_const( features ) )
        {
          doActionForFeature( layer, feature, point );
        }
      } );
      featureMenu->exec( position );
    }
    return true;
  }
  return false;
}

void QgsMapToolFeatureAction::doActionForFeature( QgsVectorLayer *layer, const QgsFeature &feature, const QgsPointXY &point )
{
  QgsAction defaultAction = layer->actions()->defaultAction( QStringLiteral( "Canvas" ) );
  if ( defaultAction.isValid() )
  {
    // define custom substitutions: layer id and clicked coords
    QgsExpressionContext context;
    context << QgsExpressionContextUtils::globalScope()
            << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
            << QgsExpressionContextUtils::mapSettingsScope( mCanvas->mapSettings() );
    QgsExpressionContextScope *actionScope = new QgsExpressionContextScope();
    actionScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "click_x" ), point.x(), true ) );
    actionScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "click_y" ), point.y(), true ) );
    actionScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "action_scope" ), QStringLiteral( "Canvas" ), true ) );
    context << actionScope;

    defaultAction.run( layer, feature, context );
  }
  else
  {
    QgsMapLayerAction *mapLayerAction = QgsGui::mapLayerActionRegistry()->defaultActionForLayer( layer );
    if ( mapLayerAction )
    {
      mapLayerAction->triggerForFeature( layer, &feature );
    }
  }
}
