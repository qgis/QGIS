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

#include "qgisapp.h"
#include "qgsactionmanager.h"
#include "qgsexception.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeatureiterator.h"
#include "qgsgui.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayeraction.h"
#include "qgsmaplayeractionregistry.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptopixel.h"
#include "qgsmessagebar.h"
#include "qgsproject.h"
#include "qgsstatusbar.h"
#include "qgsvectorlayer.h"

#include <QSettings>
#include <QStatusBar>

#include "moc_qgsmaptoolfeatureaction.cpp"

QgsMapToolFeatureAction::QgsMapToolFeatureAction( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
}

void QgsMapToolFeatureAction::canvasMoveEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
}

void QgsMapToolFeatureAction::canvasPressEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
}

void QgsMapToolFeatureAction::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  QgsMapLayer *layer = mCanvas->currentLayer();

  if ( !layer || layer->type() != Qgis::LayerType::Vector )
  {
    emit messageEmitted( tr( "To run an action, you must choose an active vector layer." ), Qgis::MessageLevel::Info );
    return;
  }

  if ( !mCanvas->layers( true ).contains( layer ) )
  {
    // do not run actions on hidden layers
    return;
  }

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  QgsMapLayerActionContext context;
  if ( vlayer->actions()->actions( u"Canvas"_s ).isEmpty() && QgsGui::mapLayerActionRegistry()->mapLayerActions( vlayer, Qgis::MapLayerActionTarget::AllActions, context ).isEmpty() )
  {
    emit messageEmitted( tr( "The active vector layer has no defined actions" ), Qgis::MessageLevel::Info );
    return;
  }

  if ( !doAction( vlayer, e->x(), e->y() ) )
    QgisApp::instance()->statusBarIface()->showMessage( tr( "No features found at this position." ), 2000 );
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
    Q_UNUSED( cse )
    // catch exception for 'invalid' point and proceed with no features found
    QgsDebugError( u"Caught CRS exception %1"_s.arg( cse.what() ) );
  }

  QgsFeature f;
  QgsFeatureList features;
  QgsFeatureIterator fit = layer->getFeatures( QgsFeatureRequest().setFilterRect( r ).setFlags( Qgis::FeatureRequestFlag::ExactIntersect ) );
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
      QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
      QgsExpression exp( layer->displayExpression() );
      exp.prepare( &context );

      QMenu *featureMenu = new QMenu();
      for ( const QgsFeature &feature : std::as_const( features ) )
      {
        context.setFeature( feature );
        QString featureTitle = exp.evaluate( &context ).toString();
        if ( featureTitle.isEmpty() )
          featureTitle = FID_TO_STRING( feature.id() );

        QAction *featureAction = featureMenu->addAction( featureTitle );
        connect( featureAction, &QAction::triggered, this, [this, layer, feature, point] { doActionForFeature( layer, feature, point ); } );
      }
      QAction *allFeatureAction = featureMenu->addAction( tr( "All Features" ) );
      connect( allFeatureAction, &QAction::triggered, this, [this, features, layer, point] {
        for ( const QgsFeature &feature : std::as_const( features ) )
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
  QgsAction defaultAction = layer->actions()->defaultAction( u"Canvas"_s );
  if ( defaultAction.isValid() )
  {
    switch ( defaultAction.type() )
    {
      case Qgis::AttributeActionType::GenericPython:
      case Qgis::AttributeActionType::Mac:
      case Qgis::AttributeActionType::Windows:
      case Qgis::AttributeActionType::Unix:
      {
        const bool allowed = QgsGui::allowExecutionOfEmbeddedScripts( QgsProject::instance() );
        if ( !allowed )
        {
          QgisApp::instance()->messageBar()->pushMessage(
            tr( "Security warning" ),
            tr( "The action contains an embedded script which has been denied execution." ),
            Qgis::MessageLevel::Warning
          );
          return;
        }
        break;
      }

      case Qgis::AttributeActionType::Generic:
      case Qgis::AttributeActionType::OpenUrl:
      case Qgis::AttributeActionType::SubmitUrlEncoded:
      case Qgis::AttributeActionType::SubmitUrlMultipart:
      {
        break;
      }
    }

    // define custom substitutions: layer id and clicked coords
    QgsExpressionContext context;
    context << QgsExpressionContextUtils::globalScope()
            << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
            << QgsExpressionContextUtils::mapSettingsScope( mCanvas->mapSettings() );
    QgsExpressionContextScope *actionScope = new QgsExpressionContextScope();
    actionScope->addVariable( QgsExpressionContextScope::StaticVariable( u"click_x"_s, point.x(), true ) );
    actionScope->addVariable( QgsExpressionContextScope::StaticVariable( u"click_y"_s, point.y(), true ) );
    actionScope->addVariable( QgsExpressionContextScope::StaticVariable( u"action_scope"_s, u"Canvas"_s, true ) );
    context << actionScope;

    defaultAction.run( layer, feature, context );
  }
  else
  {
    QgsMapLayerAction *mapLayerAction = QgsGui::mapLayerActionRegistry()->defaultActionForLayer( layer );
    if ( mapLayerAction )
    {
      QgsMapLayerActionContext context;
      Q_NOWARN_DEPRECATED_PUSH
      mapLayerAction->triggerForFeature( layer, feature );
      Q_NOWARN_DEPRECATED_POP
      mapLayerAction->triggerForFeature( layer, feature, context );
    }
  }
}
